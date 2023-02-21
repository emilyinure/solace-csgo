#pragma once
#include "base_control.hh"
#include "../input_helper/input_helper.hh"
#include <iostream>
#include <fstream>

class c_column : public c_base_control {
	vector_2d drag_offset_{ 0, 0 };
	std::vector<std::shared_ptr<c_base_control>> children_{ };
public:

	explicit c_column( ) {
		type = control_type_tab;
	}

	auto draw( ) -> void  override {
		//g.m_render->gradient( this->area.x, this->area.y, this->area.w, this->area.h, { 0x00, 0x4e, 0x9a }, { 0x42, 0x9c, 0xd4 } );
		////g.m_render->filled_rect( this->area.x, this->area.y, this->area.w, this->area.h, { 60, 60, 60 } );
		//g.m_render->outlined_rect( this->area.x, this->area.y, this->area.w, this->area.h, { 240, 240, 240 } );
		//g.m_render->filled_rect( this->area.x, this->area.y, this->area.w, this->area.h, { 255, 255, 255 } );
		//// child handling.
		if ( !this->children_.empty( ) ) {
			// handle position.

			float column_width = roundf( ( this->area.w - 20 * static_cast< float >( children_.size( ) ) ) / children_.size( ) );
			const float group_height = floorf( ( ( area.h - 10 ) - 10 * static_cast< float >( children_.size( ) ) ) / children_.size( ) );
			
			
			vector_2d child_offset{ 0, 10 };
			for ( const auto &child : this->children_ ) {

				switch ( child->type ) {
				case control_type_invalid: /* invalid control, do nothing. */ break;
				default:
				{
					child->adjust_area( { this->area.x, this->area.y, this->area.w, group_height } );
					child->adjust_position( child_offset );
					child_offset.y += group_height + 10.f;
				} break;
				}
			}

			// draw.
            size_t i = this->children_.size();
			
            do
            {
                i--;
                const auto& child = this->children_[i];
                child->draw();
            } while (i != 0);
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
	auto finish( ) {
	}
	auto add_child( std::shared_ptr<c_base_control> control ) -> void {
		this->children_.push_back( control );
	}
	void save() override {
		for (auto i : children_) {
			i->save();
		}
	}
	void load() override {
		for (auto i : children_) {
			i->load();
		}
	}
};