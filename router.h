#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "systemc.h"
#define FCT_SIZE 1
using namespace std;

class router : public sc_module, public receiver_router_I
{
public:
    static const int num_of_ports = 3;
    sc_port<router_receiver_I> fct_port[num_of_ports];   // output ports
    pair<sc_uint<8>, sc_time> buf[num_of_ports]; // input buffer
    sc_time delay;
    vector<int> address_destination;                    // if port is sending data, then destination address, -1 otherwise 
    vector<bool> ready_to_redirect;         // port has actual data to redirect

    vector<int> address_source;
    vector<bool> ready_to_send;             // is an output port ready to send data (received fct)
    
    vector<int> freed_ports;
    sc_event new_data, free_port, fct_delayed_event;           // we have data for redirection or any out-port received fct

    SC_HAS_PROCESS(router);

    router(sc_module_name mn, sc_time delay);

    virtual void write_byte(int num, sc_uint<8> data);

    virtual void fct(int num, sc_time holdup);

    void fct_delayed();

    void redirect();

    void init();

    bool inner_connect(int x);
};
#endif
