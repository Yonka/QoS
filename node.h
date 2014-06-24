#ifndef NODE_H
#define NODE_H

#include "systemc.h"
#include <vector>

#include "my_interfaces.h"
#include "defs.h"
#include "QoS.h"
#include "node_QoS_if.h"
#include "QoS_node_if.h"

using namespace std;

class node : public sc_module, public writeI, public conn_I, public QoS_node_if
{
private:
    sc_fifo<sc_uint<8> > write_buf;
    sc_uint<8> tmp_byte;
    sc_event eop, fct_event, fct_delayed_event, r_sender;
    sc_time delay;
    int received_time, address;
    bool have_time_code_to_send, have_data_to_send, have_fct_to_send, can_send;
    sc_fifo<sc_uint<8> > read_buf;
    int ready_to_write, processed;    //got fct?

    void sender();

    void fct_delayed();

    void new_time_code(int value);

public:
    int id, direct;
    QoS* m_QoS;
    sc_port<conn_I> fct_port;
    sc_port<node_QoS_if> node_QoS_port;

    SC_HAS_PROCESS(node);
    node(sc_module_name mn, int id, int addr, sc_time delay);

    virtual bool write(std::vector<sc_uint<8> >* packet);

    virtual void write_byte(int num, symbol s);

    virtual void fct(int num);

    virtual void ban_sending();

    virtual void unban_sending();

    void init();

    friend class time_manager;
};
#endif
