#include "systemc.h"
#include "cstdlib"
#include "ctime"
#include "trafgen.h"
#include "receiver.h"
#include "router.h"
using namespace std;

int sc_main(int argc, char* argv[])
{
    int k = 1;
    if (argc == 2)
        k = atoi(argv[1]);

    trafgen t0("trafgen1", k);
    receiver r0("receiver1", 0);
    t0.out_port(r0);

    trafgen t1("trafgen2", k);
    receiver r1("receiver2", 1);
    t1.out_port(r1);

    router routerE("router");
    routerE.fct_port[0](r0);
    routerE.fct_port[1](r1);

    r0.fct_port(routerE);
    r1.fct_port(routerE);
    sc_start(2000, SC_MS);

    return(0);
}