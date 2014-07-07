#include "QoS.h"
#include "node.h"

QoS::QoS(sc_module_name mn, node* parent_node, int scheduling)
    : sc_module(mn), m_scheduling(scheduling)
{
    init();
    id = parent_node->id;
    QoS_node_port(*parent_node);

    SC_THREAD(change_time);
    sensitive << m_timeCodeEvent;
}

QoS::QoS() {}

void QoS::set_scheduling(int scheduling, vector<vector<bool> > schedule_table)
{
    m_scheduling = scheduling;
    m_scheduleTable = schedule_table;
    m_epochSize = schedule_table[0].size();
    m_t_e = m_t_tc * m_epochSize;
//    QoS_node_port->unban_sending();
    new_time_slot();
}

void QoS::init()
{
    m_currentTimeSlot = 0;
    m_gotFirstTimeCode = false;

    m_hasReceivedTimeCode = false;
    m_timerHasFinished = false;

    m_t_tc = sc_time(TICK, SC_NS);
    m_epochBeginTime = SC_ZERO_TIME;
    m_timeSlotBeginTime = SC_ZERO_TIME;
}

void QoS::new_time_slot()
{
    if (m_scheduleTable[id][m_currentTimeSlot % m_epochSize]) 
        QoS_node_port->unban_sending();
    else
        QoS_node_port->ban_sending();
}

void QoS::change_time()
{
    while (true)
    {
        wait();
        if (m_scheduling == ALGORYTHM_1)
            sync_algorythm_1();
        else if (m_scheduling == ALGORYTHM_2)
            sync_algorythm_2();
    }
}

void QoS::sync_algorythm_1()
{
    if (m_hasReceivedTimeCode && !m_timerHasFinished)
    {
        m_hasReceivedTimeCode = false;
        m_currentTimeSlot = m_receivedTimeCode;
        new_time_slot();
        if (m_gotFirstTimeCode)
            m_t_tc = sc_time_stamp() - m_timeSlotBeginTime;
        else 
        {
            //////////counting statistics
            packets_count[id] = 0;
            for (int i = 0; i < traf[id].size(); i++)
                traf[id][i] = 0;
            //////////counting statistics
            m_gotFirstTimeCode = true;
            new_time_slot();
        }
        m_timeSlotBeginTime = sc_time_stamp();
        m_timeCodeEvent.cancel();
        m_timeCodeEvent.notify(m_t_tc);
        return;
    }
    if (sc_time_stamp() == m_timeSlotBeginTime + m_t_tc) 
    {
        m_timerHasFinished = true;
        m_currentTimeSlot = (m_currentTimeSlot + 1) % m_epochSize;
        new_time_slot();
        m_timeSlotBeginTime = sc_time_stamp();
    }
    if (m_timerHasFinished && m_hasReceivedTimeCode)
    {
        m_t_tc += sc_time_stamp() - m_timeSlotBeginTime;
        m_timeCodeEvent.cancel();
        m_currentTimeSlot = m_receivedTimeCode;
        m_timerHasFinished = false; 
        m_hasReceivedTimeCode = false;
    }
    m_timeCodeEvent.notify(m_t_tc);
}

void QoS::sync_algorythm_2()
{
    if (m_hasReceivedTimeCode)
    {
        m_hasReceivedTimeCode = false;
        if (!m_gotFirstTimeCode)
        {
            //////////counting statistics
            packets_count[id] = 0;
            for (int i = 0; i < traf[id].size(); i++)
                traf[id][i] = 0;
            //////////counting statistics
            m_gotFirstTimeCode = true;
            m_epochBeginTime = sc_time_stamp();
            m_timeSlotBeginTime = sc_time_stamp();
            m_timeCodeEvent.cancel();
            m_timeCodeEvent.notify(m_t_tc);
            new_time_slot();
            return;
        }
        if (m_currentTimeSlot == m_epochSize - 1)
        {
            m_currentTimeSlot = 0;
            m_t_e = sc_time_stamp() - m_epochBeginTime;
            m_t_tc = m_t_e / m_epochSize;
            m_timeSlotBeginTime = sc_time_stamp();
            m_epochBeginTime = sc_time_stamp();
            m_timeCodeEvent.notify(m_t_tc);
            new_time_slot();
        }
        else if (m_currentTimeSlot == 0)
        {
            m_t_tc += (sc_time_stamp() - m_epochBeginTime) / m_epochSize;
            m_t_e = m_t_tc * m_epochSize;
            m_timeCodeEvent.cancel();
            m_timeCodeEvent.notify(m_t_tc - (sc_time_stamp() - m_epochBeginTime));
        }
        else
        {
            m_timeCodeEvent.notify(m_t_tc - (sc_time_stamp() - m_timeSlotBeginTime));
        }
        return;
    }
    if (sc_time_stamp() == m_timeSlotBeginTime + m_t_tc) 
    {
        m_currentTimeSlot++;
        m_timeSlotBeginTime = sc_time_stamp();
        if (m_currentTimeSlot == m_epochSize)
        {
            m_currentTimeSlot = 0;
            m_epochBeginTime = sc_time_stamp();
        }
        new_time_slot();
    }
    m_timeCodeEvent.notify(m_t_tc);
}

void QoS::got_time_code(int time_code)
{
    m_receivedTimeCode = time_code;
    m_hasReceivedTimeCode = true; 
    m_timeCodeEvent.notify();
}

sc_time QoS::get_tc()
{
    return m_t_tc;
}

sc_time QoS::get_te()
{
    return m_t_e;
}

int QoS::get_time_slot()
{
    return m_currentTimeSlot;
}

int QoS::get_scheduling()
{
    return m_scheduling;
}
