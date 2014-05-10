#ifndef DEFS_H
#define DEFS_H
#include <vector>

using namespace std;

#define FCT_SIZE 1
#define EOP_SYMBOL 0
#define BROADCAST_SYMBOL 255

extern vector<vector<int> > schedule_table;

enum symbol_type
{
    lchar = 14,
    nchar = 10
};

struct symbol {
    int data;
    int addr;
    symbol_type t;
    symbol(){};
    symbol(int data, int addr, symbol_type t): data(data), addr(addr), t(t) {};
};


#endif