#ifndef RECEIVER_H
#define RECEIVER_H

#include "my_interfaces.h"
#include "systemc.h"

#define FCT_SIZE 1

class receiver : public sc_module, public writeI, public router_receiver_I
{
public:
    SC_HAS_PROCESS(receiver);
    receiver(sc_module_name mn, int addr, sc_time delay);

    virtual void write(sc_uint<8> data);
    virtual void write_byte(sc_uint<8> data);
    virtual void fct(sc_time holdup);
    void init();

    sc_port<receiver_router_I> fct_port;
private:
    sc_fifo<sc_uint<8> > write_buf;
    sc_uint<8> tmp_byte;
    sc_event eop, fct_event, fct_delayed_event;
    int address;
    sc_time delay;

    sc_fifo<sc_uint<8> > read_buf;

    bool ready_to_write;

    void sender();

    void fct_delayed();
};
#endif
