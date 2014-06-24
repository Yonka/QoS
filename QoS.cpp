#include "QoS.h"
#include "node.h"
QoS::QoS(sc_module_name mn, node* parent_node, int scheduling, vector<vector<bool> > st)
    : sc_module(mn), scheduling(scheduling), schedule_table(st)
{
    init();
    id = parent_node->id;
    epoch_size = st.size();
    QoS_node_port(*parent_node);
}
///TODO: delegating constructors
QoS::QoS(sc_module_name mn, node* parent_node, int scheduling)
    : sc_module(mn), scheduling(scheduling)
{
    init();
    id = parent_node->id;
    QoS_node_port(*parent_node);
}

QoS::QoS() {}

void QoS::init()
{
    cur_time = 0;

    mark_h = false;
    time_h = false;

    m_t_tc = sc_time(TICK, SC_NS);
    m_t_te = m_t_tc * schedule_table.size();
    m_e_begin_time = SC_ZERO_TIME;
    m_tc_begin_time = SC_ZERO_TIME;
    time_code_event.notify(m_t_tc);

    SC_THREAD(change_time);
    sensitive << time_code_event;
}

void QoS::new_time_slot()
{
    if (scheduling != 0)
    if (schedule_table[id][cur_time % epoch_size]) 
        QoS_node_port->unban_sending();
    else
        QoS_node_port->ban_sending();
}

void QoS::change_time()
{
    while (true)
    {
        wait();
        if (scheduling == 2)
            sync_v2();
        else 
            sync_v1();
    }
}

void QoS::sync_v1()
{
    if (mark_h && !time_h)
    {
        new_time_slot();
        mark_h = false;
        cur_time = received_time;
        if (m_tc_begin_time != SC_ZERO_TIME)
            m_t_tc = sc_time_stamp() - m_tc_begin_time;
        m_tc_begin_time = sc_time_stamp();
        time_code_event.notify(m_t_tc);
        return;
    }
    if (sc_time_stamp() == m_tc_begin_time + m_t_tc) 
    {
        time_h = true;
        cur_time++;
        new_time_slot();
        m_tc_begin_time = sc_time_stamp();
    }
    if (time_h && mark_h)
    {
        m_t_tc += sc_time_stamp() - m_tc_begin_time;
        time_code_event.cancel();
        cur_time = received_time;
        time_h = false; 
        mark_h = false;
    }
    time_code_event.notify(m_t_tc);
}

void QoS::sync_v2()
{
    if (mark_h)
    {
        mark_h = false;
        if (m_e_begin_time == SC_ZERO_TIME)
        {
            m_e_begin_time = sc_time_stamp();
            m_tc_begin_time = sc_time_stamp();
            time_code_event.cancel();
            time_code_event.notify(m_t_tc);
            return;
        }
        if (cur_time == epoch_size - 1)
        {
            new_time_slot();
            cur_time = 0;
            m_t_te = sc_time_stamp() - m_e_begin_time;
            m_t_tc = m_t_te / epoch_size;
            m_tc_begin_time = sc_time_stamp();
            m_e_begin_time = sc_time_stamp();
            time_code_event.notify(m_t_tc);
        }
        else if (cur_time == 0)
        {
            m_t_tc += (sc_time_stamp() - m_e_begin_time) / epoch_size;
            m_t_te = m_t_tc * epoch_size;
            time_code_event.cancel();
            time_code_event.notify(m_t_tc - (sc_time_stamp() - m_e_begin_time));
        }
        else
        {
            time_code_event.notify(m_t_tc - (sc_time_stamp() - m_tc_begin_time));
        }
        return;
    }
    if (sc_time_stamp() == m_tc_begin_time + m_t_tc) 
    {
        cur_time++;
        m_tc_begin_time = sc_time_stamp();
        if (cur_time == epoch_size)
        {
            cur_time = 0;
            m_e_begin_time = sc_time_stamp();
        }
        new_time_slot();
    }
    time_code_event.notify(m_t_tc);
}

void QoS::got_time_code(int time_code)
{
    received_time = cur_time;
    mark_h = true; 
    time_code_event.notify();
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
    return cur_time;
}
