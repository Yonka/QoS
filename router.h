#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "systemc.h"
#define FCT_SIZE 1
using namespace std;

class router : public sc_module, public receiver_router_I
{
private:
    static const int num_of_ports = 3;
    pair<sc_uint<8>, sc_time> buf[num_of_ports]; // input buffer
    sc_time delay;
    vector<int> address_destination;                    // if port is sending data, then destination address, -1 otherwise 
    vector<bool> ready_to_redirect;         // port has actual data to redirect

    vector<int> address_source;
    vector<bool> ready_to_send;             // is an output port ready to send data (received fct)
    
    vector<int> freed_ports;                
    sc_event new_data, free_port, fct_delayed_event, time_code_event;           // we have data for redirection or any out-port received fct

    sc_uint<14> cur_time;                   //current time

    void fct_delayed();
    void time_code_delayed();

    void redirect();
    bool inner_connect(int x);

    void init();

public:
    SC_HAS_PROCESS(router);
    router(sc_module_name mn, sc_time delay);

    virtual void write_byte(int num, sc_uint<8> data);
    virtual void fct(int num, sc_time holdup);
    virtual void time_code(sc_uint<14> t);

    sc_port<router_receiver_I> fct_port[num_of_ports];   // output ports

};
#endif
