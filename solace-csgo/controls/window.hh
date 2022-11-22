#pragma once
#include <memory>
#include <vector>

#include <iostream>
#include <fstream>

#include "base_control.hh"

class c_form : public c_base_control {
	const char* name_{ "Test" };
	bool should_drag_{ false }, clicked_{ false }, enabled_{ true };
	vector_2d drag_offset_{ 0, 0 };
	area_t area_{ 50, 50, 640, 480 };
	std::vector<std::shared_ptr<c_base_control>> children_{ };
	c_base_control *selected_tab = nullptr;
public:
	c_form( ) {
	}
	
	c_form( const char* name, const area_t area ) {
		this->name_ = name;
		this->area_ = area;
	}

	auto draw ( ) -> void override;

	auto update ( ) -> void override;

	auto m_enabled( ) const -> const bool {
		return this->enabled_;
	}

	auto set_enabled( bool enabled ) -> void {
		this->enabled_ = enabled;
	}

	auto add_child( std::shared_ptr<c_base_control> control ) -> void {
		if ( !selected_tab )
			selected_tab = control.get( );
		this->children_.push_back( control );
	}

	static auto finish( ) -> void {
	}
	void save() override {
		for (auto i : children_) {
			i->save();
		}
	}
	void load() override {
		for (auto i : children_) {
			i->load();
		}
	}
};
