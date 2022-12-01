#pragma once
#include "windows.h"
class area_t;
class player_t;
class entity_t;
class esp_t {
public:
	static void run ( );
	static void smoke( entity_t* ent );
	static void flying_grenade ( entity_t *ent );
	static void molotov(entity_t* ent);
	static void inferno( entity_t* ent );
	static void NoSmoke ( );
	static bool get_player_box( player_t *player, area_t *box );
	static void offscreen ( player_t *ent );
	static void player ( player_t *ent );
	static void weapon( entity_t *ent );
} inline g_esp;

