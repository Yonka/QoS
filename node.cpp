#include "node.h"

node::node(sc_module_name mn, int addr, sc_time delay = sc_time(0, SC_NS)) : sc_module(mn), address(addr), delay(delay), write_buf(40), read_buf(40)
{
    ready_to_write = false;
    have_time_code_to_send = false;
    have_fct_to_send = false;
    have_data_to_send = false;
    cur_time = 0;
    m_tc_begin_time = sc_time(0, SC_NS);
    mark_h = false;
    time_h = false;

    m_t_tc = sc_time(100, SC_NS);
    m_t_te = m_t_tc * table_size;
    m_e_begin_time = SC_ZERO_TIME;
    m_tc_begin_time = SC_ZERO_TIME;
    time_code_event.notify(m_t_tc);
    SC_METHOD(init);
    
    SC_THREAD(sender);
    dont_initialize();
    sensitive << eop << fct_event << r_sender;
    
    SC_METHOD(fct_delayed);
    dont_initialize();
    sensitive << fct_delayed_event;

    SC_THREAD(change_tc);
//    dont_initialize();
    sensitive << time_code_event;

//    SC_METHOD(time_code_delayed);
//    dont_initialize();
//    sensitive << time_code_event;
}

void node::init()
{
//    fct_port->fct(address);
    have_fct_to_send = true;
    eop.notify(SC_ZERO_TIME);
}

bool node::write(std::vector<sc_uint<8> >* packet)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    if (write_buf.num_free() < (*packet).size() + 1)   //with sender address - delete
        return false;
    write_buf.write((*packet).back());     //delete
    (*packet).pop_back();                  //delete
    write_buf.write(address + 1);     //delete
    while (!(*packet).empty())
    {
        write_buf.write((*packet).back());
        (*packet).pop_back();
    }
    eop.notify();
//    cout << this->basename() << " have new packet " << sc_time_stamp() << '\n';
    return true;    
}

void node::write_byte(symbol s)
{
    cerr << this->basename() << " received " << s.data << " at " << sc_time_stamp() << "\n";
    if (s.t == nchar && s.data == EOP_SYMBOL)
    {
        sc_uint<8> data;
        cerr << this-> basename() << " received package: ";
        while (read_buf.nb_read(data))
            cerr << data << " ";
        cerr << " at " << sc_time_stamp() << "\n";
    }
    else if (s.t == lchar)
    {
        time_code(s.data);
//TODO
    }
    else
    {
        read_buf.write(s.data);
    }
    have_fct_to_send = true;
    eop.notify(SC_ZERO_TIME);
}

void node::fct()
{
     fct_delayed_event.notify(FCT_SIZE * delay);
}

void node::fct_delayed()
{
    ready_to_write = true;
    fct_event.notify();
    cerr << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void node::change_tc()
{
    while (true)
    {
        wait();
        if (mark_h)
        {
            mark_h = false;
            if (m_e_begin_time == SC_ZERO_TIME)
            {
                m_e_begin_time = sc_time_stamp();
                m_tc_begin_time = sc_time_stamp();
                time_code_event.cancel();
                time_code_event.notify(m_t_tc);
                continue;
            }
            if (cur_time == table_size - 1)
            {
                eop.notify(SC_ZERO_TIME);
                cur_time = 0;
                m_t_te = sc_time_stamp() - m_e_begin_time;
                m_t_tc = m_t_te / table_size;
                m_tc_begin_time = sc_time_stamp();
                m_e_begin_time = sc_time_stamp();
                time_code_event.notify(m_t_tc);
            }
            else if (cur_time == 0)
            {
                m_t_tc += (sc_time_stamp() - m_e_begin_time) / table_size;
                m_t_te = m_t_tc * table_size;
                time_code_event.cancel();
                time_code_event.notify(m_t_tc - (sc_time_stamp() - m_e_begin_time));
            }
            else
            {
                time_code_event.notify(m_t_tc - (sc_time_stamp() - m_tc_begin_time));
            }
            continue;
        }
        if (sc_time_stamp() == m_tc_begin_time + m_t_tc) 
        {
            cur_time++;
                m_tc_begin_time = sc_time_stamp();
            if (cur_time == table_size)
            {
                cur_time = 0;
                m_e_begin_time = sc_time_stamp();
            }
            eop.notify(SC_ZERO_TIME);
        }
        time_code_event.notify(m_t_tc);
    }
}

void node::time_code(int t)
{
    mark_h = true; 
    time_code_event.notify();
    cout << address << " received tc\n";
}

void node::new_time_code(int value)
{
    cout << address << " send tc\n";
//    cur_time = value;
    have_time_code_to_send = true;
    eop.notify();
}

void node::sender()
{
    int receiver_addr = -1; /////////////////////ok?
//    schedule_table<vector>
    while (true)
    {
//        wait() check if it is allowed time-slot
        wait();

/////////////////////////////////////////////////////////////////TODO: check it!

        if (!(have_fct_to_send || have_time_code_to_send || schedule_table[address][cur_time % table_size] == 1 || receiver_addr != -1))
            continue;
        symbol s;
        if (have_time_code_to_send) 
        {
            s = symbol(cur_time, BROADCAST_SYMBOL, lchar);
            cerr << this->basename() << " send tc" << cur_time << " at " << sc_time_stamp() << "\n";
            have_time_code_to_send = false;
            wait(delay * s.t);
            fct_port->write_byte(address, s);
//            ready_to_write = false;       //we don't need fct to send tc
        }
        if (have_fct_to_send)
        {
            have_fct_to_send = false;
            fct_port->fct(address);
            wait(FCT_SIZE * delay);
        }
        if (have_data_to_send && ready_to_write)
        {
            s = symbol(tmp_byte, receiver_addr, nchar);
            if (tmp_byte == EOP_SYMBOL)
            {
                receiver_addr = -1;
                eop.notify(SC_ZERO_TIME);
            }
            cerr << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
            have_data_to_send = false;
            wait(delay * s.t);
            fct_port->write_byte(address, s);
            ready_to_write = false;
        }
        if (!have_data_to_send && write_buf.nb_read(tmp_byte))
        {
            have_data_to_send = true;
            if (receiver_addr = -1) 
                receiver_addr = tmp_byte;
        }
        else if (!have_data_to_send)
            receiver_addr = -1;
        if (have_fct_to_send || have_time_code_to_send || (have_data_to_send && ready_to_write))
            r_sender.notify(SC_ZERO_TIME);

    }
}
