#ifndef TRAFGEN_H
#define TRAFGEN_H
#include "systemc.h"
#include "trafgen_node_if.h"
#include "node_trafgen_if.h"
#include "defs.h"

class trafgen: 
    public sc_module,
    public node_trafgen_if
{
public:
    sc_port<trafgen_node_if> trafgen_node_port;
    std::vector<sc_uint<8> > packet;
    sc_event send_event, gen_next_event;   //sending, prepare next packet
    int runs, dest_id;
    sc_time delay;  //TODO: do we need this?
    bool success;

    SC_HAS_PROCESS(trafgen);
    trafgen(sc_module_name mn, int direction, int packets_num, sc_time delay);

    void gen_event();

    void send_packet();

    virtual void new_packet_request();
};
#endif

