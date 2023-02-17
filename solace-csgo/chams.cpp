#include "chams.h"

#include "aimbot.h"
#include "includes.h"
#include "resolver.h"

void chams_t::create_materials( ) {
	full = g.m_interfaces->material_system()->find_material( "debug/debugambientcube", "Model textures" );
	full->increment_reference_count( );

	flat = g.m_interfaces->material_system( )->find_material( "debug/debugdrawflat", "Model textures" );
	flat->increment_reference_count( );
}

void chams_t::reset() {
	g.m_interfaces->model_render( )->override_material( nullptr );
	g.m_interfaces->render_view( )->modulate_color( color( 255, 255, 255 ) );
	g.m_interfaces->render_view( )->set_blend( 1.f );
}

bool chams_t::IsInViewPlane( const vec3_t &world ) {
	const auto &matrix = g.m_interfaces->engine(  )->world_to_screen(  );

	const auto w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];

	return w > 0.001f;
}

bool chams_t::SortPlayers( ) {
	// lambda-callback for std::sort.
	// to sort the players based on distance to the local-player.
	static auto distance_predicate = [ ]( player_t *a, player_t *b ) {
		auto local = g.m_local->abs_origin( );

		// note - dex; using squared length to save out on sqrt calls, we don't care about it anyway.
		auto len1 = ( a->abs_origin( ) - local ).length_sqr( );
		auto len2 = ( b->abs_origin( ) - local ).length_sqr( );

		return len1 < len2;
	};

	// reset player container.
	m_players.clear( );

	// find all players that should be rendered.
	for ( auto i{ 1 }; i <= g.m_interfaces->globals()->m_max_clients; ++i ) {
		// get player ptr by idx.
		auto player = static_cast< player_t * >(g.m_interfaces->entity_list( )->get_client_entity( i ));

		// validate.
		if ( !player || !player->is_player( ) || !player->alive( ) || player->dormant( ) || player == g.m_local )
			continue;

		// do not draw players occluded by view plane.
		if ( !IsInViewPlane( player->world_space_center( ) ) )
			continue;

		m_players.push_back( player );
	}

	// any players?
	if ( m_players.empty( ) )
		return false;

	// sorting fixes the weird weapon on back flickers.
	// and all the other problems regarding Z-layering in this shit game.
	std::sort( m_players.begin( ), m_players.end( ), distance_predicate );

	return true;
}
void chams_t::SceneEnd( ) {
	g.m_rendering = true;
	// store and sort ents by distance.
	if ( SortPlayers( ) ) {
		// iterate each player and render them.
		for ( const auto &p : m_players )
			p->draw_model( );
	}

	// restore.
	g.m_interfaces->model_render(  )->override_material( nullptr );
	g.m_interfaces->render_view( )->modulate_color( color(255,255,255,255));
	g.m_interfaces->render_view( )->set_blend( 1.f );
	g.m_rendering = false;
}

void chams_t::player( player_t *player, uintptr_t ctx, void *state, const model_render_info_t &info, matrix_t *bone ) {
	if ( !g.m_rendering && (player != g.m_local || player == nullptr) )
		return;
	static float flash_alpha = 0;

	static auto draw_model = reinterpret_cast< void( __thiscall * )( void *, uintptr_t, void *, const model_render_info_t &, matrix_t * ) >( g.m_interfaces->model_render( ).hook( )->get_original( 21 ) );
	if ( player == g.m_local ) {
		if ( g.m_bones_setup && g.m_running_client ) {
			const auto abs_origin = g.m_local->abs_origin( );
			for ( auto i = 0; i < 128; i++ ) {
				g.m_real_bones[i].mat_val[ 0 ][ 3 ] += abs_origin.x;
				g.m_real_bones[i].mat_val[ 1 ][ 3 ] += abs_origin.y;
				g.m_real_bones[i].mat_val[ 2 ][ 3 ] += abs_origin.z;
			}
			draw_model( g.m_interfaces->model_render( ), ctx, state, info, g.m_real_bones );
			for ( auto i = 0; i < 128; i++ ) {
				g.m_real_bones[ i ].mat_val[ 0 ][ 3 ] -= abs_origin.x;
				g.m_real_bones[ i ].mat_val[ 1 ][ 3 ] -= abs_origin.y;
				g.m_real_bones[ i ].mat_val[ 2 ][ 3 ] -= abs_origin.z;
			}
		}
		else
			draw_model( g.m_interfaces->model_render( ), ctx, state, info, bone );
		return;
	}
	
	auto ent_info = &g_player_manager.m_ents[ player->index( ) - 1 ];
    auto auto_draw = !ent_info->m_valid || ent_info->m_teamate || ent_info->m_records.empty() ||
                     !ent_info->m_records.front() || !ent_info->m_records.front()->m_setup;
	if ( auto_draw ) {
		if ( ent_info->m_teamate ) {
			if ( settings::visuals::players::chams_team_covered != 0 ) {
				( (settings::visuals::players::chams_team_covered == 1) ? full : flat )->set_material_var_flag( material_var_ignorez, true );
				g.m_interfaces->model_render( )->override_material( settings::visuals::players::chams_team_covered == 1 ? full : flat );
				g.m_interfaces->render_view( )->modulate_color( color( 0x57, 0x03, 0x57 ) );
				g.m_interfaces->render_view( )->set_blend( 1.f );

				draw_model( g.m_interfaces->model_render( ), ctx, state, info, bone );
			}

			if ( settings::visuals::players::chams_team != 0 ) {
				( settings::visuals::players::chams_team == 1 ? full : flat )->set_material_var_flag( material_var_ignorez, false );
				g.m_interfaces->model_render( )->override_material( settings::visuals::players::chams_team == 1 ? full : flat );
				g.m_interfaces->render_view( )->modulate_color( color( 0xFF, 0x08, 0xFF ) );
				g.m_interfaces->render_view( )->set_blend( 1.f );

				draw_model( g.m_interfaces->model_render( ), ctx, state, info, bone );
			}

			reset( );

			if ( settings::visuals::players::chams_team == 0 ) {
				draw_model( g.m_interfaces->model_render( ), ctx, state, info, bone );
			}
		}
		else
			draw_model( g.m_interfaces->model_render( ), ctx, state, info, bone );
		return;
	}

	if ( settings::visuals::players::chams_covered != 0 ) {
		( settings::visuals::players::chams_covered == 1 ? full : flat )->set_material_var_flag( material_var_ignorez, true );
		( settings::visuals::players::chams_covered == 1 ? full : flat )->set_material_var_flag( material_var_nocull, true );
		g.m_interfaces->model_render( )->override_material( settings::visuals::players::chams_covered == 1 ? full : flat );
		g.m_interfaces->render_view( )->modulate_color( color( 0x4F,0x9C,0x14 ) );
		g.m_interfaces->render_view( )->set_blend( 1.f );

		draw_model(g.m_interfaces->model_render(), ctx, state, info, ent_info->m_records.front()->m_bones);
		
		g.m_interfaces->render_view( )->set_blend( 0.5f );
		auto const record = g_aimbot.last_record( ent_info );
		if ( record )
			draw_model( g.m_interfaces->model_render( ), ctx, state, info, record->m_bones );
	}

	if ( settings::visuals::players::chams != 0 ) {
		( settings::visuals::players::chams == 1 ? full : flat )->set_material_var_flag( material_var_ignorez, false );
		( settings::visuals::players::chams == 1 ? full : flat )->set_material_var_flag( material_var_nocull, false );
		g.m_interfaces->model_render( )->override_material( settings::visuals::players::chams == 1 ? full : flat );
		g.m_interfaces->render_view( )->modulate_color( color( 0x81, 0xFF, 0x21 ) );
		g.m_interfaces->render_view( )->set_blend( 1.f );
        const auto& front = ent_info->m_records.front();
#ifdef _DEBUG
		if ( front && front->m_setup)
			for( auto &i : front->m_fake_bones )
				draw_model( g.m_interfaces->model_render( ), ctx, state, info, i );
#endif
		
		if (front && front->m_setup)
			draw_model( g.m_interfaces->model_render( ), ctx, state, info, front->m_bones );
		

	}
	
	reset( );
	
	if ( settings::visuals::players::chams == 0 && ent_info->m_records.size() >= 2 ) {
        if (ent_info->m_records[1] && ent_info->m_records[1]->m_setup)
            draw_model(g.m_interfaces->model_render(), ctx, state, info, ent_info->m_records.front()->m_bones);
	}

}