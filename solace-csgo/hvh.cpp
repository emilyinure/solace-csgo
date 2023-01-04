#include "hvh.h"
#include "includes.h"
#include "aimbot.h"
#include "prediction.h"
#include "predictioncopy.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class AdaptiveAngle
{
public:
    float m_yaw;
    float m_dist;

public:
    // ctor.
    __forceinline AdaptiveAngle(float yaw, float penalty = 0.f);
};

AdaptiveAngle::AdaptiveAngle ( float yaw, float penalty ) {
	// set yaw.
	m_yaw = math::normalize_angle( yaw, 180 );

	// init distance.
	m_dist = 0.f;

	// remove penalty.
	m_dist -= penalty;
}

void hvh::fake_walk() const {
  vec3_t velocity{g.m_local->velocity()};
  int ticks{}, max{16};

  if (!settings::hvh::antiaim::fakewalk) return;

  if (!g.m_local->m_ground_entity()) return;

  // user was running previously and abrubtly held the fakewalk key
  // we should quick-stop under this circumstance to hit the 0.22 flick
  // perfectly, and speed up our fakewalk after running even more.
  // if( g_cl.m_initial_flick ) {
  //	Movement::QuickStop( );
  //	return;
  //}

  // reference:
  // https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1612

  // calculate friction.
  static auto sv_friction =
      g.m_interfaces->console()->get_convar("sv_friction");
  static auto sv_stopspeed =
      g.m_interfaces->console()->get_convar("sv_stopspeed");
  float friction = sv_friction->GetFloat() * g.m_local->surface_friction();

  for (; ticks < g.m_max_lag; ++ticks) {
    // calculate speed.
    float speed = velocity.length();

    // if too slow return.
    if (speed <= 0.1f) break;

    // bleed off some speed, but if we have less than the bleed, threshold,
    // bleed the threshold amount.
    float control = std::max(speed, sv_stopspeed->GetFloat());

    // calculate the drop amount.
    float drop = control * friction * g.m_interfaces->globals()->m_interval_per_tick;

    // scale the velocity.
    float newspeed = std::max(0.f, speed - drop);

    if (newspeed != speed) {
      // determine proportion of old speed we are using.
      newspeed /= speed;

      // adjust velocity according to proportion.
      velocity *= newspeed;
    }
  }

  // zero forwardmove and sidemove.
  if (ticks > ((max - 1) -
               g.m_interfaces->client_state()->chokedcommands) ||
      !g.m_interfaces->client_state()->chokedcommands)
  {
    g.m_cmd->m_forwardmove = g.m_cmd->m_sidemove = 0.f;
  }
}

void hvh::break_resolver() {
	if (!settings::hvh::antiaim::fakehead) {
		m_breaking = false;
		return;
	}
	const auto stand = settings::hvh::antiaim::body_fake_stand > 0 && !g.m_cmd->m_forwardmove && !g.m_cmd->m_sidemove;
	if ( stand ) {
		m_switch = !m_switch;
		g.m_cmd->m_forwardmove = 2.4f / ((!m_switch) + 1 );

		m_breaking = m_switch;
		*g.m_packet = m_switch;
	}
}

void hvh::IdealPitch( ) {
	const auto state = g.m_local->get_anim_state( );
	if ( !state )
		return;

	g.m_cmd->m_viewangles.x = state->m_min_pitch;
}

void hvh::AntiAimPitch( ) const {
	const auto safe = true;

	switch ( m_pitch ) {
	case 1:
		// down.
		g.m_cmd->m_viewangles.x = safe ? 89.f : 720.f;
		break;

	case 2:
		// up.
		g.m_cmd->m_viewangles.x = safe ? -89.f : -720.f;
		break;

	case 3:
		// random.
		g.m_cmd->m_viewangles.x = ( safe ? -89.f : -720.f, safe ? 89.f : 720.f );
		break;

	default:
		break;
	}
}


void hvh::AutoDirection( ) {
	// constants.
	constexpr const auto STEP{ 1.f };
	constexpr const auto RANGE{ 32.f };

	// best target.
	struct AutoTarget_t { float fov; player_t *player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// iterate players.
	for ( size_t i = 0; i < g_player_manager.m_ents.size( ); i++ ) {
		auto *player = &g_player_manager.m_ents[ i ];
		// validate player.
		if ( !player || !g_aimbot.valid( player ) )
			continue;

		// skip dormant players.
		if ( player->m_ent->dormant( ) )
			continue;

		// get best target based on fov.
		auto fov = math::get_fov( ang_t(0, m_view_angle, 0), g.m_shoot_pos, player->m_ent->world_space_center( ) );

		const auto set = fov < target.fov;
		target.fov = set ? fov : target.fov;
		target.player = set ? player->m_ent : target.player;
	}

	if ( !target.player ) {
		// set angle to backwards.
		const auto set = m_auto_last > 0.f && m_auto_time > 0.f && g.m_interfaces->globals( )->m_curtime < ( m_auto_last + m_auto_time );
		m_auto = !set ? math::normalize_angle( m_view - 180.f, 180 ) : m_auto;
		m_auto_dist = !set ? -1.f : m_auto_dist;
		return;
	}

	/*
	* data struct
	* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	*/

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back( m_view - 180.f );
	angles.emplace_back( m_view + 90.f );
	angles.emplace_back( m_view - 90.f );

	// start the trace at the enemy shoot pos.
	vec3_t start;
	target.player->get_eye_pos( &start );
	

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	auto valid{ false };

	// iterate vector of angles.
	for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ g.m_shoot_pos.x + std::cos( DEG2RAD( it->m_yaw ) ) * RANGE,
			g.m_shoot_pos.y + std::sin( DEG2RAD( it->m_yaw ) ) * RANGE,
			g.m_shoot_pos.z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		auto dir = end - start;
		const auto len = dir.length( );
		dir /= len;
		// should never happen.
		if ( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		for ( auto i{ 0.f }; i < len; i += STEP ) {
			// get the current step position.
			auto point = start + ( dir * i );

			// get the contents at this point.
			const auto contents = g.m_interfaces->trace(  )->get_point_contents( point, MASK_SHOT_HULL );

			// contains nothing that can stop a bullet.
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			auto mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			const auto set = (i > ( len * 0.5f )) ||
						  (i > ( len * 0.75f )) ||
						  (i > ( len * 0.9f ));

			mult = (i > len * 0.5f) * 1.25f +
				   (i > len * 0.75f) * 1.25f +
				   (i > len * 0.9f) * 2.f + !set * mult;

			// append 'penetrated distance'.
			it->m_dist += ( STEP * mult );

			// mark that we found anything.
			valid = true;
		}
	}

	if ( !valid ) {
		// set angle to backwards.
		m_auto = math::normalize_angle( m_view - 180.f, 180 );
		m_auto_dist = -1.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
			   [ ]( const AdaptiveAngle &a, const AdaptiveAngle &b ) {
				   return a.m_dist > b.m_dist;
			   } );

	// the best angle should be at the front now.
	const auto best = &angles.front( );

	// check if we are not doing a useless change.
	const auto set = best->m_dist != m_auto_dist;
	// set yaw to the best result.
	m_auto = set * math::normalize_angle( best->m_yaw, 180 ) + !set * m_auto;
	m_auto_dist = set * best->m_dist + !set * m_auto_dist;
	m_auto_last = set * g.m_interfaces->globals(  )->m_curtime + !set * m_auto_last;
}

void hvh::GetAntiAimDirection( ) {

	// lock while standing..
	const auto lock = settings::hvh::antiaim::dir_lock;

	// save view, depending if locked or not.
	if ( ( lock && g.m_local->velocity(  ).length(  ) > 0.1f ) || !lock )
		m_view = m_view_angle;

	if ( m_base_angle > 0 ) {
		// 'static'.
		if ( m_base_angle == 1 )
			m_view = 0.f;

		// away options.
		else {
			auto best_fov{ std::numeric_limits< float >::max( ) };
			auto best_dist{ std::numeric_limits< float >::max( ) };
			ent_info_t *best_target{ nullptr };

			for ( auto i = 0; i < g_player_manager.m_ents.size( ); i++ ) {
				auto *target = &g_player_manager.m_ents[ i ];

				if ( !target || !g_aimbot.valid( target ) )
					continue;

				if ( target->m_ent->dormant( ) )
					continue;

				// 'away crosshair'.
				if ( m_base_angle == 2 ) {

					// check if a player was closer to our crosshair.
					const auto fov = math::get_fov( ang_t(0,m_view_angle,0), g.m_shoot_pos, target->m_ent->world_space_center( ) );
					const auto set = fov < best_fov;
					best_fov = set * fov + !set * best_fov;
					best_target = reinterpret_cast< ent_info_t * >(set * reinterpret_cast< uintptr_t >(target) +
								  reinterpret_cast< uintptr_t >( best_target ) * !set);
				}

				// 'away distance'.
				else if ( m_base_angle == 3 ) {

					// check if a player was closer to us.
					const auto dist = (target->m_ent->origin( ) - g.m_local->origin( )).length_sqr( );
					const auto set = dist < best_dist;
					best_dist = set * dist + !set * best_dist;
					best_target = reinterpret_cast< ent_info_t * >( set * reinterpret_cast< uintptr_t >( target ) +
																	reinterpret_cast< uintptr_t >( best_target ) * !set );
				}
			}

			if ( best_target )
				m_view = g.m_local->origin( ).look( best_target->m_ent->origin( ) ).y;
		}
	}

	// switch direction modes.
	switch ( m_dir ) {

		// auto.
	case 0:
		AutoDirection( );
		m_direction = m_auto;
		break;

		// backwards.
	case 1:
		m_direction = m_view + 180.f;
		break;

		// left.
	case 2:
		m_direction = m_view + 90.f;
		break;

		// right.
	case 3:
		m_direction = m_view - 90.f;
		break;

	default:
		m_direction = m_view;
		break;
	}

	// normalize the direction.
	m_direction = math::normalize_angle( m_direction, 180 );
}

void hvh::DoRealAntiAim( ) {
	// if we have a yaw antaim.
	if ( m_yaw > 0 ) {

		// if we have a yaw active, which is true if we arrived here.
		// set the yaw to the direction before applying any other operations.
		g.m_cmd->m_viewangles.y = m_direction;

		const auto stand = settings::hvh::antiaim::body_fake_stand > 0 && m_mode == AntiAimMode::STAND;
		const auto air = settings::hvh::antiaim::body_fake_air > 0 && m_mode == AntiAimMode::AIR;
		auto breaker = true;

		if ( m_mode == AntiAimMode::WALK ) {
			m_break_dir = 0;
			breaker = false;
		}


		static float flLastMoveTime = FLT_MAX;
		static float flLastMoveYaw = FLT_MAX;
		static bool bGenerate = true;
		static float flGenerated = 0.f;
		static bool needs_adjust = false;
		const float time = g.ticks_to_time(g.m_local->tick_base());
		bool bDoDistort = true;
		if (g.m_local->velocity().length_sqr() > 0.01f && g.m_local->m_ground_entity() && !settings::hvh::antiaim::fakewalk) {
			flLastMoveTime = time;
			flLastMoveYaw = g.m_local->lower_body_yaw();

			bDoDistort = false;
			needs_adjust = false;
		}
		if (m_just_updated_body)
			flLastMoveTime = time;
		float flDistortion = ((flLastMoveTime - time) / 1.1f);
		auto animstate = g.m_local->get_anim_state();
		float m_flWalkToRunTransition = 0;
		if (animstate) {
			auto vel = g.m_local->velocity();
			float m_flVelocityLengthXY = vel.length_2d();
			m_flWalkToRunTransition = animstate->m_flWalkToRunTransition;
			if (m_flWalkToRunTransition > 0 && m_flWalkToRunTransition < 1)
			{
				//currently transitioning between walk and run
				if (animstate->m_bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
				{
					m_flWalkToRunTransition += g.ticks_to_time(g.m_last_lag) * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
				}
				else // m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK
				{
					m_flWalkToRunTransition -= g.ticks_to_time(g.m_last_lag) * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
				}
				m_flWalkToRunTransition = std::clamp<float>(m_flWalkToRunTransition, 0, 1);
			}

			if (m_flVelocityLengthXY > (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && animstate->m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK)
			{
				//crossed the walk to run threshold
				m_flWalkToRunTransition = fmaxf(0.01f, m_flWalkToRunTransition);
			}
			else if (m_flVelocityLengthXY < (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && animstate->m_bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
			{
				//crossed the run to walk threshold
				m_flWalkToRunTransition = fmaxf(0.99f, m_flWalkToRunTransition);
			}
		}
		// one tick before the update.
		//if ( stand && !g.m_lag && g.m_interfaces->globals( )->m_curtime >= ( g.m_body_pred - g.m_anim_frame ) && g.m_interfaces->globals( )->m_curtime < g.m_body_pred ) {
		//	// z mode.
		//	float body = g.m_cmd->m_viewangles.y;
		//	body += 110.f * (settings::hvh::antiaim::body_fake_stand == 1);
		//	body -= 110.f * (settings::hvh::antiaim::body_fake_stand == 2);
		//	body += 180.f * (settings::hvh::antiaim::body_fake_stand == 3);
		//	body += (155.f * (m_lby_side * -1)) * (settings::hvh::antiaim::body_fake_stand == 4);
		//	body = math::normalize_angle(body, 180);
		//	
		//	float offset = fmaxf(fabsf(math::normalize_angle(std::copysignf(179.f, -body) - body, 180.f)), 70);
		//
		//	g.m_cmd->m_viewangles.y = body + offset;
		//}

		// check if we will have a lby fake this tick.
		//else 
		if ( !g.m_lag && ((g.m_interfaces->globals()->m_curtime >= g.m_body_pred && (stand || air)))) {
			
			m_just_updated_body = true;
			//if ( breaker ) {
			//	//m_rot_range = 0;
			//	if ( m_break_dir == 90 )
			//		m_break_dir = 0;
			//	else if ( m_break_dir == 0 )
			//		m_break_dir = -90;
			//	else if ( m_break_dir == -90 )
			//		m_break_dir = 90;
			//	
			//	g.m_cmd->m_viewangles.y += m_break_dir;
			//}
			// there will be an lbyt update on this tick.
			if ( stand || !m_breaking) {
				//switch ( settings::hvh::antiaim::body_fake_stand ) {
				//
				//	// left.
				//case 1:
					g.m_cmd->m_viewangles.y += 110.f * (settings::hvh::antiaim::body_fake_stand == 1);
					g.m_cmd->m_viewangles.y -= 110.f * (settings::hvh::antiaim::body_fake_stand == 2);
					g.m_cmd->m_viewangles.y += 180.f * (settings::hvh::antiaim::body_fake_stand == 3);

					//float break_yaw = 85.f; ;
					//if (animstate && !m_breaking)
					//	break_yaw = fmaxf( 45, fminf( 89, g.ticks_to_time(g.m_last_lag) * (30.0f + 20.0f * m_flWalkToRunTransition)));
					//g.m_cmd->m_viewangles.y += (std::copysignf(90, m_lby_side *= -1) + (break_yaw * m_lby_side)) * (settings::hvh::antiaim::body_fake_stand == 4);
				//	break;
				//
				//default:
				//	break;
				//}
			}

			else if ( air ) {
				switch ( settings::hvh::antiaim::body_fake_air ) {
				case 1:// left.
					g.m_cmd->m_viewangles.y += 90.f;
					break;
				case 2:// right.
					g.m_cmd->m_viewangles.y -= 90.f;
					break;
				case 3:// opposite.
					g.m_cmd->m_viewangles.y += 180.f;
					break;

				default:
					break;
				}
			}
		}

		// run normal aa code.
		else {
			float break_yaw = 0.f;
			if( breaker )
				g.m_cmd->m_viewangles.y += m_break_dir;
			switch ( m_yaw ) {

				// direction.
			case 1:
				// do nothing, yaw already is direction.
				break;

				// jitter.
			case 2:
			{

				// get the range from the menu.
				const auto range = m_jitter_range / 2.f;

				// set angle.
				g.m_cmd->m_viewangles.y += g.random_float( -range, range );
				break;
			}

			// rotate.
			case 3:
			{
				// set base angle.
				g.m_cmd->m_viewangles.y = ( m_direction - m_rot_range / 2.f );

				// apply spin.
				g.m_cmd->m_viewangles.y += std::fmod( g.m_interfaces->globals()->m_curtime * ( m_rot_speed * 20.f ), m_rot_range );

				break;
			}

			// random.
			case 4:
				// check update time.
				if ( g.m_interfaces->globals()->m_curtime >= m_next_random_update ) {

					// set new random angle.
					m_random_angle = math::RandomFloat( -180.f, 180.f );

					// set next update time
					m_next_random_update = g.m_interfaces->globals()->m_curtime + m_rand_update;
				}

				// apply angle.
				g.m_cmd->m_viewangles.y = m_random_angle;
				break;
			case 5: // distortion

				if (m_mode == AntiAimMode::WALK) {
					break_yaw = ((g.ticks_to_time(g.m_last_lag) * (30.0f + 20.0f * m_flWalkToRunTransition)));
					g.m_cmd->m_viewangles.y += ( break_yaw * 4.f * (m_side));
				}
				else {

					//if (g_Vars.globals.manual_aa != -1 && !g_Vars.antiaim.distort_manual_aa)
					//	bDoDistort = false;

					if (flLastMoveTime == FLT_MAX)
						break;

					if (flLastMoveYaw == FLT_MAX)
						break;


					// don't distort for longer than this

					//if (g_Vars.antiaim.distort_twist) {
					if (animstate) {
						g.m_cmd->m_viewangles.y = 90.f + animstate->m_cur_feet_yaw + (360.f * flDistortion);
					}
				}
				break;

			default:
				break;
			}
			if ( stand && !g.m_lag && m_just_updated_body ) {
				//if (settings::hvh::antiaim::body_fake_stand == 4) {
				//	const auto diff = math::normalize_angle(g.m_body - g.m_cmd->m_viewangles.y, 180);
				//	if (fabsf(diff) >= (120.f / (g.m_interfaces->globals()->m_curtime - g.m_anim_time))) {
				//		g.m_cmd->m_viewangles.y = g.m_body + std::copysignf(119.f / (g.m_interfaces->globals()->m_curtime - g.m_anim_time), diff);
				//	}
				//}
				m_just_updated_body = false;
			}
		}
	}

	// normalize angle.
	g.m_cmd->m_viewangles.y = math::normalize_angle( g.m_cmd->m_viewangles.y, 180 );
}

void hvh::DoFakeAntiAim( ) const {
	// do fake yaw operations.

	// enforce this otherwise low fps dies.
	// cuz the engine chokes or w/e
	// the fake became the real, think this fixed it.
	*g.m_packet = true;
	switch ( settings::hvh::antiaim::fake_yaw ) {

		// default.
	case 1:
		// set base to opposite of direction.
		g.m_cmd->m_viewangles.y = m_direction + 180.f;

		// apply 45 degree jitter.
		g.m_cmd->m_viewangles.y += math::RandomFloat( -45.f, 45.f );
		break;

		// relative.
	case 2:
		// set base to opposite of direction.
		g.m_cmd->m_viewangles.y = m_direction + 180.f;

		// apply offset correction.
		g.m_cmd->m_viewangles.y += settings::hvh::antiaim::fake_relative;
		break;

		// relative jitter.
	case 3:
	{
		// get fake jitter range from menu.
		const auto range = settings::hvh::antiaim::fake_jitter_range / 2.f;

		// set base to opposite of direction.
		g.m_cmd->m_viewangles.y = m_direction + 180.f;

		// apply jitter.
		g.m_cmd->m_viewangles.y += math::RandomFloat( -range, range );
		break;
	}

	// rotate.
	case 4:
		g.m_cmd->m_viewangles.y = m_direction + 90.f + std::fmod( g.m_interfaces->globals()->m_curtime * 360.f, 180.f );
		break;

		// random.
	case 5:
		g.m_cmd->m_viewangles.y = math::RandomFloat( -180.f, 180.f );
		break;

		// local view.
	case 6:
		g.m_cmd->m_viewangles.y = m_view_angle;
		break;

	default:
		break;
	}

	// normalize fake angle.
	const auto breaker = true;
	if ( breaker )
		g.m_cmd->m_viewangles.y += m_break_dir;
	g.m_cmd->m_viewangles.y = math::normalize_angle( g.m_cmd->m_viewangles.y, 180 );
}

void hvh::AntiAim( ) {

	if ( !settings::hvh::antiaim::enabled )
		return;

	const bool attack = g.m_cmd->m_buttons & IN_ATTACK;
	const bool attack2 = g.m_cmd->m_buttons & IN_ATTACK2;

	if ( g.m_weapon && g.m_can_fire ) {
		const auto knife = g.m_weapon_type == WEAPONTYPE_KNIFE;// &&g.m_weapon_id != ZEUS;
		const auto revolver = g.m_weapon->item_definition_index() == 64;

		// if we are in attack and can fire, do not anti-aim.
		if ( attack || ( attack2 && ( knife || revolver ) ) )
			return;
	}

	// disable conditions.
	if ( ( g.m_flags & fl_frozen ) || ( g.m_cmd->m_buttons & IN_USE ) )
		return;

	// grenade throwing
	// CBaseCSGrenade::ItemPostFrame()
	// https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/shared/cstrike/weapon_basecsgrenade.cpp#L209
	if ( g.m_weapon && g.m_weapon_type == WEAPONTYPE_GRENADE
		 && ( !g.m_weapon->pin_pulled( ) || attack || attack2 )
		 && g.m_weapon->throw_time( ) > 0.f && g.m_weapon->throw_time( ) < g.m_interfaces->globals()->m_curtime )
		return;

	m_mode = AntiAimMode::STAND;

	if ( ( g.m_buttons & IN_JUMP ) || !( g.m_flags & fl_onground ) )
		m_mode = AntiAimMode::AIR;

	else if ( g.m_speed > 0.1f )
		m_mode = AntiAimMode::WALK;

	// load settings.
	if ( m_mode == AntiAimMode::STAND ) {
		m_pitch = settings::hvh::antiaim::pitch_stand;
		m_yaw = settings::hvh::antiaim::yaw_stand;
		m_jitter_range = settings::hvh::antiaim::jitter_range_stand;
		m_rot_range = settings::hvh::antiaim::rot_range_stand;
		m_rot_speed = settings::hvh::antiaim::rot_speed_stand;
		m_rand_update = settings::hvh::antiaim::rand_update_stand;
		m_dir = settings::hvh::antiaim::dir_stand;
		m_dir_custom = settings::hvh::antiaim::dir_custom_stand;
		m_base_angle = settings::hvh::antiaim::base_angle_stand;
		m_auto_time = settings::hvh::antiaim::dir_time_stand;
	}
	//
	else if ( m_mode == AntiAimMode::WALK ) {
		m_pitch = settings::hvh::antiaim::pitch_walk;
		m_yaw = settings::hvh::antiaim::yaw_walk;
		m_jitter_range = settings::hvh::antiaim::jitter_range_walk;
		m_rot_range = settings::hvh::antiaim::rot_range_walk;
		m_rot_speed = settings::hvh::antiaim::rot_speed_walk;
		m_rand_update = settings::hvh::antiaim::rand_update_walk;
		m_dir = settings::hvh::antiaim::dir_walk;
		m_dir_custom = settings::hvh::antiaim::dir_custom_walk;
		m_base_angle = settings::hvh::antiaim::base_angle_walk;
		m_auto_time = settings::hvh::antiaim::dir_time_walk;
	}
	//
	else if ( m_mode == AntiAimMode::AIR ) {
		m_pitch = settings::hvh::antiaim::pitch_air;
		m_yaw = settings::hvh::antiaim::yaw_air;
		m_jitter_range = settings::hvh::antiaim::jitter_range_air;
		m_rot_range = settings::hvh::antiaim::rot_range_air;
		m_rot_speed = settings::hvh::antiaim::rot_speed_air;
		m_rand_update = settings::hvh::antiaim::rand_update_air;
		m_dir = settings::hvh::antiaim::dir_air;
		m_dir_custom = settings::hvh::antiaim::dir_custom_air;
		m_base_angle = settings::hvh::antiaim::base_angle_air;
		m_auto_time = settings::hvh::antiaim::dir_time_air;
	}

	// set pitch.
	AntiAimPitch( );

	// if we have any yaw.
	if ( m_yaw > 0 ) {
		// set direction.
		GetAntiAimDirection( );
	}

	// we have no real, but we do have a fake.
	else if ( settings::hvh::antiaim::fake_yaw > 0 )
		m_direction = g.m_cmd->m_viewangles.y;

	//const auto stand = settings::hvh::antiaim::body_fake_stand > 0 && m_mode == AntiAimMode::STAND;
	//if(stand) {
	//	if (g.m_lag > 0 && (m_breaking || (g.m_interfaces->globals()->m_curtime + g.ticks_to_time(3) >= g.m_body_pred && (stand)) || (g.m_interfaces->globals()->m_curtime + g.ticks_to_time(1) >= g.m_body_pred && (stand)))) {
	//		*g.m_packet = true;
	//	}
	//}
	if ( settings::hvh::antiaim::fake_yaw ) {
		// do not allow 2 consecutive sendpacket true if faking angles.
		if ( *g.m_packet && g.m_old_packet )
			*g.m_packet = false;

		// run the real on sendpacket false.
		if ( !*g.m_packet || *g.m_final_packet )
			DoRealAntiAim( );

		// run the fake on sendpacket true.
		else DoFakeAntiAim( );
	}

	// no fake, just run real.
	else DoRealAntiAim( );
	//if ( !*g.m_packet ) {
	//	c_g::choked_log log;
	//	log.flags = g.m_flags;
	//	log.commandnr = g.m_cmd->m_command_number;
	//	log.origin = g.m_local->origin( );
	//	log.shoot_pos = g.m_shoot_pos;
	//	log.view_angles = g.m_view_angles;
	//	log.view_angle = m_view_angle;
	//	log.m_body_pred = g.m_body_pred;
	//	log.m_anim_frame = g.m_anim_frame;
	//	log.sidemove = g.m_cmd->m_sidemove;
	//	log.forwardmove = g.m_cmd->m_forwardmove;
	//	log.m_velocity = g.m_local->velocity( );
	//	log.curtime = g.m_interfaces->globals( )->m_curtime;
	//	g.m_choked_logs.push_back( log );
	//	if ( g.m_choked_logs.size( ) > 150 )
	//		g.m_choked_logs.pop_front( );
	//}
}

void hvh::SendPacket( ) {
	// if not the last packet this shit wont get sent anyway.
	// fix rest of hack by forcing to false.
	if ( !*g.m_final_packet )
		*g.m_packet = false;

	// fake-lag enabled.
	if ( settings::hvh::antiaim::lag_enable && !( g.m_flags & fl_frozen ) ) {
		// limit of lag.
		const auto limit = std::min( static_cast< int >(settings::hvh::antiaim::lag_limit), g.m_max_lag );

		// indicates wether to lag or not.
		bool active{ };

		// get current origin.
		auto cur = g.m_local->origin( );

		// get prevoius origin.
		const auto prev = g.m_net_pos.empty( ) ? g.m_local->origin( ) : g.m_net_pos.front( ).m_pos;

		// delta between the current origin and the last sent origin.
		const auto delta = ( cur - prev ).length_sqr( );

		const auto activation = settings::hvh::antiaim::lag_active;
		// move.
		if ( activation & ( 1 << 0 ) && (g.m_speed > 0.1f || (g.m_cmd && (g.m_cmd->m_forwardmove != 0.f || g.m_cmd->m_sidemove != 0)))) {
			active = true;
		}

		// air.
		else if ( activation & (1 << 1) && ( ( g.m_buttons & IN_JUMP ) || !( g.m_flags & fl_onground ) ) ) {
			active = true;
		}

		// crouch.
		else if ( activation & (1 << 2) && g.m_local->ducking( ) ) {
			active = true;
		}

		const auto mode = settings::hvh::antiaim::lag_mode;
		if ( active ) {

			// max.

			// break.
			if ( mode == 0 || (mode == 1 && delta <= 4096.f) || mode == 5  )
				*g.m_packet = false;

			// random.
			else if ( mode == 2 ) {
				// compute new factor.
				if ( g.m_lag >= m_random_lag )
					m_random_lag = math::RandomInt( 2, limit );

				// factor not met, keep choking.
				else *g.m_packet = false;
			}

			// break step.
			else if ( mode == 3 ) {
				// normal break.
				if ( m_step_switch ) {
					if ( delta <= 4096.f )
						*g.m_packet = false;
				}

				// max.
				else *g.m_packet = false;
			}

			if ( g.m_lag >= limit )
				*g.m_packet = true;
		}
		if (g.m_lag < 2)
			*g.m_packet = false;
		if (settings::hvh::antiaim::fakehead) {
			if (g.m_lag < g.m_max_lag)
				*g.m_packet = false;
			else
				*g.m_packet = true;
		}
	}
	
	//if ( settings::hvh::antiaim::lag_land ) {
	//	if( !g.m_unpred_ground && g.m_interfaces->entity_list(  )->get_client_entity_handle( g.m_local->m_ground_entity(  ) ) )
	//		*g.m_packet = false;
	//}

	// force fake-lag to 14 when fakewalking.
	if ( settings::hvh::antiaim::fakewalk ) {
		*g.m_packet = false;
	}
	else if( g.m_old_shot )
		*g.m_packet = true;


	// we somehow reached the maximum amount of lag.
	// we cannot lag anymore and we also cannot shoot anymore since we cant silent aim.
	if ( g.m_lag >= g.m_max_lag ) {
		// set bSendPacket to true.
		*g.m_packet = true;

		// disable firing, since we cannot choke the last packet.
		g.m_can_fire = false;
	}
}