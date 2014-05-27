#include "systemc.h"
#include "defs.h"
#include "cstdlib"
#include "ctime"
#include "trafgen.h"
#include "node.h"
#include "router.h"
#include "time_manager.h"
using namespace std;

vector<vector<int> > schedule_table;
vector<sc_time> delays;
int table_size;
int scheduling;
int sc_main(int argc, char* argv[])
{
    int k = 1;
    if (argc == 2)
        k = atoi(argv[1]);
    int nodes, tmp;
    ifstream in;
    in.open("config");
    in >> scheduling;
    in >> nodes >> table_size;

    //filling schedule table
    schedule_table.resize(nodes);
    for (int i = 0; i < nodes; i++)
    {
        for (int j = 0; j < table_size; j++)
        {
            in >> tmp;
            schedule_table[i].push_back(tmp);
        }
    }

    vector<trafgen *> t;
    vector<node *> n;
    for (int i = 0; i < nodes; i++)
    {
        double delay_traf, delay_node;
        string name;
        in >> delay_traf >> delay_node >> name;
        delays.push_back(sc_time(delay_node, SC_NS));
        char* a = new char[name.size()];
        a[name.size()] = 0;
        memcpy(a,name.c_str(), name.size());
        trafgen *t0 = new trafgen("trafgen", 10000, sc_time(delay_traf, SC_NS));
        node *n0 = new node(a, i, (i + 1) % nodes, sc_time(delay_node, SC_NS));
        (*t0).out_port(*n0);
        t.push_back(t0);
        n.push_back(n0);
    }
    int num_routers;
    in >> num_routers;
    int delay_router;
    in >> delay_router;
    vector<router*> routers;
    int ports;
    for (int i = 0; i < num_routers; i++)
    {
        in >> ports;
        vector<int> table;
        int tmp;
        for (int i = 0; i < nodes; i++)
        {
            in >> tmp;
            table.push_back(tmp);
        }
        router* r = new router("router", nodes + i, ports, sc_time(delay_router, SC_NS), table);
        routers.push_back(r);
    }
    for (int i = 0; i < num_routers; i++)
    {
        in >> ports;
        int a, b, c;
        for (int j = 0; j < ports; j++)
        {
            in >> a >> b;
            sc_port<conn_I>* port = (*routers[i]).fct_port[a];
            (*port)(*n[b]);
            (*n[b]).fct_port(*routers[i]);
            (*n[b]).direct = a;
        }
        for (int j = 0; j < (*routers[i]).fct_port.size() - ports; j++)
        {
            in >> c >> a >> b;
            sc_port<conn_I>* port = (*routers[i]).fct_port[a];
            (*port)(*routers[c]);
            (*routers[i]).direct[a] = b;
        }
    }
    time_manager tm("time_manager_node", n[2]);

//    sc_start(0);
    sc_start(20000, SC_MS);

    return(0);
}
