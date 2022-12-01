#pragma once
#include "base_control.hh"
#include "../input_helper/input_helper.hh"
#include <iostream>
#include <fstream>

class c_button : public c_base_control {
	typedef void( *button_fn )( void );
	std::function<void( )> function_{ nullptr };
	bool holding_{ false };
public:
	c_button( const char* name, std::function<void( )> function ) {
		this->name = name;
		this->function_ = function;
		this->type = control_type_button;
	}

	auto disable( ) -> void override {
		this->holding_ = false;
	}

	auto draw( ) -> void override {
		menu.main_theme.set_a( 50 );
		g.m_render->gradient( this->area.x + 1, this->area.y + 1, this->area.w - 2, this->area.h - 2, this->holding_ ? menu.main_theme : color{ 0, 0, 0, 0 }, color{ 0, 0, 0, 0 } );
		menu.main_theme.set_a( 255 );
		g.m_render->outlined_rect( this->area.x, this->area.y, this->area.w, this->area.h, { 240,240,240, 7 } );

		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x + ( this->area.w / 2 ), this->area.y + this->area.h / 2, { 240, 240, 240 }, this->name, Horizontal | Vertical );
	}

	auto update( ) -> void override {
		if ( input_helper.hovering( this->area ) && input_helper.key_down( VK_LBUTTON ) ) {
			if ( !this->holding_ )
				this->holding_ = input_helper.key_pressed( VK_LBUTTON );
		} else this->holding_ = false;

		if ( input_helper.hovering( this->area ) && input_helper.key_pressed( VK_LBUTTON ) && this->function_ != nullptr ) {
			input_helper.set_key( VK_LBUTTON, false );
			this->function_( );
		}
	}

	void save( ) override {
	}
	void load( )  override {
	}
};
