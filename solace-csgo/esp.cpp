#include "esp.h"
#include "sdk.h"
#include "includes.h"
#include "menu.hh"
#include "delaunator.h"

#define render g.m_render
void esp_t::run ( ) {
	if ( !g.m_local )
		return;
	auto weapon = static_cast< weapon_t * >(g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->active_weapon( ) ));
	if( weapon )
		render->text( render->m_tahoma_12( ), 100, 100, color( 255, 255, 255 ), std::to_string( weapon->index( ) ).c_str( ) );
	for ( auto i = 0; i < g.m_interfaces->entity_list( )->get_highest_index( ); i++ ) {
		auto *const ent = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( i ) );
		if ( !ent )
			continue;
		auto *networkable = ent->networkable( );
		if ( !networkable )
			continue;
		const auto client_class_id = networkable->client_class( );
		if ( client_class_id->m_ClassID == 35 )
			player( ent );
		else if ( strcmp( client_class_id->m_pNetworkName, "CInferno" ) == 0 )
			inferno( ent );
		else if ( strcmp( client_class_id->m_pNetworkName, "CSmokeGrenadeProjectile" ) == 0 )
			smoke( ent );
		else if ( strcmp( client_class_id->m_pNetworkName, "CMolotovProjectile" ) == 0 )
			molotov( ent );
		else if ( strcmp( client_class_id->m_pNetworkName, "CBaseCSGrenadeProjectile" ) == 0 )
			flying_grenade( ent );
	}
}

void esp_t::smoke( entity_t* ent ) {
	if ( !settings::visuals::world::wire_smoke )
		return;
	if ( ((weapon_t*)ent)->smoke_effect_begin_tick() )
		g.m_render->world_circle( ent->abs_origin( ), 144, menu.main_theme );
	else {
		vec3_t screen;
		if ( !math::world_to_screen( ent->abs_origin( ), screen ) )
			return;
		g.m_render->text( g.m_render->m_tahoma_14( ), screen.x, screen.y, menu.dark_accent, "SMOKE", Horizontal | Vertical );
	}
}

void esp_t::flying_grenade( entity_t *ent ) {
	const model_t* model = ent->model( );

	if ( model ) {
		// grab modelname.
		std::string name{ model->name };

		vec3_t screen;
		if ( !math::world_to_screen( ent->abs_origin( ), screen ) )
			return;

		if ( name.find( "flashbang" ) != std::string::npos )
			render->text( render->m_tahoma_14( ), screen.x, screen.y, menu.dark_accent, "FLASH", Horizontal | Vertical );

		else if ( name.find( "fraggrenade" ) != std::string::npos ) {
			render->text(render->m_tahoma_14(), screen.x, screen.y, menu.dark_accent, "FRAG", Horizontal | Vertical );
		}
	}
}

void esp_t::molotov( entity_t* ent ) {
	if ( !settings::visuals::world::molotov )
		return;
	vec3_t screen;
	if ( !math::world_to_screen( ent->abs_origin( ), screen ) )
		return;
	g.m_render->text( g.m_render->m_tahoma_14( ), screen.x, screen.y, menu.dark_accent, "MOLOTOV", Horizontal | Vertical );
}

void esp_t::inferno(entity_t* ent) {
	if (!settings::visuals::world::molotov)
		return;
	auto* inferno = reinterpret_cast<inferno_t*>(ent);
	auto* thrower = static_cast<player_t*>(g.m_interfaces->entity_list()->get_client_entity_handle(inferno->m_thrower()));
	bool team = false;
	if (thrower)
		team = thrower->on_team(g.m_local);
	const auto origin = ent->origin();
	auto* const fire_x = inferno->m_fire_x();
	auto* const fire_y = inferno->m_fire_y();
	auto* const fire_z = inferno->m_fire_z();
	auto* const burning = inferno->m_fire_burning();
	const int fire_count = inferno->m_fire_count();
	
	std::vector<double> points = {};
	for (auto k = 0; k < fire_count; k++) {
		if (!burning[k])
			continue;
		const auto point = vec3_t{ static_cast<float>(fire_x[k]), static_cast<float>(fire_y[k]), static_cast<float>(fire_z[k]) };
		vec3_t screen = origin + point;
		//			m_render->text( m_render->m_tahoma_12( ), screen.x, screen.y, color( 255, 255, 255 ), std::to_string( k ).c_str( ) );
		points.push_back(screen.x);
		points.push_back(screen.y);
		points.push_back(screen.z);
	}
	if (points.size() < 9)
		return;
	delaunator::Delaunator delaunator(points);

	menu.main_theme.set_a( 100 );
	menu.dark_accent.set_a( 100 );
	color point_color = team ? menu.main_theme : menu.dark_accent;
	menu.main_theme.set_a( 255 );
	menu.dark_accent.set_a( 255 );
	render_t::vertex_t verts[3] = {};
	vec3_t point;
	for (std::size_t i = 0; i < delaunator.triangles.size(); i += 3) {
	
		point = vec3_t(static_cast<float>(delaunator.coords[3 * delaunator.triangles[i]]), static_cast<float>(delaunator.coords[3 * delaunator.triangles[i] + 1]), static_cast<float>(delaunator.coords[3 * delaunator.triangles[i] + 2]));
		vec3_t screen;
		if (!math::world_to_screen(point, screen))
			continue;
		verts[0] = render_t::vertex_t{ screen.x, screen.y, 0, 1, point_color };
	
		point = vec3_t(static_cast<float>(delaunator.coords[3 * delaunator.triangles[i + 1]]), static_cast<float>(delaunator.coords[3 * delaunator.triangles[i + 1] + 1]), static_cast<float>(delaunator.coords[3 * delaunator.triangles[i + 1] + 2]));
		if (!math::world_to_screen(point, screen))
			continue;
		verts[1] = render_t::vertex_t{ screen.x, screen.y, 0, 1, point_color };
	
		point = vec3_t(static_cast<float>(delaunator.coords[3 * delaunator.triangles[i + 2]]), static_cast<float>(delaunator.coords[3 * delaunator.triangles[i + 2] + 1]), static_cast<float>(delaunator.coords[3 * delaunator.triangles[i + 2] + 2]));
		if (!math::world_to_screen(point, screen))
			continue;
		verts[2] = render_t::vertex_t{ screen.x, screen.y, 0, 1, point_color };
	
		g.m_render->render_triangle(verts, 1);
	
		g.m_render->line(verts[0].x, verts[0].y, verts[1].x, verts[1].y, color(255, 255, 255, 30), 1);
		g.m_render->line(verts[2].x, verts[2].y, verts[0].x, verts[0].y, color(255, 255, 255, 30), 1);
		g.m_render->line(verts[2].x, verts[2].y, verts[1].x, verts[1].y, color(255, 255, 255, 30), 1);
	}

	//for ( std::size_t i = 0; i < delaunator.halfedges.size( ); i += 3 ) {
	//	//verts[ 0 ] = ( render_t::vertex_t{ static_cast< float >( delaunator.coords[ 2 * delaunator.halfedges[ i ] ] ), static_cast< float >( delaunator.coords[ 2 * delaunator.halfedges[ i ] + 1 ] ), 0, 1, color( 180, 0,0, 100 ) } );
	//	//verts[ 1 ] = ( render_t::vertex_t{ static_cast< float >( delaunator.coords[ 2 * delaunator.halfedges[ i + 1 ] ] ), static_cast< float >( delaunator.coords[ 2 * delaunator.halfedges[ i + 1 ] + 1 ] ), 0, 1, color( 180, 0,0, 100 ) } );
	//	g.m_render->line( static_cast< float >(delaunator.coords[3 * delaunator.halfedges[i]]),
	//	                  static_cast< float >(delaunator.coords[3 * delaunator.halfedges[i] + 1]),
	//	                  static_cast< float >(delaunator.coords[3 * delaunator.halfedges[i + 1]]),
	//	                  static_cast< float >(delaunator.coords[3 * delaunator.halfedges[i + 1] + 1]),
	//	                  color( 180, 0, 0, 100 ) );
	//}
}

void esp_t::NoSmoke( ) {
	static i_material *smoke1;
	static i_material *smoke2;
	static i_material *smoke3;
	static i_material *smoke4;
	if ( !smoke1 )
		smoke1 = g.m_interfaces->material_system(  )->find_material(  "particle/vistasmokev1/vistasmokev1_fire", "Other textures" );

	if ( !smoke2 )
		smoke2 = g.m_interfaces->material_system( )->find_material( "particle/vistasmokev1/vistasmokev1_smokegrenade" ,  "Other textures"  );

	if ( !smoke3 )
		smoke3 = g.m_interfaces->material_system( )->find_material(  "particle/vistasmokev1/vistasmokev1_emods" ,  "Other textures"  );

	if ( !smoke4 )
		smoke4 = g.m_interfaces->material_system( )->find_material( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ,  "Other textures"  );

	if ( settings::visuals::world::wire_smoke ) {
		if ( !smoke1->get_material_var_flag( material_var_wireframe ) )
			smoke1->set_material_var_flag( material_var_wireframe, true );

		if ( !smoke2->get_material_var_flag( material_var_wireframe ) )
			smoke2->set_material_var_flag( material_var_wireframe, true );

		if ( !smoke3->get_material_var_flag( material_var_wireframe ) )
			smoke3->set_material_var_flag( material_var_wireframe, true );

		if ( !smoke4->get_material_var_flag( material_var_wireframe ) )
			smoke4->set_material_var_flag( material_var_wireframe, true );
	}

	else {
		if ( smoke1->get_material_var_flag( material_var_wireframe ) )
			smoke1->set_material_var_flag( material_var_wireframe, false );

		if ( smoke2->get_material_var_flag( material_var_wireframe ) )
			smoke2->set_material_var_flag( material_var_wireframe, false );

		if ( smoke3->get_material_var_flag( material_var_wireframe ) )
			smoke3->set_material_var_flag( material_var_wireframe, false );

		if ( smoke4->get_material_var_flag( material_var_wireframe ) )
			smoke4->set_material_var_flag( material_var_wireframe, false );
	}
}

bool esp_t::get_player_box( player_t *player, area_t *box ) {
	vec3_t bottom, top;

	// get hitbox bounds.
	auto origin = player->origin( );
	auto mins = player->mins( );
	auto maxs = player->maxs( );
	auto &info = g_player_manager.m_ents[ player->index( ) - 1 ];
	if ( !info.m_records.empty(  ) ) {
		auto const &record = info.m_records[ 0 ];
		if ( record ) {
			origin = record->m_origin;
			mins = record->m_mins;
			maxs = record->m_maxs;
		}
	}

	// correct x and y coordinates.
	mins = { origin.x, origin.y, origin.z + mins.z - 0.8f };
	maxs = { origin.x, origin.y, origin.z + maxs.z };

	if ( !math::world_to_screen( mins, bottom ) || !math::world_to_screen( maxs, top ) )
		return false;

	box->h = floorf(bottom.y - top.y);
	box->w = ceilf(box->h / 2.f);
	box->x = floorf(bottom.x - box->w / 2.f);
	box->y = ceilf(bottom.y - box->h);
	auto size = g.m_render->m_screen_size();
	if (box->h + box->y < size.Y)
		return false;
	if (box->x + box->w < size.X)
		return false;

	if ( box->y > size.Y + size.Height )
		return false;
	if ( box->x > size.X + size.Width)
		return false;

	return true;
}

void esp_t::offscreen( player_t *ent ) {
	if ( !g.m_local )
		return;
	const auto vec_delta = ent->origin( ) - g.m_shoot_pos;
	const auto yaw = RAD2DEG( atan2( vec_delta.y, vec_delta.x ) );
	ang_t angles;
	g.m_interfaces->engine( )->get_view_angles( angles );
	const auto yaw_delta = DEG2RAD((angles.y - yaw) - 90.f );

	const auto clr = ent->on_team( g.m_local ) ? color{ 0xFF, 0x08, 0xFF } : color{ 0x81, 0xFF, 0x21 };
	auto r_x = settings::visuals::players::offscreen, r_y = settings::visuals::players::offscreen;
	const auto screen_size = g.m_render->m_screen_size( );
	r_x *= screen_size.Width / 2.f;
	r_y *= screen_size.Height / 2.f;

	const auto triangle_size = 15.f;
	
	render_t::vertex_t triag[ 3 ] = {
		{screen_size.Width / 2.f + cos( yaw_delta ) * ( r_x ), screen_size.Height / 2.f + sin( yaw_delta ) * ( r_y ),0, 1, clr},
		{screen_size.Width / 2.f + ( cos( yaw_delta ) * ( r_x - triangle_size ) ) + ( cos( yaw_delta + DEG2RAD(90) ) * triangle_size ), screen_size.Height / 2.f + ( sin( yaw_delta ) * ( r_y - triangle_size ) + ( sin( yaw_delta + DEG2RAD(90) ) * triangle_size ) ),0, 1, clr},
		{screen_size.Width / 2.f + ( cos( yaw_delta ) * ( r_x - triangle_size ) ) + ( cos( yaw_delta + DEG2RAD(90) ) * -triangle_size ), screen_size.Height / 2.f + ( sin( yaw_delta ) * ( r_y - triangle_size ) + ( sin( yaw_delta + DEG2RAD(90) ) * -triangle_size ) ),0, 1, clr},
	};
	
	g.m_render->render_triangle( triag, 1 );
}

void esp_t::player ( player_t *ent ) {
	if ( !ent->alive( ) )
		return;
	if ( ent->dormant( ) )
		return;
	const auto on_team = ent->on_team( g.m_local );
	if ( on_team && !settings::visuals::players::team )
		return;
	area_t box(0,0,0,0);
	if ( !get_player_box( ent, &box ) ) {
		offscreen( ent );
		return;
	}
	if ( settings::visuals::players::box ) {
		g.m_render->outlined_rect( box.x - 1, box.y - 1, box.w + 2, box.h + 2, color( 0, 0, 0, 200 ) );
		g.m_render->outlined_rect( box.x, box.y, box.w, box.h, on_team ? color{ 0xFF, 0x08, 0xFF } : color{ 0x81, 0xFF, 0x21 } );
		g.m_render->outlined_rect( box.x + 1, box.y + 1, box.w - 2, box.h - 2, color( 0, 0, 0, 200 ) );
	}
    if (settings::visuals::players::health)
    {
        const area_t health_box(box.x - 7, box.y, 3, box.h);
		
		g.m_render->outlined_rect( health_box.x - 1, health_box.y - 1, health_box.w + 2, health_box.h + 2, color( 50, 50, 50, 150 ) );
		g.m_render->filled_rect( health_box.x, health_box.y, health_box.w, health_box.h, color( 40, 40, 40 ));
		const auto health_percent = static_cast< float >( ent->health( ) ) / 100.f;
		g.m_render->filled_rect( health_box.x,
		                         health_box.y + health_box.h * (1 - health_percent ),
		                         health_box.w, health_box.h * health_percent,
		                         color( 255.f *(1 - health_percent),
		                                255.f * health_percent, 0 ) );
	}
	if ( settings::visuals::players::name ) {
		engine_player_info_t info{};
		g.m_interfaces->engine( )->get_player_info( ent->index( ), &info );
		const auto height = g.m_render->get_text_height( info.name, g.m_render->m_tahoma_12( ) );
		g.m_render->text( g.m_render->m_tahoma_12( ), box.x + box.w / 2.f, box.y - (height + 1), color( 255, 255, 255 ), info.name, Horizontal );
	}
	if ( settings::visuals::players::weapon ) {
		auto *weapon = static_cast< weapon_t * >(g.m_interfaces->entity_list( )->get_client_entity_handle( ent->active_weapon( ) ));
		if ( weapon ) {
			auto *const weapon_data = g.m_interfaces->weapon_system( )->get_weapon_data( weapon->item_definition_index( ) );
			if ( weapon_data ) {
				std::string name = weapon_data->m_weapon_name;
				if ( name.find( "weapon_" ) != std::string::npos )
					name = name.substr( 7 );
				g.m_render->text( g.m_render->m_tahoma_12( ), box.x + box.w / 2.f, box.y + box.h + 1, color( 255, 255, 255 ), name.c_str( ), Horizontal );
			}
		}
	}
	//	auto& info = g_player_manager.m_ents[ent->index() - 1];
	//	if (!info.m_records.empty()) {
	//		auto const& record = info.m_records[0];
	//		if ( record ) {
	//			std::string name = std::to_string( record->m_anim_velocity.length( ) );
	//			g.m_render->text( g.m_render->m_tahoma_12( ), box.x + box.w / 2.f, box.y + box.h + 1, color( 255, 255, 255 ), name.c_str( ), 1 );
	//		}
	//	}
}

void esp_t::weapon ( entity_t *ent ) {
	
}
