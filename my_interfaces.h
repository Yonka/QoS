#ifndef MY_INTERFACES_H
#define MY_INTERFACES_H
#include "systemc.h"
class writeI : virtual public sc_interface
{
public:
    virtual void write(sc_uint<8> data) = 0;
};

class fctI : virtual public sc_interface
{
public:
    virtual void fct(int num) = 0;
    virtual void write_byte(int num, sc_uint<8> data) = 0;
};
#endif