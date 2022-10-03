#include "block_bot.h"
#include "includes.h"
#include "movement.h"

void block_bot_t::on_draw( ) const {
	if ( m_draw.empty( ) )
		return;
	static vec3_t last_world;
	math::world_to_screen( m_draw[ 0 ], last_world );
	for ( auto i = 1; i < m_draw.size( ); i++ ) {
		vec3_t world;
		math::world_to_screen( m_draw[ i ], world );
		g.m_render->line( last_world.x, last_world.y, world.x, world.y, color( 255, 255, 255 ), 1);
		last_world = world;
	}
}

void block_bot_t::friction ( float surface_friction, vec3_t *velocity ) {
	static auto sv_friction = g.m_interfaces->console( )->get_convar( "sv_friction" );
	static auto sv_stopspeed = g.m_interfaces->console( )->get_convar( "sv_stopspeed" );
	auto speed = velocity->length_2d( );
	if ( speed > 0.f ) {
		auto friction = sv_friction->GetFloat(  );
		auto stop_speed = std::max< float >( speed, sv_stopspeed->GetFloat( ) );
		auto control = fminf( speed, stop_speed );
		auto drop = control * friction * surface_friction * g.m_interfaces->globals( )->m_interval_per_tick;
		auto newspeed = speed - drop;
		if ( newspeed < 0 )
			newspeed = 0;

		newspeed /= speed;
		*velocity *= newspeed;

		speed = velocity->length_2d( );
	}
	velocity->z = 0;
}

void block_bot_t::on_tick ( ) {
	return;
	m_draw.clear( );
	if ( !settings::misc::griefing::block_bot ) {
		target = nullptr;
		return;
	}
	if ( !g.m_local )
		return;
	static auto original_origin = vec3_t( );
	static auto original_local = vec3_t( );
	const auto &local_origin = g.m_local->origin( );
	auto valid = [ ]( ent_info_t *ent ) {
		if ( !ent )
			return false;
		if ( !ent->m_valid )
			return false;
		if ( !ent->m_teamate )
			return false;
		auto delta = ent->m_ent->origin( ) - g.m_local->origin( );
		if ( delta.length( ) > 7000.f )
			return false;
		return true;
	};
	
	if ( !valid(target) )
		target = nullptr; // lost target, find someone new

	static auto axis = ang_t( );
	if ( target == nullptr) {
		std::vector<ent_info_t *> possible_targets = {};
		for ( auto &i : g_player_manager.m_ents ) {
			if ( valid( &i ) )
				possible_targets.push_back( &i );
		}
		struct compare {
			vec3_t m_local;
			compare( vec3_t local ) : m_local( local ) { };
			bool operator() ( ent_info_t *struct1, ent_info_t *struct2 ) const {
				return ( struct1->m_ent->origin( ) - m_local ).length_sqr( ) < ( struct2->m_ent->origin( ) - m_local ).length_sqr( );
			}
		};
		std::sort( possible_targets.begin( ), possible_targets.end( ), compare( local_origin ) );
		if ( possible_targets.empty( ) )
			return;
		target = possible_targets[ 0 ];
		auto parellel = static_cast<int>(roundf( local_origin.look( possible_targets[ 0 ]->m_ent->origin( ) ).y ));
		const auto remainder = parellel % 90;
		if ( remainder >= 45 )
			parellel += 90 - remainder;
		else
			parellel -= remainder;
		
		axis = ang_t(0, parellel + 90, 0);
		original_origin = possible_targets[ 0 ]->m_ent->origin( );
		const auto plane = axis.forward( );
		
		original_local = (local_origin + plane * ( original_origin - local_origin ).dot(plane)) / g.m_interfaces->globals( )->m_interval_per_tick;
	}

	if ( target == nullptr )
		return;

	if ( target->m_records.size( ) < 3 )
		return;

	auto target_velocity = target->m_records[ 0 ]->m_velocity;
	auto old_dir = RAD2DEG( std::atan2( target->m_records[1]->m_velocity.y, target->m_records[ 1 ]->m_velocity.x ) );
	auto dir = RAD2DEG( std::atan2( target_velocity.y, target_velocity.x ) );

	auto change = dir - old_dir;

	target_velocity.z = 0;
	// only over predict if they're moving in a consistent direction
	if ( change < 5.f ) {
		auto hyp = target_velocity.length_2d( );
		
		target_velocity.x = std::cos( DEG2RAD( dir + change ) ) * hyp;
		target_velocity.y = std::sin( DEG2RAD( dir + change ) ) * hyp;
	}
	else {
		target_velocity.reset();
	}
	
		
	auto target_origin = (target->m_ent->origin( ) / g.m_interfaces->globals( )->m_interval_per_tick) + target_velocity;
	

	static auto last_origin = local_origin;
	auto velocity = ( local_origin - last_origin ) / g.m_interfaces->globals( )->m_interval_per_tick;
	velocity.z = 0;

	static auto sv_accelerate = g.m_interfaces->console( )->get_convar( "sv_accelerate" );
	friction( g.m_local->surface_friction(  ), &velocity );

	float max_accel = 450;
	auto accel_speed = sv_accelerate->GetFloat(  ) * g.m_interfaces->globals( )->m_interval_per_tick * 450;
	if ( max_accel > accel_speed )
		max_accel = accel_speed;

	if ( auto *ground_ent = static_cast< player_t * >(g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->m_ground_entity( ) ) ) ) {

		if ( ground_ent->index() != target->m_ent->index() ) {
			auto delta = target_origin - original_origin;
			delta.z = 0;
			const auto plane = axis.forward( );
			const auto fraction = delta.dot( plane );
			const auto projected_origin = original_local + plane * fraction;

			auto local_delta = (projected_origin - local_origin) / g.m_interfaces->globals( )->m_interval_per_tick;
			if (g.m_weapon_info && g.m_weapon_info->m_max_player_speed * g.m_weapon_info->m_max_player_speed < local_delta.length_sqr( ) ) {
				auto fRatio = g.m_weapon_info->m_max_player_speed / local_delta.length( );
				local_delta *= fRatio;
			}
			local_delta.z = 0;

			local_delta -= velocity;
			const auto delta_len = local_delta.length_2d( );
			g.m_view_angles = ang_t( 0, static_cast< float >( atan2( local_delta.y, local_delta.x ) * 180.f / M_PI ), 0 );;
			const auto current_speed = velocity.dot( local_delta.normalized( ) );
			auto projected_delta = fminf( 450.f, local_delta.length( ) + current_speed );

			const auto add_speed = projected_delta - current_speed;
			if ( max_accel < add_speed ) {
				//distance is farther than we can account for
				//go as fast as we can
				g.m_cmd->m_forwardmove = 450;
				g.m_cmd->m_sidemove = 0;
			} else {
				// const float kAccelerationScale = MAX( 250.0f, wishspeed );
				// accelspeed = accel * gpGlobals->frametime * kAccelerationScale * player->m_surfaceFriction;
				accel_speed = sv_accelerate->GetFloat( ) * g.m_interfaces->globals( )->m_interval_per_tick * fmaxf( 250, delta_len );

				g.m_cmd->m_forwardmove = 0;
				if ( accel_speed <= add_speed ) {
					projected_delta = delta_len / ( sv_accelerate->GetFloat( ) * g.m_interfaces->globals( )->m_interval_per_tick );
				}
				g.m_cmd->m_forwardmove = projected_delta;
				g.m_cmd->m_sidemove = 0;
			}
		} else {
			//distance to compensate
			auto local_delta = ( target_origin - local_origin / g.m_interfaces->globals( )->m_interval_per_tick ) ;
			local_delta.z = 0;

			if (g.m_weapon_info && g.m_weapon_info->m_max_player_speed * g.m_weapon_info->m_max_player_speed < local_delta.length_sqr( ) ) {
				//adjust to maxspeed
				const auto fRatio = g.m_weapon_info->m_max_player_speed / local_delta.length( );
				local_delta *= fRatio;
			}

			// remove our velocity
			// local_delta is now the desired velocity change 
			local_delta -= velocity;
			
			const auto delta_len = fminf( local_delta.length_2d( ), 450 );
			g.m_view_angles = ang_t( 0, static_cast< float >( atan2( local_delta.y, local_delta.x ) * 180.f / M_PI ), 0 );

			const auto current_speed = velocity.dot( local_delta.normalized( ) );

			auto projected_delta = fminf( 450.f, local_delta.length( ) + current_speed );

			const auto add_speed = projected_delta - current_speed;
			if ( max_accel < add_speed ) {
				//distance is farther than we can account for
				//go as fast as we can
				g.m_cmd->m_forwardmove = 450;
				g.m_cmd->m_sidemove = 0;
			} else {
				// const float kAccelerationScale = MAX( 250.0f, wishspeed );
				// accelspeed = accel * gpGlobals->frametime * kAccelerationScale * player->m_surfaceFriction;
				accel_speed = sv_accelerate->GetFloat( ) * g.m_interfaces->globals( )->m_interval_per_tick * fmaxf( 250, delta_len );

				g.m_cmd->m_forwardmove = 0;
				if ( accel_speed <= add_speed ) {
					//add speed is too high, try to correct the accelspeed
					projected_delta = delta_len / ( sv_accelerate->GetFloat( ) * g.m_interfaces->globals( )->m_interval_per_tick );
				}
				g.m_cmd->m_forwardmove = std::clamp<float>( projected_delta, -450.f, 450.f );
				g.m_cmd->m_sidemove = 0.f;
			}
		}
	}
	else {
		g.m_view_angles = ang_t( 0, local_origin.look( target_origin ).y, 0 );
		g_movement.set_force_strafe( true );
		g.m_cmd->m_forwardmove = 450;
		g.m_cmd->m_sidemove = 0;
	}
	last_origin = local_origin;
}
	
