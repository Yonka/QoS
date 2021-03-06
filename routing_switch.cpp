#include "routing_switch.h"

routing_switch::routing_switch(sc_module_name mn, int id, int ports, sc_time delay/* = sc_time(0, SC_NS)*/, vector<int> table)
    : sc_module(mn), id(id), m_ports(ports), m_delay(delay), m_routingTable(table)
{
    init();
    SC_METHOD(initFCT);
    
    SC_METHOD(redirecting);
    dont_initialize();
    sensitive << m_newDataToSend << m_portFreed;
}

void routing_switch::init()
{
    std::srand (unsigned(std::time(0)));
    m_currentTime = -1;
    m_timeSenderPortID = 0;
    m_addressDestination.resize(m_ports);
    fill_n(m_addressDestination.begin(), m_ports, FREE_PORT);

    m_addressSource.resize(m_ports);
    fill_n(m_addressSource.begin(), m_ports, FREE_PORT);

    m_processed.resize(m_ports);
    fill_n(m_processed.begin(), m_ports, 0);

    m_readyToSend.resize(m_ports);
    fill_n(m_readyToSend.begin(), m_ports, 0);

    m_destForTimeCode.resize(m_ports);
    m_destForFct.resize(m_ports);
    m_inProc.resize(m_ports);
    fill_n(m_inProc.begin(), m_ports, make_pair(redirectingSymbolType::nothing, sc_time(0, SC_NS)));

    m_outProc.resize(m_ports);
    fill_n(m_outProc.begin(), m_ports, redirectingSymbolType::nothing);

    direct.resize(m_ports);

    for (int i = 0; i < m_ports; i++)
    {
        queue<symbol> t;
        m_dataBuffer.push_back(t);
        m_timeBuffer.push_back(symbol(0, 0, 0, nchar));
        data_port.push_back(new sc_port<data_if>);
    }
}

void routing_switch::fct(int inPortID)
{
    stat_n++;
    m_readyToSend[inPortID] = 8;
    m_portFreed.notify();
}

void routing_switch::write_byte(int inPortID, symbol symb)
{
//    cerr << this->basename() << " received " << s.data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (symb.symbolType == lchar)
    {
        stat_k++;
        if (symb.data == (m_currentTime + 1) % 64)
        {
            m_timeBuffer[inPortID] = symb;
            m_newDataToSend.notify(SC_ZERO_TIME);
            m_timeSenderPortID = inPortID;
            fill_n(m_destForTimeCode.begin(), m_ports, true);
            m_destForTimeCode[inPortID] = false;   //we already have it
        }
        m_currentTime = symb.data;
    }
    else
    {
        stat_m++;
        if (m_addressDestination[inPortID] == FREE_PORT)
        {
            int addr = m_routingTable[symb.address];
            m_addressDestination[inPortID] = addr;
        }
        m_dataBuffer[inPortID].push(symb);
        m_newDataToSend.notify(SC_ZERO_TIME);
    }
}

void routing_switch::redirecting()
{
    redirectingDataCloseConnection();    
    redirectingFCTCloseConnection();
    if (m_timeBuffer[m_timeSenderPortID].symbolType == lchar)
        redirectingTime();  
    redirectingFCTOpenConnection();
    redirectingDataOpenConnection();     
}

void routing_switch::redirectingDataCloseConnection()
{
    for (int i = 0; i < m_ports; i++)
    {
        if (m_outProc[i] == redirectingSymbolType::informSymbol && m_inProc[m_addressDestination[i]].second > sc_time_stamp())    //too early to free ports
        {
            sc_time t = m_inProc[m_addressDestination[i]].second - sc_time_stamp();
            m_portFreed.notify(t); 
            continue;
        }
        else if (m_outProc[i] == redirectingSymbolType::informSymbol)
        {
            m_inProc[m_addressDestination[i]].first = redirectingSymbolType::nothing;
            m_outProc[i] = redirectingSymbolType::nothing;

            (*data_port[m_addressDestination[i]])->write_byte(direct[m_addressDestination[i]], m_dataBuffer[i].front());
            m_processed[i]++;
            if (m_processed[i] == 8)
            {
                m_redirectingFCT.insert(i);
                m_processed[i] = 0;
            }
            m_newDataToSend.notify(SC_ZERO_TIME);
            if (m_dataBuffer[i].front().data == EOP_SYMBOL)
            {
                m_addressSource[m_addressDestination[i]] = FREE_PORT; 
                m_addressDestination[i] = FREE_PORT;                   
            }
            m_dataBuffer[i].pop();
        }
    }
}

void routing_switch::redirectingFCTCloseConnection()
{
    for (set<int>::iterator i = m_redirectingFCT.begin(); i != m_redirectingFCT.end();) 
    {
        if (m_inProc[*i].first == redirectingSymbolType::fct && m_inProc[*i].second > sc_time_stamp())
        {
            m_portFreed.notify(m_inProc[*i].second - sc_time_stamp());
            i++;
            continue;
        }
        if (m_inProc[*i].first == redirectingSymbolType::fct)
        {
            (*data_port[*i])->fct(direct[*i]);
            m_inProc[*i].first = redirectingSymbolType::nothing;
            m_redirectingFCT.erase(i++);
        }              
        else
            ++i;
    }
}

void routing_switch::redirectingTime()
{
    if ((m_outProc[m_timeSenderPortID] == redirectingSymbolType::nothing || m_outProc[m_timeSenderPortID] == redirectingSymbolType::timeCode))
    {
        m_outProc[m_timeSenderPortID] = redirectingSymbolType::timeCode;

        for (int j = 0; j < m_ports; j++)
        {
            if (!m_destForTimeCode[j])
                continue;
            if (m_inProc[j].first == redirectingSymbolType::nothing)
            {
                m_inProc[j].first = redirectingSymbolType::timeCode;
                m_inProc[j].second = sc_time_stamp() + delays[j] * m_timeBuffer[m_timeSenderPortID].symbolType + m_delay;
                m_portFreed.notify(delays[j] * m_timeBuffer[m_timeSenderPortID].symbolType + m_delay);
                continue;
            }
            if (m_inProc[j].first == redirectingSymbolType::timeCode && m_inProc[j].second > sc_time_stamp())
            {
                m_portFreed.notify(m_inProc[j].second - sc_time_stamp());
                continue;
            }
            if (m_inProc[j].first == redirectingSymbolType::timeCode)
            {
                m_inProc[j].first = redirectingSymbolType::nothing;
                m_destForTimeCode[j] = false;
                (*data_port[j])->write_byte(direct[j], m_timeBuffer[m_timeSenderPortID]);
            }              
        }
        bool freed = true;
        for (int j = 0; j < m_ports; j++)
            if (m_destForTimeCode[j])
                freed = false;
        if (freed)
        {
            m_timeBuffer[m_timeSenderPortID].symbolType = nchar;
            m_outProc[m_timeSenderPortID] = redirectingSymbolType::nothing;
            m_portFreed.notify(SC_ZERO_TIME);
        }
    }
}

void routing_switch::redirectingFCTOpenConnection()
{
    for (set<int>::iterator i = m_redirectingFCT.begin(); i != m_redirectingFCT.end();) 
    {
        if (m_inProc[*i].first == redirectingSymbolType::nothing)
        {
            m_inProc[*i].first = redirectingSymbolType::fct;
            m_inProc[*i].second = sc_time_stamp() + delays[*i] * FCT_SIZE;
            i++;
            continue;
        }
        ++i;
    }
}

void routing_switch::redirectingDataOpenConnection()
{
    vector<int> shuffled;
    for (int i = 0; i < m_ports; i++) shuffled.push_back(i);
    std::random_shuffle(shuffled.begin(), shuffled.end());
    for (int j = 0; j < m_ports; j++)
    {
        int i = shuffled[j];
        if (!innerConnect(i))
            continue;
        m_inProc[m_addressDestination[i]].second = sc_time_stamp() + delays[m_addressDestination[i]] * m_dataBuffer[i].front().symbolType + m_delay;
        m_newDataToSend.notify(delays[m_addressDestination[i]] * m_dataBuffer[i].front().symbolType + m_delay);
        m_readyToSend[m_addressDestination[i]]--;
        m_inProc[m_addressDestination[i]].first = redirectingSymbolType::informSymbol;
        m_outProc[i] = redirectingSymbolType::informSymbol;
    }
}

bool routing_switch::innerConnect(int x)
{
    if (m_addressDestination[x] == FREE_PORT || m_dataBuffer[x].size() == 0)
        return false;
    if (m_addressSource[m_addressDestination[x]] == FREE_PORT)
        m_addressSource[m_addressDestination[x]] = x;
    if (m_inProc[m_addressDestination[x]].first != redirectingSymbolType::nothing || m_outProc[x] != redirectingSymbolType::nothing)
        return false;
    if (m_addressSource[m_addressDestination[x]] != x || m_readyToSend[m_addressDestination[x]] == 0)
        return false;
    return true;
}

void routing_switch::initFCT()
{
    for (int i = 0; i < m_ports; i++)
    {
        m_redirectingFCT.insert(i);
        m_newDataToSend.notify();  
    }
}
