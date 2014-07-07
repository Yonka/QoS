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
    int id;                                                 // node id

private:
    vector<vector<bool> > m_scheduleTable;
    int m_scheduling;
    bool m_hasReceivedTimeCode, m_timerHasFinished, m_gotFirstTimeCode;
    sc_time m_t_tc, m_t_e, m_timeSlotBeginTime, m_epochBeginTime;  //t_tc - time-slot timer, t_te - epoch timer, time-slot beginning time, epoch beginning time
    int m_currentTimeSlot, m_receivedTimeCode;              // current & received time-slot number
    int m_epochSize;                                        // number of time slots in epoch
    sc_event m_timeCodeEvent;

public:
    SC_HAS_PROCESS(QoS);

    QoS(sc_module_name mn, node* parent_node, int scheduling);

    QoS();

    virtual void got_time_code(int time_code);              //node got TC = time_code

    virtual sc_time get_te();

    virtual sc_time get_tc();

    virtual int get_time_slot();
    
    virtual int get_scheduling();

    void set_scheduling(int scheduling, vector<vector<bool> > schedule_table);

private:
    void init();

    void change_time();         

    void sync_algorythm_1();

    void sync_algorythm_2();

    void new_time_slot();
};  
#endif