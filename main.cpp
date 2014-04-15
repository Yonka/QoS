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

    trafgen t0("trafgen0", 0);
    receiver r0("receiver0", 0);
    t0.out_port(r0);

    trafgen t1("trafgen1", 3);
    receiver r1("receiver1", 1);
    t1.out_port(r1);

    trafgen t2("trafgen2", 2);
    receiver r2("receiver2", 2);
    t2.out_port(r2);

    router routerE("router");
    routerE.fct_port[0](r0);
    routerE.fct_port[1](r1);
    routerE.fct_port[2](r2);

    r0.fct_port(routerE);
    r1.fct_port(routerE);
    r2.fct_port(routerE);


    sc_start(20000, SC_MS);

    return(0);
}