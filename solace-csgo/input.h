#pragma once
#include "includes.h"
#include "checksum_crc.h"

#define IN_ATTACK		(1 << 0)
#define IN_JUMP			(1 << 1)
#define IN_DUCK			(1 << 2)
#define IN_FORWARD		(1 << 3)
#define IN_BACK			(1 << 4)
#define IN_USE			(1 << 5)
#define IN_CANCEL		(1 << 6)
#define IN_LEFT			(1 << 7)
#define IN_RIGHT		(1 << 8)
#define IN_MOVELEFT		(1 << 9)
#define IN_MOVERIGHT	(1 << 10)
#define IN_ATTACK2		(1 << 11)
#define IN_RUN			(1 << 12)
#define IN_RELOAD		(1 << 13)
#define IN_ALT1			(1 << 14)
#define IN_ALT2			(1 << 15)
#define IN_SCORE		(1 << 16)   // Used by client.dll for when scoreboard is held down
#define IN_SPEED		(1 << 17)	// Player is holding the speed key
#define IN_WALK			(1 << 18)	// Player holding walk key
#define IN_ZOOM			(1 << 19)	// Zoom key for HUD zoom
#define IN_WEAPON1		(1 << 20)	// weapon defines these bits
#define IN_WEAPON2		(1 << 21)	// weapon defines these bits
#define IN_BULLRUSH		(1 << 22)
#define IN_GRENADE1		(1 << 23)	// grenade 1
#define IN_GRENADE2		(1 << 24)	// grenade 2
#define	IN_ATTACK3		(1 << 25)

class c_input_system {
public:
	void EnableInput( bool state ) {
		using original_fn = void( __thiscall* )( c_input_system*, bool );
		return ( *( original_fn** )this )[ 11 ]( this, state );
	}
};

class cmd_t {
public:
	int m_pad = 0;
	int m_command_number = 0;
	int m_tick_count = 0;
	ang_t m_viewangles;
	vec3_t m_aimdirection;
	float m_forwardmove = 0;
	float m_sidemove = 0;
	float m_upmove = 0;
	int m_buttons = 0;
	char m_impulse = 0;
	int m_weaponselect = 0;
	int m_weaponsubtype = 0;
	int m_randomseed = 0;
	short m_mousedx = 0;
	short m_mousedy = 0;
	bool m_predicted = false;
	vec3_t ulk;
	vec3_t uk;
	CRC32_t GetChecksum( void ) const {
		CRC32_t crc;
		CRC32_Init( &crc );
		CRC32_ProcessBuffer( &crc, &m_command_number, sizeof( m_command_number ) );
		CRC32_ProcessBuffer( &crc, &m_tick_count, sizeof( m_tick_count ) );
		CRC32_ProcessBuffer( &crc, &m_viewangles, sizeof( m_viewangles ) );
		CRC32_ProcessBuffer( &crc, &m_aimdirection, sizeof( m_aimdirection ) );
		CRC32_ProcessBuffer( &crc, &m_forwardmove, sizeof( m_forwardmove ) );
		CRC32_ProcessBuffer( &crc, &m_sidemove, sizeof( m_sidemove ) );
		CRC32_ProcessBuffer( &crc, &m_upmove, sizeof( m_upmove ) );
		CRC32_ProcessBuffer( &crc, &m_buttons, sizeof( m_buttons ) );
		CRC32_ProcessBuffer( &crc, &m_impulse, sizeof( m_impulse ) );
		CRC32_ProcessBuffer( &crc, &m_weaponselect, sizeof( m_weaponselect ) );
		CRC32_ProcessBuffer( &crc, &m_weaponsubtype, sizeof( m_weaponsubtype ) );
		CRC32_ProcessBuffer( &crc, &m_randomseed, sizeof( m_randomseed ) );
		CRC32_ProcessBuffer( &crc, &m_mousedx, sizeof( m_mousedx ) );
		CRC32_ProcessBuffer( &crc, &m_mousedy, sizeof( m_mousedy ) );
		CRC32_Final( &crc );
		return crc;
	}
};
class verified_cmd_t {
public:
	cmd_t m_cmd;
	unsigned long m_crc;
};

class i_input {
public:
	void *vtable;
	// 0x00
	bool              m_trackir;					// 0x04
	bool              m_mouse_init;					// 0x05
	bool              m_mouse_active;				// 0x06
	bool              m_joystick_adv_init;			// 0x07
	char p[ 0x2C ];									// 0x08
	void *m_keys;						// 0x34
	char p_1[ 0x6C ];									// 0x38
	bool              m_camera_intercepting_mouse;	// 0x9C
	bool              m_camera_in_third_person;		// 0x9D
	bool              m_camera_moving_with_mouse;	// 0x9E
	vec3_t            m_camera_offset;				// 0xA0
	bool              m_camera_distance_move;		// 0xAC
	int               m_camera_old_x;				// 0xB0
	int               m_camera_old_y;				// 0xB4
	int               m_camera_x;					// 0xB8
	int               m_camera_y;					// 0xBC
	bool              m_camera_is_orthographic;		// 0xC0
	vec3_t             m_previous_view_angles;		// 0xC4
	vec3_t             m_previous_view_angles_tilt;	// 0xD0
	float             m_last_forward_move;			// 0xDC
	int               m_clear_input_state;			// 0xE0
	cmd_t *m_commands;					// 0xEC
	void *m_verified;					// 0xF0
public:
	vec3_t camera_offset;

	VFUNC( get_user_cmd( int slot, int sequence_num ), 8, cmd_t *( __thiscall * )( decltype( this ), int, int ), slot, sequence_num );
	VFUNC( camera_to_third_person( ), 35, void( __thiscall * )( decltype( this ) ) )
	VFUNC( camera_to_first_person( ), 36, void( __thiscall * )( decltype( this ) ) )
	VFUNC( camera_is_third_person( int slot = -1 ), 32, bool( __thiscall * )( decltype( this ), int ) , slot )
};