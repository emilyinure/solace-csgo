#pragma once

#include <deque>


#include "hvh.h"
#include "vec3.h"
#include "animstate.h"

class cmd_t;
class interfaces_t;
class offsets_t;
class hooks_t;
class render_t;
class weapon_t;
class weapon_info_t;
class player_t;
class IDirect3DDevice9;
class i_net_channel;

using cl_move_t = void( __cdecl * ) ( float, bool );

class NetPos {
public:
	float  m_time;
	vec3_t m_pos;

public:
	__forceinline NetPos( ) : m_time{}, m_pos{} {};
	__forceinline NetPos( float time, vec3_t pos ) : m_time{ time }, m_pos{ pos } {};
};

class Sequence {
public:
	float m_time;
	int   m_state;
	int   m_seq;

public:
	__forceinline Sequence( ) : m_time{}, m_state{}, m_seq{} {};
	__forceinline Sequence( float time, int state, int seq ) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
};

enum CSWeaponType : int;

class c_g {
public:
	interfaces_t *m_interfaces = nullptr;
	offsets_t *m_offsets = nullptr;
	hooks_t *m_hooks = nullptr;
	render_t *m_render = nullptr;

	player_t *m_local = nullptr;
	weapon_t *m_weapon = nullptr;
	weapon_info_t *m_weapon_info = nullptr;

	vec3_t m_shoot_pos = vec3_t( );

	bool m_running_client = false;
	float spawn_time = 0;
     
	
	bool m_in_pred = false;
	ang_t m_view_angles = ang_t();
	bool  m_onground = false;;
	int   m_lag = 0;;
	int   m_last_lag = 0;
	int   m_max_lag = 0;
	vec3_t m_origin = vec3_t( );
	
	bool  m_force_view = false;
	HWND m_window;
	WNDPROC m_old_window;
	float m_latency;
	float m_lerp;
	ang_t m_rotation;
	int m_stage;
	bool m_can_fire = false;
	int m_player_fire;
	CSWeaponType m_weapon_type;
	int m_flags;
	bool m_shift = false;
	int m_available_ticks = 0;
	bool m_can_shift = false;
	int m_tickbase = false;
	bool m_old_packet = false;
	int m_old_lag = 0;
	ang_t m_radar;
	std::deque< NetPos >   m_net_pos;
	bool m_lagcomp = false;
	float m_body_pred =0;
	float m_anim_frame = 0;
	float m_speed = 0;
	int m_buttons = 0;
	bool m_old_shot = 0;
	bool  *m_packet;
	bool  *m_final_packet;
	bool m_shot = false;
	float m_poses[ 24 ];
	animation_layer_t m_layers[ 15 ];
	float m_abs_yaw = 0;
	float m_anim_time = 0;
	ang_t m_angle;
	float m_body = 0;
	bool m_ground = 0;

	std::deque< Sequence > m_sequences = {};
	bool m_datamap_updated;
	unsigned char*m_pStartData;
	unsigned char *m_pEndData;
	alignas( 16 ) bone_array_t * m_real_bones = nullptr;
	CIKContext m_ipk;
	bool m_bones_setup = false;
	bool m_map_setup = false;
	bool m_resolving = false;
	std::vector<int> m_cmds = {};
	bool m_rendering = false;
	bool m_valid_round = false;
	unsigned char *m_pPostPred;
	bool ran;
	int restore_choke;
	void * m_unpred_ground;
	bool m_ran;
	//activity_modifiers_wrapper m_activity;

	struct choked_log {
		vec3_t shoot_pos;
		vec3_t origin;
		ang_t view_angles;
		float m_body_pred;
		float m_anim_frame;
		vec3_t m_velocity;
		int commandnr;
		int flags;
		float curtime;
		float view_angle;
		float sidemove;
		float forwardmove;
	};
	std::deque<choked_log> m_choked_logs;

	void  init_cheat( );

	void ModifyEyePosition( anim_state* state, matrix_t* mat, vec3_t* pos );
	void release ( ) const;
	void on_render ( IDirect3DDevice9 *device );
	void on_tick ( cmd_t * cmd );
	void SetAngles ( ) const;
	void UpdateAnimations ( ) const;
	void UpdateInformation ( );
	int time_to_ticks ( float time ) const;
	float ticks_to_time ( int ticks ) const;
	bool can_weapon_fire ( ) const;
	std::vector<std::pair<int, int>> m_tick_base_log;
	void add_tickbase_log( int cmd_num, int tick_base );
	void get_tickbase_log( int cmd_num, int* tick_base );
	void on_move ( float, bool, cl_move_t original_cl_move );
	void UpdateIncomingSequences ( i_net_channel *net );
	void net_data_received ( ) const;

	cmd_t *m_cmd = nullptr;
	
	using random_seed_t = void(__cdecl *)(int);
	using random_int_t = int(__cdecl *)(int, int);
	using random_float_t = float(__cdecl *)(float, float);
	random_seed_t  random_seed;
	random_int_t   random_int ;
	random_float_t random_float;

	bool start_move ( cmd_t *cmd );
	void generate_shoot_position( );
	void end_move( cmd_t *cmd );
} inline g;
