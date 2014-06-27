#ifndef NODE_QOS_IF_H
#define NODE_QOS_IF_H
#include "systemc.h"

class node_QoS_if : virtual public sc_interface
{
public:
    virtual void got_time_code(int time) = 0;           // node got tc = time

    virtual sc_time get_te() = 0;                      

    virtual sc_time get_tc() = 0;

    virtual int get_time_slot() = 0;                    // get current time-slot number

    virtual int get_scheduling() = 0;                   // get scheduling type
};

#endif