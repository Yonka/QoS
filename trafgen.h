#ifndef TRAFGEN_H
#define TRAFGEN_H
#include "my_interfaces.h"
#include "systemc.h"
class trafgen : public sc_module
{
public:
    sc_port<writeI> out_port;
    sc_uint<8> x;
    sc_event send;
    int runs;

    SC_HAS_PROCESS(trafgen);
    
    trafgen(sc_module_name mn, int param);

    void gen_event();

    void send_byte();
};
#endif

