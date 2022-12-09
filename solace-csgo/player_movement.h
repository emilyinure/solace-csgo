#pragma once
#include "includes.h"
#include <cstdint>

#include "vec3.h"

class player_t;
class cmd_t;

class player_move_helper {
public:
	bool	first_run_of_iunctions : 1;
	bool	game_code_moved_player : 1;
	int	player_handle;
	int	impulse_command;
	vec3_t	view_angles;
	vec3_t	abs_view_angles;
	int	buttons;
	int	old_buttons;
	float	forward_move;
	float	side_move;
	float	up_move;
	float	max_speed;
	float	client_max_speed;
	vec3_t	velocity;
	vec3_t	angles;
	vec3_t	old_angles;
	float	out_step_height;
	vec3_t	wish_velocity;
	vec3_t	jump_velocity;
	vec3_t	constraint_center;
	float	constraint_radius;
	float	constraint_width;
	float	constraint_speed_factor;
	float	u0[ 5 ];
	vec3_t	abs_origin;
	virtual	void u1( ) = 0;
	virtual void set_host( player_t *host ) = 0;
};

struct player_move_data {
public:
	bool    first_run_of_instructions : 1;
	bool    game_code_moved_player : 1;
	int     player_handle;
	int     impulse_command;
	ang_t	view_angles;
	vec3_t	abs_view_angles;
	int     buttons;
	int     old_buttons;
	float   fw_move;
	float   sd_move;
	float   up_move;
	float   max_speed;
	float   client_max_speed;
	vec3_t	velocity;
	vec3_t	angles;
	vec3_t	old_angles;
	float   step_height;
	vec3_t	wish_velocity;
	vec3_t	jump_velocity;
	vec3_t	constraint_center;
	float   constraint_radius;
	float   constraint_width;
	float   constraint_speed_factor;
	float   u0[ 5 ];
	vec3_t	abs_origin;
};

class virtual_game_movement {

public:
	virtual				~virtual_game_movement( void ) { }
	virtual void			process_movement( player_t *player, player_move_data *move ) = 0;
	virtual void			reset( void ) = 0;
	virtual void			start_track_prediction_errors( player_t *player ) = 0;
	virtual void			finish_track_prediction_errors( player_t *player ) = 0;
	virtual void			diff_print( char const *fmt, ... ) = 0;
	virtual vec3_t const &get_player_mins( bool ducked ) const = 0;
	virtual vec3_t const &get_player_maxs( bool ducked ) const = 0;
	virtual vec3_t const &get_player_view_offset( bool ducked ) const = 0;
	virtual bool			is_moving_player_stuck( void ) const = 0;
	virtual player_t *get_moving_player( void ) const = 0;
	virtual void			unblock_posher( player_t *player, player_t *pusher ) = 0;
	virtual void			setup_movement_bounds( player_move_data *move ) = 0;
};

class player_game_movement : public virtual_game_movement {
public:
	virtual ~player_game_movement( void ) { }
};

class player_prediction {
public:
	uint8_t		pad0[ 0x4 ];				// 0x0000
	std::uintptr_t	hLastGround;			// 0x0004
	bool			bInPrediction;			// 0x0008
	bool			bOldCLPredictValue;		// 0x0009
	bool			bEnginePaused;			// 0x000A
	uint8_t		pad2[ 0xD ];				// 0x000B
	bool			bIsFirstTimePredicted;	// 0x0018
public:

	void SuppressHostEvents(void* player) {
		static auto addr = **address(util::find("client.dll", "A1 ? ? ? ? 0F 94 C1 85 C0")).as< DWORD** >(1);
		auto next = addr;
		while (next)
		{
			*(DWORD*)(next + 12) = (int)player;
			*(DWORD*)(next + 8) = player != nullptr;
			if (*(DWORD*)(next + 16) > 0)
				*(DWORD*)(next + 16) = 0;
			next = *(DWORD*)( next + 4);
		}
	}

	bool in_prediction( ) {
		typedef bool( __thiscall *o_in_prediction )( void * );
		return util::get_virtual_function<o_in_prediction>( this, 14 )( this );
	}

	void run_command( player_t *player, cmd_t *cmd, player_move_helper *helper ) {
		typedef void( __thiscall *o_run_command )( void *, player_t *, cmd_t *, player_move_helper * );
		return util::get_virtual_function<o_run_command>( this, 19 )( this, player, cmd, helper );
	}

	void setup_move( player_t *player, cmd_t *cmd, player_move_helper *helper, void *data ) {
		typedef void( __thiscall *o_setup_move )( void *, player_t *, cmd_t *, player_move_helper *, void * );
		return util::get_virtual_function<o_setup_move>( this, 20 )( this, player, cmd, helper, data );
	}

	void check_moving_ground( player_t *player, double dbFrametime ) {
		typedef void( __thiscall *check_moving_ground_t )( void *, player_t *, double dbFrametime );
		util::get_virtual_function< check_moving_ground_t >( this, 18 )( this, player, dbFrametime );
	}

	void update( int startframe, bool validframe, int incoming_acknowledged, int outgoing_command ) {
		typedef void( __thiscall *o_update )( void *, int, bool, int, int );
		return util::get_virtual_function<o_update>( this, 3 )( this, startframe, validframe, incoming_acknowledged, outgoing_command );
	}

	void SetLocalViewAngles( const ang_t &ang ) {
		return util::get_virtual_function< void( __thiscall * )( decltype( this ), const ang_t & ) >( this, 13 )( this, ang );
	}

	void finish_move( player_t *player, cmd_t *cmd, void *data ) {
		typedef void( __thiscall *o_finish_move )( void *, player_t *, cmd_t *, void * );
		return util::get_virtual_function<o_finish_move>( this, 21 )( this, player, cmd, data );
	}
};
