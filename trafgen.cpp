#include "trafgen.h"
#include "cstdlib"
#include "ctime"
#include <vector>

trafgen::trafgen(sc_module_name mn, int dest_id, int packets_num = 1) : sc_module(mn), dest_id(dest_id), runs(packets_num)
{
    srand((unsigned int) time(0));

    SC_THREAD(genEvent);
    sensitive << m_gen_next_event;
    
    SC_METHOD(sendPacket);
    dont_initialize();
    sensitive << m_send_event;
}

void trafgen::genEvent()
{
    for (int r = 0; r < runs; r++)
    {
        m_packet.push_back(EOP_SYMBOL);
        for (int i = 0; i < PACKET_SIZE - 2; i++)
        {
            m_packet.push_back(rand() % 254 + 1);
        }
        m_packet.push_back(dest_id);
        m_send_event.notify(SC_ZERO_TIME);
        wait();
    }
}

void trafgen::sendPacket()
{
    trafgen_node_port->write_packet(&m_packet);
}

void trafgen::new_packet_request()
{
    m_gen_next_event.notify(SC_ZERO_TIME);
}
