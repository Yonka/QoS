#ifndef RECEIVER_H
#define RECEIVER_H

#include "my_interfaces.h"
#include "systemc.h"
#include "defs.h"
#include <vector>

using namespace std;

class node : public sc_module, public writeI, public router_node_I
{
public:
    SC_HAS_PROCESS(node);
    node(sc_module_name mn, int addr, sc_time delay);

    virtual bool write(std::vector<sc_uint<8> > packet);
    virtual void write_byte(symbol s);
    virtual void fct();
    virtual void time_code(int t);
    void init();

    sc_port<node_router_I> fct_port;

    friend class time_manager;
private:
    sc_fifo<sc_uint<8> > write_buf;
    sc_uint<8> tmp_byte;
    sc_event eop, fct_event, fct_delayed_event, time_code_event;
    int address;

    sc_time delay;
    int cur_time, received_time;
    sc_time m_t_tc, m_t_te, m_tc_begin_time, m_e_begin_time; //t_tc value, prev t_tc value, interval beginning time, 
    bool have_time_code_to_send;
    sc_fifo<sc_uint<8> > read_buf;
    bool ready_to_write;    //got fct?
    bool mark_h, time_h;    //tick & timer
//    bool event1, event2;


    void change_tc();

    void sender();

    void fct_delayed();

    void new_time_code(int value);
};
#endif
