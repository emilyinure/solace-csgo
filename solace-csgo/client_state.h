#pragma once
#include "includes.h"
#include <cstdint>

#include "vec3.h"
class i_net_channel;
class c_client_class;
class event_info_t {
public:
	enum {
		EVENT_INDEX_BITS = 8,
		EVENT_DATA_LEN_BITS = 11,
		MAX_EVENT_DATA = 192,  // ( 1<<8 bits == 256, but only using 192 below )
	};

	// 0 implies not in use
	short					m_class_id;
	float					m_fire_delay;
	const void *m_send_table;
	const c_client_class *m_client_class;
	int						m_bits;
	uint8_t *m_data;
	int						m_flags;
	PAD( 0x18 );
	event_info_t *m_next;
};

struct CClockDriftMgr
{
private:
    uint8_t m_pad_00[72];

public:
    int32_t m_iCurClockOffset;
    int32_t m_nServerTick;
    int32_t m_nClientTick;
};

class client_state_t
{
public:
    int& m_nMaxClients()
    {
        return *(int*)((uintptr_t)this + 0x0310);
    }
    void* m_vmt_1;
    void* m_vmt_2;
    void* m_vmt_3;

private:
    uint8_t m_pad_00[144];

public:
    i_net_channel* m_NetChannel;

private:
    uint8_t m_pad_01[104];

public:
    int32_t m_nSignonState;

private:
    uint8_t m_pad_02[4];

public:
    double m_flNextCmdTime;
    int32_t m_nServerCount;
    int32_t m_nCurrentSequence;
    CClockDriftMgr m_ClockDriftMgr;
    int32_t m_nDeltaTick;

private:
    uint8_t m_pad_03[19252];

public:
    int32_t lastoutgoingcommand;
    int32_t chokedcommands;
    int32_t last_command_ack;
    int32_t last_server_tick;
    int32_t command_ack;
    PAD(0x12C);             // 0x4CB8
    event_info_t* m_events; // 0x4DEC
};