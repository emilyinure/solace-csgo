#pragma once
#include <cstdint>

#include "block_bot.h"
struct model_render_info_t;
class player_t;
struct matrix_t;
class i_material;
class chams_t {
	i_material *flat = nullptr;
	i_material *full = nullptr;
	std::vector< player_t * > m_players;
public:
	bool override_model( int i );
	void create_materials( );
	static void reset ( );
	bool IsInViewPlane ( const vec3_t &world );
	bool SortPlayers ( );
	void SceneEnd ( );
	void player ( player_t *player, uintptr_t ctx, void *state, const model_render_info_t &info, matrix_t *bone );
} inline g_chams;

