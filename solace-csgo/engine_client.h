#pragma once
#include "includes.h"

struct engine_player_info_t {
	int64_t __pad0;
	union {
		int64_t xuid;
		struct {
			int xuidlow;
			int xuidhigh;
		};
	};
	char name[ 128 ];
	int userid;
	char guid[ 33 ];
	unsigned int friendsid;
	char friendsname[ 128 ];
	bool fakeplayer;
	bool ishltv;
	unsigned int customfiles[ 4 ];
	unsigned char filesdownloaded;
};

class i_net_channel {
private:
	PAD( 0x14 );

public:
	bool m_processing_messages;		// 0x0014
	bool m_should_delete;			// 0x0015

private:
	PAD( 0x2 );

public:
	int m_out_seq;					// 0x0018 last send outgoing sequence number
	int m_in_seq;					// 0x001C last received incoming sequnec number
	int m_out_seq_ack;				// 0x0020 last received acknowledge outgoing sequnce number
	int m_out_rel_state;			// 0x0024 state of outgoing reliable data (0/1) flip flop used for loss detection
	int m_in_rel_state;				// 0x0028 state of incoming reliable data
	int m_choked_packets;			// 0x002C number of choked packets

private:
	PAD( 0x414 );					// 0x0030
public:
	VFUNC( GetLatency( int flow ), 9, float( __thiscall * )( decltype( this ), int flow ), flow );
	VFUNC( GetAvgLatency( int flow ), 10, float( __thiscall * )( decltype( this ), int flow ), flow );
	VFUNC( SendDatagram( void *data = nullptr ), 48, int( __thiscall * )( decltype( this ), void* ), data );
	VFUNC( CanPacket( ), 58, bool( __thiscall * )( decltype( this ) ) );
	VFUNC( IsLoopBack( ), 6, bool( __thiscall * )( decltype( this ) ) );
	VFUNC( SetChoked( ), 47, void( __thiscall * )( decltype( this ) ) );
	VFUNC( IsTimingOut( ), 7, bool( __thiscall * )( decltype( this ) ) );
};


class c_engine_client {
public:
	const matrix_t &world_to_screen( ) {
		typedef const matrix_t &( __thiscall *oWorldToScreenMatrix )( PVOID );
		return util::get_virtual_function<oWorldToScreenMatrix>( this, 37 )( this );
	}
	__forceinline int GetPlayerForUserID( int uid ) {
		return util::get_virtual_function< int( __thiscall * )( decltype( this ), int ) >( this, 9 )( this, uid );
	}
	__forceinline void FireEvents( ) {
		return util::get_virtual_function< void( __thiscall * )( decltype( this ) ) >( this, 59 )( this );
	}

	VFUNC( get_screen_size( int& width, int& height ), 5, void( __thiscall* )( decltype( this ), int&, int& ), width, height );
	VFUNC( in_game( ), 26, bool( __thiscall* )( decltype( this ) ) );
	VFUNC( is_connected( ), 27, bool( __thiscall* )( decltype( this ) ) );
	VFUNC( client_cmd_urestricted( const char* cmd_string ), 108, void( __thiscall* )( decltype( this ), const char* ), cmd_string );
	VFUNC( local_player_index( ), 12, int( __thiscall * )( decltype( this ) ) );
	VFUNC( set_view_angles( ang_t &angles ), 19, void( __thiscall * )( decltype( this ), ang_t & ), angles );
	void get_view_angles( ang_t &angles ) {
		return util::get_virtual_function<void( __thiscall * )( void *, ang_t & )>( this, 18 )( this, angles );
	}
	i_net_channel *get_net_channel_info( ) {
		using original_fn = i_net_channel * ( __thiscall * )( decltype(this) );
		return ( *( original_fn ** )this )[ 78 ]( this );
	}
	bool get_player_info( int index, engine_player_info_t *info ) {
		using original_fn = bool( __thiscall * )( decltype(this), int, engine_player_info_t * );
		return ( *( original_fn ** )this )[ 8 ]( this, index, info );
	}
};