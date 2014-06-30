#include "node.h"

node::node(sc_module_name mn, int id, int dest_id, sc_time delay = sc_time(0, SC_NS)) 
    : sc_module(mn), id(id), m_dest_id(dest_id), m_delay(delay), m_out_buffer(4000), m_in_buffer(4000)
{
    m_QoS = new QoS("QoS", this, 0); 
    node_QoS_port(*m_QoS);
    m_ready_to_write = 0;
    m_processed = 0;
    m_have_time_code_to_send = false;
    m_have_fct_to_send = false;
    m_have_data_to_send = false;

    SC_METHOD(init);
    
    SC_THREAD(sender);
    dont_initialize();
    sensitive << m_fct_event << m_run_sender;
    
    SC_METHOD(fctDelayed);
    dont_initialize();
    sensitive << m_fct_delayed_event;
}

void node::init()
{
    m_have_fct_to_send = true;
    m_run_sender.notify(SC_ZERO_TIME);
}

void node::set_scheduling(int scheduling, vector<vector<bool> > schedule_table)
{
    m_QoS->set_scheduling(scheduling, schedule_table);
}

bool node::write_packet(std::vector<int>* packet)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    if (m_out_buffer.num_free() < (*packet).size())   
        return false;
    while (!(*packet).empty())
    {
        m_out_buffer.write((*packet).back());
        (*packet).pop_back();
    }
    m_run_sender.notify();
//    cout << this->basename() << " have new packet " << sc_time_stamp() << '\n';
    return true;    
}

void node::write_byte(int num, symbol s)
{
//    cerr << sc_time_stamp() << ": " << this->basename() << " received from " << s.source << ": " << s.data <<"\n";
    if (s.t == nchar && s.data == EOP_SYMBOL)
    {
        stat_m++;
        int data;
        cerr << this-> basename() << " received package from "<< s.source<<": ";
        traf[id][s.source]++;
        GV[id]++;
        while (m_in_buffer.nb_read(data));
//            cerr << data << " ";
        cerr << " at " << sc_time_stamp() << "\n";
        m_processed++;
    }
    else if (s.t == lchar)
    {
        stat_k++;
        node_QoS_port->got_time_code(s.data);
    }
    else
    {
        stat_m++;
        m_in_buffer.write(s.data);
        m_processed++;
    }
    m_run_sender.notify(SC_ZERO_TIME);
    if (m_processed == 8) 
    {
        m_have_fct_to_send = true;
        m_processed = 0;
    }
}

void node::fct(int num)
{
     m_fct_delayed_event.notify(FCT_SIZE * m_delay);
     stat_n++;
}

void node::fctDelayed()
{
    m_ready_to_write = 8;
    m_fct_event.notify();
//    cerr << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void node::newTimeCode(int value)
{
    cerr << id << " send tc\n";
    m_QoS->got_time_code(value);
    m_have_time_code_to_send = true;
    m_run_sender.notify();
}

void node::ban_sending()
{
    m_can_send = false;
    m_run_sender.notify(SC_ZERO_TIME);
}

void node::unban_sending()
{
    m_can_send = true;
    m_run_sender.notify(SC_ZERO_TIME);
}

void node::sender()
{
    int receiver_addr = -1;
    while (true)
    {
        wait();
        if (!(m_have_fct_to_send || m_have_time_code_to_send || m_can_send || receiver_addr != -1))
            continue;
        symbol s;
        if (m_have_time_code_to_send) 
        {
            s = symbol(node_QoS_port->get_time_slot(), -1, id, lchar);
            cerr << this->basename() << " send TC" << node_QoS_port->get_time_slot() << " at " << sc_time_stamp() << "\n";
            m_have_time_code_to_send = false;
            wait(m_delay * s.t);
            data_port->write_byte(direct, s);
        }
        if (m_have_fct_to_send)
        {
            m_have_fct_to_send = false;
            data_port->fct(direct);
            wait(FCT_SIZE * m_delay);
        }
        if (m_dest_id != id)
        {
            if (m_have_data_to_send && m_ready_to_write !=0)
            {
                s = symbol(m_tmp_byte, receiver_addr, id, nchar);
                if (m_tmp_byte == EOP_SYMBOL)
                {
                    receiver_addr = -1;
                    node_trafgen_port->new_packet_request();
                    m_run_sender.notify(SC_ZERO_TIME);
                }
//              cerr << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
                m_have_data_to_send = false;
                wait(m_delay * s.t);
                data_port->write_byte(direct, s);
                m_ready_to_write--;
            }
            if (receiver_addr != -1 || m_can_send)
            {
                if (!m_have_data_to_send && m_out_buffer.nb_read(m_tmp_byte))
                {
                    m_have_data_to_send = true;
                    if (receiver_addr == -1) 
                        receiver_addr = m_tmp_byte;
                }
                else if (!m_have_data_to_send)
                    receiver_addr = -1;
            }
        }
        if (m_have_fct_to_send || m_have_time_code_to_send || (m_have_data_to_send && m_ready_to_write != 0))
            m_run_sender.notify(SC_ZERO_TIME);
    }
}
