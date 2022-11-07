#pragma once
#include <iostream>
#include <fstream>

class c_toggle : public c_base_control {
	bool* enabled_;
	area_t button_area{0,0,0,0};
public:
	c_toggle( const char* name, bool* enabled ) {
		this->name = name;
		this->enabled_ = enabled;
		this->type = control_type_toggle;
	}

	auto draw( ) -> void override {
		button_area = { this->area.x + this->area.w - (this->area.h), this->area.y, this->area.h, this->area.h };
		if ( *this->enabled_ )
			g.m_render->filled_rect( button_area.x + 1, button_area.y+1, button_area.h - 2, button_area.h-2, menu.main_theme );
		g.m_render->outlined_rect( button_area.x, button_area.y, button_area.h, button_area.h, { 240,240,240, 14 } );
		auto text_height = g.m_render->get_text_height( this->name, g.m_render->m_constantia_12( ) );
		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x, this->area.y + (area.h / 2) - text_height/2, menu.bright, this->name );
	}

	auto update( ) -> void override {
		if ( input_helper.hovering( button_area ) && input_helper.key_pressed( VK_LBUTTON ) ) {
			input_helper.set_key( VK_LBUTTON, false );
			*this->enabled_ = !*this->enabled_;
		}
	}

	void save() override {
		std::cout << *enabled_ << "\n";
	}
	void load()  override {
		std::string line;
		std::getline(std::cin, line);
		std::istringstream(line) >> *enabled_;
	}
};