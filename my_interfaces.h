#ifndef MY_INTERFACES_H
#define MY_INTERFACES_H
#include "systemc.h"
class writeI : virtual public sc_interface
{
public:
    virtual void write(sc_uint<8> data) = 0;
};

class receiver_router_I : virtual public sc_interface
{
public:
    virtual void fct(int num, sc_time holdup) = 0;
    virtual void write_byte(int num, sc_uint<8> data) = 0;
};

class router_receiver_I : virtual public sc_interface
{
public:
    virtual void fct(sc_time holdup) = 0;
    virtual void write_byte(sc_uint<8> data) = 0;
};
#endif