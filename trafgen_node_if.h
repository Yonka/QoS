#ifndef TRAFGEN_NODE_IF_H
#define TRAFGEN_NODE_IF_H
#include "systemc.h"

class trafgen_node_if : virtual public sc_interface
{
public:
    virtual bool write_packet(std::vector<int> *packet) = 0;
};
#endif