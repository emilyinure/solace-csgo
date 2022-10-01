#pragma once


#include <iostream>
#include <fstream>

class c_key_bind : public c_base_control {
	area_t item_area_{ 0, 0, 0, 0 };
	bool open_{ false }, capturing_{ false };
	key_bind_t value_;
	std::vector<const char*> key_bind_types_{ "Always", "Hold", "Toggle", "Off Key" };
	area_t button_area_{0,0,0,0};
public:
	c_key_bind( const char* name, bool *value, int default_type = 0 ) {
		this->name = name;
		this->value_ = key_bind_t(name, value, default_type );
		this->type = control_type_key_bind;
		menu.append_bind( &this->value_ );
	}

	auto disable() -> void override {
		this->open_ = false;
	}

	auto draw( ) -> void override {
		if ( this->capturing_ ) {
			for ( auto i = 0; i < 255; ++i ) {
				if ( GetAsyncKeyState( i ) && i != 3 ) {
					if ( i == VK_LBUTTON && input_helper.hovering( this->area ) )
						continue;

					if ( i == VK_ESCAPE ) {
						this->value_.key = 0;
						menu.focused_control = nullptr;
						this->capturing_ = false;
						break;
					} else if ( strcmp( "No bind", input_helper.m_key( i ) ) != 0 ) {
						this->value_.key = i;
						menu.focused_control = nullptr;
						this->capturing_ = false;
						break;
					}
				}
			}
		}
		
		button_area_ = { this->area.x, this->area.y + this->area.h / 2.f, this->area.w, this->area.h / 2.f };
		const auto text_size = g.m_render->get_text_width( input_helper.m_key( this->value_.key ), g.m_render->m_constantia_12( ) );
		const auto text_height = g.m_render->get_text_height( input_helper.m_key( this->value_.key ), g.m_render->m_constantia_12( ) );

		
		if ( this->open_ || this->capturing_ )
			g.m_render->filled_rect( button_area_.x + 1, button_area_.y + 1, button_area_.w - 2, button_area_.h - 2, { 0xDB, 0x2E, 0x2C, 100 } );
		g.m_render->outlined_rect( button_area_.x, button_area_.y, button_area_.w, button_area_.h, { 240,240,240, 7 } );

		g.m_render->text( g.m_render->m_constantia_12( ), this->area.x, this->area.y + button_area_.h / 2 - text_height / 2.f, { 240,240,240, 100 }, this->name );
		g.m_render->text( g.m_render->m_constantia_12( ), this->button_area_.x + this->button_area_.w / 2 - ( text_size / 2.f ), ( this->button_area_.y + ( this->button_area_.h / 2 ) ) - ( text_height / 2.f ), { 240,240,240, 100 }, input_helper.m_key( this->value_.key ) );

		if ( !this->open_ )
			return;

		this->item_area_ = { button_area_.x, button_area_.y + button_area_.h, button_area_.w, button_area_.h * static_cast< int >( this->key_bind_types_.size( ) ) };


		g.m_render->filled_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 0x30, 0x2E, 0x2C } );
		g.m_render->filled_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 240,240,240, 7 * 3 } );
		g.m_render->outlined_rect( this->item_area_.x, this->item_area_.y, this->item_area_.w, this->item_area_.h, { 240, 240,240, 7 } );
		
		for ( auto i{ 0 }; i < static_cast< int >( this->key_bind_types_.size( ) ); i++ ) {
			const auto* item{ this->key_bind_types_[ i ] };
			const area_t item_area{ this->item_area_.x, this->item_area_.y + ( i * button_area_.h ), this->item_area_.w, button_area_.h };
			const auto selected = i == this->value_.type;

			if( selected )
				g.m_render->filled_rect( item_area.x + 1, item_area.y + 1, item_area.w - 2, item_area.h - 2, color{ 0xDB, 0x2E, 0x2C, 100 } );

			g.m_render->text( g.m_render->m_constantia_12( ), item_area.x + 6, item_area.y + item_area.h / 2 - text_height / 2, { 240,240,240, 100 }, item );
		}
	}

	auto update( ) -> void override {
		if ( menu.focused_control && menu.focused_control != this )
			return;

		if ( !this->open_ && input_helper.hovering( button_area_ ) && input_helper.key_pressed( VK_LBUTTON ) ) {
			input_helper.set_key( VK_LBUTTON, false );
			this->capturing_ = true;
		}

		if ( !this->capturing_ && input_helper.hovering( button_area_ ) && input_helper.key_pressed( VK_RBUTTON ) )
			this->open_ = true;

		

		if ( this->open_ && !input_helper.hovering( button_area_ ) && !input_helper.hovering( this->item_area_ ) && ( input_helper.key_pressed( VK_LBUTTON ) ) ) {
			input_helper.set_key( VK_LBUTTON, false );
			menu.focused_control = nullptr;
			this->open_ = false;
			return;
		}

		if ( !this->capturing_ && !this->open_ )
			return;

		menu.focused_control = this;

		if ( this->capturing_ && this->open_ ) {
			for ( auto i{ 0 }; i < static_cast< int >( this->key_bind_types_.size( ) ); i++ ) {
				const area_t item_area{ this->item_area_.x, this->item_area_.y + ( i * 17 ), this->item_area_.w, 17 };

				if ( input_helper.hovering( item_area ) && input_helper.key_pressed( VK_LBUTTON ) ) {
					input_helper.set_key( VK_LBUTTON, false );
					this->value_.type = i;
					menu.focused_control = nullptr;
					this->open_ = false;
				}
			}
		}
	}
	void save() override {
		std::cout << this->value_.type << "\n";
		std::cout << this->value_.key << "\n";
	}
	void load() override {
		std::string line;
		std::getline(std::cin, line);
		std::istringstream(line) >> this->value_.type;
		line = "";
		std::getline(std::cin, line);
		std::istringstream(line) >> this->value_.key;
	}
};