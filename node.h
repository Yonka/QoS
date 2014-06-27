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
    int id, direct;
    QoS* m_QoS;
    sc_port<data_if> data_port;
    sc_port<node_trafgen_if> node_trafgen_port;
    sc_port<node_QoS_if> node_QoS_port;

private:
    sc_fifo<sc_uint<8> > m_write_buf;
    sc_uint<8> m_tmp_byte;
    sc_event m_eop, m_fct_event, m_fct_delayed_event, m_repeat_sender;
    sc_time m_delay;
    int m_received_time, m_dest_id;
    bool m_have_time_code_to_send, m_have_data_to_send, m_have_fct_to_send, m_can_send;
    sc_fifo<sc_uint<8> > m_read_buf;
    int m_ready_to_write, m_processed;    

public:
    SC_HAS_PROCESS(node);

    node(sc_module_name mn, int id, int dest_id, sc_time delay);

    virtual bool write_packet(std::vector<sc_uint<8> >* packet);

    virtual void write_byte(int num, symbol s);

    virtual void fct(int num);

    virtual void ban_sending();

    virtual void unban_sending();

    void init();

    void set_scheduling(int scheduling, vector<vector<bool> > schedule_table);

private:
    void sender();

    void fctDelayed();

    void newTimeCode(int value);
};
#endif
