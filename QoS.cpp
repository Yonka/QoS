#include "QoS.h"
#include "defs.h"

QoS::QoS(sc_module_name mn, int id, int scheduling, vector<vector<bool> > st)
    : sc_module(mn), scheduling(scheduling), schedule_table(st)
{
    init();
    epoch_size = st.size();
}

QoS::QoS(sc_module_name mn, int id, int scheduling)
    : sc_module(mn), scheduling(scheduling)
{
    init();
}
QoS::QoS()
{
    scheduling = 0;
    init();
}

void QoS::init()
{
    cur_time = 0;
    m_tc_begin_time = sc_time(0, SC_NS);
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

bool QoS::can_send()
{
    if (scheduling == 0)
        return true;
    return schedule_table[id][cur_time % epoch_size];
}

void QoS::change_time()
{
//TODO: add notification about beginning of a new time slot
    while (true)
    {
        wait();
        if (scheduling == 2)
        {
            change_time_scheduling2();
        }
        else 
        {
            change_time_scheduling1();
        }
    }
}

void QoS::change_time_scheduling1()
{
    if (mark_h && !time_h)
    {
        eop.notify(SC_ZERO_TIME);
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
        eop.notify(SC_ZERO_TIME);
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

void QoS::change_time_scheduling2()
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
        if (cur_time == epoch - 1)
        {
            eop.notify(SC_ZERO_TIME);
            cur_time = 0;
            m_t_te = sc_time_stamp() - m_e_begin_time;
            m_t_tc = m_t_te / epoch;
            m_tc_begin_time = sc_time_stamp();
            m_e_begin_time = sc_time_stamp();
            time_code_event.notify(m_t_tc);
        }
        else if (cur_time == 0)
        {
            m_t_tc += (sc_time_stamp() - m_e_begin_time) / epoch;
            m_t_te = m_t_tc * epoch;
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
        if (cur_time == epoch)
        {
            cur_time = 0;
            m_e_begin_time = sc_time_stamp();
        }
        eop.notify(SC_ZERO_TIME);
    }
    time_code_event.notify(m_t_tc);
}

void QoS::got_time_code(int time)
{
    received_time = time;
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

int QoS::get_time_code()
{
    return cur_time;
}
