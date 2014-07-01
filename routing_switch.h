#ifndef ROUTING_SWITCH_H
#define ROUTING_SWITCH_H

#include "systemc.h"
#include <algorithm>
#include <set>
#include <map>
#include <ctime>
#include <queue>

#include "data_if.h"
#include "defs.h"

using namespace std;

class routing_switch : public sc_module, public data_if
{
public:
    int id;
    vector<int> direct;
    vector<sc_port<data_if>*> data_port;   // output ports

private:
    int m_ports;
    int m_cur_time;
    int m_time_source;                          // where time comes from
    vector<queue<symbol> > m_data_buffer;       // buffer for FCT
    vector<symbol> m_time_buffer;               // buffer for TC
    sc_time m_delay;

    vector<int> m_address_destination;          // if port is sending data, then destination address, -1 otherwise 
    vector<bool> m_ready_to_redirect;           // port has actual data to redirect
    vector<int> m_processed;                    // how many symbols passed through this port (shouldn't we send fct?)

    vector<int> m_address_source;               // if port is receiving data, then source address, -1 otherwise 
    vector<int> m_ready_to_send;                // is an output port ready to send data (received fct)

    vector<int> m_out_proc;                     // is something redirecting now from this port: 0 - nothing, 1 - lchar, 2 - fct, 3 - nchar
    vector<pair<int, sc_time> > m_in_proc;      // is something redirecting to this port: -//- and finishing time

    set<int> m_redirecting_data, m_redirecting_fct;
    vector<int> m_routing_table;
    map<int, sc_time> m_freed_ports;            // map of ports - num + sending begin-time
    vector<bool> m_dest_for_tc, m_dest_for_fct; // receivers of TC & FCT
    sc_event m_new_data, m_free_port;           // we have data for redirection or any out-port received fct
    sc_event m_fct_delayed_event;    

public:
    SC_HAS_PROCESS(routing_switch);

    routing_switch(sc_module_name mn, int id, int ports, sc_time delay, vector<int> table);

    virtual void write_byte(int num, symbol s);

    virtual void fct(int num);

private:
    void fctDelayed();

    void redirect();

    void redirectClose();

    void redirectTime();

    void redirectFCT();

    void redirectData();

    bool innerConnect(int x);

    void initFCT();

    void init();

};
#endif
