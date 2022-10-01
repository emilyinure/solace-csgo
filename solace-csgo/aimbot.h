#pragma once
#include "includes.h"
#include "ray_tracer.h"



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
	std::vector < std::pair<float, vec3_t>> m_list_eye_pos = {};
	std::vector< ent_info_t * > m_targets;
	backup_record_t m_backup[64] = {};
	ent_info_t *m_best_target = nullptr;
	ent_info_t * m_last_target = nullptr;
	vec3_t m_shoot_pos;
	static bool valid ( ent_info_t *ent );
	void get_targets ( );
	static void get_points( ent_info_t *info, studio_hdr_t *studio_model );
	static bool collides( math::custom_ray ray, ent_info_t* info, bone_array_t bones[ 128 ], float add = 0.f );
	static bool get_aim_matrix ( ang_t angle, bone_array_t *ret );
	static bool get_best_point ( ent_info_t *info, bone_array_t *bones );
	bool best_target ( ent_info_t *&info ) const;
	void apply ( ) const;
	static player_record_t *best_record( ent_info_t *info );
	void backup_players ( bool restore );
	void add_to_threads( ent_info_t* info, int id );
	std::vector<ent_info_t *> mp_threading ( ) const;
	static std::shared_ptr<player_record_t> last_record( ent_info_t *info );
	void draw_hitboxes ( player_t* ent = nullptr, bone_array_t *bones = nullptr ) const;
	static bool check_hitchance(ent_info_t* info, ang_t& view, std::shared_ptr<player_record_t> record, aim_point_t& point);
	virtual void on_tick ( );
} inline g_aimbot;


