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
    int m_currentTime;
    int m_timeSenderPortID;                     // where time comes from
    vector<queue<symbol> > m_dataBuffer;        // buffer for informSymbol
    vector<symbol> m_timeBuffer;                // buffer for timeCode
    sc_time m_delay;

    vector<int> m_addressDestination;           // if port is sending data, then destination address, FREE_PORT otherwise 
    vector<int> m_processed;                    // how many symbols passed through this port

    vector<int> m_addressSource;                // if port is receiving data, then source address, FREE_PORT otherwise 
    vector<int> m_readyToSend;                  // is an output port ready to send data (received fct)

    vector<int> m_outProc;                      // is something redirecting now from this port
    vector<pair<int, sc_time> > m_inProc;       // first - is something redirecting to this port; second - finishing time

    set<int> m_redirectingFCT;
    vector<int> m_routingTable;
    map<int, sc_time> m_freedPorts;             // map of ports - num + sending begin-time
    vector<bool> m_destForTimeCode, m_destForFct; // receivers for TC & FCT
    sc_event m_newDataToSend, m_portFreed;      // we have data for redirecting / out-port received fct

public:
    SC_HAS_PROCESS(routing_switch);

    routing_switch(sc_module_name mn, int id, int ports, sc_time delay, vector<int> table);

    virtual void write_byte(int inPortID, symbol symb);

    virtual void fct(int inPortID);

private:
    void redirecting();

    void redirectingDataCloseConnection();

    void redirectingTime();

    void redirectingFCTOpenConnection();

    void redirectingFCTCloseConnection();

    void redirectingDataOpenConnection();

    bool innerConnect(int x);

    void initFCT();

    void init();

};
#endif
