#pragma once
#include "controls/window.hh"
#include <iostream>
#include <fstream>

class c_tab : public c_base_control {
	vector_2d drag_offset_{ 0, 0 };
	std::vector<std::shared_ptr<c_base_control>> children_{ };
public:

	explicit c_tab( const char *name_ ) {
		name = name_;
		type = control_type_tab;
	}

	auto draw( ) -> void  override {
		g.m_render->push_clip( this->area.x, this->area.y, this->area.w, this->area.h );
		g.m_render->filled_rect( this->area.x, this->area.y, this->area.w, this->area.h, color( 240,240,240, 10) );
		g.m_render->pop_clip( );
		//g.m_render->outlined_rect( this->area.x, this->area.y, this->area.w, this->area.h, { 240,240,240 } );
		// child handling.
		if ( !this->children_.empty( ) ) {
			// handle position.
			const auto column_width = floorf( ((this->area.w-10) - 10 * static_cast< float >(children_.size( ))) / children_.size( ) );
			vector_2d child_offset{ 10, 0 };
			for ( const auto &child : this->children_ ) {

				switch ( child->type ) {
				case control_type_invalid: /* invalid control, do nothing. */ break;
				default:
				{
					child->adjust_area( { this->area.x, this->area.y, column_width, this->area.h } );
					child->adjust_position( child_offset );
					child_offset.x += column_width + 10;
				} break;
				}
			}

			// draw.
			for ( const auto &child : this->children_ ) {
				child->draw( );
			}
		}
		//this->area.h = ( 16 * this->children_.size( ) );
	}

	auto update( ) -> void  override {
		// ugh... ghetto...
		const auto mouse_position = input_helper.m_mouse_position( );

		// child handling.
		if ( !this->children_.empty( ) ) {
			for ( const auto &child : this->children_ ) {
				child->update( );
			}
		}
	}
	auto disable( ) -> void  override {
		for ( const auto &child : this->children_ ) {
			child->disable( );
		}
	}
	auto finish () {
	}
	auto add_child( std::shared_ptr<c_base_control> control ) -> void {
		this->children_.push_back( control );
	}
	void save() override {
		for (auto i : children_) {
			i->save();
		}
	}
	void load()  override {
		for (auto i : children_) {
			i->load();
		}
	}
};