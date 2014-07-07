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
    vector<sc_port<data_if>*> fct_port;   // output ports

private:
    int m_ports;
    int m_currentTime;
    int m_timeSenderPortID;                     // where time comes from
    vector<queue<symbol> > m_dataBuffer;        // buffer for informSymbol
    vector<symbol> m_timeBuffer;                // buffer for timeCode
    sc_time m_delay;

    vector<int> m_addressDestination;           // if port is sending data, then destination address, -1 otherwise 
    vector<bool> m_readyToRedirect;             // port has actual data to redirect
    vector<int> m_processed;                    // how many symbols passed through this port (shouldn't we send fct?)

    vector<int> m_addressSource;                // if port is receiving data, then source address, -1 otherwise 
    vector<int> m_readyToSend;                  // is an output port ready to send data (received fct)

    vector<int> m_outProc;                      // is something redirecting now from this port
    vector<pair<int, sc_time> > m_inProc;       // first - is something redirecting to this port; 2 - finishing time

    set<int> m_redirectingData, m_redirectingFct;
    vector<int> m_routingTable;
    map<int, sc_time> m_freedPorts;             // map of ports - num + sending begin-time
    vector<bool> m_destForTimeCode, m_destForFct; // receivers of TC & FCT
    sc_event m_newDataToSend, m_portFreed;      // we have data for redirection or any out-port received fct
    sc_event m_fctDelayedEvent;    

public:
    SC_HAS_PROCESS(routing_switch);

    routing_switch(sc_module_name mn, int id, int ports, sc_time delay, vector<int> table);

    virtual void write_byte(int inPortID, symbol symb);

    virtual void fct(int inPortID);

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
