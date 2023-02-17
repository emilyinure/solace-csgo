#pragma once
#include <deque>

#include "vec3.h"
#include "Windows.h"

class datamap_t;
class player_t;

class managed_netvar {
public:
	uintptr_t m_offset = 0;
	virtual void pre_update( player_t *player ) = 0;
	virtual void post_update( player_t *player ) = 0;
};
class shared_netvar : public managed_netvar {
public:
	float m_value;
	float m_old_value;
	float m_tolerance;
	char *m_name;
	shared_netvar(uintptr_t offset, float tolerance, const char * name) : m_tolerance(tolerance) {
		m_offset = offset;
		m_name = _strdup(name);
	}

	void pre_update( player_t *player ) override;
	void post_update( player_t *player ) override;
};
class managed_vec : public managed_netvar {
public:
	vec3_t m_value;
	vec3_t m_old_value;
	float m_tolerance = 0;
	char *m_name;
	bool m_coord;
	managed_vec( uintptr_t offset, float tolerance, const char *name, bool coord = false ) : m_tolerance( tolerance ), m_coord(coord) {
		m_offset = offset;
		m_name = _strdup( name );
	}

	void pre_update( player_t *player ) override;
	void post_update( player_t *player ) override;
};

class prediction_netvar_manager {
public:
    ~prediction_netvar_manager();
    bool initalized = false;
	bool setup_vars = false;
	std::vector<std::unique_ptr<managed_netvar>> vars;
	std::vector<managed_netvar*> weapon_vars;
    bool called_once = false;
    void reset()
    {
        called_once = false;
	}
	void pre_update( player_t * );
	void post_update( player_t * );
	void init ( datamap_t * map );
} inline g_pred_manager;

