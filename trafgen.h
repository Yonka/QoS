#ifndef TRAFGEN_H
#define TRAFGEN_H
#include "my_interfaces.h"
#include "systemc.h"
class trafgen : public sc_module
{
public:
    sc_port<writeI> out_port;
    std::vector<sc_uint<8> > packet;
    sc_event send, send_next;   //sending, prepare next packet
    int runs, packet_size;
    sc_time delay;
    bool success;

    SC_HAS_PROCESS(trafgen);
    trafgen(sc_module_name mn, int param, sc_time delay);

    void gen_event();

    void send_byte();
};
#endif

