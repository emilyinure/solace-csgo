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
class client_state_t {
private:
	PAD( 0x9C );                                // 0x0000

public:
	i_net_channel *m_net_channel;				// 0x009C

private:
	PAD( 0x70 );                                // 0x00A0

public:
	int				m_next_message_time;		// 0x0110

public:
	float           m_net_cmd_time;             // 0x0114
	uint32_t        m_server_count;             // 0x0118
	uint32_t        current_sequence;             // 0x011C
private:
	PAD( 0x48 );								// 0x0120

public:
	int             m_unk;                      // 0x0168
	int             m_server_tick;              // 0x016C
	int             m_client_tick;              // 0x0170
	int             m_delta_tick;               // 0x0174

private:
	PAD( 0x4B30 );                              // 0x0178

public:
	float           m_frame_time;               // 0x4CA8
	int             m_last_outgoing_command;    // 0x4CAC
	int             m_choked_commands;          // 0x4CB0
	int             m_last_command_ack;         // 0x4CB4
	int			last_server_tick;	// same update pattern as last_command_ack, but with server ticks
	int			command_ack;		// current command sequence acknowledged by server
	PAD( 0x12C );                               // 0x4CB8
	event_info_t *m_events;					// 0x4DEC
};