#pragma once
#include <vector>

#include "console.h"
class vec3_t;
class ent_info_t;
class block_bot_t {
	ent_info_t *target = nullptr;
	std::vector<vec3_t> m_draw;

public:
	void on_draw ( ) const;
	void friction ( float surface_friction, vec3_t *velocity );
	void on_tick( );
} inline g_block_bot;

