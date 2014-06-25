#ifndef NODE_TRAFGEN_IF_H
#define NODE_TRAFGEN_IF_H
#include "systemc.h"

class node_trafgen_if : virtual public sc_interface
{
public:
    virtual void new_packet_request() = 0;
};
#endif