#pragma once
#include "utils.h"

enum CSWeaponType : int {
	WEAPONTYPE_UNKNOWN = -1,
	WEAPONTYPE_KNIFE,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_TASER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_HEALTHSHOT = 11
};

class weapon_info_t {
private:
	PAD( 0x4 );											// 0x0000

public:
	const char *m_weapon_name;						// 0x0004 -- actual weapon name, even for usp-s and revolver. ex: "weapon_revolver"
	PAD( 0xC );												// 0x0008
	int               m_max_clip1;							// 0x0014
	int				  m_max_clip2;							// 0x0018
	int				  m_default_clip1;						// 0x001C
	int		          m_default_clip2;						// 0x0020
	int               m_max_reserve;						// 0x0024
	PAD( 0x4 );												// 0x0028
	const char *m_world_model;						// 0x002C
	const char *m_view_model;							// 0x0030
	const char *m_world_dropped_model;				// 0x0034
	PAD( 0x48 );											// 0x0038
	const char *m_ammo_type;							// 0x0080
	uint8_t           pad_0084[ 4 ];						// 0x0084
	const char *m_sfui_name;							// 0x0088
	const char *m_deprecated_weapon_name;				// 0x008C -- shitty weapon name, shows "weapon_deagle" for revolver / etc.
	uint8_t           pad_0090[ 56 ];						// 0x0090
	CSWeaponType      m_weapon_type;						// 0x00C8
	int			      m_in_game_price;						// 0x00CC
	int               m_kill_award;							// 0x00D0
	const char *m_animation_prefix;					// 0x00D4
	float			  m_cycletime;							// 0x00D8
	float			  m_cycletime_alt;						// 0x00DC
	float			  m_time_to_idle;						// 0x00E0
	float			  m_idle_interval;						// 0x00E4
	bool			  m_is_full_auto;						// 0x00E5
	PAD( 0x3 );												// 0x00E8
	int               m_damage;								// 0x00EC
	float             m_armor_ratio;						// 0x00F0
	int               m_bullets;							// 0x00F4
	float             m_penetration;						// 0x00F8
	float             m_flinch_velocity_modifier_large;		// 0x00FC
	float             m_flinch_velocity_modifier_small;		// 0x0100
	float             m_range;								// 0x0104
	float             m_range_modifier;						// 0x0108
	float			  m_throw_velocity;						// 0x010C
	PAD( 0xC );												// 0x0118
	bool			  m_has_silencer;						// 0x0119
	PAD( 0x3 );												// 0x011C
	const char *m_silencer_model;						// 0x0120
	int				  m_crosshair_min_distance;				// 0x0124
	int				  m_crosshair_delta_distance;			// 0x0128
	float             m_max_player_speed;					// 0x012C
	float             m_max_player_speed_alt;				// 0x0130
	float			  m_spread;								// 0x0134
	float			  m_spread_alt;							// 0x0138
	float             m_inaccuracy_crouch;					// 0x013C
	float             m_inaccuracy_crouch_alt;				// 0x0140
	float             m_inaccuracy_stand;					// 0x0144
	float             m_inaccuracy_stand_alt;				// 0x0148
	float             m_inaccuracy_jump_initial;			// 0x014C
	float             m_inaccuracy_jump;					// 0x0150
	float             m_inaccuracy_jump_alt;				// 0x0154
	float             m_inaccuracy_land;					// 0x0158
	float             m_inaccuracy_land_alt;				// 0x015C
	float             m_inaccuracy_ladder;					// 0x0160
	float             m_inaccuracy_ladder_alt;				// 0x0164
	float             m_inaccuracy_fire;					// 0x0168
	float             m_inaccuracy_fire_alt;				// 0x016C
	float             m_inaccuracy_move;					// 0x0170
	float             m_inaccuracy_move_alt;				// 0x0174
	float             m_inaccuracy_reload;					// 0x0178
	int               m_recoil_seed;						// 0x017C
	float			  m_recoil_angle;						// 0x0180
	float             m_recoil_angle_alt;					// 0x0184
	float             m_recoil_angle_variance;				// 0x0188
	float             m_recoil_angle_variance_alt;			// 0x018C
	float             m_recoil_magnitude;					// 0x0190
	float             m_recoil_magnitude_alt;				// 0x0194
	float             m_recoil_magnitude_variance;			// 0x0198
	float             m_recoil_magnitude_variance_alt;		// 0x019C
	float             m_recovery_time_crouch;				// 0x01A0
	float             m_recovery_time_stand;				// 0x01A4
	float             m_recovery_time_crouch_final;			// 0x01A8
	float             m_recovery_time_stand_final;			// 0x01AC
	float             m_recovery_transition_start_bullet;	// 0x01B0
	float             m_recovery_transition_end_bullet;		// 0x01B4
	bool			  m_unzoom_after_shot;					// 0x01B5
	PAD( 0x3 );												// 0x01B8
	bool		      m_hide_view_model_zoomed;				// 0x01B9
	bool			  m_zoom_levels;						// 0x01BA
	PAD( 0x2 );												// 0x01BC
	int				  m_zoom_fov[ 2 ];						// 0x01C4
	float			  m_zoom_time[ 3 ];						// 0x01D0
	PAD( 0x8 );												// 0x01D8
	float             m_addon_scale;						// 0x01DC
	PAD( 0x8 );												// 0x01E4
	int				  m_tracer_frequency;					// 0x01E8
	int				  m_tracer_frequency_alt;				// 0x01EC
	PAD( 0x18 );											// 0x0200
	int				  m_health_per_shot;					// 0x0204
	PAD( 0x8 );												// 0x020C
	float			  m_inaccuracy_pitch_shift;				// 0x0210
	float			  m_inaccuracy_alt_sound_threshold;		// 0x0214
	float			  m_bot_audible_range;					// 0x0218
	PAD( 0x8 );												// 0x0220
	const char *m_wrong_team_msg;						// 0x0224
	bool			  m_has_burst_mode;						// 0x0225
	PAD( 0x3 );												// 0x0228
	bool			  m_is_revolver;						// 0x0229
	bool			  m_can_shoot_underwater;				// 0x022A
	PAD( 0x2 );
};
