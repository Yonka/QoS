#include "trafgen.h"
#include "cstdlib"
#include "ctime"
#include <vector>

trafgen::trafgen(sc_module_name mn, int param = 1, sc_time delay = sc_time(0, SC_NS)) : sc_module(mn), runs(param), delay(delay)
{
    srand((unsigned int) time(0));
    SC_THREAD(gen_event);
    sensitive << send_next;
    SC_METHOD(send_byte);
    sensitive << send;
}

void trafgen::gen_event()
{
    for (int r = 0; r < runs; r++)
    {
        std::vector<sc_uint<8> > packet;
        packet.push_back(255);
        for (int i = 0; i < 19; i++)
        {
            packet.push_back((sc_uint<8>)rand() % 255);
        }
        while (!packet.empty())
        {
            x = packet.back();
            packet.pop_back();
            send.notify(delay);
            wait();
        }
        wait(1000, SC_NS);
    }
}

void trafgen::send_byte()
{
//    cout << "send " << x << '\n';
    out_port->write(x);
    send_next.notify(SC_ZERO_TIME);
}
