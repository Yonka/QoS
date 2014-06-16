#ifndef RECEIVER_H
#define RECEIVER_H

#include "my_interfaces.h"
#include "systemc.h"
#include "defs.h"
#include <vector>

using namespace std;

class node : public sc_module, public writeI, public conn_I
{
private:
    sc_fifo<sc_uint<8> > write_buf;
    sc_uint<8> tmp_byte;
    sc_event eop, fct_event, fct_delayed_event, time_code_event, r_sender;
    sc_time delay;
    int cur_time, received_time, address;
    sc_time m_t_tc, m_t_te, m_tc_begin_time, m_e_begin_time; //t_tc value, prev t_tc value, interval beginning time, 
    bool have_time_code_to_send, have_data_to_send, have_fct_to_send;
    sc_fifo<sc_uint<8> > read_buf;
    bool ready_to_write;    //got fct?
    bool mark_h, time_h;    //tick & timer

    void change_tc();

    void change_tc_scheduling1();

    void change_tc_scheduling2();

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

    virtual void time_code(int t);

    void init();

    friend class time_manager;
};
#endif
