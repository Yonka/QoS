#ifndef NODE_H
#define NODE_H

#include "systemc.h"
#include <vector>

#include "data_if.h"
#include "trafgen_node_if.h"
#include "node_trafgen_if.h"
#include "defs.h"
#include "QoS.h"
#include "node_QoS_if.h"
#include "QoS_node_if.h"

using namespace std;

class node: 
    public sc_module, 
    public trafgen_node_if, 
    public data_if, 
    public QoS_node_if
{
    friend class time_manager;

public:
    int id, direct;                             // node id, node binded to direct port
    QoS* m_QoS;
    sc_port<data_if> data_port;
    sc_port<node_trafgen_if> node_trafgen_port;
    sc_port<node_QoS_if> node_QoS_port;

private:
    int m_tmpByte;
    sc_event m_fctEvent, m_fctDelayedEvent, m_runSender;
    sc_time m_delay;
    int m_receivedTime, m_destID;
    bool m_haveTimeTodeToSend, m_haveDataToSend, m_haveFctToSend;
    bool m_canSend;                    // is it allowed time-slot
    sc_fifo<int> m_inBuffer, m_outBuffer;
    int m_readyToWrite, m_processed;  // number of nchars we can send/received    

public:
    SC_HAS_PROCESS(node);

    node(sc_module_name mn, int id, int dest_id, sc_time delay);

    virtual bool write_packet(std::vector<int>* packet);

    virtual void write_byte(int inPortID, symbol symb);

    virtual void fct(int inPortID);

    virtual void ban_sending();

    virtual void unban_sending();

    void set_scheduling(int scheduling, vector<vector<bool> > schedule_table);

private:
    void sender();

    void fctDelayed();

    void newTimeCode(int value);

    void init();
};
#endif
