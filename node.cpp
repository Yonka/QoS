#include "node.h"

node::node(sc_module_name mn, int id, int dest_id, sc_time delay = sc_time(0, SC_NS)) 
    : sc_module(mn), id(id), dest_id(dest_id), delay(delay), write_buf(4000), read_buf(4000)
{
    m_QoS = new QoS("QoS", this, 0); 
    node_QoS_port(*m_QoS);
    ready_to_write = 0;
    processed = 0;
    have_time_code_to_send = false;
    have_fct_to_send = false;
    have_data_to_send = false;

    SC_METHOD(init);
    
    SC_THREAD(sender);
    dont_initialize();
    sensitive << eop << fct_event << repeat_sender;
    
    SC_METHOD(fct_delayed);
    dont_initialize();
    sensitive << fct_delayed_event;
}

void node::init()
{
    have_fct_to_send = true;
    eop.notify(SC_ZERO_TIME);
}

void node::set_scheduling(int scheduling, vector<vector<bool> > schedule_table)
{
    m_QoS->set_scheduling(scheduling, schedule_table);
}

bool node::write_packet(std::vector<sc_uint<8> >* packet)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    if (write_buf.num_free() < (*packet).size())   
        return false;
    while (!(*packet).empty())
    {
        write_buf.write((*packet).back());
        (*packet).pop_back();
    }
    eop.notify();
//    cout << this->basename() << " have new packet " << sc_time_stamp() << '\n';
    return true;    
}

void node::write_byte(int num, symbol s)
{
//    cerr << sc_time_stamp() << ": " << this->basename() << " received from " << s.sour << ": " << s.data <<"\n";
    if (s.t == nchar && s.data == EOP_SYMBOL)
    {
        stat_m++;
        sc_uint<8> data;
        cerr << this-> basename() << " received package from "<< s.source<<": ";
        traf[id][s.source]++;
        GV[id]++;
        while (read_buf.nb_read(data));
//            cerr << data << " ";
        cerr << " at " << sc_time_stamp() << "\n";
        processed++;
    }
    else if (s.t == lchar)
    {
        stat_k++;
        node_QoS_port->got_time_code(s.data);
    }
    else
    {
        stat_m++;
        read_buf.write(s.data);
        processed++;
    }
    eop.notify(SC_ZERO_TIME);
    if (processed == 8) 
    {
        have_fct_to_send = true;
        processed = 0;
    }
}

void node::fct(int num)
{
     fct_delayed_event.notify(FCT_SIZE * delay);
     stat_n++;
}

void node::fct_delayed()
{
    ready_to_write = 8;
    fct_event.notify();
//    cerr << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void node::new_time_code(int value)
{
    cerr << id << " send tc\n";
//    if (scheduling == 1)
//        cur_time = value;
    have_time_code_to_send = true;
    eop.notify();
}

void node::ban_sending()
{
    can_send = false;
    eop.notify(SC_ZERO_TIME);
}

void node::unban_sending()
{
    can_send = true;
    eop.notify(SC_ZERO_TIME);
}

void node::sender()
{
    int receiver_addr = -1;
    while (true)
    {
        wait();
//////////////////////////////////////////////////////////TODO: check it!
        if (!(have_fct_to_send || have_time_code_to_send || can_send || receiver_addr != -1))
            continue;
        symbol s;
        if (have_time_code_to_send) 
        {
            s = symbol(node_QoS_port->get_time_slot(), -1, id, lchar);
            cerr << this->basename() << " send tc" << node_QoS_port->get_time_slot() << " at " << sc_time_stamp() << "\n";
            have_time_code_to_send = false;
            wait(delay * s.t);
            data_port->write_byte(direct, s);
        }
        if (have_fct_to_send)
        {
            have_fct_to_send = false;
            data_port->fct(direct);
            wait(FCT_SIZE * delay);
        }
        if (dest_id != id)
        {
            if (have_data_to_send && ready_to_write !=0)
            {
                s = symbol(tmp_byte, receiver_addr, id, nchar);
                if (tmp_byte == EOP_SYMBOL)
                {
                    receiver_addr = -1;
                    node_trafgen_port->new_packet_request();
                    eop.notify(SC_ZERO_TIME);
                }
//              cerr << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
                have_data_to_send = false;
                wait(delay * s.t);
                data_port->write_byte(direct, s);
                ready_to_write--;
            }
            if (receiver_addr != -1 || can_send)
            {
                if (!have_data_to_send && write_buf.nb_read(tmp_byte))
                {
                    have_data_to_send = true;
                    if (receiver_addr == -1) 
                        receiver_addr = tmp_byte;
                }
                else if (!have_data_to_send)
                    receiver_addr = -1;
            }
        }
        if (have_fct_to_send || have_time_code_to_send || (have_data_to_send && ready_to_write != 0))
            repeat_sender.notify(SC_ZERO_TIME);
    }
}
