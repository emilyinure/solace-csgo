#pragma once
#include "base_control.hh"
#include "../input_helper/input_helper.hh"
#include <iostream>
#include <fstream>

class c_slider : public c_base_control {
    int* ivalue_;
	float* fvalue_;
	float minimum_{ 0.f }, maximum_{ 100.f };
	bool dragging_{ false };
	area_t button_area_{ 0,0,0,0 };
public:
    c_slider( const char* name, void* value, float minimum = 0.f, float maximum = 100.f, bool int_display = false )
    {
		this->name = name;
        if (int_display)
            this->ivalue_ = static_cast<int*>(value);
        else
		    this->fvalue_ = static_cast<float*>(value);
		this->minimum_ = minimum;
		this->maximum_ = maximum;
		this->type = control_type_slider;
	}

	auto disable( ) -> void override {
		this->dragging_ = false;
	}
	
	auto draw( ) -> void override {
		button_area_ = { this->area.x, this->area.y + ( this->area.h / 2 ) + ( this->area.h / 2 ) / 4.f, this->area.w, ( ( this->area.h / 2 ) / 2.f ) };
        float temp_value = 0;
        if (this->ivalue_)
            temp_value = static_cast<float>(*this->ivalue_);
        else if (this->fvalue_)
            temp_value = *this->fvalue_;
		const auto slider_fill{ ( ( temp_value - this->minimum_ ) / ( this->maximum_ - this->minimum_ ) * ( button_area_.w-2.f ) ) };

        const auto text_height = render_t::get_text_height( this->name, g.m_render->m_constantia_12( ) );

		g.m_render->filled_rect( button_area_.x + 1.f, button_area_.y + 1.f, slider_fill, button_area_.h - 2.f, menu.main_theme );
		g.m_render->outlined_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h, { 240,240,240, 14 } );

        render_t::text( g.m_render->m_constantia_12( ), this->area.x, this->area.y + this->area.h/4 - text_height/2, menu.bright, this->name );

		// display.
        char display[32];
        if (this->ivalue_)
            sprintf_s(display, "%d / %d", *this->ivalue_, int(round(this->maximum_)) );
        else if (this->fvalue_)
		    sprintf_s( display, "%.1f / %.1f", *this->fvalue_, this->maximum_ );
		const auto text_size = g.m_render->get_text_width( display, g.m_render->m_constantia_12( ) );

        render_t::text( g.m_render->m_constantia_12( ), button_area_.x + button_area_.w - static_cast<float>(text_size), this->area.y + this->area.h/4 - static_cast<float>(text_height) / 2, menu.main_theme, display );
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

        if (this->ivalue_) {
            *this->ivalue_ = static_cast<int>(roundf(this->minimum_ + (this->maximum_ - this->minimum_) *
                                                     (input_helper.m_mouse_position().x - (button_area_.x)) /
                                                     (button_area_.w)));
            *this->ivalue_ = std::clamp(*this->ivalue_, int(roundf(this->minimum_)), int(roundf(this->maximum_)));
        }
        else if (this->fvalue_)
        {
            *this->fvalue_ = this->minimum_ + (this->maximum_ - this->minimum_) *
                                                 (input_helper.m_mouse_position().x - (button_area_.x)) /
                                                 (button_area_.w);
            *this->fvalue_ = std::clamp(*this->fvalue_, this->minimum_, this->maximum_);
        }
	}

	void save() override
    {
        if (this->ivalue_)
            std::cout << *this->ivalue_ << "\n";
        else if (this->fvalue_)
            std::cout << *this->fvalue_ << "\n";
	}
	void load() override {
		std::string line;
        std::getline(std::cin, line);
        if (this->ivalue_)
            std::istringstream(line) >> *this->ivalue_;
        else if (this->fvalue_)
            std::istringstream(line) >> *this->fvalue_;
	}
};