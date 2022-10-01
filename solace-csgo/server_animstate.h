#pragma once

class player_t;
class ang_t;

class server_anim_state {
public:
	void update(ang_t ang);
	void reset();
	void init(player_t* ent);
};

extern server_anim_state* CreateCSGOPlayerAnimstate(player_t* pEntity);