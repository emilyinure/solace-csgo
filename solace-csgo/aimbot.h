#pragma once
#include "includes.h"



class aimbot_t {
public:

	struct hitbox_t {
		int m_bone = 0;
		bool m_box = false;
		vec3_t m_center = vec3_t( );
		vec3_t m_min = vec3_t( ), m_max = vec3_t();
		ang_t m_rot = ang_t();
		float m_radius = 0;
		hitbox_t( vec3_t min, vec3_t max, float radius ) : m_min( min ), m_max( max ), m_radius( radius ) { m_box = false; };
		hitbox_t( int bone, vec3_t center, vec3_t min, vec3_t max, ang_t rot ) : m_bone( bone ), m_center( center ), m_min( min ), m_max( max ), m_rot( rot ) { m_box = true; };
	};
	struct saved_eye_pos {
		saved_eye_pos( float& pitch, vec3_t& eye ) : pitch( pitch ), eye( eye ) {}
		float pitch;
		vec3_t eye;
	};
	std::vector < saved_eye_pos > m_list_eye_pos = {};
	std::vector< ent_info_t * > m_targets;
	backup_record_t m_backup[64] = {};
	ent_info_t *m_best_target = nullptr;
	ent_info_t * m_last_target = nullptr;
	vec3_t m_shoot_pos;
	static bool valid ( ent_info_t *ent );
	void get_targets ( );
	static void get_points( ent_info_t *info, studio_hdr_t *studio_model );
	static bool collides( math::custom_ray_t ray, ent_info_t* info, bone_array_t bones[ 128 ], float add = 0.f );
	static bool get_aim_matrix ( ang_t angle, bone_array_t *ret );
	bool get_best_point( ent_info_t* info, vec3_t &eye);
	bool best_target ( ent_info_t *&info ) const;
	void apply (bone_array_t* bones);
	static player_record_t *best_record( ent_info_t *info );
	void backup_players ( bool restore );
	std::vector<ent_info_t *> mp_threading ( );
	static std::shared_ptr<player_record_t> last_record( ent_info_t *info );
	void draw_hitboxes ( player_t* ent = nullptr, bone_array_t *bones = nullptr ) const;
	bool check_hitchance( ent_info_t* info, ang_t& view, std::shared_ptr<player_record_t> record, aim_point_t* point );
    bool autoscope();
	void StripAttack( );
	virtual void on_tick ( );
} inline g_aimbot;


