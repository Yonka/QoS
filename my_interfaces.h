#ifndef MY_INTERFACES_H
#define MY_INTERFACES_H
#include "systemc.h"
#include "defs.h"
class writeI : virtual public sc_interface
{
public:
    virtual void write(sc_uint<8> data) = 0;
//    virtual void writeTick(double tickValue) = 0;
};

class node_router_I : virtual public sc_interface
{
public:
    virtual void fct(int num, sc_time holdup) = 0;
    virtual void write_byte(int num, symbol s) = 0;
};

class router_node_I : virtual public sc_interface
{
public:
    virtual void fct(sc_time holdup) = 0;
    virtual void write_byte(symbol s) = 0;
};
#endif
