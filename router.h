#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "systemc.h"
using namespace std;

class router : public sc_module, public fctI
{
public:
    static const int num_of_ports = 2;
    sc_port<fctI> fct_port[num_of_ports];   // output ports
    sc_fifo<sc_uint<8> > buf[num_of_ports]; // input buffer
    vector<int> address;                    // if port is sending data, then destination address, -1 otherwise 
    vector<bool> busy;                      // is an output port busy
    sc_event freed[num_of_ports];           // output port is not busy anymore
    vector<bool> ready;                     // other part of output port is ready to receive data (we got fct)
    sc_event fct_event[num_of_ports];       //other part of output port s ready 
                                            
    SC_HAS_PROCESS(router);

    router(sc_module_name mn);

    virtual void write_byte(int num, sc_uint<8> data);

    virtual void fct(int num);

    void redirect(int num, sc_uint<8> data);

    void init();
};
#endif
