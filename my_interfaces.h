#ifndef MY_INTERFACES_H
#define MY_INTERFACES_H
#include "systemc.h"
#include "defs.h"
class writeI : virtual public sc_interface
{
public:
    virtual bool write(std::vector<sc_uint<8> > *packet) = 0;
};

class conn_I : virtual public sc_interface
{
public:
    virtual void fct(int num) = 0;
    virtual void write_byte(int num, symbol s) = 0;
};

//class router_router_I : virtual public sc_interface
//{
//public:
//    virtual void fct(int num) = 0;
//    virtual void write_byte(int num, symbol s) = 0;
//};

//class router_node_I : virtual public sc_interface
//{
//public:
//    virtual void fct(int num) = 0;
//    virtual void write_byte(int num, symbol s) = 0;
//};
#endif
