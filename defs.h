#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>

#include <vector>

using namespace std;

#define FCT_SIZE 4
#define EOP_SYMBOL 255
#define TICK 500000
#define SIM_TIME 64.0005
#define PACKETS 20000
#define PACKET_SIZE 1024

#define WITHOUT_SCHEDULING 0
#define ALGORYTHM_1 1
#define ALGORYTHM_2 2
#define FREE_PORT -1

extern vector<sc_time> delays;
extern vector<int> packets_count; 
extern vector<vector<int> > traf;
extern int stat_n, stat_m, stat_k;

struct redirectingSymbolType{
enum : int 
{
    nothing,
    timeCode,
    fct,
    informSymbol
};

};
enum symbol_type
{
    lchar = 14,
    nchar = 10
};

struct symbol {
    int data;
    int address;
    int source;
    symbol_type symbolType; ///!!!!
    symbol(){};
    symbol(int data, int address, int source, symbol_type symbolType): data(data), address(address), source(source), symbolType(symbolType) {};
};
#endif
