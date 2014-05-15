#include "trafgen.h"
#include "cstdlib"
#include "ctime"
#include <vector>

trafgen::trafgen(sc_module_name mn, int param = 1, sc_time delay = sc_time(0, SC_NS)) : sc_module(mn), runs(param), delay(delay)
{
    packet_size = 4;
    srand((unsigned int) time(0));

    SC_THREAD(gen_event);
    sensitive << send_next;
    
    SC_METHOD(send_byte);
    dont_initialize();
    sensitive << send;
}

void trafgen::gen_event()
{
    for (int r = 0; r < runs; r++)
    {
        success = false;
        packet.push_back(EOP_SYMBOL);
        for (int i = 0; i < packet_size; i++)
        {
            packet.push_back((sc_uint<8>)rand() % 254 + 1);
        }
        while (!success)
        {
            send.notify(SC_ZERO_TIME);
            wait();
//TODO : vector -> sc_vector(?) and use empty event(if exists Oo)
            if (packet.empty())
                break;
        }
    }
}

void trafgen::send_byte()
{
//    cout << "send " << x << '\n';

    //////////////////////////////////////////////////////////////////////////
    //TODO:rename event
    if (out_port->write(&packet))
    {
        success = true;
        send_next.notify(SC_ZERO_TIME); 
    }
    else 
        send_next.notify(delay); 
}
