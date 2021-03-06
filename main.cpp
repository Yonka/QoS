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

vector<vector<bool> > schedule_table;
vector<vector<int> > traf;
vector<sc_time> delays;
int table_size;
int scheduling;
vector<int> packets_count;
int stat_n, stat_m, stat_k;

char* make_name(string s, int n)
{
    stringstream sstm;
    sstm.str(std::string());
    sstm << s << n;
    string st = sstm.str();
    char* a = new char[st.size()];
    a[st.size()] = 0;
    memcpy(a, st.c_str(), st.size());
    return a;
}

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
    packets_count.resize(nodes, 0);
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

    vector<trafgen *> trafgenVector;
    vector<node *> nodeVector;
    for (int i = 0; i < nodes; i++)
    {
        double delay_node;
        int dest_id;
        in >> delay_node >> dest_id;
        delays.push_back(sc_time(delay_node, SC_NS));
        string name_node_tmp = "node_";
        string name_trafgen_tmp = "trafgen_";
        char* name_node = make_name(name_node_tmp, i);
        char* name_trafgen = make_name(name_trafgen_tmp, i);
        trafgen *trafgenUnit = new trafgen(name_trafgen, dest_id, PACKETS);
        node *nodeUnit = new node(name_node, i, dest_id, sc_time(delay_node, SC_NS));
        (*nodeUnit).set_scheduling(scheduling, schedule_table);
        (*trafgenUnit).trafgen_node_port(*nodeUnit);
        (*nodeUnit).node_trafgen_port(*trafgenUnit);
        trafgenVector.push_back(trafgenUnit);
        nodeVector.push_back(nodeUnit);
    }
    int num_routers, delay_router;
    in >> num_routers >> delay_router;
    vector<routing_switch*> routerVector;
    int ports;
    for (int i = 0; i < num_routers; i++)
    {
        string name_router_tmp = "router_";
        in >> ports;
        char* name_router = make_name(name_router_tmp, i);
        vector<int> table;
        int tmp;
        for (int j = 0; j < nodes; j++)
        {
            in >> tmp;
            table.push_back(tmp);
        }
        routing_switch* routerUnit = new routing_switch(name_router, i, ports, sc_time(delay_router, SC_NS), table);
        routerVector.push_back(routerUnit);
    }
    for (int i = 0; i < num_routers; i++)
    {
        in >> ports;
        int a, b, c;
        for (int j = 0; j < ports; j++)
        {
            in >> a >> b;
            sc_port<data_if>* port = (*routerVector[i]).data_port[a];
            (*port)(*nodeVector[b]);
            (*nodeVector[b]).data_port(*routerVector[i]);
            (*nodeVector[b]).direct = a;
        }
        for (int j = 0; j < (*routerVector[i]).data_port.size() - ports; j++)
        {
            in >> c >> a >> b;
            sc_port<data_if>* port = (*routerVector[i]).data_port[a];
            (*port)(*routerVector[c]);
            (*routerVector[i]).direct[a] = b;
        }
    }
    int tm_num;
    in >> tm_num;
    time_manager tm("time_manager_node", nodeVector[tm_num]);

//    sc_start(0);
    sc_start(SIM_TIME, SC_MS);
   
    //////////counting statistics

    int sum = 0;
    ofstream out;
    out.open("result1");
    out << scheduling << "\n";
    int k = 0;
    for (int i = 0; i < packets_count.size(); i++)
    {
        if (packets_count[i] == 0)
            continue;
        out << i << ":" << packets_count[i] << "\n";
        for (int j = 0; j < nodes; j++)
        {
            if (traf[i][j] != 0)
            {
                k++;
                out << "\t" << j << ":\t" << traf[i][j] << "\n";
            }
        }
        sum += packets_count[i];
    }
    double x_ = sum / k;
    double sq_sum = 0;
    for (int i = 0; i < packets_count.size(); i++)
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
