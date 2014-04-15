#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "systemc.h"
using namespace std;

class router : public sc_module, public receiver_router_I
{
public:
    static const int num_of_ports = 3;
    sc_port<router_receiver_I> fct_port[num_of_ports];   // output ports
    sc_fifo<sc_uint<8> > buf[num_of_ports]; // input buffer

    vector<int> address_destination;                    // if port is sending data, then destination address, -1 otherwise 
    vector<bool> ready_to_redirect;         // port has actual data to redirect

    vector<int> address_source;
    vector<bool> ready_to_send;             // is an output port ready to send data (received fct)

    sc_event new_data, free_port;           // we have data for redirection or any out-port received fct

    SC_HAS_PROCESS(router);

    router(sc_module_name mn);

    virtual void write_byte(int num, sc_uint<8> data);

    virtual void fct(int num);

    void redirect();

    void init();

    bool inner_connect(int x);
};
#endif
