#include "menu.hh"

#include "includes.h"
#include "multiselect.hh"
#include "tab.hh"
#include "controls/window.hh"
#include "controls/button.hh"
#include "controls/combobox.hh"
#include "controls/key_bind.hh"
#include "controls/slider.hh"
#include "controls/toggle.hh"
#include "column.hh"
#include "groupbox.hh"

bool test;
float test2;
int test3;

void test5( ) {

}

auto c_menu::init( ) -> void {
	auto main_form = std::make_shared<c_form>( "Solace", area_t{ 200, 200, 390, 312 } );
	this->m_forms.push_back( main_form );
	
	auto aim_tab = std::make_shared<c_tab>( "rage aim" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto general_group = std::make_shared<c_group_box>( "General" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "General" );
				general_tab->add_child( std::make_shared<c_toggle>( "enabled", &settings::rage::general::enabled ) );
				general_tab->add_child( std::make_shared<c_key_bind>( "key", &settings::rage::general::key ) );
				general_tab->add_child( std::make_shared<c_toggle>( "auto shoot", &settings::rage::general::auto_shoot ) );
				general_tab->add_child( std::make_shared<c_toggle>( "silent", &settings::rage::general::silent ) );
				general_tab->add_child( std::make_shared<c_slider>( "delay shot", &settings::rage::general::delay_shot, 0, 1.f ) );
				general_group->add_child( general_tab );
			};
			left_column->add_child( general_group );
		}
		aim_tab->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto selection_group = std::make_shared<c_group_box>( "Selection" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Selection" );
				general_tab->add_child( std::make_shared<c_slider>( "fov", &settings::rage::selection::fov, 0, 180 ) );
				general_tab->add_child( std::make_shared<c_slider>( "hitchance", &settings::rage::selection::hitchance ) );
				general_tab->add_child( std::make_shared<c_slider>( "minimum damage", &settings::rage::selection::min_damage ) );
				general_tab->add_child( std::make_shared<c_slider>( "extra lethal damage", &settings::rage::selection::lethal_damage ) );
				general_tab->add_child( std::make_shared<c_multiselect>( "hitboxes", &settings::rage::selection::hitboxes, std::vector<const char *> { "head", "chest", "body", "arms", "legs" } ) );
				general_tab->add_child( std::make_shared<c_slider>( "head scale", &settings::rage::selection::point_scale ) );
				general_tab->add_child( std::make_shared<c_slider>( "body scale", &settings::rage::selection::body_scale ) );
				general_tab->add_child( std::make_shared<c_toggle>( "body-aim lethal", &settings::rage::selection::body_aim_lethal ) );
				selection_group->add_child( general_tab );
			}
			right_column->add_child( selection_group );
		}
		aim_tab->add_child( right_column );
	}
	main_form->add_child( aim_tab );
	auto visual_tab = std::make_shared<c_tab>( "visuals" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto esp_tab = std::make_shared<c_group_box>( "Players" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "box", &settings::visuals::players::box ) );
				general_tab->add_child( std::make_shared<c_toggle>( "team", &settings::visuals::players::team ) );
				general_tab->add_child( std::make_shared<c_toggle>( "name", &settings::visuals::players::name ) );
				general_tab->add_child( std::make_shared<c_toggle>( "health", &settings::visuals::players::health ) );
				general_tab->add_child( std::make_shared<c_toggle>( "weapon", &settings::visuals::players::weapon ) );
				general_tab->add_child( std::make_shared<c_combobox>( "chams - enemy", &settings::visuals::players::chams, std::vector<const char *> {"off", "full", "flat"} ) );
				general_tab->add_child( std::make_shared<c_combobox>( "chams - enemy covered", &settings::visuals::players::chams_covered, std::vector<const char *> {"off", "full", "flat"} ) );
				general_tab->add_child( std::make_shared<c_combobox>( "chams - team", &settings::visuals::players::chams_team, std::vector<const char *> {"off", "full", "flat"} ) );
				general_tab->add_child( std::make_shared<c_combobox>( "chams - team covered", &settings::visuals::players::chams_team_covered, std::vector<const char *> {"off", "full", "flat"} ) );
				esp_tab->add_child( general_tab );
			}
			left_column->add_child( esp_tab );
			auto chams_group = std::make_shared<c_group_box>( "Weapons" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "box", &settings::visuals::weapons::box ) );
				general_tab->add_child( std::make_shared<c_toggle>( "name", &settings::visuals::weapons::name ) );
				general_tab->add_child( std::make_shared<c_toggle>( "box", &settings::visuals::weapons::noscope ) );
				chams_group->add_child( general_tab );
			}
			left_column->add_child( chams_group );
		} visual_tab->add_child( left_column );


		auto right_column = std::make_shared<c_column>( );
		{
			auto world_group = std::make_shared<c_group_box>( "World" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "molotov", &settings::visuals::world::molotov ) );
				general_tab->add_child( std::make_shared<c_toggle>( "wireframe smoke", &settings::visuals::world::wire_smoke ) );
				world_group->add_child( general_tab );
			}
			right_column->add_child( world_group );
			auto misc_group = std::make_shared<c_group_box>( "Misc" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_slider>( "fov", &settings::visuals::misc::fov, 0, 180 ) );
				general_tab->add_child( std::make_shared<c_slider>( "aspect ratio", &settings::visuals::misc::aspectratio, 0, 200 ) );
				misc_group->add_child( general_tab );
			}
			right_column->add_child( misc_group );
		} visual_tab->add_child( right_column );
	}
	main_form->add_child( visual_tab );
	auto legit_tab = std::make_shared<c_tab>( "legit aim" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto general_group = std::make_shared<c_group_box>( "General" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "enabled", &settings::legit::general::enabled ) );
				general_tab->add_child( std::make_shared<c_key_bind>( "key", &settings::legit::general::key ) );
				general_tab->add_child( std::make_shared<c_toggle>( "auto shoot", &settings::legit::general::auto_shoot ) );
				general_tab->add_child( std::make_shared<c_toggle>( "silent", &settings::legit::general::silent ) );
				general_tab->add_child( std::make_shared<c_slider>( "smoothing", &settings::legit::general::smoothing, 0, 100 ) );
				general_group->add_child( general_tab );
			};
			left_column->add_child( general_group );
		} legit_tab->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto selection_tab = std::make_shared<c_group_box>( "Selection" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_slider>( "fov", &settings::legit::selection::fov, 0, 180 ) );
				general_tab->add_child( std::make_shared<c_slider>( "minimum damage", &settings::legit::selection::min_damage ) );
				general_tab->add_child( std::make_shared<c_multiselect>( "hitboxes", &settings::legit::selection::hitboxes, std::vector<const char *> { "head", "chest", "body", "arms", "legs" } ) );
				selection_tab->add_child( general_tab );
			}
			right_column->add_child( selection_tab );
			auto recoil_group = std::make_shared<c_group_box>( "Recoil" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "enabled", &settings::legit::recoil::enabled ) );
				general_tab->add_child( std::make_shared<c_slider>( "x factor", &settings::legit::recoil::x_factor ) );
				general_tab->add_child( std::make_shared<c_slider>( "y factor", &settings::legit::recoil::y_factor ) );
				recoil_group->add_child( general_tab );
			}
			right_column->add_child( recoil_group );
		} legit_tab->add_child( right_column );
	}
	main_form->add_child( legit_tab );

	auto hvh_form = std::make_shared<c_tab>( "hvh" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto antiaim_tab = std::make_shared<c_group_box>( "Angles" );
			{
				auto stand_tab = std::make_shared<c_group_tab>( "Stand" );
				{
					stand_tab->add_child( std::make_shared<c_toggle>( "enabled", &settings::hvh::antiaim::enabled ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "body", &settings::hvh::antiaim::body_fake_stand, std::vector<const char *> { "off", "left", "right", "opposite", "z" } ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "fake", &settings::hvh::antiaim::fake_yaw, std::vector<const char *> { "off", "default", "relative", "relative jitter", "rotate", "random", "local view" } ) );
					stand_tab->add_child( std::make_shared<c_slider>( "fake relative", &settings::hvh::antiaim::fake_relative, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "fake jitter range", &settings::hvh::antiaim::fake_jitter_range, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "pitch", &settings::hvh::antiaim::pitch_stand, std::vector<const char *> { "off", "down", "up", "random", "ideal" } ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "yaw offset", &settings::hvh::antiaim::yaw_stand, std::vector<const char *> { "off", "direction", "jitter", "rotate", "random", "distort" }));
					stand_tab->add_child( std::make_shared<c_combobox>( "yaw dir", &settings::hvh::antiaim::dir_stand, std::vector<const char *> { "direction", "backwards", "left", "right" } ) );
					stand_tab->add_child( std::make_shared<c_slider>( "stand jitter range", &settings::hvh::antiaim::jitter_range_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "rot range", &settings::hvh::antiaim::rot_range_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "rot speed", &settings::hvh::antiaim::rot_speed_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "random update speed", &settings::hvh::antiaim::rand_update_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "base angle", &settings::hvh::antiaim::base_angle_stand, std::vector<const char *> { "off", "static", "away crosshair", "away distance" } ) );
				} antiaim_tab->add_child( stand_tab );
				auto move_tab = std::make_shared<c_group_tab>( "Move" );
				{
					move_tab->add_child( std::make_shared<c_toggle>( "enabled", &settings::hvh::antiaim::enabled ) );
					move_tab->add_child( std::make_shared<c_combobox>( "fake", &settings::hvh::antiaim::fake_yaw, std::vector<const char *> { "off", "default", "relative", "relative jitter", "rotate", "random", "local view" } ) );
					move_tab->add_child( std::make_shared<c_slider>( "fake relative", &settings::hvh::antiaim::fake_relative, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "fake jitter range", &settings::hvh::antiaim::fake_jitter_range, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_combobox>( "pitch", &settings::hvh::antiaim::pitch_walk, std::vector<const char *> { "off", "down", "up", "random", "ideal" } ) );
					move_tab->add_child( std::make_shared<c_combobox>( "yaw offset walking", &settings::hvh::antiaim::yaw_walk, std::vector<const char *> { "off", "direction", "jitter", "rotate", "random", "break" } ) );
					move_tab->add_child( std::make_shared<c_combobox>( "yaw dir walking", &settings::hvh::antiaim::dir_walk, std::vector<const char *> { "direction", "backwards", "left", "right" } ) );
					move_tab->add_child( std::make_shared<c_slider>( "walk jitter range", &settings::hvh::antiaim::jitter_range_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "rot range walk", &settings::hvh::antiaim::rot_range_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "rot speed walk", &settings::hvh::antiaim::rot_speed_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "random update speed walk", &settings::hvh::antiaim::rand_update_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_combobox>( "base angle walk", &settings::hvh::antiaim::base_angle_walk, std::vector<const char *> { "off", "static", "away crosshair", "away distance" } ) );

				} antiaim_tab->add_child( move_tab );
				auto air_tab = std::make_shared<c_group_tab>( "Air" );
				{
					air_tab->add_child( std::make_shared<c_toggle>( "enabled", &settings::hvh::antiaim::enabled ) );
					air_tab->add_child( std::make_shared<c_combobox>( "body", &settings::hvh::antiaim::body_fake_air, std::vector<const char *> { "off", "left", "right", "opposite" } ) );
					air_tab->add_child( std::make_shared<c_combobox>( "fake", &settings::hvh::antiaim::fake_yaw, std::vector<const char *> { "off", "default", "relative", "relative jitter", "rotate", "random", "local view" } ) );
					air_tab->add_child( std::make_shared<c_slider>( "fake relative", &settings::hvh::antiaim::fake_relative, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "fake jitter range", &settings::hvh::antiaim::fake_jitter_range, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_combobox>( "pitch", &settings::hvh::antiaim::pitch_air, std::vector<const char *> { "off", "down", "up", "random", "ideal" } ) );
					air_tab->add_child( std::make_shared<c_combobox>( "yaw offset", &settings::hvh::antiaim::yaw_air, std::vector<const char *> { "off", "direction", "jitter", "rotate", "random" } ) );
					air_tab->add_child( std::make_shared<c_combobox>( "yaw dir", &settings::hvh::antiaim::dir_air, std::vector<const char *> { "direction", "backwards", "left", "right" } ) );
					air_tab->add_child( std::make_shared<c_slider>( "air jitter range", &settings::hvh::antiaim::jitter_range_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "rot range", &settings::hvh::antiaim::rot_range_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "rot speed", &settings::hvh::antiaim::rot_speed_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "random update speed", &settings::hvh::antiaim::rand_update_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_combobox>( "base angle", &settings::hvh::antiaim::base_angle_air, std::vector<const char *> { "off", "static", "away crosshair", "away distance" } ) );

				} antiaim_tab->add_child( air_tab );
				
				
			}
			left_column->add_child( antiaim_tab );
		} hvh_form->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto lag_tab = std::make_shared<c_group_box>( "Lag" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "" );
				{
					general_tab->add_child( std::make_shared<c_toggle>( "lag", &settings::hvh::antiaim::lag_enable ) );
					general_tab->add_child( std::make_shared<c_multiselect>( "lag activation", &settings::hvh::antiaim::lag_active, std::vector<const char *> { "move", "air", "crouch" } ) );
					general_tab->add_child( std::make_shared<c_combobox>( "lag mode", &settings::hvh::antiaim::lag_mode, std::vector<const char *> { "max", "random", "break step", "rotate", "random", "peek" } ) );
					general_tab->add_child( std::make_shared<c_slider>( "lag limit", &settings::hvh::antiaim::lag_limit, 0, 15 ) );
				}
				lag_tab->add_child( general_tab );
			}
			right_column->add_child( lag_tab );
			auto misc_tab = std::make_shared<c_group_box>( "Misc" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "" );
				{
					general_tab->add_child(std::make_shared<c_key_bind>("fake walk", &settings::hvh::antiaim::fakewalk, 1));
					general_tab->add_child(std::make_shared<c_key_bind>("fake head", &settings::hvh::antiaim::fakehead, 1));
					general_tab->add_child( std::make_shared<c_key_bind>( "auto peek", &settings::hvh::antiaim::auto_peek, 1 ) );
				}
				misc_tab->add_child( general_tab );
			} right_column->add_child( misc_tab );
		} hvh_form->add_child( right_column );
	} main_form->add_child( hvh_form );
	
	auto misc_form = std::make_shared<c_tab>( "misc");
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto movement_group = std::make_shared<c_group_box>( "Movement" );
			{
				auto movement_tab = std::make_shared<c_group_tab>( "Movement" );
				movement_tab->add_child( std::make_shared<c_toggle>( "bhop", &settings::misc::movement::bhop ) );
				movement_tab->add_child( std::make_shared<c_combobox>( "auto strafer", &settings::misc::movement::autostrafe, std::vector<const char *> { "off", "normal", "directional" } ) );
				movement_group->add_child( movement_tab );
			}
			left_column->add_child( movement_group );
		} misc_form->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto griefing_tab = std::make_shared<c_group_box>( "Griefing" );
			{
				auto movement_tab = std::make_shared<c_group_tab>( "Movement" );
				movement_tab->add_child( std::make_shared<c_key_bind>( "block bot", &settings::misc::griefing::block_bot, key_bind_type_toggle ) );
				griefing_tab->add_child( movement_tab );
			}
			right_column->add_child( griefing_tab );
			auto misc_tab = std::make_shared<c_group_box>( "Misc" );
			{
				auto movement_tab = std::make_shared<c_group_tab>( "Movement" );
				movement_tab->add_child( std::make_shared<c_toggle>( "ping exploit", &settings::misc::misc::fake_latency ) );
				movement_tab->add_child( std::make_shared<c_slider>( "ping amount", &settings::misc::misc::fake_latency_amt, 0, 1000) );
				movement_tab->add_child( std::make_shared<c_key_bind>( "thirdperson", &settings::misc::misc::thirdperson, 2 ) );
				misc_tab->add_child( movement_tab );
			}
			right_column->add_child( misc_tab );
		} misc_form->add_child( right_column );
	} main_form->add_child( misc_form );
}

auto c_menu::draw( ) const -> void {	
	for ( auto form : this->m_forms ) {
		if ( !form->m_enabled( ) ) {
			if ( form.get( ) == this->focused_form )	
				menu.focused_form = nullptr;

			continue;
		}

		// we want to draw the focused form later.
		if ( form.get( ) == this->focused_form )
			continue;

		form->draw( );
	}

	if ( this->focused_form )
		this->focused_form->draw( );
}

auto c_menu::update( ) const -> void {
	for ( auto form : this->m_forms ) {
		if ( form->m_enabled( ) )
			form->update( );
	}
}

void c_menu::append_bind ( key_bind_t *value ) {
	n_binds.push_back( value );
}

void c_menu::save() {
	for (auto &i : m_forms) {
		i->save();
	}
}

void c_menu::load() {
	for (auto &i : m_forms) {
		i->load();
	}
}

void key_bind_t::update ( ) {
	type = std::clamp( type, 0, static_cast< int >(this->key_bind_types_.size( )) );

	switch ( type ) {
	case key_bind_type_always : *enabled = true;
		break;
	case key_bind_type_hold : *enabled = input_helper.key_down( key );
		break;
	case key_bind_type_toggle : {
		if ( input_helper.key_pressed_prestine( key ) )
			*enabled = !*enabled;
	}
	break;
	case key_bind_type_off_key : *enabled = !GetAsyncKeyState( key );
		break;
	default : *enabled = true;
		break;
	}
}
