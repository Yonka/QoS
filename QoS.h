#ifndef QOS_H
#define QOS_H

#include "systemc.h"
#include <vector>
#include "defs.h"
#include "node_QoS_if.h"
#include "QoS_node_if.h"

using namespace std;
class node;
class QoS : public sc_module, public node_QoS_if
{
public:
    sc_port<QoS_node_if> QoS_node_port;
    int id;                                 // node id

private:
    vector<vector<bool> > m_scheduleTable;
    int m_scheduling;                         // 0 - without, 1 - first, 2 - second
    bool m_mark, m_timer, m_started;                    // tick & timer handlers
    sc_time m_t_tc, m_t_te, m_tc_beginTime, m_e_beginTime; //t_tc value, t_te value, interval beginning time, epoch beginning time
    int m_currentTimeSlot, m_receivedTimeCode;            // current & received time-slot number
    int m_epochSize;                         // number of time slots in epoch
    sc_event m_timeCodeEvent;

public:
    SC_HAS_PROCESS(QoS);

    QoS(sc_module_name mn, node* parent_node, int scheduling, vector<vector<bool> > st);

    QoS(sc_module_name mn, node* parent_node, int scheduling);

    QoS();

    void init();

    virtual void got_time_code(int time_code);           //node got tc = time

    virtual sc_time get_te();

    virtual sc_time get_tc();

    virtual int get_time_slot();

    void set_scheduling(int scheduling, vector<vector<bool> > schedule_table);

private:
    void change_time();         

    void sync_v1();

    void sync_v2();

    void new_time_slot();
};  
#endif