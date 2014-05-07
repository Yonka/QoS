#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "systemc.h"
#include "defs.h"
#include <time.h>

using namespace std;

class router : public sc_module, public node_router_I
{
private:
    static const int num_of_ports = 3;
    pair<symbol, sc_time> buf[num_of_ports], tmp_buf[num_of_ports]; // input buffer
    sc_time delay;
    vector<int> address_destination, ad2, ad3;                    // if port is sending data, then destination address, -1 otherwise 
    vector<bool> ready_to_redirect;         // port has actual data to redirect

    vector<int> address_source;
    vector<bool> ready_to_send;             // is an output port ready to send data (received fct)
    vector<int> routing_table;
    vector<int> freed_ports;     
    vector<bool> dest_for_tc;
    sc_event new_data, free_port;           // we have data for redirection or any out-port received fct
    sc_event fct_delayed_event, time_code_event;    

    sc_uint<14> cur_time;                   //current time

    void fct_delayed();
//    void time_code_delayed();

    void redirect();
    bool inner_connect(int x);

    void init_fct();

    void init();

public:
    SC_HAS_PROCESS(router);
    router(sc_module_name mn, sc_time delay);

    virtual void write_byte(int num, symbol s);
    virtual void fct(int num, sc_time holdup);
//    virtual void time_code(sc_uint<14> t);

    sc_port<router_node_I> fct_port[num_of_ports];   // output ports

};
#endif
