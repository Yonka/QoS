#ifndef DEFS_H
#define DEFS_H
#include <vector>

#include <stdio.h>

using namespace std;

#define FCT_SIZE 4
#define EOP_SYMBOL 255
#define TICK 500000
#define SIM_TIME 64
#define epoch 64
#define PACKETS 20000
typedef double tick_value_type;

//extern vector<vector<int> > schedule_table;
//extern int table_size;
extern vector<sc_time> delays;
extern int scheduling;
extern vector<int> GV; 
extern vector<vector<int> > traf;
extern int stat_n, stat_m, stat_k;

enum symbol_type
{
    lchar = 14,
    nchar = 10
};

struct symbol {
    int data;
    int addr;
    int source;
    symbol_type t;
    symbol(){};
    symbol(int data, int addr, int source, symbol_type t): data(data), addr(addr), source(source), t(t) {};
};


#endif
