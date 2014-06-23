#ifndef ROUTER_H
#define ROUTER_H
#include "my_interfaces.h"
#include "defs.h"
#include "systemc.h"
#include <algorithm>
#include <set>
#include <map>
#include <ctime>
#include <queue>

using namespace std;

class routing_switch : public sc_module, public conn_I
{
private:
    int ports;
    int cur_time;
    int time_source;                        // where time come from
    vector<queue<symbol> > data_buffer;     // input buffer
    vector<symbol> time_buffer;             // buffer for tc
    sc_time delay;

    vector<int> address_destination;        // if port is sending data, then destination address, -1 otherwise 
    vector<bool> ready_to_redirect;         // port has actual data to redirect
    vector<int> processed;                  // how many symbols passed through this port (shouldn't we send fct?)

    vector<int> address_source;             // if port is receiving data, then source address, -1 otherwise 
    vector<int> ready_to_send;              // is an output port ready to send data (received fct)

    vector<int> out_proc;                   // is something redirecting now from this port: 0 - nothing, 1 - lchar, 2 - fct, 3 - nchar
    vector<pair<int, sc_time> > in_proc;    // is something redirecting to this port: -//- and finishing time

    set<int> redirecting_data, redirecting_fct;
    vector<int> routing_table;
    map<int, sc_time> freed_ports;          // map of ports - num + sending begin-time
    vector<bool> dest_for_tc, dest_for_fct; // need to send TC 
    sc_event new_data, free_port;           // we have data for redirection or any out-port received fct
    sc_event fct_delayed_event;    

    void fct_delayed();

    void redirect();

    void redirect_close();

    void redirect_time();

    void redirect_fct();

    void redirect_data();

    bool inner_connect(int x);

    void init_fct();

    void init();

public:
    int id;
    vector<int> direct;

    SC_HAS_PROCESS(routing_switch);
    routing_switch(sc_module_name mn, int id, int ports, sc_time delay, vector<int> table);

    vector<sc_port<conn_I>*> fct_port;   // output ports

    virtual void write_byte(int num, symbol s);

    virtual void fct(int num);
};
#endif
