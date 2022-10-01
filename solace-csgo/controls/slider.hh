#pragma once
#include <iostream>
#include <fstream>

class c_slider : public c_base_control {
	float* value_;
	float minimum_{ 0.f }, maximum_{ 100.f };
	bool dragging_{ false };
	area_t button_area_{ 0,0,0,0 };
public:
	c_slider( const char* name, float* value, float minimum = 0.f, float maximum = 100.f ) {
		this->name = name;
		this->value_ = value;
		this->minimum_ = minimum;
		this->maximum_ = maximum;
		this->type = control_type_slider;
	}

	auto disable( ) -> void override {
		this->dragging_ = false;
	}
	
	auto draw( ) -> void override {
		button_area_ = { this->area.x, this->area.y + ( this->area.h / 2 ) + ( this->area.h / 2 ) / 4.f, this->area.w, ( ( this->area.h / 2 ) / 2.f ) };
		const auto slider_fill{ ( ( *this->value_ - this->minimum_ ) / ( this->maximum_ - this->minimum_ ) * ( button_area_.w-2.f ) ) };
		
		auto text_height = g.m_render->get_text_height( this->name, g.m_render->m_constantia_12( ) );

		g.m_render->filled_rect( button_area_.x + 1, button_area_.y + 1, slider_fill, button_area_.h - 2, { 0xDB, 0x2E, 0x2C, 100 } );
		g.m_render->outlined_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h, { 240,240,240, 7 } );

		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x, this->area.y + this->area.h/4 - text_height/2, { 240,240,240, 100 }, this->name );

		// display.
		char display[ 32 ];
		sprintf_s( display, "%.1f / %.1f", *this->value_, this->maximum_ );
		const auto text_size = g.m_render->get_text_width( display, g.m_render->m_constantia_12( ) );

		g.m_render->text( g.m_render->m_constantia_12( ), button_area_.x + button_area_.w - ( text_size ), this->area.y + this->area.h/4 - text_height / 2, { 240,240,240, 100 }, display );
	}

	auto update( ) -> void override {
		if ( menu.focused_control && menu.focused_control != this )
			return;

		if ( input_helper.hovering( this->button_area_ ) && input_helper.key_pressed( VK_LBUTTON ) ) {
			this->dragging_ = true;
		}

		if ( this->dragging_ && !GetAsyncKeyState( VK_LBUTTON ) ) {
			menu.focused_control = nullptr;
			this->dragging_ = false;
		}

		if ( !this->dragging_ )
			return;

		menu.focused_control = this;

		*this->value_ = this->minimum_ + ( this->maximum_ - this->minimum_ ) * ( input_helper.m_mouse_position( ).x - ( button_area_.x ) ) / ( button_area_.w );
		*this->value_ = std::clamp( *this->value_, this->minimum_, this->maximum_ );
	}

	void save() override {
		std::cout << *this->value_ << "\n";
	}
	void load() override {
		std::string line;
		std::getline(std::cin, line);
		std::istringstream(line) >> *this->value_;
	}
};