#include "movement.h"
#include "includes.h"
#include "predictioncopy.h"
#include "render.h"
#include "prediction.h"

void movement::draw() {
	if ( !g.m_running_client )
		return;

	//auto s = std::to_string(jump_count);
	//
	//g.m_render->text(g.m_render->m_tahoma_14(), 960, 540, color(255, 255, 255), s.c_str(), 2);

	if ( settings::hvh::antiaim::auto_peek )
		m_time_left += g.m_interfaces->globals( )->m_abs_frametime;
	else
		m_time_left -= g.m_interfaces->globals( )->m_abs_frametime;
	m_time_left = fminf( 0.5f, fmaxf( m_time_left, 0.f ));
	const auto radius = 17.f * ( m_time_left / 0.5f );
	if ( radius > 0 ) {
		const auto origin = m_stop_pos;
		vec3_t origin_w2s;

		if ( !math::world_to_screen( origin, origin_w2s ) )
			return;

		render_t::vertex_t verts[ 3 ] = {};
		auto point_color = color{ 0xFF, 0x08, 0xFF };
		for ( auto i = 0; i < 360; i+=5 ) {
			auto rot = origin + ang_t( 0, i, 0 ).forward( ) * radius;
			auto rot_2 = origin + ang_t( 0, i + 5.f, 0 ).forward( ) * radius;

			vec3_t point_wts;
			vec3_t point_wts_2;


			if ( !math::world_to_screen( rot, point_wts ) )
				continue;
			if ( !math::world_to_screen( rot_2, point_wts_2 ) )
				continue;
			
			point_color.set_a( 20 );
			verts[ 0 ] = { roundf(point_wts.x), roundf( point_wts.y ), 0, 1, point_color };
			verts[ 1 ] = { roundf( point_wts_2.x), roundf( point_wts_2.y), 0, 1, point_color };
			point_color.set_a( 100 );
			verts[ 2 ] = { roundf( origin_w2s.x ) , roundf( origin_w2s.y ), 0, 1, point_color };
			g.m_render->render_triangle( verts, 1 );
			point_color.set_a( 50 );
			g.m_render->line( roundf( point_wts.x), roundf( point_wts.y), roundf( point_wts_2.x), roundf( point_wts_2.y), point_color, 1 );
		}
	}
}

void movement::bhop( ) {
	if (!settings::misc::movement::bhop) 
		return;
	if (!(g.m_cmd->m_buttons & IN_JUMP))
		return;
	if (g.m_interfaces->entity_list()->get_client_entity_handle(g.m_local->m_ground_entity()))
		return;
	
	g.m_cmd->m_buttons &= ~IN_JUMP;
}

void movement::QuickStop( ) {
	if ( !m_should_stop )
		return;

	set_should_stop( false );

	if ( !g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->m_ground_entity( ) ) )
		return;
	
	static auto sv_friction = g.m_interfaces->console( )->get_convar( "sv_friction" );
	static auto sv_stopspeed = g.m_interfaces->console( )->get_convar( "sv_stopspeed" );
	const auto friction = sv_friction->GetFloat( ) * g.m_local->surface_friction( );

	const auto speed = g.m_local->velocity( ).length( );
	// calculate speed.

	if ( speed <= 0.1f ) {
		g.m_cmd->m_forwardmove = 0.f;
		g.m_cmd->m_sidemove = 0.f;
		return;
	}

	// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
	const auto control = fmaxf( speed, sv_stopspeed->GetFloat( ) );

	// calculate the drop amount.
	const auto drop = control * friction * g.m_interfaces->globals( )->m_interval_per_tick;

	// scale the velocity.
	const auto newspeed = fmaxf( 0.f, speed - drop );

	g.m_view_angles = g.m_local->velocity( ).look( vec3_t( ) );

	if ( newspeed > 0.1f ) {
		g.m_cmd->m_forwardmove = newspeed;
		g.m_cmd->m_sidemove = 0.f;
	} else {
		g.m_cmd->m_forwardmove = 0.f;
		g.m_cmd->m_sidemove = 0.f;
	}
}

void movement::PreciseMove( ) {
	if ( !g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->m_ground_entity( ) ) || settings::hvh::antiaim::fakewalk )
		return;
	if ( g.m_cmd->m_buttons & IN_JUMP )
		return;
	if ( g.m_cmd->m_forwardmove != 0.f || g.m_cmd->m_sidemove != 0.f )
		return;
	static auto sv_friction = g.m_interfaces->console( )->get_convar( "sv_friction" );
	static auto sv_stopspeed = g.m_interfaces->console( )->get_convar( "sv_stopspeed" );
	const auto friction = sv_friction->GetFloat( ) * g.m_local->surface_friction( );

	const auto speed = g.m_local->velocity( ).length( );
	// calculate speed.

	if ( speed <= 0.1f ) {
		g.m_cmd->m_forwardmove = 0.f;
		g.m_cmd->m_sidemove = 0.f;
		return;
	}

	// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
	const auto control = fmaxf( speed, sv_stopspeed->GetFloat( ) );

	// calculate the drop amount.
	const auto drop = control * friction * g.m_interfaces->globals( )->m_interval_per_tick;

	// scale the velocity.
	const auto newspeed = fmaxf( 0.f, speed - drop );
	g.m_view_angles = g.m_local->velocity( ).look( vec3_t( ) );

	if ( newspeed > 0.1f ) {
		g.m_cmd->m_forwardmove = newspeed;
		g.m_cmd->m_sidemove = 0.f;
	} else {
		g.m_cmd->m_forwardmove = 0.f;
		g.m_cmd->m_sidemove = 0.f;
	}
}

void movement::auto_peek( ) {
	// set to invert if we press the button.
	static auto last = false;
	if ( settings::hvh::antiaim::auto_peek ) {
		if ( last != true ) {
			m_stop_pos = g.m_local->origin( );
			m_time_left = 0;
		}
			
		if ( g.m_old_shot )
			m_invert = true;
		 
		if ( m_invert ) {
			move_to( m_stop_pos );
		}
	}

	else {
		m_invert = false;
	}
	last = settings::hvh::antiaim::auto_peek;
}

void movement::move_to(vec3_t target_origin) const {
	const auto local_origin = g.m_local->origin( );
	static auto sv_friction = g.m_interfaces->console( )->get_convar( "sv_friction" );
	static auto sv_stopspeed = g.m_interfaces->console( )->get_convar( "sv_stopspeed" );
	
	auto velocity = g.m_local->velocity(  );
	auto speed = velocity.length_2d( );

	if (speed > 0.1f) {
		const auto friction = sv_friction->GetFloat() * g.m_local->surface_friction();
		const auto control = fmaxf(speed, sv_stopspeed->GetFloat());
		auto drop = control * friction * g.m_interfaces->globals()->m_interval_per_tick;
		auto newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;

		newspeed /= speed;
		velocity *= newspeed;
	}
	
	auto local_delta = ( target_origin - local_origin  ) / g.m_interfaces->globals( )->m_interval_per_tick;

	local_delta -= velocity;
	//if ( g.m_weapon_info && g.m_weapon_info->m_max_player_speed * g.m_weapon_info->m_max_player_speed < local_delta.length_sqr( ) ) {
	//	const auto fRatio = g.m_weapon_info->m_max_player_speed / local_delta.length( );
	//	local_delta *= fRatio;
	//}


	//if (local_delta.length_2d() > 0.1f) {
	//	drop = sv_stopspeed->GetFloat() * friction * g.m_interfaces->globals()->m_interval_per_tick;
	//	newspeed = speed + drop;
	//
	//	local_delta *= newspeed / speed;
	//}
	
	g.m_view_angles = ang_t( 0, static_cast< float >( atan2( local_delta.y, local_delta.x ) * 180.f / M_PI ), 0 );;
	float projected_delta = fminf(450.f, local_delta.length_2d());
	if (projected_delta <= 0.1f ) {
		g.m_cmd->m_forwardmove = 0.f;
		g.m_cmd->m_sidemove = 0;
		return;
	}
	g.m_cmd->m_forwardmove = projected_delta;
	g.m_cmd->m_sidemove = 0.f;
}


bool bCheck() // checks for edgebug
{
	if (g.m_local->velocity().z >= -7.f && floorf(g.m_local->velocity().z) != 7.f && !(g.m_local->flags() & fl_onground))
		return true;
	else
		return false;
}

void movement::edge_bug() {
	return;
	auto* map = g.m_local->GetPredDescMap();

	if (map) {
		const auto size = max(map->m_packed_size, 4);
		static auto startdata = new byte[size];
		auto CopyHelper = CPredictionCopy(PC_EVERYTHING, static_cast<byte*>(startdata), true, reinterpret_cast<const byte*>(g.m_local), false, CPredictionCopy::TRANSFERDATA_COPYONLY);
		CopyHelper.TransferData("edgebug_pre", g.m_local->index(), map);
		float backup_fmove = g.m_cmd->m_forwardmove;
		float backup_smove = g.m_cmd->m_sidemove;
		bool hit = false;

		static bool found = false;
		if (found) {
			if (bCheck() || g.m_local->flags() & fl_onground)
				found = false;
			else {
				g.m_cmd->m_forwardmove = g.m_cmd->m_sidemove = 0.f;
			}
		}


		if (!found) {
			g.m_cmd->m_forwardmove = g.m_cmd->m_sidemove = 0.f;
			bool ran = false;
			vec3_t original_orig = g.m_local->origin();
			for (auto i = 0; i < 1 / (g.m_interfaces->globals()->m_interval_per_tick / 10.f); i++) {
				if (g.m_local->flags() & fl_onground) {
					break;
				}
				ran = true;
				prediction::start(g.m_cmd);
				if (bCheck() && original_orig.z > g.m_local->origin().z) {
					hit = true;
					found = true;
					break;
				}
			}
			if (!hit) {
				g.m_cmd->m_forwardmove = backup_fmove;
				g.m_cmd->m_sidemove = backup_smove;
			}
			if (ran)
				prediction::end();
		}

		CopyHelper = CPredictionCopy(PC_EVERYTHING, reinterpret_cast<byte*>(g.m_local), false, static_cast<const byte*>(startdata), true, CPredictionCopy::TRANSFERDATA_COPYONLY);
		CopyHelper.TransferData("edgebug_post", g.m_local->index(), map);
	}
}

void movement::auto_strafe ( ) {
	if ( settings::misc::movement::autostrafe == 0 )
		return;

	if ( !( g.m_cmd->m_buttons & IN_JUMP ) && !m_force_strafe )
		return;

	m_force_strafe = false; // this will be reset every tick anyways
	
	if ( g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->m_ground_entity( ) ) )
		return;

	const auto velocity = g.m_local->velocity( );

	const float speed = velocity.length_2d( );

	// compute the ideal strafe angle for our velocity.
	const float ideal = ( speed > 0.f ) ? RAD2DEG( std::asin( 15.f / speed ) ) : 90.f;
	const float ideal2 = ( speed > 0.f ) ? RAD2DEG( std::asin( 30.f / speed ) ) : 90.f;

	m_switch *= -1;

	auto direction = vec3_t( );
	if ( g.m_cmd->m_sidemove == 0 && g.m_cmd->m_forwardmove == 0 )
		direction += vec3_t( 1, 0, 0 );

	if ( g.m_cmd->m_forwardmove < 0 )
		direction += vec3_t( -1, 0, 0 );
	else if ( g.m_cmd->m_forwardmove > 0 )
		direction += vec3_t( 1, 0, 0 );

	if ( g.m_cmd->m_sidemove > 0 )
		direction += vec3_t( 0, -1, 0 );
	else if ( g.m_cmd->m_sidemove < 0 )
		direction += vec3_t( 0, 1, 0 );

	g.m_view_angles.y += static_cast< float >(atan2( direction.y, direction.x ) * 180.f / M_PI);

	const auto delta = math::normalize_angle( g.m_view_angles.y - m_old_yaw, 180 );

	// convert to absolute change.
	const auto abs_delta = std::abs( delta );

	g.m_cmd->m_sidemove = 0;
	g.m_cmd->m_forwardmove = 0;
	// set strafe direction based on mouse direction change.
	if ( delta > 0.f )
		g.m_cmd->m_sidemove = -450.f;

	else if ( delta < 0.f )
		g.m_cmd->m_sidemove = 450.f;

	// we can accelerate more, because we strafed less then needed
	// or we got of track and need to be retracked.

	/*
	* data struct
	* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	*/


	if ( abs_delta <= ideal2 || abs_delta >= 30.f ) {
		// compute angle of the direction we are traveling in.
		const auto velocity_angle = RAD2DEG( atan2( velocity.y, velocity.x ) );

		// get the delta between our direction and where we are looking at.
		auto velocity_delta = velocity_angle - g.m_view_angles.y;
		while ( velocity_delta > 180 )
			velocity_delta -= 360;
		while ( velocity_delta < -180 )
			velocity_delta += 360;

		// correct our strafe amongst the path of a circle.
		if ( fabsf( velocity_delta ) > ideal2 ) {
			g.m_view_angles.y = velocity_angle - std::copysignf( ideal2, velocity_delta );
			g.m_cmd->m_sidemove = std::copysignf( 450.f, velocity_delta );
		}
		else {
			g.m_view_angles.y += std::copysignf( ideal, delta );
			g.m_cmd->m_sidemove = std::copysignf( 450.f, delta );
		}
	}

	m_old_yaw = g.m_view_angles.y;
}
