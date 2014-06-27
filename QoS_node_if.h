#ifndef QOS_NODE_IF_H
#define QOS_NODE_IF_H
#include "systemc.h"

class QoS_node_if : virtual public sc_interface
{
public:
    virtual void ban_sending() = 0;

    virtual void unban_sending() = 0;
};

#endif