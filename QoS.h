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
private:
    vector<vector<bool> > schedule_table;
    int scheduling;                         // 0 - without, 1 - first, 2 - second
    int id;                                 // node id
    bool mark_h, time_h;                    // tick & timer handlers
    sc_time m_t_tc, m_t_te, m_tc_begin_time, m_e_begin_time; //t_tc value, t_te value, interval beginning time, epoch beginning time
    int cur_time, received_time;
    int epoch_size;                         //number of time slots in epoch
    sc_event time_code_event;

    void change_time();                     //
    void sync_v1();
    void sync_v2();

    void new_time_slot();

public:
    sc_port<QoS_node_if> QoS_node_port;

    SC_HAS_PROCESS(QoS);
    QoS(sc_module_name mn, node* parent_node, int scheduling, vector<vector<bool> > st);
    QoS(sc_module_name mn, node* parent_node, int scheduling);
    QoS();
    void init();

    virtual void got_time_code(int time_code);           //node got tc = time

    virtual sc_time get_te();

    virtual sc_time get_tc();

    virtual int get_time_slot();
};  
#endif