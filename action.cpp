#include "action.h"
void initi()
{
    int tmp;
    ifstream in;
    in.open("config");
    in >> nodes >> table_size;

    //filling schedule tablelabel
    schedule_table.resize(nodes);
    for (int i = 0; i < nodes; i++)
    {
        for (int j = 0; j < table_size; j++)
        {
            in >> tmp;
            schedule_table[i].push_back(tmp);
        }
        nfct.push_back(false);
        rfct.push_back(false);
    }


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
    router0 = new router("router", sc_time(delay_router, SC_NS));

    for (int i = 0; i < nodes; i++)
    {
        (*router0).fct_port[i](*n[i]);
        (*n[i]).fct_port(*router0);
    }

    tM = new time_manager("time_manager_node", n[2]);

}
void next_step()
{
    cerr << sc_delta_count() << "\n";
    sc_start(1, SC_NS);

}

