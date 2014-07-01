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
    int runs, dest_id;

private:
    std::vector<int> m_packet;
    sc_event m_send_event, m_gen_next_event;   //sending, prepare next packet

public:
    SC_HAS_PROCESS(trafgen);

    trafgen(sc_module_name mn, int direction, int packets_num);

    virtual void new_packet_request();

private:
    void genPacket();

    void sendPacket();

};
#endif

