#pragma once
#include <map>
#include <string>
#include <vector>

#include "includes.h"

namespace events {
	void round_start( IGameEvent *evt );
	void init( );
	void destroy( );
	void player_hurt ( IGameEvent *evt );
	void bullet_impact ( IGameEvent *evt );
	void round_end( IGameEvent *evt );
}