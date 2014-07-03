#ifndef DATA_IF_H
#define DATA_IF_H
#include "systemc.h"
#include "defs.h"

class data_if : virtual public sc_interface
{
public:
    virtual void fct(int inPortID) = 0;
    virtual void write_byte(int inPortID, symbol symb) = 0;
};
#endif
