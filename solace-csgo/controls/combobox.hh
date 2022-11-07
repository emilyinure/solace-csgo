#pragma once
#include <iostream>
#include <fstream>

class c_combobox : public c_base_control {
	int* value_;
	area_t item_area_{ 0, 0, 0, 0 };
	area_t button_area_{ 0,0,0,0 };
	bool open_{ false };
	std::vector<const char*> items_{ };
public:
	c_combobox( const char* name, int* value, std::vector<const char*> items, const int sub_tab = -1 ) {
		this->name = name;
		this->value_ = value;
		this->items_ = std::move( items );
		this->type = control_type_combobox;
	}

	auto disable() -> void override {
		this->open_ = false;
	}

	auto draw( ) -> void override {
		button_area_ = { this->area.x, this->area.y + this->area.h / 2.f, this->area.w, this->area.h / 2.f };
		auto text_size = g.m_render->get_text_width( this->items_[ *this->value_ ], g.m_render->m_constantia_12( ) );
		auto text_height = g.m_render->get_text_height( "A", g.m_render->m_constantia_12( ) );

		if ( !this->open_ )
			g.m_render->outlined_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h, { 240,240,240, 7 } );
		else {
			this->item_area_ = { button_area_.x, button_area_.y + button_area_.h - 1, button_area_.w, button_area_.h * static_cast< int >( this->items_.size( ) ) };
			g.m_render->filled_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h + this->item_area_.h - 1, menu.dark );
			g.m_render->filled_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h + this->item_area_.h - 1, { 240,240,240, 7 * 4 } );
			g.m_render->outlined_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h + this->item_area_.h - 1, { 240,240,240, 7 } );
		}

		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x, this->area.y + button_area_.h / 2 - text_height / 2.f, menu.bright, this->name );
		g.m_render->text( g.m_render->m_constantia_12( ), this->button_area_.x + 5, (this->button_area_.y + (this->button_area_.h / 2)) - (text_height/ 2.f ), menu.main_theme, this->items_[ *this->value_ ] );

		// item drawing.
		if ( !this->open_ )
			return;
		

		for ( auto i{ 0 }; i < static_cast< int >( this->items_.size( ) ); i++ ) {
			const auto* item{ this->items_[ i ] };
			const area_t item_area{ this->item_area_.x, this->item_area_.y + ( i * button_area_.h ), this->item_area_.w, button_area_.h };
			const auto selected = i == *this->value_;

			text_size = g.m_render->get_text_width( item, g.m_render->m_constantia_12( ) );
			text_height = g.m_render->get_text_height( item, g.m_render->m_constantia_12( ) );
			g.m_render->text( g.m_render->m_constantia_12( ), item_area.x + 5, item_area.y + item_area.h / 2 - text_height / 2, selected ? menu.main_theme : menu.bright, item );
		}

		//g.m_render->outlined_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 240,240,240 } );
	}

	auto update( ) -> void override {
		if ( menu.focused_control && menu.focused_control != this )
			return;

		if ( input_helper.hovering( this->area ) && input_helper.key_pressed( VK_LBUTTON ) ) {
			this->open_ = !this->open_;
			if ( !this->open_ )
				menu.focused_control = nullptr;
			input_helper.set_key( VK_LBUTTON, false );
		}

		*this->value_ = std::clamp( *this->value_, 0, static_cast< int >( this->items_.size( ) ) );

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
				*this->value_ = i;
				menu.focused_control = nullptr;
				this->open_ = false;
			}
		}
	}
	void save()  override {
		std::cout << *this->value_ << "\n";
	}
	void load() override {
		std::string line;
		std::getline(std::cin, line);
		std::istringstream(line) >> *this->value_;
	}
};