#pragma once
#include "../includes.h"
#include "../menu.hh"

enum e_control_type {
	control_type_invalid = -1,
	control_type_toggle,
	control_type_slider,
	control_type_button,
	control_type_combobox,
	control_type_key_bind,
	control_type_tab
};

class c_base_control {
public:
	virtual ~c_base_control ( ) = default;
	virtual auto draw( ) -> void = 0;
	virtual auto update( ) -> void = 0;
	virtual auto disable( ) -> void {}
	virtual void save() = 0;
	virtual void load() = 0;

	const char* name{ "c_base_control" };
	area_t area{ 0, 0, 0, 0 }, original_area{ 0, 0, 0, 0 };
	e_control_type type{ control_type_invalid };

	auto adjust_area( const area_t area ) -> void {
		this->area = original_area;
		this->area = area;
	}

	auto adjust_position( const vector_2d pos ) {
		this->area.x += pos.x;
		this->area.y += pos.y;
	}
};