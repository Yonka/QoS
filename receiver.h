#ifndef RECEIVER_H
#define RECEIVER_H

#include "my_interfaces.h"
#include "systemc.h"
class receiver : public sc_module, public writeI, public fctI
{
public:
    sc_fifo<sc_uint<8> > write_buf;
    sc_uint<8> tmp_byte;
    sc_event eop, fct_event;
    int address;

    sc_port<fctI> fct_port;
    sc_fifo<sc_uint<8> > read_buf;

    bool ready_to_read, ready_to_write;

    SC_HAS_PROCESS(receiver);

    receiver(sc_module_name mn, int addr);

    virtual void write(sc_uint<8> data);

    virtual void write_byte(int num, sc_uint<8> data);

    virtual void fct(int num);

    void sender();
};
#endif
