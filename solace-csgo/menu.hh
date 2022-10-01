#pragma once
#include <algorithm>
#include <memory>
#include <vector>

#include "render.h"

enum key_bind_type_t {
	key_bind_type_always = 0,
	key_bind_type_hold,
	key_bind_type_toggle,
	key_bind_type_off_key
};

struct key_bind_t {
	key_bind_t(){}
	key_bind_t( const char *name, bool *value, int default_type ) {
		this->name = name;
		enabled = value;
		type = default_type;
	}
	std::vector<const char *> key_bind_types_{ "Always", "Hold", "Toggle", "Off Key" };
	void update ( );
	const char *name = "";
	int key = 0;
	int type = key_bind_type_always;
	bool *enabled = nullptr;
};

class c_form;
class c_base_control;
class vector_2d {
public:
	float x = 0, y = 0;
	vector_2d( float x, float y ) : x( x ), y( y ){}
};
class c_menu {
public:
	auto init( ) -> void;
	auto draw( ) const -> void;
	auto update( ) const -> void;
	void append_bind ( key_bind_t * value );

	std::vector< key_bind_t * > n_binds = {};
	
	c_base_control* focused_control;
	c_form* focused_form;
	std::vector<std::shared_ptr<c_form>> m_forms;


	void save();
	void load();
	// simple render wrapper so it'll be easy to port to opengl / surface.
}; inline c_menu menu;