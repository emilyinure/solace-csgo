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
	bool open = true;
	color main_theme = color( 0x8A, 0x86, 0xA6, 0xFF );
	color bright = color( 0xBF,0xBE,0xBD, 0xFF );
	color bright_accent = color( 209, 176, 194, 0xFF );
	color dark = color( 0x1B, 0x17, 0x26, 0xFF );
	color dark_accent = color( 0x8A, 0x86, 0xA6, 0xFF );
	auto init( ) -> void;
	auto draw( ) -> void;
	auto update( ) -> void;
	void append_bind ( key_bind_t * value );

	std::vector< key_bind_t * > n_binds = {};
	
	c_base_control* focused_control = nullptr;
	c_form* focused_form = nullptr;
	std::vector<std::shared_ptr<c_form>> m_forms = {};


	void save();
	void load();
	// simple render wrapper so it'll be easy to port to opengl / surface.
}; inline c_menu menu;