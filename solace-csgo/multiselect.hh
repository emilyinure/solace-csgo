#pragma once
#include "controls/base_control.hh"
#include "input_helper/input_helper.hh"
#include <iostream>
#include <fstream>

class c_multiselect : public c_base_control {
	int *value_;
	area_t item_area_{ 0, 0, 0, 0 };
	bool open_{ false };
	std::vector<const char *> items_{ };
	area_t button_area_{0,0,0,0};
public:
	c_multiselect( const char *name, int *value, std::vector<const char *> items, const int sub_tab = -1 ) {
		this->name = name;
		this->value_ = value;
		this->items_ = std::move( items );
		this->type = control_type_combobox;
	}

	auto disable( ) -> void override {
		this->open_ = false;
	}


	auto draw( ) -> void override {
		button_area_ = { this->area.x, this->area.y + this->area.h / 2.f, this->area.w, this->area.h / 2.f };
		auto count = 0;
		for ( auto i = 0; i < this->items_.size( ); i++ )
			if ( *value_ & ( 1 << i ) )
				count++;
		std::stringstream string;
		string << count << " items selected";
		auto text = _strdup( string.str( ).c_str( ) );

		auto text_size = g.m_render->get_text_width( text, g.m_render->m_constantia_12( ) );
		auto text_height = g.m_render->get_text_height( text, g.m_render->m_constantia_12( ) );

		if ( this->open_ )
			g.m_render->filled_rect( button_area_.x + 1, button_area_.y + 1, button_area_.w - 2, button_area_.h - 2, { 0xDB, 0x2E, 0x2C, 100 } );
		g.m_render->outlined_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h, { 240,240,240, 7 } );

		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x, this->area.y + button_area_.h / 2 - text_height / 2.f, { 240,240,240, 100 }, this->name );
		g.m_render->text( g.m_render->m_constantia_12( ), this->button_area_.x + 5, ( this->button_area_.y + ( this->button_area_.h / 2 ) ) - ( text_height / 2.f ), { 240,240,240, 100 }, text );

		// item drawing.
		if ( !this->open_ )
			return;

		this->item_area_ = { button_area_.x, button_area_.y + button_area_.h - 1, button_area_.w, button_area_.h * static_cast< int >( this->items_.size( ) ) };

		g.m_render->filled_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 0x30, 0x2E, 0x2C } );
		g.m_render->filled_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 240,240,240, 7 * 3} );
		g.m_render->push_clip( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h - 1 );
		g.m_render->outlined_rect( this->item_area_.x, this->item_area_.y-1, this->item_area_.w, this->item_area_.h+1, { 240, 240,240, 7 } );
		g.m_render->pop_clip( );

		for ( auto i{ 0 }; i < static_cast< int >( this->items_.size( ) ); i++ ) {
			const auto *item{ this->items_[ i ] };
			const area_t item_area{ this->item_area_.x, this->item_area_.y + ( i * 17 ), this->item_area_.w, 17 };
			const auto selected = *this->value_ & (1 << i);

			if ( selected )
				g.m_render->filled_rect( item_area.x + 1, item_area.y + 1, item_area.w - 2, item_area.h - 2, color{ 0xDB, 0x2E, 0x2C, 100 } );

			text_size = g.m_render->get_text_width( item, g.m_render->m_constantia_12( ) );
			text_height = g.m_render->get_text_height( item, g.m_render->m_constantia_12( ) );
			g.m_render->text( g.m_render->m_constantia_12( ), item_area.x + 5, item_area.y + item_area.h / 2 - text_height / 2, { 240,240,240, 100 }, item );
		}

		//g.m_render->outlined_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 240,240,240 } );
	}

	auto update( ) -> void override {
		if ( menu.focused_control && menu.focused_control != this )
			return;

		if ( input_helper.hovering( this->area ) && input_helper.key_pressed( VK_LBUTTON ) ) {
			this->open_ = !this->open_;
			if( !this->open_ )
				menu.focused_control = nullptr;
			input_helper.set_key( VK_LBUTTON, false );
		}

		if ( this->open_ && !input_helper.hovering( this->button_area_ ) && !input_helper.hovering( this->item_area_ ) && input_helper.key_pressed_prestine( VK_LBUTTON ) ) {
			menu.focused_control = nullptr;
			this->open_ = false;
			input_helper.set_key( VK_LBUTTON, false );
			return;
		}

		if ( !this->open_ )
			return;

		menu.focused_control = this;

		for ( auto i{ 0 }; i < static_cast< int >( this->items_.size( ) ); i++ ) {
			const area_t item_area{ this->item_area_.x, this->item_area_.y + ( i * 17 ), this->item_area_.w, 17 };

			if ( input_helper.hovering( item_area ) && input_helper.key_pressed( VK_LBUTTON ) ) {
				input_helper.set_key( VK_LBUTTON, false );
				if( *this->value_ & ( 1 << i ) )
					*this->value_ &= ~( 1 << i );
				else
					*this->value_ |= (1 << i);
				//menu.focused_control = nullptr;
				//this->open_ = false;
			}
		}
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
