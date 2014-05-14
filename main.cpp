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
int sc_main(int argc, char* argv[])
{
    int k = 1;
    if (argc == 2)
        k = atoi(argv[1]);
    int nodes, tmp;
    ifstream in;
    in.open("config");
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
        a[name.size()]=0;
        memcpy(a,name.c_str(), name.size());
        trafgen *t0 = new trafgen("trafgen", 10, sc_time(delay_traf, SC_NS));
        node *n0 = new node(a, i, sc_time(delay_node, SC_NS));
        (*t0).out_port(*n0);
        t.push_back(t0);
        n.push_back(n0);
    }

    int delay_router;
    in >> delay_router;
    router router0("router", sc_time(delay_router, SC_NS));

    for (int i = 0; i < nodes; i++)
    {
        router0.fct_port[i](*n[i]);
        (*n[i]).fct_port(router0);
    }

    time_manager tm("time_manager_node", n[2]);

//    sc_start(0);
    sc_start(20000, SC_MS);

    return(0);
}
