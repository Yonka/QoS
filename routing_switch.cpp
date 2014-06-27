#include "routing_switch.h"

routing_switch::routing_switch(sc_module_name mn, int id, int ports, sc_time delay/* = sc_time(0, SC_NS)*/, vector<int> table)
    : sc_module(mn), id(id), m_ports(ports), m_delay(delay), m_routing_table(table)
{
    init();
    SC_METHOD(initFCT);
    
    SC_METHOD(redirect);
    dont_initialize();
    sensitive << m_new_data << m_free_port;
    
    SC_METHOD(fctDelayed);
    dont_initialize();
    sensitive << m_fct_delayed_event;
}

void routing_switch::init()
{
    std::srand (unsigned(std::time(0)));
    m_cur_time = -1;
    m_time_source = 0;
    m_address_destination.resize(m_ports);
    fill_n(m_address_destination.begin(), m_ports, -1);

    m_address_source.resize(m_ports);
    fill_n(m_address_source.begin(), m_ports, -1);

    m_processed.resize(m_ports);
    fill_n(m_processed.begin(), m_ports, 0);

    m_ready_to_send.resize(m_ports);
    fill_n(m_ready_to_send.begin(), m_ports, 0);

    m_ready_to_redirect.resize(m_ports);
    fill_n(m_ready_to_redirect.begin(), m_ports, false);

    m_dest_for_tc.resize(m_ports);
    m_dest_for_fct.resize(m_ports);
    m_in_proc.resize(m_ports);
    fill_n(m_in_proc.begin(), m_ports, make_pair(0, sc_time(0, SC_NS)));

    m_out_proc.resize(m_ports);
    fill_n(m_out_proc.begin(), m_ports, false);

    direct.resize(m_ports);

    for (int i = 0; i < m_ports; i++)
    {
        queue<symbol> t;
        m_data_buffer.push_back(t);
        m_time_buffer.push_back(symbol(0, 0, 0, nchar));
        fct_port.push_back(new sc_port<data_if>);
    }
}

void routing_switch::fct(int num)
{
    stat_n++;
    m_freed_ports.insert(pair<int, sc_time>(num, sc_time_stamp()));
    m_fct_delayed_event.notify(FCT_SIZE * delays[num]);
}

void routing_switch::fctDelayed()
{
    for (int i = 0; i < m_ports; i++)
    {
        map<int, sc_time>::iterator it;
        it = m_freed_ports.find(i);
        if (it != m_freed_ports.end())
        {
            if (it->second + FCT_SIZE * delays[i] > sc_time_stamp())
            {
                m_fct_delayed_event.notify(it->second + FCT_SIZE * delays[i] - sc_time_stamp());
            }
            else
            {
//                cerr << this->basename() << " received fct from " << i << " at "<< sc_time_stamp() << "\n";
                m_ready_to_send[i] = 8;
                m_freed_ports.erase(it);
                m_free_port.notify();
            }
        }
    }
}

void routing_switch::write_byte(int num, symbol s)
{
//    cerr << this->basename() << " received " << s.data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (s.t == lchar)
    {
        stat_k++;
        if (s.data == (m_cur_time + 1) % 64)
        {
            m_time_buffer[num] = s;
            m_new_data.notify(SC_ZERO_TIME);
            m_time_source = num;
            fill_n(m_dest_for_tc.begin(), m_ports, true);
            m_dest_for_tc[num] = false;   //we already have it
        }
        m_cur_time = s.data;
    }
    else
    {
        stat_m++;
        if (m_address_destination[num] == -1)
        {
            int addr = m_routing_table[s.address];
            m_address_destination[num] = addr;
        }
        m_data_buffer[num].push(s);
        m_new_data.notify(SC_ZERO_TIME);
    }
}

void routing_switch::redirect()
{
    redirectClose();           //free from data
    if (m_time_buffer[m_time_source].t == lchar)
        redirectTime();        //time
    redirectFCT();
    redirectData();            //data
}

void routing_switch::redirectClose()
{
    for (int i = 0; i < m_ports; i++)
    {
        if (m_out_proc[i] == 3  && m_in_proc[m_address_destination[i]].second > sc_time_stamp())    //too early to free ports
        {
            sc_time t = m_in_proc[m_address_destination[i]].second - sc_time_stamp();
            m_free_port.notify(t); 
            continue;
        }
        else if (m_out_proc[i] == 3)
        {
            m_in_proc[m_address_destination[i]].first = 0;
            m_out_proc[i] = 0;

            (*fct_port[m_address_destination[i]])->write_byte(direct[m_address_destination[i]], m_data_buffer[i].front());
            m_processed[i]++;
            if (m_processed[i] == 8)
            {
                m_redirecting_fct.insert(i);
                m_processed[i] = 0;
            }
            m_new_data.notify(SC_ZERO_TIME);
            if (m_data_buffer[i].front().data == EOP_SYMBOL)
            {
                m_address_source[m_address_destination[i]] = -1;
                m_address_destination[i] = -1;
            }
            m_data_buffer[i].pop();
        }
    }
}

void routing_switch::redirectTime()
{
    if ((m_out_proc[m_time_source] == 0 || m_out_proc[m_time_source] == 1))
    {
        m_out_proc[m_time_source] = 1;

        for (int j = 0; j < m_ports; j++)
        {
            if (!m_dest_for_tc[j])
                continue;
            if (m_in_proc[j].first == 0)
            {
                m_in_proc[j].first = 1;
                m_in_proc[j].second = sc_time_stamp() + delays[j] * m_time_buffer[m_time_source].t + m_delay;
                m_free_port.notify(delays[j] * m_time_buffer[m_time_source].t + m_delay);
                continue;
            }
            if (m_in_proc[j].first == 1 && m_in_proc[j].second > sc_time_stamp())
            {
                m_free_port.notify(m_in_proc[j].second - sc_time_stamp());
                continue;
            }
            if (m_in_proc[j].first == 1)
            {
                m_in_proc[j].first = 0;
                m_dest_for_tc[j] = false;
                (*fct_port[j])->write_byte(direct[j], m_time_buffer[m_time_source]);
            }              
        }
        bool freed = true;
        for (int j = 0; j < m_ports; j++)
            if (m_dest_for_tc[j])
                freed = false;
        if (freed)
        {
            m_time_buffer[m_time_source].t = nchar;
            m_out_proc[m_time_source] = 0;
            m_free_port.notify(SC_ZERO_TIME);
        }
    }
}

void routing_switch::redirectFCT()
{
    for (set<int>::iterator i = m_redirecting_fct.begin(); i != m_redirecting_fct.end(); ) 
    {
        if (m_in_proc[*i].first == 0)
        {
            m_in_proc[*i].first = 2;
            m_in_proc[*i].second = sc_time_stamp() + delays[*i] * FCT_SIZE;
            (*fct_port[*i])->fct(direct[*i]);
            i++;
            continue;
        }
        if (m_in_proc[*i].first == 2 && m_in_proc[*i].second > sc_time_stamp())
        {
            m_free_port.notify(m_in_proc[*i].second - sc_time_stamp());
            i++;
            continue;
        }
        if (m_in_proc[*i].first == 2)
        {
            m_in_proc[*i].first = 0;
            m_redirecting_fct.erase(i++);
        }              
        else
            ++i;
    }
}

void routing_switch::redirectData()
{
    vector<int> shuffled;
    for (int i = 0; i < m_ports; i++) shuffled.push_back(i);
    std::random_shuffle(shuffled.begin(), shuffled.end());
    for (int j = 0; j < m_ports; j++)
    {
        int i = shuffled[j];
        if (!innerConnect(i))
            continue;
        m_in_proc[m_address_destination[i]].second = sc_time_stamp() + delays[m_address_destination[i]] * m_data_buffer[i].front().t + m_delay;
        m_new_data.notify(delays[m_address_destination[i]] * m_data_buffer[i].front().t + m_delay);
        m_ready_to_send[m_address_destination[i]]--;
        m_in_proc[m_address_destination[i]].first = 3;
        m_out_proc[i] = 3;
    }
}

bool routing_switch::innerConnect(int x)
{
    if (m_address_destination[x] == -1 || m_data_buffer[x].size() == 0)
        return false;
    if (m_address_source[m_address_destination[x]] == -1)
        m_address_source[m_address_destination[x]] = x;
    if (m_in_proc[m_address_destination[x]].first != 0 || m_out_proc[x] != 0)
        return false;
    if (m_address_source[m_address_destination[x]] != x || m_ready_to_send[m_address_destination[x]] == 0)
        return false;
    return true;
}

void routing_switch::initFCT()
{
    for (int i = 0; i < m_ports; i++)
    {
        m_redirecting_fct.insert(i);
        m_new_data.notify();  
    }
}
