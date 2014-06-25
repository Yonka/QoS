#include "QoS.h"
#include "node.h"

QoS::QoS(sc_module_name mn, node* parent_node, int scheduling)
    : sc_module(mn), m_scheduling(scheduling)
{
    init();
    id = parent_node->id;
    QoS_node_port(*parent_node);
}

QoS::QoS() {}

void QoS::set_scheduling(int scheduling, vector<vector<bool> > schedule_table)
{
    m_scheduling = scheduling;
    m_scheduleTable = schedule_table;
    m_epochSize = schedule_table.size();
}

void QoS::init()
{
    m_currentTimeSlot = 0;

    m_mark = false;
    m_timer = false;

    m_t_tc = sc_time(TICK, SC_NS);
    m_t_te = m_t_tc * m_scheduleTable.size();
    m_e_beginTime = SC_ZERO_TIME;
    m_tc_beginTime = SC_ZERO_TIME;
    m_timeCodeEvent.notify(m_t_tc); ///WTF?

    SC_THREAD(change_time);
    sensitive << m_timeCodeEvent;
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
        if (m_scheduling == 1)
            sync_v1();
        else if (m_scheduling == 2)
            sync_v2();
    }
}

void QoS::sync_v1()
{
    if (m_mark && !m_timer)
    {
        new_time_slot();
        m_mark = false;
        m_currentTimeSlot = m_receivedTimeCode;
        if (m_tc_beginTime != SC_ZERO_TIME)
            m_t_tc = sc_time_stamp() - m_tc_beginTime;
        m_tc_beginTime = sc_time_stamp();
        m_timeCodeEvent.notify(m_t_tc);
        return;
    }
    if (sc_time_stamp() == m_tc_beginTime + m_t_tc) 
    {
        m_timer = true;
        m_currentTimeSlot++;
        new_time_slot();
        m_tc_beginTime = sc_time_stamp();
    }
    if (m_timer && m_mark)
    {
        m_t_tc += sc_time_stamp() - m_tc_beginTime;
        m_timeCodeEvent.cancel();
        m_currentTimeSlot = m_receivedTimeCode;
        m_timer = false; 
        m_mark = false;
    }
    m_timeCodeEvent.notify(m_t_tc);
}

void QoS::sync_v2()
{
    if (m_mark)
    {
        m_mark = false;
        if (m_e_beginTime == SC_ZERO_TIME)
        {
            m_e_beginTime = sc_time_stamp();
            m_tc_beginTime = sc_time_stamp();
            m_timeCodeEvent.cancel();
            m_timeCodeEvent.notify(m_t_tc);
            return;
        }
        if (m_currentTimeSlot == m_epochSize - 1)
        {
            new_time_slot();
            m_currentTimeSlot = 0;
            m_t_te = sc_time_stamp() - m_e_beginTime;
            m_t_tc = m_t_te / m_epochSize;
            m_tc_beginTime = sc_time_stamp();
            m_e_beginTime = sc_time_stamp();
            m_timeCodeEvent.notify(m_t_tc);
        }
        else if (m_currentTimeSlot == 0)
        {
            m_t_tc += (sc_time_stamp() - m_e_beginTime) / m_epochSize;
            m_t_te = m_t_tc * m_epochSize;
            m_timeCodeEvent.cancel();
            m_timeCodeEvent.notify(m_t_tc - (sc_time_stamp() - m_e_beginTime));
        }
        else
        {
            m_timeCodeEvent.notify(m_t_tc - (sc_time_stamp() - m_tc_beginTime));
        }
        return;
    }
    if (sc_time_stamp() == m_tc_beginTime + m_t_tc) 
    {
        m_currentTimeSlot++;
        m_tc_beginTime = sc_time_stamp();
        if (m_currentTimeSlot == m_epochSize)
        {
            m_currentTimeSlot = 0;
            m_e_beginTime = sc_time_stamp();
        }
        new_time_slot();
    }
    m_timeCodeEvent.notify(m_t_tc);
}

void QoS::got_time_code(int time_code)
{
    m_receivedTimeCode = m_currentTimeSlot;
    m_mark = true; 
    m_timeCodeEvent.notify();
}

sc_time QoS::get_tc()
{
    return m_t_tc;
}

sc_time QoS::get_te()
{
    return m_t_te;
}

int QoS::get_time_slot()
{
    return m_currentTimeSlot;
}
