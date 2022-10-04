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
	__forceinline void store( player_t *player ) {
		// get bone cache ptr.
		bone_cache_t *cache = &player->bone_cache( );

		// store bone data.
		m_bones = cache->m_pCachedBones;
		m_bone_count = cache->m_CachedBoneCount;
		m_origin = player->origin( );
		m_mins = player->mins(  );
		m_maxs = player->maxs(  );
		m_abs_origin = player->abs_origin( );
		m_abs_ang = player->abs_angles( );
	}

	__forceinline void restore( player_t *player ) const {
		// get bone cache ptr.
		bone_cache_t *cache = &player->bone_cache( );

		cache->m_pCachedBones = m_bones;
		cache->m_CachedBoneCount = m_bone_count;

		player->origin( ) = m_origin;
		player->mins(  ) = m_mins;
		player->maxs(  ) = m_maxs;
		player->set_abs_angles( m_abs_ang );
		player->set_abs_origin( m_abs_origin );
	}
};
struct player_record_t {
	player_record_t( ) = default ;
	bone_array_t *m_bones = nullptr;
	player_t *m_ent = nullptr;
	ent_info_t *m_info = nullptr;
	
	bool m_setup = false;
	bool m_ukn_vel = false;
	int m_tick{};
	int m_lag = 0;
	bool m_dormant = false;
	float m_duck_speed = 0.f;
	float m_sim_time = 0.0f;
	float m_old_sim_time = 0.0f;
	float m_anim_time = 0.0f;
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
	ang_t m_stand1_abs_angles[11];
	ang_t m_stand2_abs_angles[11];
	ang_t m_lby_abs_angles[2];
	float m_stand1_angles[11]{};
	float m_stand2_angles[11]{};

	float m_stand1_poses[11][24]{};
	float m_stand2_poses[11][24]{};
	float m_lby_poses[2][24]{};
	int m_mode = 0;
	bool m_body_update = false;
	float m_base_angle{};
	bone_array_t ** m_fake_bones = {};
	
	~player_record_t ( );

	void cache ( int index = -1 ) const;

	bool valid ( ) const;
	explicit player_record_t ( ent_info_t *info, float last_sim );
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

struct ent_info_t {
	ent_info_t( ): m_missed_shots( 0 ), m_body_index( 0 ), m_stand_index( 0 ), m_stand2_index( 0 ), m_moved( false ),
	               m_old_body( 0 ) {};

	std::deque< std::shared_ptr< player_record_t > > m_records = {};
	std::shared_ptr<player_record_t> m_selected_record = nullptr;

	bool m_manual_update = true;
	float m_damage = 0;
	anim_state m_stand1_state[10];
	anim_state m_stand2_state[10];
	anim_state m_lby_state[2];
	vec3_t m_shoot_pos = vec3_t();
	using helper_array = std::vector< hitbox_helper_t >;
	helper_array m_hitboxes = {};
	player_t *m_ent = nullptr;
	bool m_fake_player = false;
	CIKContext m_ik;
	aim_point_t* m_aim_point{};
	int m_shots = 0;
	int m_missed_shots;
	int m_body_index;
	int m_stand_index;
	int m_stand2_index;
	player_record_t m_walk_record;
	bool m_moved{};
	float m_old_body{};
	float m_shot_wanted{};
	bool m_possible_stand2_indexs[ 11 ] = { true,true,true, true,true,true,true,  true,true,true, true };
	bool m_stand2_backup_indexs[ 11 ] = {true, true, true , true,true,true,true,  true,true,true, true };
	int m_backup_stand_index2 = 0;
	bool m_possible_stand_indexs[11] = { true, true, true, true,true,true,true, true,true,true, true };
	bool m_stand_backup_indexs[ 11 ] = { true, true, true , true,true,true,true, true,true,true, true };
	int m_backup_stand_index = 0;
	__forceinline player_t *operator->( ) const { return m_ent; }
	
	bool m_valid = false;
	bool m_teamate = false;
	bool m_setup = false;
	float m_spawn = -1.f;
	bool m_body_update = false;
	float m_body_update_time = -1;
	float m_last_body_update = 0;
	float m_body = 0;

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

