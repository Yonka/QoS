#include "node.h"

node::node(sc_module_name mn, int id, int dest_id, sc_time delay = sc_time(0, SC_NS)) 
    : sc_module(mn), id(id), m_destID(dest_id), m_delay(delay), m_outBuffer(4000), m_inBuffer(4000)
{
    m_QoS = new QoS("QoS", this, 0); 
    node_QoS_port(*m_QoS);
    m_readyToWrite = 0;
    m_processed = 0;
    m_haveTimeTodeToSend = false;
    m_haveFctToSend = false;
    m_haveDataToSend = false;

    SC_METHOD(init);
    
    SC_THREAD(sender);
    dont_initialize();
    sensitive << m_fctEvent << m_runSender;
    
    SC_METHOD(fctDelayed);
    dont_initialize();
    sensitive << m_fctDelayedEvent;
}

void node::init()
{
    m_haveFctToSend = true;
    m_runSender.notify(SC_ZERO_TIME);
}

void node::set_scheduling(int scheduling, vector<vector<bool> > schedule_table)
{
    m_QoS->set_scheduling(scheduling, schedule_table);
}

bool node::write_packet(std::vector<int>* packet)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    if (m_outBuffer.num_free() < (*packet).size())   
        return false;
    while (!(*packet).empty())
    {
        m_outBuffer.write((*packet).back());
        (*packet).pop_back();
    }
    m_runSender.notify();
//    cout << this->basename() << " have new packet " << sc_time_stamp() << '\n';
    return true;    
}

void node::write_byte(int inPortID, symbol symb) 
{
//    cerr << sc_time_stamp() << ": " << this->basename() << " received from " << s.source << ": " << s.data <<"\n";
    if (symb.symbolType == nchar && symb.data == EOP_SYMBOL)
    {
        stat_m++;
        int data;
        cerr << this-> basename() << " received package from "<< symb.source<<": ";
        traf[id][symb.source]++;
        packets_count[id]++;
        while (m_inBuffer.nb_read(data));
//            cerr << data << " ";
        cerr << " at " << sc_time_stamp() << "\n";
        m_processed++;
    }
    else if (symb.symbolType == lchar)
    {
        stat_k++;
        node_QoS_port->got_time_code(symb.data);
    }
    else
    {
        stat_m++;
        m_inBuffer.write(symb.data);
        m_processed++;
    }
    m_runSender.notify(SC_ZERO_TIME);
    if (m_processed == 8) 
    {
        m_haveFctToSend = true;
        m_processed = 0;
    }
}

void node::fct(int inPortID)
{
     m_fctDelayedEvent.notify(FCT_SIZE * m_delay);
     stat_n++;
}

void node::fctDelayed()
{
    m_readyToWrite = 8;
    m_fctEvent.notify();
//    cerr << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void node::newTimeCode(int value)
{
    cerr << id << " send tc\n";
    m_QoS->got_time_code(value);
    m_haveTimeTodeToSend = true;
    m_runSender.notify();
}

void node::ban_sending()
{
    m_canSend = false;
    m_runSender.notify(SC_ZERO_TIME);
}

void node::unban_sending()
{
    m_canSend = true;
    m_runSender.notify(SC_ZERO_TIME);
}

void node::sender()
{
    int receiver_addr = -1;
    while (true)
    {
        wait();
        if (!(m_haveFctToSend || m_haveTimeTodeToSend || m_canSend || receiver_addr != -1))
            continue;
        symbol s;
        if (m_haveTimeTodeToSend) 
        {
            s = symbol(node_QoS_port->get_time_slot(), -1, id, lchar);
            cerr << this->basename() << " send TC" << node_QoS_port->get_time_slot() << " at " << sc_time_stamp() << "\n";
            m_haveTimeTodeToSend = false;
            wait(m_delay * s.symbolType);
            data_port->write_byte(direct, s);
        }
        if (m_haveFctToSend)
        {
            m_haveFctToSend = false;
            data_port->fct(direct);
            wait(FCT_SIZE * m_delay);
        }
        if (m_destID != id)
        {
            if (m_haveDataToSend && m_readyToWrite !=0)
            {
                s = symbol(m_tmpByte, receiver_addr, id, nchar);
                if (m_tmpByte == EOP_SYMBOL)
                {
                    receiver_addr = -1;
                    node_trafgen_port->new_packet_request();
                    m_runSender.notify(SC_ZERO_TIME);
                }
//              cerr << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
                m_haveDataToSend = false;
                wait(m_delay * s.symbolType);
                data_port->write_byte(direct, s);
                m_readyToWrite--;
            }
            if (receiver_addr != -1 || m_canSend)
            {
                if (!m_haveDataToSend && m_outBuffer.nb_read(m_tmpByte))
                {
                    m_haveDataToSend = true;
                    if (receiver_addr == -1) 
                        receiver_addr = m_tmpByte;
                }
                else if (!m_haveDataToSend)
                    receiver_addr = -1;
            }
        }
        if (m_haveFctToSend || m_haveTimeTodeToSend || (m_haveDataToSend && m_readyToWrite != 0))
            m_runSender.notify(SC_ZERO_TIME);
    }
}
