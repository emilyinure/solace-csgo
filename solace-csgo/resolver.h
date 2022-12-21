#pragma once
#include <deque>

#include "array"
#include "vec3.h"
#include "player_manager.h"


class IGameEvent;
struct player_record_t;
struct ent_info_t;
class player_t;
class weapon_info_t;
struct shot_record_t {
	~shot_record_t( );
	player_t *m_target{};
	std::weak_ptr<player_record_t> m_record;
	float m_time{};
	float m_pred_time{};
	float m_lat{};
	float m_damage{};
	vec3_t m_pos;
	bool m_matched = false;
	weapon_info_t m_weapon_info;
	bool m_updated_time;
	int m_tick;
};

struct impact_record_t {
    std::weak_ptr<shot_record_t> m_shot;
	int m_tick;
	vec3_t m_pos;
	int m_index;
	bool m_indexs[8] = {};
	bool m_resolved = false;
	bool m_hit;
	int m_group = 0;
};

class hit_record_t {
public:
	__forceinline hit_record_t( ) : m_impact{}, m_group{ -1 }, m_damage{} {}

public:
	impact_record_t *m_impact;
	int           m_group;
	float         m_damage;
};


class resolver {
public:
	enum class trace_ret {
		hit,
		spread,
		occlusion
	};
	std::array< std::string, 8 > m_groups = {
		( "body" ),
		( "head" ),
		( "chest" ),
		( "stomach" ),
		( "left arm" ),
		( "right arm" ),
		( "left leg" ),
		( "right leg" )
	};
	enum Modes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_WALK,
		RESOLVE_STAND,
		RESOLVE_STAND1,
		RESOLVE_STAND2,
		RESOLVE_AIR,
		RESOLVE_BODY,
		RESOLVE_STOPPED_MOVING,
	};
	std::deque<std::shared_ptr<shot_record_t>> m_shots;
	std::deque<impact_record_t> m_impacts;
	std::deque<hit_record_t> m_hits;
	static std::shared_ptr<player_record_t> FindIdealRecord ( ent_info_t *data );
	std::shared_ptr<player_record_t> FindIdealBackRecord( ent_info_t* data );
	static void MatchShot ( ent_info_t *data, std::shared_ptr<player_record_t> record );
	void update_shot_timing ( int sent_tick );
	static void OnBodyUpdate ( ent_info_t *player, float value );
	static void SetMode (std::shared_ptr<player_record_t> record );
	static void ResolveWalk ( ent_info_t *data, std::shared_ptr<player_record_t> record );
	static float GetAwayAngle (std::shared_ptr<player_record_t> record );
	void ResolveStand ( ent_info_t *data, std::shared_ptr<player_record_t> record ) const;
	void ResolveAir ( ent_info_t *data, std::shared_ptr<player_record_t> record ) const;
	void resolve( ent_info_t &info, std::shared_ptr<player_record_t> record ) const;
	void clear ( );
	static float get_rel (std::shared_ptr<player_record_t> record, int index );
	float get_freestand_yaw ( player_t *target ) const;
	void recieve_shot ( vec3_t point, int hit );
	void on_impact ( IGameEvent *evt );
	void OnHurt ( IGameEvent *evt );
	int miss_scan_boxes_and_eliminate( impact_record_t* impact, vec3_t& start, vec3_t& end );
	int hit_scan_boxes_and_eliminate( impact_record_t* impact, vec3_t& start, vec3_t& end ) const;
	void resolve_hit ( impact_record_t *impact ) const;
	void resolve_miss ( impact_record_t *impact );
	void update_shots ( );
	void add_shot( ent_info_t* target, float damage, int bullets, std::shared_ptr<player_record_t> record );
} inline g_resolver;

