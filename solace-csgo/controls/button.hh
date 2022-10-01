#pragma once
#include "base_control.hh"
#include "../input_helper/input_helper.hh"
#include <iostream>
#include <fstream>

class c_button : public c_base_control {
	typedef void( *button_fn )( void );
	button_fn function_{ nullptr };
	bool holding_{ false };
public:
	c_button( const char* name, button_fn function ) {
		this->name = name;
		this->function_ = function_;
		this->type = control_type_button;
	}

	auto draw( ) -> void override {
		g.m_render->gradient( this->area.x, this->area.y, this->area.w, this->area.h, this->holding_ ? color( 0xe5, 0xc1, 0xcd ) : color{ 0x04, 0x1b, 0x2d }, this->holding_ ? color{ 0x04, 0x1b, 0x2d } : color{ 0x04, 0x1b, 0x2d } );
		g.m_render->outlined_rect( this->area.x, this->area.y, this->area.w, this->area.h, { 240, 240, 240 } );

		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x + ( this->area.w / 2 ), this->area.y + 2, { 240, 240, 240 }, this->name, true );
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
};
