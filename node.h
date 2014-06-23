#ifndef RECEIVER_H
#define RECEIVER_H

#include "my_interfaces.h"
#include "systemc.h"
#include "defs.h"
#include "QoS.h"
#include <vector>

using namespace std;

class node : public sc_module, public writeI, public conn_I
{
private:
    sc_fifo<sc_uint<8> > write_buf;
    sc_uint<8> tmp_byte;
    sc_event eop, fct_event, fct_delayed_event, r_sender;
    sc_time delay;
    int received_time, address;
    bool have_time_code_to_send, have_data_to_send, have_fct_to_send;
    sc_fifo<sc_uint<8> > read_buf;
    int ready_to_write, processed;    //got fct?
    QoS m_QoS;

    void sender();

    void fct_delayed();

    void new_time_code(int value);

public:
    int id, direct;
    sc_port<conn_I> fct_port;

    SC_HAS_PROCESS(node);
    node(sc_module_name mn, int id, int addr, sc_time delay);

    virtual bool write(std::vector<sc_uint<8> >* packet);

    virtual void write_byte(int num, symbol s);

    virtual void fct(int num);

    void init();

    friend class time_manager;
};
#endif
