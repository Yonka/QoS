#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "systemc.h"
#include "defs.h"
#include <map>
#include <time.h>

using namespace std;

class router : public sc_module, public node_router_I
{
private:
    static const int num_of_ports = 6;
    symbol buf[num_of_ports], tmp_buf[num_of_ports]; // input buffer, buffer for tc
    sc_time delay;

    vector<int> address_destination;        // if port is sending data, then destination address, -1 otherwise 
    vector<bool> ready_to_redirect;         // port has actual data to redirect

    vector<int> address_source;             // if port is receiving data, then source address, -1 otherwise 
    vector<bool> ready_to_send;             // is an output port ready to send data (received fct)
    vector<int> out_proc;                   // is something redirecting now from this port: 0 - nothing, 1 - lchar, 2 - fct, 3 - nchar
    vector<pair<int, sc_time> > in_proc;    // is something redirecting to this port: -//- and finishing time
    vector<int> routing_table;
    map<int, sc_time> freed_ports;          // map of ports - num + sending begin-time
    vector<bool> dest_for_tc, dest_for_fct;               // need to send tc 
    sc_event new_data, free_port;           // we have data for redirection or any out-port received fct
    sc_event fct_delayed_event, time_code_event;    

    void fct_delayed();

    void redirect();

    void redirect_ports();

    void redirect_time();

    void redirect_fct();

    void redirect_connect();

    bool inner_connect(int x);

    void init_fct();

    void init();

public:
    int id;
    SC_HAS_PROCESS(router);
    router(sc_module_name mn, int id, sc_time delay);

    sc_port<router_node_I> fct_port[num_of_ports];   // output ports

    virtual void write_byte(int num, symbol s);

    virtual void fct(int num);
};
#endif
