#include "menu.hh"

#include "includes.h"
#include "controls/multiselect.hh"
#include "controls/tab.hh"
#include "controls/window.hh"
#include "controls/button.hh"
#include "controls/combobox.hh"
#include "controls/key_bind.hh"
#include "controls/slider.hh"
#include "controls/toggle.hh"
#include "controls/column.hh"
#include "controls/groupbox.hh"

std::array<std::string, 3> config_names = { "Auto", "Scout", "Pistol" };

auto c_menu::init( ) -> void {
	auto main_form = std::make_shared<c_form>( "Solace", area_t{ 200, 400, 580, 424 } );
	this->m_forms.push_back( main_form );
	
	auto aim_tab = std::make_shared<c_tab>( "Rage aim" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto general_group = std::make_shared<c_group_box>( "General" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "General" );
				general_tab->add_child( std::make_shared<c_toggle>( "Enabled", &settings::rage::general::enabled ) );
				general_tab->add_child( std::make_shared<c_key_bind>( "Key", &settings::rage::general::key ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Auto shoot", &settings::rage::general::auto_shoot ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Silent", &settings::rage::general::silent ) );
				general_group->add_child( general_tab );
			};
			left_column->add_child( general_group );
			auto hitbox_group = std::make_shared<c_group_box>( "Hitboxes" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Selection" );
				general_tab->add_child( std::make_shared<c_multiselect>( "Hitboxes", &settings::rage::hitbox::hitboxes, std::vector<const char*> { "Head", "Chest", "Body", "Arms", "Legs" } ) );
				general_tab->add_child( std::make_shared<c_slider>( "Head scale", &settings::rage::hitbox::point_scale ) );
				general_tab->add_child( std::make_shared<c_slider>( "Body scale", &settings::rage::hitbox::body_scale ) );
				general_tab->add_child( std::make_shared<c_multiselect>( "Body-aim conditions", &settings::rage::hitbox::baim_conditions, std::vector<const char*> { "Prefer Lethal", "Force In-Air" } ) );
				hitbox_group->add_child( general_tab );
			}
			left_column->add_child( hitbox_group );
		}
		aim_tab->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto selection_group = std::make_shared<c_group_box>( "Selection" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Selection" );
				general_tab->add_child( std::make_shared<c_slider>( "Fov", &settings::rage::selection::fov, 0, 180 ) );
				general_tab->add_child( std::make_shared<c_slider>( "Hitchance", &settings::rage::selection::hitchance ) );
				general_tab->add_child( std::make_shared<c_slider>( "Minimum damage", &settings::rage::selection::min_damage ) );
				general_tab->add_child( std::make_shared<c_slider>( "Extra lethal damage", &settings::rage::selection::lethal_damage ) );
				selection_group->add_child( general_tab );
			}
			right_column->add_child( selection_group );
		}
		aim_tab->add_child( right_column );
	}
	main_form->add_child( aim_tab );
	auto visual_tab = std::make_shared<c_tab>( "Visuals" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto esp_tab = std::make_shared<c_group_box>( "Players" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "Box", &settings::visuals::players::box ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Team", &settings::visuals::players::team ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Name", &settings::visuals::players::name ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Health", &settings::visuals::players::health ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Weapon", &settings::visuals::players::weapon ) );
				general_tab->add_child( std::make_shared<c_combobox>( "Chams - enemy", &settings::visuals::players::chams, std::vector<const char *> {"Off", "Full", "Flat"} ) );
				general_tab->add_child( std::make_shared<c_combobox>( "Chams - enemy covered", &settings::visuals::players::chams_covered, std::vector<const char *> {"Off", "Full", "Flat"} ) );
				general_tab->add_child( std::make_shared<c_combobox>( "Chams - team", &settings::visuals::players::chams_team, std::vector<const char *> {"Off", "Full", "Flat"} ) );
				general_tab->add_child( std::make_shared<c_combobox>( "chams - team covered", &settings::visuals::players::chams_team_covered, std::vector<const char *> {"Off", "Full", "Flat"} ) );
				esp_tab->add_child( general_tab );
			}
			left_column->add_child( esp_tab );
			auto chams_group = std::make_shared<c_group_box>( "Weapons" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "Box", &settings::visuals::weapons::box ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Name", &settings::visuals::weapons::name ) );
				general_tab->add_child( std::make_shared<c_toggle>( "No scope", &settings::visuals::weapons::noscope ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Grendae Prediction", &settings::visuals::weapons::grenade_prediction ) );
				chams_group->add_child( general_tab );
			}
			left_column->add_child( chams_group );
		} visual_tab->add_child( left_column );


		auto right_column = std::make_shared<c_column>( );
		{
			auto world_group = std::make_shared<c_group_box>( "World" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "Molotov", &settings::visuals::world::molotov ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Wireframe smoke", &settings::visuals::world::wire_smoke ) );
				world_group->add_child( general_tab );
			}
			right_column->add_child( world_group );
			auto misc_group = std::make_shared<c_group_box>( "Misc" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_slider>( "Fov", &settings::visuals::misc::fov, 0, 180 ) );
				general_tab->add_child( std::make_shared<c_slider>( "Aspect ratio", &settings::visuals::misc::aspectratio, 0, 200 ) );
				misc_group->add_child( general_tab );
			}
			right_column->add_child( misc_group );
		} visual_tab->add_child( right_column );
	}
	main_form->add_child( visual_tab );
	/// This was left in from when this wasn't a legacy cheat, no real point in a game ver only used for hvh
	/*	auto legit_tab = std::make_shared<c_tab>( "Legit aim" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto general_group = std::make_shared<c_group_box>( "General" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "Enabled", &settings::legit::general::enabled ) );
				general_tab->add_child( std::make_shared<c_key_bind>( "Key", &settings::legit::general::key ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Auto shoot", &settings::legit::general::auto_shoot ) );
				general_tab->add_child( std::make_shared<c_toggle>( "Silent", &settings::legit::general::silent ) );
				general_tab->add_child( std::make_shared<c_slider>( "Smoothing", &settings::legit::general::smoothing, 0, 100 ) );
				general_group->add_child( general_tab );
			};
			left_column->add_child( general_group );
		} legit_tab->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto selection_tab = std::make_shared<c_group_box>( "Selection" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_slider>( "Fov", &settings::legit::selection::fov, 0, 180 ) );
				general_tab->add_child( std::make_shared<c_slider>( "Minimum damage", &settings::legit::selection::min_damage ) );
				general_tab->add_child( std::make_shared<c_multiselect>( "Hitboxes", &settings::legit::selection::hitboxes, std::vector<const char *> { "Head", "Chest", "Body", "Arms", "Legs" } ) );
				selection_tab->add_child( general_tab );
			}
			right_column->add_child( selection_tab );
			auto recoil_group = std::make_shared<c_group_box>( "Recoil" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "Players" );
				general_tab->add_child( std::make_shared<c_toggle>( "Enabled", &settings::legit::recoil::enabled ) );
				general_tab->add_child( std::make_shared<c_slider>( "X factor", &settings::legit::recoil::x_factor ) );
				general_tab->add_child( std::make_shared<c_slider>( "Y factor", &settings::legit::recoil::y_factor ) );
				recoil_group->add_child( general_tab );
			}
			right_column->add_child( recoil_group );
		} legit_tab->add_child( right_column );
	}
	main_form->add_child( legit_tab );*/

	auto hvh_form = std::make_shared<c_tab>( "Hvh" );
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto antiaim_tab = std::make_shared<c_group_box>( "Angles" );
			{
				auto stand_tab = std::make_shared<c_group_tab>( "Stand" ); //todo: reformat this to look a bit cleaner
				{
					stand_tab->add_child( std::make_shared<c_toggle>( "Enabled", &settings::hvh::antiaim::enabled ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "Body", &settings::hvh::antiaim::body_fake_stand, std::vector<const char *> { "Off", "Left", "Right", "Opposite", "Z" } ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "Fake", &settings::hvh::antiaim::fake_yaw, std::vector<const char *> { "Off", "Default", "Relative", "Relative jitter", "Rotate", "Random", "Local view" } ) );
					stand_tab->add_child( std::make_shared<c_slider>( "Fake relative", &settings::hvh::antiaim::fake_relative, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "Fake jitter range", &settings::hvh::antiaim::fake_jitter_range, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "Pitch", &settings::hvh::antiaim::pitch_stand, std::vector<const char *> { "Off", "Down", "Up", "Random" } ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "Yaw offset", &settings::hvh::antiaim::yaw_stand, std::vector<const char *> { "Off", "Direction", "Jitter", "Rotate", "Random", "Distort" }));
					stand_tab->add_child( std::make_shared<c_combobox>( "Yaw dir", &settings::hvh::antiaim::dir_stand, std::vector<const char *> { "Direction", "Backwards", "Left", "Right" } ) );
					stand_tab->add_child( std::make_shared<c_slider>( "Stand jitter range", &settings::hvh::antiaim::jitter_range_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "Rot range", &settings::hvh::antiaim::rot_range_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "Rot speed", &settings::hvh::antiaim::rot_speed_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_slider>( "Random update speed", &settings::hvh::antiaim::rand_update_stand, -180, 180 ) );
					stand_tab->add_child( std::make_shared<c_combobox>( "Base angle", &settings::hvh::antiaim::base_angle_stand, std::vector<const char *> { "Off", "Static", "Away crosshair", "Away distance" } ) );
				} antiaim_tab->add_child( stand_tab );
				auto move_tab = std::make_shared<c_group_tab>( "Move" );
				{
					move_tab->add_child( std::make_shared<c_toggle>( "Enabled", &settings::hvh::antiaim::enabled ) );
					move_tab->add_child( std::make_shared<c_combobox>( "Fake", &settings::hvh::antiaim::fake_yaw, std::vector<const char *> { "Off", "Default", "Relative", "Relative jitter", "Rotate", "Random", "Local view" } ) );
					move_tab->add_child( std::make_shared<c_slider>( "Fake relative", &settings::hvh::antiaim::fake_relative, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "Fake jitter range", &settings::hvh::antiaim::fake_jitter_range, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_combobox>( "Pitch", &settings::hvh::antiaim::pitch_walk, std::vector<const char *> { "Off", "Down", "Up", "Random" } ) );
					move_tab->add_child( std::make_shared<c_combobox>( "Yaw offset walking", &settings::hvh::antiaim::yaw_walk, std::vector<const char *> { "Off", "Direction", "Jitter", "Rotate", "Random", "Break" } ) );
					move_tab->add_child( std::make_shared<c_combobox>( "Yaw dir walking", &settings::hvh::antiaim::dir_walk, std::vector<const char *> { "Direction", "Backwards", "Left", "Right" } ) );
					move_tab->add_child( std::make_shared<c_slider>( "Walk jitter range", &settings::hvh::antiaim::jitter_range_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "Rot range walk", &settings::hvh::antiaim::rot_range_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "Rot speed walk", &settings::hvh::antiaim::rot_speed_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_slider>( "Random update speed walk", &settings::hvh::antiaim::rand_update_walk, -180, 180 ) );
					move_tab->add_child( std::make_shared<c_combobox>( "Base angle walk", &settings::hvh::antiaim::base_angle_walk, std::vector<const char *> { "Off", "Static", "Away crosshair", "Away distance" } ) );

				} antiaim_tab->add_child( move_tab );
				auto air_tab = std::make_shared<c_group_tab>( "Air" );
				{
					air_tab->add_child( std::make_shared<c_toggle>( "Enabled", &settings::hvh::antiaim::enabled ) );
					air_tab->add_child( std::make_shared<c_combobox>( "Body", &settings::hvh::antiaim::body_fake_air, std::vector<const char *> { "Off", "Left", "Right", "Opposite" } ) );
					air_tab->add_child( std::make_shared<c_combobox>( "Fake", &settings::hvh::antiaim::fake_yaw, std::vector<const char *> { "Off", "Default", "Relative", "Relative jitter", "Rotate", "Random", "Local view" } ) );
					air_tab->add_child( std::make_shared<c_slider>( "Fake relative", &settings::hvh::antiaim::fake_relative, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "Fake jitter range", &settings::hvh::antiaim::fake_jitter_range, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_combobox>( "Pitch", &settings::hvh::antiaim::pitch_air, std::vector<const char *> { "Off", "Down", "Up", "Random" } ) );
					air_tab->add_child( std::make_shared<c_combobox>( "Yaw offset", &settings::hvh::antiaim::yaw_air, std::vector<const char *> { "Off", "Direction", "Jitter", "Rotate", "Random" } ) );
					air_tab->add_child( std::make_shared<c_combobox>( "Yaw dir", &settings::hvh::antiaim::dir_air, std::vector<const char *> { "Direction", "Backwards", "Left", "Right" } ) );
					air_tab->add_child( std::make_shared<c_slider>( "Air jitter range", &settings::hvh::antiaim::jitter_range_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "Rot range", &settings::hvh::antiaim::rot_range_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "Rot speed", &settings::hvh::antiaim::rot_speed_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_slider>( "Random update speed", &settings::hvh::antiaim::rand_update_air, -180, 180 ) );
					air_tab->add_child( std::make_shared<c_combobox>( "Base angle", &settings::hvh::antiaim::base_angle_air, std::vector<const char *> { "Off", "Static", "Away crosshair", "Away distance" } ) );

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
					general_tab->add_child( std::make_shared<c_toggle>( "Lag", &settings::hvh::antiaim::lag_enable ) );
					general_tab->add_child( std::make_shared<c_multiselect>( "Lag activation", &settings::hvh::antiaim::lag_active, std::vector<const char *> { "Move", "Air", "Crouch" } ) );
					general_tab->add_child( std::make_shared<c_combobox>( "Lag mode", &settings::hvh::antiaim::lag_mode, std::vector<const char *> { "Max", "Random", "Break step", "Rotate", "Random", "Peek" } ) );
					general_tab->add_child( std::make_shared<c_slider>( "Lag limit", &settings::hvh::antiaim::lag_limit, 0, 15 ) );
				}
				lag_tab->add_child( general_tab );
			}
			right_column->add_child( lag_tab );
			auto misc_tab = std::make_shared<c_group_box>( "Misc" );
			{
				auto general_tab = std::make_shared<c_group_tab>( "" );
				{
					general_tab->add_child(std::make_shared<c_key_bind>("Fake walk", &settings::hvh::antiaim::fakewalk, 1));
					general_tab->add_child(std::make_shared<c_key_bind>("Fake head", &settings::hvh::antiaim::fakehead, 1));
					general_tab->add_child( std::make_shared<c_key_bind>( "Auto peek", &settings::hvh::antiaim::auto_peek, 1 ) );
				}
				misc_tab->add_child( general_tab );
			} right_column->add_child( misc_tab );
		} hvh_form->add_child( right_column );
	} main_form->add_child( hvh_form );
	
	auto misc_form = std::make_shared<c_tab>( "Misc");
	{
		auto left_column = std::make_shared<c_column>( );
		{
			auto movement_group = std::make_shared<c_group_box>( "Movement" );
			{
				auto movement_tab = std::make_shared<c_group_tab>( "Movement" );
				movement_tab->add_child( std::make_shared<c_toggle>( "Bhop", &settings::misc::movement::bhop ) );
				movement_tab->add_child( std::make_shared<c_key_bind>( "Pre speed", &settings::misc::movement::pre_speed, key_bind_type_toggle ) );
				movement_tab->add_child( std::make_shared<c_combobox>( "Auto Strafer", &settings::misc::movement::autostrafe, std::vector<const char *> { "Off", "Normal", "Directional" } ) );
				movement_group->add_child( movement_tab );
			}
			left_column->add_child( movement_group );
		} misc_form->add_child( left_column );
		auto right_column = std::make_shared<c_column>( );
		{
			auto griefing_tab = std::make_shared<c_group_box>( "Griefing" );
			{
				auto movement_tab = std::make_shared<c_group_tab>( "Movement" );
				movement_tab->add_child( std::make_shared<c_key_bind>( "Block bot", &settings::misc::griefing::block_bot, key_bind_type_toggle ) );
				griefing_tab->add_child( movement_tab );
			}
			right_column->add_child( griefing_tab );
			auto misc_tab = std::make_shared<c_group_box>( "Misc" );
			{
				auto misc_tab2 = std::make_shared<c_group_tab>( "Misc" ); {
					misc_tab2->add_child( std::make_shared<c_toggle>( "Ping exploit", &settings::misc::misc::fake_latency ) );
					misc_tab2->add_child( std::make_shared<c_slider>( "Ping amount", &settings::misc::misc::fake_latency_amt, 0, 1000 ) );
					misc_tab2->add_child( std::make_shared<c_key_bind>( "Thirdperson", &settings::misc::misc::thirdperson, 2 ) );
					misc_tab->add_child( misc_tab2 );
				}
				auto movement_tab = std::make_shared<c_group_tab>( "Config" ); {
					movement_tab->add_child( std::make_shared<c_combobox>( "Config", &settings::misc::config::slot, std::vector<const char*> { "Auto", "Scout", "Pistol" } ) );
					movement_tab->add_child( std::make_shared<c_button>( "Save", [ ] {
						std::string name = config_names[ settings::misc::config::slot ];
						std::ofstream file( name.c_str( ) );
						std::streambuf* coutbuf = std::cout.rdbuf( ); //save old buf
						std::cout.rdbuf( file.rdbuf( ) ); //redirect std::cin to in.txt!

						if ( file.good( ) ) {
							menu.save( );
						}

						std::cout.rdbuf( coutbuf ); //reset to standard output again

						file.close( );
						} ) );
					movement_tab->add_child( std::make_shared<c_button>( "Load", [ ] {
						std::string name = config_names[ settings::misc::config::slot ];
						std::ifstream in( name.c_str( ) );
						std::streambuf* cinbuf = std::cin.rdbuf( ); //save old buf
						std::cin.rdbuf( in.rdbuf( ) ); //redirect std::cin to in.txt!

						menu.load( );

						std::cin.rdbuf( cinbuf ); //reset to standard output again
						in.close( );
						} ) );
					misc_tab->add_child( movement_tab );
				}
			}
			right_column->add_child( misc_tab );
		} misc_form->add_child( right_column );
	} main_form->add_child( misc_form );
}

auto c_menu::draw( ) -> void {
	if ( !open )
		return;

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

auto c_menu::update( ) -> void {
	if ( input_helper.key_pressed( VK_INSERT ) )
		open = !open;
	if ( !open )
		return;
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
