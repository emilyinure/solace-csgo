#include "resolver.h"

#include <memory>


#include "aimbot.h"
#include "penetration.h"
#include "notification.h"
#include "tfm.h"

#include "includes.h"

#ifdef max
#undef max
#endif
#include "thread_handler.h"

shot_record_t::~shot_record_t( ) { 
}

std::shared_ptr<player_record_t> resolver::FindIdealRecord( ent_info_t *data ) {
	if ( data->m_records.empty( ) )
		return nullptr;

	bool set = false;
	std::shared_ptr<player_record_t> first_valid = nullptr;

	// iterate records.
	for ( const auto &it : data->m_records ) {
		auto const &current = it;
		if ( !current || !current->m_setup || current->m_dormant || !current->valid( ) )
			continue;

		// get current record.

		// first record that was valid, store it for later.
		if ( !first_valid )
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if ( it->m_mode == RESOLVE_BODY || it->m_mode == RESOLVE_WALK )
			return current;
	}

	// none found above, return the first valid record if possible.
	return first_valid;
}

void resolver::MatchShot( ent_info_t *data, std::shared_ptr<player_record_t> record ) {
	// do not attempt to do this in nospread mode.
	//if ( g_menu.main.config.mode.get( ) == 1 )
	//	return;

	auto shoot_time = -1.f;

	auto *weapon = static_cast< weapon_t * >(g.m_interfaces->entity_list( )->get_client_entity_handle( data->m_ent->active_weapon( ) ) );
	if ( weapon ) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->last_shot_time( );
	}

	// this record has a shot on it.
	if ( g.time_to_ticks( shoot_time ) > g.time_to_ticks( record->m_sim_time ) - record->m_lag && g.time_to_ticks( shoot_time ) <= g.time_to_ticks( record->m_sim_time ) ) {
		if ( record->m_lag <= 2 )
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else for ( auto i = 1; i < data->m_records.size(); i++)
		{
			if (static_cast<int>(data->m_records.size()) > i) {
				auto const& previous = data->m_records[i];

				if (previous && !previous->m_dormant && !previous->m_shot) {
					record->m_eye_angles.x = previous->m_eye_angles.x;
					record->m_poses[ 12 ] = ( previous->m_eye_angles.x + 90.f ) / 180.f;
					break;
				}
			}
			if (i == static_cast< int >(data->m_records.size( ) - 1)) {
				record->m_poses[ 12 ] = ( 89.f + 90.f ) / 180.f;
				record->m_eye_angles.x = 89.f;
			}
		}
	}
}

void resolver::update_shot_timing ( int sent_tick ) {
	for ( int i = 0; i < m_shots.size(); i++ ) {
		
		const auto delta = g.m_interfaces->globals()->m_curtime - m_shots[ i ]->m_time;

		// fuck this.
		if ( delta > 2.f ) {
			m_shots.erase( m_shots.begin( ) + i );
			i--;
			continue;
		}
		auto& shot = m_shots[ i ];
		if ( !shot->m_updated_time ) {
			shot->m_time += g.ticks_to_time( sent_tick - shot->m_tick );
			shot->m_updated_time = true;
		}
	}
}

void resolver::SetMode(std::shared_ptr<player_record_t> record ) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	const auto speed = record->m_anim_velocity.length( );

	// if on ground, moving, and not fakewalking.
	if ( ( record->m_flags & fl_onground ) && speed > 0.1f && !(record->m_fake_walk || record->m_ukn_vel))
		record->m_mode = Modes::RESOLVE_WALK;

	// if on ground, not moving or fakewalking.
	if ( ( record->m_flags & fl_onground ) && (speed <= 0.1f || record->m_fake_walk || record->m_ukn_vel))
		record->m_mode = Modes::RESOLVE_STAND;

	// if not on ground.
	else if ( !( record->m_flags & fl_onground ) )
		record->m_mode = Modes::RESOLVE_AIR;
}

void resolver::ResolveWalk( ent_info_t *data, std::shared_ptr<player_record_t> record ) {
	// apply lby to eyeangles.


	record->m_eye_angles.y = record->m_body;

	// delay body update.
	data->m_resolver_data.m_body_update_time = record->m_anim_time + 0.22f;

	// reset stand and body index.
	//data->m_stand_index = 0;
	//data->m_stand_index2 = 0;
	data->m_resolver_data.m_mode_data[ resolver_data::LBY_MOVING ].m_index = 0;
	//for ( auto i = 0; i < 8; i++ ) {
	//	data->m_possible_stand_indexs[ i ] = true;
	//	data->m_possible_stand2_indexs[ i ] = true;
	//}

	// copy the last record that this player was walking
	// we need it later on because it gives us crucial data.
	data->m_walk_record = *record;	
}

float resolver::GetAwayAngle(std::shared_ptr<player_record_t> record ) {
	const auto away = g.m_local->origin( ).look( record->m_pred_origin );
	return away.y;
}

void resolver::OnBodyUpdate(ent_info_t* player, float value) {
	// set data.
	player->m_manual_update = false;
	player->m_resolver_data.m_old_body = player->m_resolver_data.m_body;
	player->m_resolver_data.m_body = value;
	if (!player->m_resolver_data.m_body_update) {
		player->m_resolver_data.m_body_update = value != player->m_resolver_data.m_old_body;
		player->m_manual_update = true;
	}
}

float resolver::get_rel(std::shared_ptr<player_record_t> record, int index ) {
	if ( record->m_mode == Modes::RESOLVE_STAND1 ) {
		switch ( index ) {

		case 0:
			return 0;
		case 1:
			return 35;
		case 2:
			return -35;
		case 3:
			return 90;
		case 4:
			return -90;
		case 5:
			return 180;
		case 6:
			return 135;
		case 7:
			return -135;
		default:
			break;
		} 
	} else if ( record->m_mode == Modes::RESOLVE_STAND2 ) {
		switch ( index ) {
		case 0:
			return 180;
		case 1:
			return -135;
		case 2:
			return 135;
		case 3:
			return 0;
		case 4:
			return 90;
		case 5:
			return -90;
		case 6:
			return 45;
		case 7:
			return -45;
		default:
			break;
		}
	}
	return 0;
}

float resolver::get_freestand_yaw( player_t *target ) const {
	vec3_t eye_pos;
	ang_t angle;

	eye_pos = target->origin( ) + vec3_t( 0.f, 0.f, 64.f );
	angle = g.m_shoot_pos.look( eye_pos );

	static auto get_rotated_pos = [ ]( vec3_t start, float rotation, float distance ) {
		float rad = DEG2RAD( rotation );
		start.x += cos( rad ) * distance;
		start.y += sin( rad ) * distance;

		return start;
	};

	weapon_info_t *freestand_rifle = g.m_weapon_info;
	if( !freestand_rifle )
		return angle.y + 180.f;

	float best_rotation = 9999.f;
	float lowest_damage = 9999.f;
	float highest_damage = 0.f;

	for ( float rot = 0.f; rot < 360.f; rot += 45.f ) {
		vec3_t pos = get_rotated_pos( eye_pos, angle.y + rot, -30.f );
		
		penetration::PenetrationInput_t in;
		in.m_from = g.m_local;
		in.m_pos = pos;
		in.m_start = g.m_shoot_pos;
		in.m_target = target;
		in.m_simulated_shot = true;
		penetration::PenetrationOutput_t out;
		penetration::run( &in, &out );
		const float damage = out.m_damage;

		if ( damage > highest_damage ) {
			highest_damage = damage;
		} else if ( damage < lowest_damage ) {
			lowest_damage = damage;
			best_rotation = rot;
		}
	}

	if ( lowest_damage > 0.0f || highest_damage == 0.0f ) {
		return angle.y + 180.f;
	}

	return best_rotation;
}

void resolver::ResolveStand( ent_info_t *data, std::shared_ptr<player_record_t> record ) const {
	// get predicted away angle for the player.
	auto away = GetAwayAngle( record );

	// pointer for easy access.
	auto *move = &data->m_walk_record;
	data->m_resolver_data.m_moved = false;

	// we have a valid moving record.
	if (data->m_manual_update && !record->m_ukn_vel)
	{
		record->m_mode = Modes::RESOLVE_BODY;
		record->m_base_angle = record->m_body;
		record->m_eye_angles.y = record->m_body;
		record->m_body_reliable = true;
		return;
	}
	if ( move->m_sim_time > 0.f ) {
		const auto delta = move->m_origin - record->m_origin;

		// check if moving record is close.
		if ( delta.length_sqr( ) <= 16384.f ) {
			// indicate that we are using the moving lby.
			data->m_resolver_data.m_moved = true;
		}
		
		if (record->m_ukn_vel) 
			data->m_resolver_data.m_body_update_time = -1;

		if ( data->m_resolver_data.m_body_update_time > 0 && record->m_anim_time >= data->m_resolver_data.m_body_update_time && !record->m_ukn_vel ) {
			// only shoot the LBY flick 3 times.
			// if we happen to miss then we most likely mispredicted.
			if ( data->m_resolver_data.m_mode_data[resolver_data::LBY_MOVING].m_index <= 2 ) {
				// set angles to current LBY.
				record->m_eye_angles.y = record->m_body;

				// predict next body update.
				data->m_resolver_data.m_body_update_time = record->m_anim_time + 1.1f;

				// set the resolve mode.
				record->m_mode = Modes::RESOLVE_BODY;
				record->m_base_angle = record->m_body;

				return;
			}
		}
	}
	if ( data->m_resolver_data.m_moved ) {
		const auto delta = record->m_anim_time - move->m_anim_time;

		record->m_base_angle = move->m_body;
		record->m_mode = Modes::RESOLVE_STAND1;

		int i = 0;

		auto& mode_data = data->m_resolver_data.m_mode_data[ resolver_data::STAND1 ];
		auto& record_dir_data = record->m_resolver_data.m_dir_data;

		record_dir_data.emplace_back( get_freestand_yaw( data->m_ent ) );
		if ( mode_data.m_index == 0 )
			record->m_eye_angles.y = get_freestand_yaw( data->m_ent );

		record_dir_data.emplace_back( move->m_body );
		if ( mode_data.m_index == 1 )
			record->m_eye_angles.y = move->m_body;

		record_dir_data.emplace_back( record->m_body );
		if ( mode_data.m_index == 2 )
			record->m_eye_angles.y = record->m_body;

		record_dir_data.emplace_back( away );
		if ( mode_data.m_index == 3 )
			record->m_eye_angles.y = away;

		record_dir_data.emplace_back( record->m_body + 180.f );
		if ( mode_data.m_index == 4 )
			record->m_eye_angles.y = record->m_body + 180.f;

		record_dir_data.emplace_back( record->m_body + 135.f );
		if ( mode_data.m_index == 5 )
			record->m_eye_angles.y = record->m_body + 135.f;

		record_dir_data.emplace_back( record->m_body - 135.f );
		if ( mode_data.m_index == 6 )
			record->m_eye_angles.y = record->m_body - 135.f;

		record_dir_data.emplace_back( record->m_body - 90.F );
		if ( mode_data.m_index == 7 )
			record->m_eye_angles.y = record->m_body - 90.F;

		record_dir_data.emplace_back( record->m_body + 90.F );
		if ( mode_data.m_index == 8 )
			record->m_eye_angles.y = record->m_body + 90.F;

		record_dir_data.emplace_back( record->m_body - 45.F );
		if ( mode_data.m_index == 9 )
			record->m_eye_angles.y = record->m_body - 45.F;

		record_dir_data.emplace_back( record->m_body + 45.F );
		if ( mode_data.m_index == 10 )
			record->m_eye_angles.y = record->m_body + 45.F;
		return;
	}

	
	// stand2 -> no known last move.
	record->m_base_angle = record->m_body;
	record->m_mode = Modes::RESOLVE_STAND2;
	int i = 0;
	auto& mode_data = data->m_resolver_data.m_mode_data[ resolver_data::STAND2 ];
	auto& record_dir_data = record->m_resolver_data.m_dir_data;

	record_dir_data.emplace_back(get_freestand_yaw( data->m_ent ));
	if (mode_data.m_index == 0)
		record->m_eye_angles.y = get_freestand_yaw(data->m_ent);

	record_dir_data.emplace_back( record->m_body + 180 );
	if ( mode_data.m_index == 1 )
		record->m_eye_angles.y = record->m_body + 180;

	record_dir_data.emplace_back( away );
	if ( mode_data.m_index == 2 )
		record->m_eye_angles.y = away;

	record_dir_data.emplace_back( record->m_body + 135.f );
	if ( mode_data.m_index == 3 )
		record->m_eye_angles.y = record->m_body + 135.f;

	record_dir_data.emplace_back( record->m_body - 135.f );
	if ( mode_data.m_index == 4 )
		record->m_eye_angles.y = record->m_body - 135.f;

	record_dir_data.emplace_back( record->m_body );
	if (mode_data.m_index == 5)
		record->m_eye_angles.y = record->m_body;

	record_dir_data.emplace_back( away + 180.f );
	if (mode_data.m_index == 6)
		record->m_eye_angles.y = away + 180.f;

	record_dir_data.emplace_back( record->m_body - 90.F);
	if ( mode_data.m_index == 7 )
		record->m_eye_angles.y = record->m_body - 90.F;

	record_dir_data.emplace_back( record->m_body + 90.F );
	if ( mode_data.m_index == 8 )
		record->m_eye_angles.y = record->m_body + 90.F;

	record_dir_data.emplace_back( record->m_body - 45.F );
	if ( mode_data.m_index == 9 )
		record->m_eye_angles.y = record->m_body - 45.F;

	record_dir_data.emplace_back( record->m_body + 45.F );
	if ( mode_data.m_index == 10 )
		record->m_eye_angles.y = record->m_body + 45.F;
	//record->m_eye_angles.y = record->m_body + get_rel( record, data->m_stand_index2 );
}

void resolver::ResolveAir( ent_info_t *data, std::shared_ptr<player_record_t> record ) const {
	// else run our matchmaking air resolver.

	// we have barely any speed. 
	// either we jumped in place or we just left the ground.
	// or someone is trying to fool our resolver.
	if ( record->m_velocity.length_2d( ) < 60.f ) {
		// set this for completion.
		// so the shot parsing wont pick the hits / misses up.
		// and process them wrongly.
		record->m_mode = RESOLVE_STAND;

		// invoke our stand resolver.
		ResolveStand( data, record );

		// we are done.
		return;
	}

	// try to predict the direction of the player based on his velocity direction.
	// this should be a rough estimation of where he is looking.
	const auto velyaw = RAD2DEG( std::atan2( record->m_velocity.y, record->m_velocity.x ) );

	switch ( data->m_resolver_data.m_shots % 3 ) {
	case 0:
		record->m_eye_angles.y = velyaw + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = velyaw - 90.f;
		break;

	case 2:
		record->m_eye_angles.y = velyaw + 90.f;
		break;
	default:
		break;
	}
}

void resolver::resolve ( ent_info_t &info, std::shared_ptr<player_record_t> record ) const {
	MatchShot( &info, record );

	SetMode( record );


	// we arrived here we can do the acutal resolve.
	if ( record->m_mode == Modes::RESOLVE_WALK )
		ResolveWalk( &info, record );

	else if ( record->m_mode == Modes::RESOLVE_STAND )
		ResolveStand( &info, record );

	else if ( record->m_mode == Modes::RESOLVE_AIR )
		ResolveAir( &info, record );

	math::normalize_angle( record->m_eye_angles.y, 180 );
}

void resolver::clear( ) {
	m_shots.clear( );
	m_hits.clear( );
	m_impacts.clear( );
}

resolver::trace_ret check_hit( penetration::PenetrationInput_t in, ent_info_t &info, bone_array_t *bones, bool check_hitbox = false, int hitgroup = 0 ) {
	penetration::PenetrationOutput_t out;
	trace_t trace;
	const math::custom_ray_t ray{ in.m_start, in.m_pos };
	in.m_resolving = true;
	//if (check_hitbox) {
	//	static trace_filter_one_entity filter; filter.pEntity = info.m_ent;
	//	g.m_interfaces->trace()->trace_ray(ray_t(in.m_start, in.m_pos), CONTENTS_HITBOX, &filter, &trace);
	//	if( !trace.entity || trace.hitGroup != hitgroup )
	//		return resolver::trace_ret::spread;
	//}
	if ( !g_aimbot.collides( ray, &info, bones ) )// !g_aimbot.collides( ray, info, cache->m_pCachedBones ) )
		return resolver::trace_ret::spread;
	//if ( !penetration::run( &in, &out ) )
	//	return resolver::trace_ret::occlusion;
	return resolver::trace_ret::hit;
}


void resolver::on_impact( IGameEvent *evt ) {
	vec3_t dir, start, end;
	trace_t trace;

	// screw this.
	if ( !evt )
		return;
	
	// get attacker, if its not us, screw it.
	const auto attacker = g.m_interfaces->engine( )->GetPlayerForUserID( evt->m_keys->FindKey( "userid" )->GetInt( ) );
	if ( attacker != g.m_interfaces->engine(  )->local_player_index(  ) )
		return;
	g.m_local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	if ( !g.m_local )
		return;
	// decode impact coordinates and convert to vec3.
	const vec3_t pos = {
		evt->m_keys->FindKey( "x" )->GetFloat( ),
		evt->m_keys->FindKey( "y" )->GetFloat( ),
		evt->m_keys->FindKey( "z" )->GetFloat( )
	};
	g.m_interfaces->debug_overlay( )->AddBoxOverlay( pos, -vec3_t( 2, 2, 2 ), vec3_t( 2, 2, 2 ), ang_t( ), 255, 0, 0, 150, 5 );

	// get prediction time at this point.
	const auto time = g.m_interfaces->globals()->m_curtime;

	// add to visual impacts if we have features that rely on it enabled.
	// todo - dex; need to match shots for this to have proper GetShootPosition, don't really care to do it anymore.
	//if ( g_menu.main.visuals.impact_beams.get( ) )
	//	m_vis_impacts.push_back( { pos, g_cl.m_local->GetShootPosition( ), g_cl.m_local->m_nTickBase( ) } );

	// we did not take a shot yet.
	if ( m_shots.empty( ) )
		return;

	struct ShotMatch_t { float delta = 0; std::shared_ptr<shot_record_t> shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max( );
	match.shot = nullptr;

	// iterate all shots.
	for ( const auto &s : m_shots ) {

		// this shot was already matched
		// with a 'bullet_impact' event.
		if ( s->m_matched || !s->m_updated_time )
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.

		// get the delta between the current time
		// and the predicted arrival time of the shot.

		auto predicted = s->m_time;
		auto *nci = g.m_interfaces->engine( )->get_net_channel_info( );
		if ( nci ) {
			const auto latency = nci->GetLatency( 2 );
			predicted += latency;
		}

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		const auto delta = std::abs( time - predicted );

		// fuck this.
		if ( delta > 1.f )
			continue;

		// store this shot as being the best for now.
		if ( delta < match.delta ) {
			match.delta = delta;
			match.shot = s;
		}
	}

	// no valid shotrecord was found.
	const auto &shot = match.shot;
	if ( !shot )
		return;


	//g_cl.print( "imp %x time: %f lat: %f dmg: %f\n", shot->m_record, shot->m_time, shot->m_lat, shot->m_damage );
	// add to track.

	// nospread mode.
	//if ( g_menu.main.config.mode.get( ) == 1 )
	//	return;

	// not in nospread mode, see if the shot missed due to spread.
	auto *const target = shot->m_target;
	if ( !target )
		return;
	
	for ( auto i = 0; i < m_impacts.size( ); i++ ) {
		auto *impact = &m_impacts[ i ];
		if ( impact->m_tick == g.m_local->tick_base( ) ) {
			m_impacts.erase( m_impacts.begin( ) + i );
			i--;
			continue;
		}
	}

	// not gonna bother anymore.

	// create new impact instance that we can match with a player hurt.
	impact_record_t impact;
	impact.m_shot = shot;
	impact.m_tick = g.m_local->tick_base( );
	impact.m_pos = pos;
	impact.m_hit = false;
	
	m_impacts.push_front( impact );

	// this record was deleted already.
}
void resolver::OnHurt( IGameEvent *evt ) {

	g.m_local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	
	if ( !evt || !g.m_local )
		return;

	const auto attacker = g.m_interfaces->engine( )->GetPlayerForUserID( evt->m_keys->FindKey( "attacker" )->GetInt( ) );
	const auto victim = g.m_interfaces->engine( )->GetPlayerForUserID( evt->m_keys->FindKey( "userid" )->GetInt( ) );

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if ( attacker < 1 || attacker > 64 || victim < 1 || victim > 64 || g.m_interfaces->engine( )->local_player_index( ) == victim )
		return;

	// we were not the attacker or we hurt ourselves.
	if ( attacker != g.m_interfaces->engine( )->local_player_index( ) || victim == g.m_interfaces->engine( )->local_player_index( ) )
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	const auto group = evt->m_keys->FindKey( "hitgroup" )->GetInt( );

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if ( group == hitgroup_gear )
		return;

	// get the player that was hurt.
	auto *const target = static_cast< player_t * >(g.m_interfaces->entity_list( )->get_client_entity( victim ));
	if ( !target )
		return;

	// get player info.
	engine_player_info_t info;
	if ( !g.m_interfaces->engine()->get_player_info( victim, &info ) )
		return;

	// get player name;
	const auto name = std::string( info.name ).substr( 0, 24 );

	// get damage reported by the server.
	const auto damage = static_cast< float >(evt->m_keys->FindKey( "dmg_health" )->GetInt( ));

	// get remaining hp.
	const auto hp = evt->m_keys->FindKey( "health" )->GetInt( );

	// hitmarker.
	//if ( g_menu.main.misc.hitmarker.get( ) ) {
	//	g_visuals.m_hit_duration = 1.f;
	//	g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;
	//	g_visuals.m_hit_end = g_visuals.m_hit_start + g_visuals.m_hit_duration;
	//
	//	g_csgo.m_sound->EmitAmbientSound( XOR( "buttons/arena_switch_press_02.wav" ), 1.f );
	//}

	// print this shit.
	//if ( g_menu.main.misc.notifications.get( 1 ) ) {
	//	std::string out = tfm::format( XOR( "hit %s in the %s for %i (%i remaining)\n" ), name, m_groups[ group ], ( int )damage, hp );
	//	g_notify.add( out );
	//}

	if ( group == hitgroup_generic )
		return;

	// if we hit a player, mark vis impacts.
	//if ( !m_vis_impacts.empty( ) ) {
	//	for ( auto &i : m_vis_impacts ) {
	//		if ( i.m_tickbase == g_cl.m_local->m_nTickBase( ) )
	//			i.m_hit_player = true;
	//	}
	//}

	const auto out = tfm::format( "Hit %s in the %s for %i(%i remaining) \n", name, m_groups[ group ], static_cast< int >(damage), hp );
	g_notification.add( out );
	
	// no impacts to match.
	if ( m_impacts.empty( ) )
		return;

	impact_record_t *impact{ nullptr };

	// iterate stored impacts.
	for ( auto &i : m_impacts ) {

		// this impact doesnt match with our current hit.
		if ( i.m_tick != g.m_local->tick_base( ) )
			continue;

		// wrong player.
		if ( i.m_shot->m_target != target )
			continue;

		// shit fond.
		impact = &i;
		break;
	}

	// no impact matched.
	if ( !impact )
		return;

	// setup new data for hit track and push to hit track.
	impact->m_hit = true;
	impact->m_group = group;
}
int resolver::miss_scan_boxes_and_eliminate( impact_record_t* impact, vec3_t& start, vec3_t& end ) {
	ent_info_t& data = *impact->m_shot->m_record->m_info;
	std::shared_ptr<player_record_t> record = impact->m_shot->m_record;


	penetration::PenetrationInput_t pen_in;

	pen_in.m_from = g.m_local;
	pen_in.m_target = impact->m_shot->m_target;
	pen_in.m_pos = end;
	pen_in.m_start = start;

	resolver_data::mode_data *move_data = nullptr;
	if ( record->m_mode == RESOLVE_STAND1 )
		move_data = &data.m_resolver_data.m_mode_data[ resolver_data::modes::STAND1 ];
	else
		move_data = &data.m_resolver_data.m_mode_data[ resolver_data::modes::STAND2 ];

	int eliminations = 0;
	std::vector<int> new_possible = {};
	auto fake_index = 0;
	auto any_true = false;

	for ( auto i1 = 0; i1 < move_data->m_dir_data.size(); i1++ ) {
		auto& dir_data = move_data->m_dir_data[ i1 ];
		if ( i1 == move_data->m_index ) {
			if ( dir_data.dir_enabled )
				any_true = true;
			else {
				new_possible.push_back( i1 );
			}
		}
		else {
			auto hit_type = check_hit( pen_in, data, record->m_fake_bones[ fake_index ] );
			if ( hit_type == trace_ret::hit ) {
				if ( dir_data.dir_enabled ) {

#ifdef _DEBUG
					g_aimbot.draw_hitboxes( impact->m_shot->m_target, record->m_fake_bones[ fake_index ] );
#endif
					dir_data.dir_enabled = false;
					eliminations++;
				}
			}
			else if ( dir_data.dir_enabled )
				any_true = true;
			else {
				new_possible.push_back( i1 );
			}
			fake_index++;
		}
	}
	if ( !any_true && !new_possible.empty( ) ) {
		for ( auto i1 : new_possible )
			move_data->m_dir_data[ i1 ].dir_enabled = true;
		move_data->m_index = new_possible[ 0 ];
	}
	else {
		auto set = false;
		for ( auto i1 = 0; i1 < move_data->m_dir_data.size( ); i1++ ) {
			if ( !any_true )
				move_data->m_dir_data[ i1 ].dir_enabled = true;
			if ( !set && move_data->m_dir_data[ i1 ].dir_enabled ) {
				move_data->m_index = i1;
				set = true;
			}
		}
	}
	return eliminations;
}
int resolver::hit_scan_boxes_and_eliminate( impact_record_t* impact, vec3_t& start, vec3_t& end ) const {
	ent_info_t& data = *impact->m_shot->m_record->m_info;
	std::shared_ptr<player_record_t> record = impact->m_shot->m_record;

	resolver_data::mode_data* move_data = nullptr;
	if ( record->m_mode == RESOLVE_STAND1 )
		move_data = &data.m_resolver_data.m_mode_data[ resolver_data::modes::STAND1 ];
	else
		move_data = &data.m_resolver_data.m_mode_data[ resolver_data::modes::STAND2 ];

	int eliminations = 0;
	std::vector<int> possible_hit = {};
	auto any_true = false;
	auto fake_index = 0;

	penetration::PenetrationInput_t pen_in;

	pen_in.m_from = g.m_local;
	pen_in.m_target = impact->m_shot->m_target;
	pen_in.m_pos = end;
	pen_in.m_start = start;

	for ( auto i2 = 0; i2 < move_data->m_dir_data.size( ); i2++ ) {
		auto& dir_data = move_data->m_dir_data[ i2 ];
		auto hit_type = check_hit( pen_in, data, record->m_fake_bones[ fake_index ] );
		if ( hit_type != trace_ret::hit ) {
			if ( dir_data.dir_enabled ) {
#ifdef _DEBUG
				//g_aimbot.draw_hitboxes( data.m_ent, i2 == *current_index ? record->m_bones : record->m_fake_bones[ fake_index ] );
#endif
				eliminations++;
				dir_data.dir_enabled = false;
			}
		}
		else if ( dir_data.dir_enabled ) {
			possible_hit.push_back( i2 );
		}
		if ( dir_data.dir_enabled )
			any_true = true;
		if ( i2 != move_data->m_index )
			fake_index++;
	}
	if ( !any_true ) {
		auto set = false;
		if ( !possible_hit.empty( ) ) {
			for ( auto i1 : possible_hit ) {
				if ( !set ) {
					move_data->m_index = i1;
					set = true;
				}
				move_data->m_dir_data[ i1 ].dir_enabled = true;
			}
		}
		else {
			for ( auto i1 = 0; i1 < move_data->m_dir_data.size( ); i1++ ) {
				move_data->m_dir_data[ i1 ].dir_enabled = true;
				if ( !set ) {
					move_data->m_index = i1;
					set = true;
				}
			}
		}
	}
	else {
		auto set = false;
		for ( auto i1 = 0; i1 < move_data->m_dir_data.size( ); i1++ ) {
			if ( move_data->m_dir_data[ i1 ].dir_enabled && !set ) {
				move_data->m_index = i1;
				set = true;
			}
		}
	}
	return eliminations;
}

void resolver::resolve_hit ( impact_record_t *impact ) const {
	auto &data = *impact->m_shot->m_record->m_info;
	auto &record = impact->m_shot->m_record;

	size_t mode = record->m_mode;

	auto start = impact->m_shot->m_pos;
	auto dir = ( impact->m_pos - start ).normalized( );

	// get end pos by extending direction forward.
	// todo; to do this properly should save the weapon range at the moment of the shot, cba..

	g.m_weapon_info = impact->m_shot->m_weapon_info.get( );
	auto end = impact->m_pos;

	math::custom_ray_t ray( start, end );
	std::vector<int> possible_hit = {};

	penetration::PenetrationInput_t in;
	in.m_from = g.m_local;
	in.m_target = impact->m_shot->m_target;
	in.m_pos = end;
	in.m_start = start;

	auto eliminations = 0;
	if ( mode == RESOLVE_STAND1 || mode == RESOLVE_STAND2 ) {
		eliminations = hit_scan_boxes_and_eliminate( impact, start, end );
	}
	if ( eliminations > 0 ) {
		auto out = tfm::format( "eliminated %i resolves by hit\n", eliminations );
		g_notification.add( out );
	}
}

void resolver::resolve_miss ( impact_record_t *impact ) {
	auto &data = *impact->m_shot->m_record->m_info;
	auto record = impact->m_shot->m_record;

	// start position of trace is where we took the shot.
	auto start = impact->m_shot->m_pos;
	auto dir = ( impact->m_pos - start ).normalized( );

	// the impact pos contains the spread from the server
	// which is generated with the server seed, so this is where the bullet
	// actually went, compute the direction of this from where the shot landed
	// and from where we actually took the shot.

	// get end pos by extending direction forward.
	// todo; to do this properly should save the weapon range at the moment of the shot, cba..
	g.m_weapon_info = impact->m_shot->m_weapon_info.get( );
	auto end = start + dir * impact->m_shot->m_weapon_info->m_range;
	math::custom_ray_t ray( start, end );

	// we did not hit jackshit, or someone else.
	penetration::PenetrationInput_t in;
	in.m_from = g.m_local;
	in.m_target = impact->m_shot->m_target;
	in.m_pos = end;
	in.m_start = start;
	//record->cache( -1 );

	auto hit_type = check_hit( in, data, record->m_bones );

	int eliminations = 0;
	std::vector<int> new_possible = {};
	size_t mode = record->m_mode;
	if ( hit_type != trace_ret::hit ) {
		if ( hit_type == trace_ret::occlusion )
			g_notification.add( ( "shot missed due to occlusion\n" ) );
		else
			g_notification.add( ( "shot missed due to spread\n" ) );
	}

		// we should have 100% hit this player..
		// this is a miss due to wrong angles.
	else {
		// if we miss a shot on body update.
		// we can chose to stop shooting at them.
		if ( mode == Modes::RESOLVE_BODY ) {
			auto& idx = data.m_resolver_data.m_mode_data[ resolver_data::modes::LBY_MOVING ].m_index;
			++idx;
			if ( idx > 2 )
				idx = 0;
		}
		else if ( mode == Modes::RESOLVE_STAND1 ) {
			auto& dir_data = data.m_resolver_data.m_mode_data[ resolver_data::modes::STAND1 ];
			dir_data.m_dir_data[ dir_data.m_index ].dir_enabled = false;
		}
		else if ( mode == Modes::RESOLVE_STAND2 ) {
			auto& dir_data = data.m_resolver_data.m_mode_data[ resolver_data::modes::STAND2 ];
			dir_data.m_dir_data[ dir_data.m_index ].dir_enabled = false;
		}
	}
	if ( mode == Modes::RESOLVE_STAND1 || mode == Modes::RESOLVE_STAND2 ) {
		impact->m_resolved = true;
		eliminations = miss_scan_boxes_and_eliminate( impact, start, end );
	}

	if ( eliminations > 0 ) {
		auto out = tfm::format( "eliminated %i resolves by miss\n", eliminations );
		g_notification.add( out );
	}
}

void resolver::update_shots( ) {
	for ( size_t i = 0; i < m_impacts.size( ); i++ ) {
		auto *impact = &m_impacts[ i ];
		if ( !impact->m_shot ) {
			m_impacts.erase( m_impacts.begin( ) + i );
			i--;
			continue;
		}
		if ( fabsf(impact->m_shot->m_time - g.m_interfaces->globals( )->m_curtime ) > 0.6f ) {
			m_impacts.erase( m_impacts.begin( ) + i );
			i--;
			continue;
		}

		if ( !impact->m_shot->m_record->m_setup )
			continue;
		if ( impact->m_shot->m_target->health( ) <= 0 )
			continue;
		impact->m_shot->m_matched = true;
		g_aimbot.m_backup[ impact->m_shot->m_target->index( ) - 1 ].store( impact->m_shot->m_target );
		if ( impact->m_hit ) {

			//g_cl.print( "hit %x time: %f lat: %f dmg: %f\n", record, impact->m_shot->m_time, impact->m_shot->m_lat, impact->m_shot->m_damage );


			resolve_hit( impact );
		}
		else {

			resolve_miss( impact );
		}
		g_aimbot.m_backup[ impact->m_shot->m_target->index( ) - 1 ].restore( impact->m_shot->m_target );
	}
	m_impacts.clear( );
}

void resolver::add_shot( ent_info_t *target, float damage, int bullets, std::shared_ptr<player_record_t> record ) {

	// iterate all bullets in this shot.
	for ( int i{ }; i < bullets; ++i ) {
		// setup new shot data.
		shot_record_t shot;
		shot.m_target = target->m_ent;
		shot.m_record = record;
		shot.m_time = g.m_interfaces->globals( )->m_curtime;
		shot.m_lat = g.m_latency;
		shot.m_damage = damage;
		shot.m_pos = g.m_shoot_pos;
		shot.m_weapon_info = std::make_shared<weapon_info_t>();
		*shot.m_weapon_info =  *g.m_weapon_info;
		target->m_resolver_data.m_shots++;
		shot.m_updated_time = false;
		shot.m_tick = g.m_lag;
		// add to tracks.
		m_shots.push_back( std::make_shared<shot_record_t>(shot) );
	}

	// no need to keep an insane amount of shots.
	while ( m_shots.size( ) > 128 )
		m_shots.pop_front( );
}