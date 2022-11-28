#pragma once
#include <memory>
#include <array>
#include <deque>

#include "includes.h"
#include "animstate.h"

struct ent_info_t;
class backup_record_t {
public:
	bone_array_t *m_bones;
	int        m_bone_count;
	vec3_t     m_origin, m_abs_origin;
	vec3_t     m_mins;
	vec3_t     m_maxs;
	ang_t      m_abs_ang;

public:
	void store( player_t* player );

	void restore( player_t* player ) const;
};
struct player_record_t {
	player_record_t( ) = default ;
	bone_array_t *m_bones = nullptr;
	player_t *m_ent = nullptr;
	ent_info_t* m_info = nullptr;
	
	bool m_setup = false;
	bool m_ukn_vel = false;
	int m_tick{};
	int m_lag = 0;
	bool m_dormant = false;
	float m_duck_speed = 0.f;
	float m_sim_time = 0.0f;
	float m_old_sim_time = 0.0f;
	float m_anim_time = 0.0f;
	float m_cycle = 0.f;
	int m_sequence = 0;
	animation_layer_t m_layers[ 15 ]{};
	vec3_t           m_origin, m_abs_origin;
	vec3_t           m_mins, m_maxs;
	vec3_t           m_velocity;
	int              m_flags{};
	float            m_duck{}, m_body{};
	vec3_t m_anim_velocity;
	ang_t m_eye_angles, m_abs_angles;
	float            m_poses[ 24 ]{};
	bone_array_t * m_render_bones = nullptr;
	bool m_fake_walk = false;
	bool m_body_reliable = false;
	
	struct resolver_data {
		struct dir_data {
			dir_data( float angle ) {
				angles = angle;
			}
			ang_t m_abs_angles{};
			float angles{};
			float poses[ 24 ]{};
		};
		int m_index = 0;
		std::vector< dir_data > m_dir_data;
	} m_resolver_data;

	int m_mode = 0;
	bool m_body_update = false;
	float m_base_angle{};
	std::vector<bone_array_t*> m_fake_bones = {};
	
	~player_record_t ( );

	void cache( int index = -1 ) const;

	bool valid ( ) const;
	player_record_t( ent_info_t * info, float last_sim );
	// lagfix stuff.
	bool   m_broke_lc = false;
	vec3_t m_pred_origin;
	vec3_t m_pred_velocity;
	float  m_pred_time = 0.0f;
	int    m_pred_flags = 0;

	enum aim_flags {
		NONE,
		SHOT,
		PRIORITY
	};
	aim_flags m_aim_flags = NONE;
	bool m_shot = false;
};

enum hitscan_mode : int {
	prefer_safe_point = (1 << 0),
	lethal = ( 1 << 1 ),
	prefer = ( 1 << 2 )
};

struct aim_point_t {
	vec3_t m_point = vec3_t();
	int m_mode = 0;
	int m_hitbox = 0;
	vec3_t m_shoot_pos = vec3_t( );
	aim_point_t( ) = default;
	aim_point_t( vec3_t point, int mode, int hitbox ) : m_point(point), m_mode( mode ), m_hitbox(hitbox) {}
};

struct hitbox_helper_t {
	int         m_index;
	using point_helper = std::vector< aim_point_t >;
	point_helper m_points = {};
	studio_hdr_t* hdr;
	ent_info_t* info;
};


struct resolver_data {
	resolver_data( ) {};
	enum modes : int {
		STAND1,
		STAND2,
		LBY_MOVING
	};
	struct mode_data {
		struct dir_data {
			dir_data( bool dir_enabled, bool backup_enabled ) : dir_enabled( dir_enabled ), backup_enabled( backup_enabled ) {};
			bool dir_enabled = true;
			bool backup_enabled = true;
		};
		std::vector< dir_data > m_dir_data = {};
		int m_index = 0;
	} m_mode_data[ 3 ];
	std::vector<anim_state> m_states = {};

	void init( ) {
		m_missed_shots = 0;
		m_body_update = false;
		m_body_update_time = -1.f;
		m_last_body_update = 0.f;
		m_body = 0;
		m_moved = false;
		m_shots = 0;
		m_old_body = 0.f;
		float m_old_body = 0.f;
		for ( auto i = 0; i < 3; i++ ) {
			auto& mode_data = m_mode_data[ i ];

			mode_data.m_index = 0;
			mode_data.m_dir_data.clear( );
			if ( i == 0 ) {
				for ( auto k = 0; k < 7; k++ ) {
					mode_data.m_dir_data.emplace_back( true, true );
				}
			}
			else if ( i == 1 ) {
				for ( auto k = 0; k < 6; k++ ) {
					mode_data.m_dir_data.emplace_back( true, true );
				}
			}
			else for ( auto k = 0; k < 3; k++ ) {
				mode_data.m_dir_data.emplace_back( true, true );
			}
		}
		m_states.clear( );
		for ( auto k = 0; k < 11; k++ ) {
			m_states.emplace_back( );
		}
	}

	int m_missed_shots = 0;
	bool m_body_update = false;
	float m_body_update_time = -1.f;
	float m_last_body_update = 0.f;
	float m_body = 0;
	bool m_moved = false;
	int m_shots = 0;
	float m_old_body = 0.f;
};

struct ent_info_t {
	ent_info_t( ) {};

	std::deque< std::shared_ptr< player_record_t > > m_records = {};
	std::shared_ptr<player_record_t> m_selected_record = nullptr;

	bool m_manual_update = true;
	float m_damage = 0;
	vec3_t m_shoot_pos = vec3_t();
	using helper_array = std::vector< hitbox_helper_t >;
	helper_array m_hitboxes = {};
	player_t *m_ent = nullptr;
	bool m_fake_player = false;
	CIKContext m_ik;
	aim_point_t* m_aim_point{};
	player_record_t m_walk_record;
	float m_shot_wanted{};

	
	resolver_data m_resolver_data;
	bool build_fake_bones( std::shared_ptr<player_record_t> current );

	__forceinline player_t *operator->( ) const { return m_ent; }
	
	bool m_valid = false;
	bool m_teamate = false;
	bool m_setup = false;
	float m_spawn = -1.f;

	bool m_dormant = false;
	bone_array_t *m_used_bones = nullptr;

	float test_velocity(std::shared_ptr<player_record_t> record, float speed, float pVec);

	float solve_velocity_len(std::shared_ptr<player_record_t> record);

	void UpdateAnimations (std::shared_ptr<player_record_t> record );
	void update( player_t *ent );
};
struct animation_backup_t {
	vec3_t           m_origin, m_abs_origin;
	vec3_t           m_velocity, m_abs_velocity;
	int              m_flags{}, m_eflags{};
	float            m_duck{}, m_body{};
	animation_layer_t m_layers[ 15 ]{};
	float m_poses[24]{};
};
class player_manager_t {
public:
	player_manager_t( ) {
		
	}
	std::array< ent_info_t , 64 > m_ents{};
	bool m_animating = false;
	void update( );
} inline g_player_manager;

