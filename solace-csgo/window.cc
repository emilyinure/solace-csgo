#include "controls/window.hh"
#include "tab.hh"

auto c_form::draw ( ) -> void {

	g.m_render->filled_rect( this->area_.x - 1, this->area_.y - 1, this->area_.w + 2, this->area_.h + 2, color{ 0x30, 0x2E, 0x2C } );
	auto text_height = g.m_render->get_text_height( this->name_, g.m_render->m_tahoma_14( ) );
	g.m_render->text( g.m_render->m_tahoma_14( ), this->area_.x + 7, this->area_.y + 10 - ( text_height / 2.f ), { 0xDB, 0x2E, 0x2C, 100 }, this->name_ );
	g.m_render->filled_rect( this->area_.x, this->area_.y + 20, this->area_.w, 2, { 0xDB, 0x2E, 0x2C, 20 } );
	g.m_render->outlined_rect( this->area_.x - 1, this->area_.y - 1, this->area_.w + 2, this->area_.h + 2, { 0xDB, 0x2E, 0x2C, 20 } );

	// child handling.
	if ( !this->children_.empty( ) ) {
		// handle position.
		vector_2d child_offset{0, 0};
		for ( const auto &child : this->children_ ) {

			switch ( child->type ) {
			case control_type_invalid : /* invalid control, do nothing. */ break;
			default : {	
				child->adjust_area( { this->area_.x, this->area_.y + 22.f, this->area_.w, this->area_.h - 22.f } );
			}
			break;
			}
		}

		auto text_x = this->area_.x + this->area_.w - 6.f;
		for ( int i = children_.size( ) - 1; i >= 0; i-- ) {
			auto *const child = children_[ i ].get(  );
			const auto name_width = g.m_render->get_text_width( child->name, g.m_render->m_constantia_12( ) );
			text_height = g.m_render->get_text_height( this->name_, g.m_render->m_constantia_12( ) );
			text_x -= name_width + 7.f;
			g.m_render->text( g.m_render->m_constantia_12( ), text_x, this->area_.y + 10 - ( text_height / 2.f ), child == selected_tab ? color{ 0xDB, 0x2E, 0x2C, 100 } : color{ 240,240,240, 100 }, child->name );
		}
		// draw.
		for ( auto child : this->children_ ) {
			if ( child.get( ) != selected_tab )
				continue;

			child->draw( );
		}
	}
	//this->area_.h = 15;
	//if ( this->selected_tab != nullptr )
	//	this->area_.h += this->selected_tab->area_.h;
}

auto c_form::update ( ) -> void {
	// ugh... ghetto...
	const auto mouse_position = input_helper.m_mouse_position( );

	if ( input_helper.key_pressed( VK_LBUTTON ) && input_helper.hovering( this->area_ ) )
		menu.focused_form = this;

	if ( input_helper.hovering( {this->area_.x, this->area_.y, this->area_.w, 15} ) && input_helper.
		key_pressed( VK_LBUTTON ) )
		this->clicked_ = true;

	if ( !GetAsyncKeyState( VK_LBUTTON ) )
		this->clicked_ = false;

	if ( this->should_drag_ && !this->clicked_ )
		this->should_drag_ = false;

	if ( this->should_drag_ && this->clicked_ ) {
		this->area_.x = mouse_position.x - this->drag_offset_.x;
		this->area_.y = mouse_position.y - this->drag_offset_.y;
	}

	if ( input_helper.hovering( {this->area_.x, this->area_.y, this->area_.w, 15} ) ) {
		this->should_drag_ = true;
		this->drag_offset_.x = mouse_position.x - this->area_.x;
		this->drag_offset_.y = mouse_position.y - this->area_.y;
	}

	if ( !this->children_.empty( ) ) {
		int text_x = this->area_.x + this->area_.w - 6.f;
		for ( int i = children_.size( ) - 1; i >= 0; i-- ) {
			const auto child = children_[ i ].get( );
			const auto name_width = g.m_render->get_text_width( child->name, g.m_render->m_constantia_12( ) );
			const auto name_height = g.m_render->get_text_height( child->name, g.m_render->m_constantia_12( ) );
			text_x -= name_width + 7.f;
			if ( input_helper.hovering( { static_cast<float>(text_x), this->area_.y + 1, static_cast< float >( name_width ), static_cast<float>(name_height) } ) && input_helper.key_pressed( VK_LBUTTON ) ) {
				input_helper.set_key( VK_LBUTTON, false );
				selected_tab = child;
				for ( auto i : children_ )
					if( i.get() != selected_tab )
						i->disable( );
				break;
			}
		}
		// child handling.
		if ( this->selected_tab != nullptr ) {
			this->selected_tab->update( );
		}
	}
}
