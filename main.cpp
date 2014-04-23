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

    trafgen t0("trafgen0", 0, sc_time(4, SC_NS));
    receiver r0("receiver0", 0, sc_time(4, SC_NS));
    t0.out_port(r0);

    trafgen t1("trafgen1", 1, sc_time(5, SC_NS));
    receiver r1("receiver1", 1, sc_time(1, SC_NS));
    t1.out_port(r1);

    trafgen t2("trafgen2", 2, sc_time(1, SC_NS));
    receiver r2("receiver2", 2, sc_time(10, SC_NS));
    t2.out_port(r2);

    router routerE("router", sc_time(10, SC_NS));
    routerE.fct_port[0](r0);
    routerE.fct_port[1](r1);
    routerE.fct_port[2](r2);

    r0.fct_port(routerE);
    r1.fct_port(routerE);
    r2.fct_port(routerE);


    sc_start(20000, SC_MS);

    return(0);
}