#include <math.h>
#include "systemc.h"
#include "defs.h"
#include "cstdlib"
#include "ctime"
#include "trafgen.h"
#include "node.h"
#include "routing_switch.h"
#include "time_manager.h"
using namespace std;

vector<vector<int> > schedule_table;
vector<vector<int> > traf;
vector<sc_time> delays;
int table_size;
int scheduling;
vector<int> GV;
int stat_n, stat_m, stat_k;

int sc_main(int argc, char* argv[])
{
    int nodes, tmp;
    ifstream in;
    in.open("config");
    in >> scheduling;
    in >> nodes >> table_size;
    stat_n = 0; stat_m = 0; stat_k = 0;
    //filling schedule table
    schedule_table.resize(nodes);
    GV.resize(nodes, 0);
    traf.resize(nodes);
    for (int i = 0; i < nodes; i++)
    {
        traf[i].resize(nodes, 0);
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
        int direction;
        in >> delay_traf >> delay_node >> direction >> name;
        delays.push_back(sc_time(delay_node, SC_NS));
        char* a = new char[name.size()];
        a[name.size()] = 0;
        memcpy(a,name.c_str(), name.size());
        trafgen *t0 = new trafgen("trafgen", PACKETS, sc_time(delay_traf, SC_NS));
        node *n0 = new node(a, i, direction, sc_time(delay_node, SC_NS));
        (*t0).out_port(*n0);
        t.push_back(t0);
        n.push_back(n0);
    }
    int num_routers;
    in >> num_routers;
    int delay_router;
    in >> delay_router;
    vector<routing_switch*> routers;
    int ports;
    for (int i = 0; i < num_routers; i++)
    {
        string name;
        in >> ports >> name;
        char* a = new char[name.size()];
        a[name.size()] = 0;
        memcpy(a,name.c_str(), name.size());
        vector<int> table;
        int tmp;
        for (int i = 0; i < nodes; i++)
        {
            in >> tmp;
            table.push_back(tmp);
        }
        routing_switch* r = new routing_switch(a, i, ports, sc_time(delay_router, SC_NS), table);
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
    int tm_num;
    in >> tm_num;
    time_manager tm("time_manager_node", n[tm_num]);

//    sc_start(0);
    sc_start(SIM_TIME, SC_MS);
    int sum = 0;
    ofstream out;
    out.open("result_suffle_stat0");
    out << scheduling << "\n";
    int k = 0;
    for (int i = 0; i < GV.size(); i++)
    {
        if (GV[i] == 0)
            continue;
        out << i << ":" << GV[i] << "\n";
        for (int j = 0; j < nodes; j++)
        {
            if (traf[i][j] != 0)
            {
                k++;
                out << "\t" << j << ":\t" << traf[i][j] << "\n";
            }
        }
        sum += GV[i];
    }
    double x_ = sum / k;
    double sq_sum = 0;
    for (int i = 0; i < GV.size(); i++)
    {
        for (int j = 0; j < nodes; j++)
        {
            if (traf[i][j] != 0)
                sq_sum += (x_ - traf[i][j]) * (x_ - traf[i][j]);
        }
    }
    double sigma = sqrt(sq_sum / k);
    out << "\n" << sum << " " << sigma << "\n";
    out << stat_n << " " << stat_m << " "  << stat_k << "\n";
    out << (double)stat_m * 8 / (4 * (double)stat_n + 10 * (double)stat_m + 14 * (double)stat_k) << "\n";
    out << (double)stat_m * 10 / (4 * (double)stat_n + 10 * (double)stat_m + 14 * (double)stat_k )<< "\n";
    out.close();
    return(0);
}
