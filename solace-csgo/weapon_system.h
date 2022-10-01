#pragma once
#include "includes.h"
class weapon_info_t;

class i_weapon_system {
	virtual void unused0( ) = 0;
	virtual void unused1( ) = 0;
public:
	[[nodiscard]] virtual weapon_info_t *get_weapon_data( unsigned int idx ) const = 0;
};