#ifndef QOS_H
#define QOS_H

#include "systemc.h"
#include <vector>

using namespace std;
class QoS : public sc_module
{
private:
    vector<vector<bool> > schedule_table;
    int scheduling;                         //0 - without, 1 - first, 2 - second
    int id;
    bool mark_h, time_h;                    //tick & timer handlers
    sc_time m_t_tc, m_t_te, m_tc_begin_time, m_e_begin_time; //t_tc value, t_te value, interval beginning time, epoch beginning time
    int cur_time, received_time, address;
    int epoch_size;
    sc_event time_code_event;

    void change_time();                     //
    void change_time_scheduling1();
    void change_time_scheduling2();
    

public:
    SC_HAS_PROCESS(QoS);
    QoS(sc_module_name mn, int id, int scheduling, vector<vector<bool> > schedule_table);
    QoS(sc_module_name mn, int id, int scheduling);
    QoS();
    void init();

    bool can_send();                        //can this node send lchar now

    void got_time_code(int time);           //node got tc = time

    sc_time get_te();

    sc_time get_tc();

    int get_time_code();
};  
#endif