#pragma once
#include "includes.h"

class c_entity_list {
public:
	VFUNC( get_client_entity( int index ), 3, void *( __thiscall * )( decltype( this ), int ), index )
	VFUNC( get_client_entity_handle( uintptr_t handle ), 4, void *( __thiscall * )( decltype( this ), uintptr_t ), handle )
	VFUNC( get_highest_index( ), 6, int( __thiscall * )( decltype( this ) ) )
};
