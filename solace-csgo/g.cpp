#include <mutex>



#include "aimbot.h"
#include "block_bot.h"
#include "bones.h"
#include "includes.h"
#include "hooks.h"
#include "esp.h"
#include "events.h"
#include "hvh.h"
#include "menu.hh"
#include "movement.h"
#include "notification.h"
#include "prediction.h"
#include "predictioncopy.h"
#include "resolver.h"
#include "input_helper/input_helper.hh"

void c_g::init_cheat( ) {
	m_interfaces = new interfaces_t( );
	m_offsets = new offsets_t( );
	m_hooks = new hooks_t( );
	m_render = new render_t( );
	random_seed = reinterpret_cast< random_seed_t >( GetProcAddress( GetModuleHandleA( "vstdlib.dll" ), "RandomSeed" ) );
	random_int = reinterpret_cast< random_int_t >( GetProcAddress( GetModuleHandleA( "vstdlib.dll" ), "RandomFloat" ) );
	random_float = reinterpret_cast< random_float_t >( GetProcAddress( GetModuleHandleA( "vstdlib.dll" ), "RandomInt" ) );
	events::init( );
	menu.init( );
}

void c_g::release ( ) const {
	delete m_hooks;
	delete m_offsets;
	delete m_interfaces;
}

void c_g::on_render ( IDirect3DDevice9 *device ) {
	m_render->setup(device);
	m_render->start( );

	input_helper.update( );
	
	static auto open = true;
	if ( input_helper.key_pressed( VK_INSERT ) )
		open = !open;

	for ( auto &it : menu.n_binds )
		it->update( );
	
	if ( open ) {
		menu.update( );
		menu.draw( );
	}
	
	m_local = static_cast< player_t * >( m_interfaces->entity_list(  )->get_client_entity(m_interfaces->engine()->local_player_index( ) ) );
	g_movement.draw( );
	g_esp.run( );
	g_block_bot.on_draw( );

	if ( settings::visuals::weapons::noscope ) {
		if ( g.m_local && g.m_local->is_scoped( ) ) {
			m_render->line( m_render->m_screen_size( ).Width / 2, 0, m_render->m_screen_size( ).Width / 2, m_render->m_screen_size( ).Height, color( 0, 0, 0 ), 1);
			m_render->line( 0, m_render->m_screen_size( ).Height / 2, m_render->m_screen_size( ).Width, m_render->m_screen_size( ).Height / 2, color( 0, 0, 0 ), 1);
		}
	}
	
	g_notification.think( );

	m_render->finish( );
}

void c_g::on_tick( cmd_t *cmd ) {
	m_local = static_cast< player_t * >( m_interfaces->entity_list( )->get_client_entity( m_interfaces->engine( )->local_player_index( ) ) );

	if ( !cmd || !cmd->m_command_number )
		return;

	m_cmd = cmd;
	m_tick = cmd->m_tick_count;
	m_view_angles = cmd->m_viewangles;

	auto *nci = g.m_interfaces->engine( )->get_net_channel_info( );
	if ( nci ) {
		m_latency = nci->GetLatency( 0 );
	}

	static auto *cl_interp = g.m_interfaces->console( )->get_convar( "cl_interp" );
	static auto *cl_interp_ratio = g.m_interfaces->console( )->get_convar( "cl_interp_ratio" );
	static auto *cl_updaterate = g.m_interfaces->console( )->get_convar( "cl_updaterate" );
	m_lerp = std::fmaxf( cl_interp->GetFloat( ), cl_interp_ratio->GetFloat( ) / cl_updaterate->GetFloat( ) );

	if ( m_local && m_local->alive( ) ) {
		prediction::update( );
		//m_unpred_ground = m_interfaces->entity_list( )->get_client_entity_handle( m_local->m_ground_entity( ) );
		auto *map = m_local->GetPredDescMap( );
		
		if ( map ) {
			const auto size = max( map->m_packed_size, 4 );
			if ( !m_pStartData ) {
				m_pStartData = new byte[ size ];
				memset( g.m_pStartData, 0, size );
				m_pEndData = new byte[ size ];
				memset( g.m_pEndData, 0, size );
				m_pPostPred = new byte[ size ];
				memset( g.m_pPostPred, 0, size );
			}

			auto CopyHelper = CPredictionCopy( PC_EVERYTHING, static_cast< byte * >(m_pStartData), true, reinterpret_cast< const byte * >(g.m_local), false, CPredictionCopy::TRANSFERDATA_COPYONLY );
			CopyHelper.TransferData( "CM_Start", m_local->index( ), map );

			memcpy( m_pEndData, m_pStartData, sizeof( byte ) * size );
		}

		m_running_client = true;
		m_flags = m_local->flags( );
		m_onground = ( m_flags & fl_onground );
	} else {
		m_running_client = false;
		return;
	}
	
	//if ( m_local ) {
	//	CPredictionCopy CopyHelper( PC_EVERYTHING, m_pStartData, PC_DATA_PACKED, m_local, PC_DATA_NORMAL );
	//	memcpy( m_pEndData, m_pStartData, sz );
	//}

	m_last_lag = m_lag;
	m_lag = g.m_interfaces->client_state()->m_choked_commands;
	
	cmd->m_buttons |= IN_BULLRUSH;
	g_block_bot.on_tick( );
	g_movement.auto_strafe( );
	g_movement.edge_bug();
	g_movement.bhop( );
	g_hvh.fake_walk( );
	g_movement.auto_peek( );
	g_movement.PreciseMove( );
	g_hvh.SendPacket();
	g_hvh.break_resolver();

	if ( g.should_stop )
		g_movement.QuickStop( );

	//auto old_ground = g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->m_ground_entity( ) );
	if ( start_move( m_cmd ) ) {
		//datamap_t *map = m_local->GetPredDescMap( );
		//if ( map && !old_ground && g.m_interfaces->entity_list( )->get_client_entity_handle( g.m_local->m_ground_entity( ) ) ) {
		//	prediction::end( );
		//	m_cmd->m_buttons |= IN_DUCK;
		//	CPredictionCopy CopyHelper( PC_EVERYTHING, ( byte * )g.m_local, false, ( const byte * )m_pStartData, true, CPredictionCopy::TRANSFERDATA_COPYONLY );
		//	CopyHelper.TransferData( "CM_REPREDICT", m_local->index( ), map );
		//	prediction::start( m_cmd );
		//}
		//prediction::start( cmd ); {
		g_aimbot.backup_players( false );
		g_aimbot.on_tick( );
		g_aimbot.backup_players( true );
		ang_t view_angles;
		g.m_interfaces->engine( )->get_view_angles( view_angles );
		g_hvh.m_view_angle = view_angles.y;
		g_hvh.AntiAim( );
		//} prediction::end( );
		if ( g.m_can_shift && g.m_can_fire && ( m_cmd->m_buttons & IN_ATTACK ) )
			g.m_shift = true;

		//if (!m_can_fire && (m_cmd->m_buttons & IN_ATTACK))
		//	m_cmd->m_buttons &= ~IN_ATTACK;

		if ( g.m_shift )
			*g.m_packet = false;
		
		m_cmd->m_viewangles.y = math::normalize_angle( m_cmd->m_viewangles.y, 180.f );
		m_cmd->m_viewangles.x = std::clamp<float>( m_cmd->m_viewangles.x, -89.f, 89.f );
		m_cmd->m_viewangles.z = 0;
		math::correct_movement( m_cmd );
		end_move( m_cmd );
	}
}

void c_g::SetAngles( ) const {
	if ( !m_local || !m_running_client )
		return;

	// set the nointerp flag.
	m_local->m_fEffects( ) |= 0x008;

	// apply the rotation.
	m_local->set_abs_angles( m_rotation );
	m_local->m_angRotation( ) = m_rotation;
	//m_local->m_angNetworkAngles( ) = m_rotation;

	// set radar angles.
	if ( m_interfaces->input(  )->camera_is_third_person( ) )
		m_interfaces->prediction(  )->SetLocalViewAngles( m_radar );
}

void c_g::UpdateAnimations( ) const {
	if ( !m_local || !m_running_client )
		return;

	auto *state = m_local->get_anim_state( );
	if ( !state )
		return;

	// prevent model sway on player.
	m_local->anim_overlay( )[ 12 ].m_weight = 0.f;

	// update animations with last networked data.
	m_local->SetPoseParameters( g.m_poses );

	// update abs yaw with last networked abs yaw.
	m_local->set_abs_angles( ang_t( 0.f, g.m_abs_yaw, 0.f ) );
}

void c_g::UpdateInformation( ) {
	if ( g.m_lag > 0 && !(g.m_can_fire && (g.m_cmd->m_buttons & IN_ATTACK)))
		return;
	//if (!m_animstate)
	//{
	//	m_animstate = CreateCSGOPlayerAnimstate(g.m_local);
	//} else if (g.m_local->spawn_time() != spawn_time) {
	//	// reset animation state.
	//	m_animstate->reset();
	//
	//	// note new spawn time.
	//	spawn_time = g.m_local->spawn_time();
	//}
	auto* state = g.m_local->get_anim_state();
	if (!state)
		return;
	// update time.
	m_anim_frame = g.m_interfaces->globals(  )->m_curtime - m_anim_time;
	m_anim_time = g.m_interfaces->globals()->m_curtime;

	// current angle will be animated.
	m_angle = g.m_cmd->m_viewangles;

	m_angle.x = std::clamp( m_angle.x, -90.f, 90.f );
	

	// write angles to model.
	g.m_interfaces->prediction(  )->SetLocalViewAngles( m_angle );

	// set lby to predicted value.
	m_local->lower_body_yaw( ) = m_body;
	m_local->eye_angles() = m_angle;

	// call original, bypass hook.
	bool moveRight = (g.m_cmd->m_buttons & (IN_MOVERIGHT)) != 0;
	bool moveLeft = (g.m_cmd->m_buttons & (IN_MOVELEFT)) != 0;
	bool moveForward = (g.m_cmd->m_buttons & (IN_FORWARD)) != 0;
	bool moveBackward = (g.m_cmd->m_buttons & (IN_BACK)) != 0;

	//Vector vForward, vRight;
	//AngleVectors( QAngle(0,m_flEyeYaw,0), &vForward, &vRight, NULL );
	//vForward *= 10;
	//vRight *= 10;
	//if ( moveRight )
	//	debugoverlay->AddTriangleOverlay( m_vecPositionCurrent + vRight * 2, m_vecPositionCurrent - vForward, m_vecPositionCurrent + vForward, 200, 0, 0, 255, true, 0 );
	//if ( moveLeft )
	//	debugoverlay->AddTriangleOverlay( m_vecPositionCurrent - vRight * 2, m_vecPositionCurrent + vForward, m_vecPositionCurrent - vForward, 200, 0, 0, 255, true, 0 );
	//if ( moveForward )
	//	debugoverlay->AddTriangleOverlay( m_vecPositionCurrent + vForward * 2, m_vecPositionCurrent + vRight, m_vecPositionCurrent - vRight, 200, 0, 0, 255, true, 0 );
	//if ( moveBackward )
	//	debugoverlay->AddTriangleOverlay( m_vecPositionCurrent - vForward * 2, m_vecPositionCurrent + vRight, m_vecPositionCurrent - vRight, 200, 0, 0, 255, true, 0 );

	vec3_t vecForward;
	vec3_t vecRight;
	ang_t(0, state->m_cur_feet_yaw, 0).vectors(&vecForward, &vecRight, nullptr);
	vecRight.normalize();
	float m_flVelocityLengthXY = fminf(g.m_local->velocity().length(), 260.0f); 
	vec3_t m_vecVelocityNormalizedNonZero = state->m_vecVelocityNormalizedNonZero;
	if (m_flVelocityLengthXY > 0)
	{
		m_vecVelocityNormalizedNonZero = g.m_local->velocity().normalized();
	}

	float flMaxSpeedRun = g.m_weapon_info ? fmaxf(g.m_weapon_info->m_max_player_speed, 0.001f) : 260.0f;
	float m_flSpeedAsPortionOfWalkTopSpeed = m_flVelocityLengthXY / (flMaxSpeedRun * 0.52f);


	float flVelToRightDot = m_vecVelocityNormalizedNonZero.dot(vecRight);
	float flVelToForwardDot = m_vecVelocityNormalizedNonZero.dot(vecForward);

	// We're interested in if the player's desired direction (indicated by their held buttons) is opposite their current velocity.
	// This indicates a strafing direction change in progress.
	//bool bPreviouslyOnLadder = state->m_bOnLadder;
	//bool m_bOnLadder = !m_local->m_ground_entity( ) && m_local->move_type( ) == MoveType_t::MOVETYPE_LADDER;
	//bool bStartedLadderingThisFrame = ( !bPreviouslyOnLadder && m_bOnLadder );
	//bool bStoppedLadderingThisFrame = ( bPreviouslyOnLadder && !m_bOnLadder );
	//
	//if ( state->m_flLadderWeight > 0 || m_bOnLadder ) {
	//	if ( bStartedLadderingThisFrame ) {
	//		m_local->anim_overlay()[ ]( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, SelectSequenceFromActMods( ACT_CSGO_CLIMB_LADDER ) );
	//	}
	//}
	bool bStrafeRight = (m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && moveRight && !moveLeft && flVelToRightDot < -0.63f);
	bool bStrafeLeft = (m_flSpeedAsPortionOfWalkTopSpeed >= 0.73f && moveLeft && !moveRight && flVelToRightDot > 0.63f);
	bool bStrafeForward = (m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && moveForward && !moveBackward && flVelToForwardDot < -0.55f);
	bool bStrafeBackward = (m_flSpeedAsPortionOfWalkTopSpeed >= 0.65f && moveBackward && !moveForward && flVelToForwardDot > 0.55f);
	*(bool*)(g.m_local + 14816) = (bStrafeRight || bStrafeLeft || bStrafeForward || bStrafeBackward);
	g.m_hooks->players_hook( g.m_local->index( ) - 1 )->get_original< void( __thiscall * ) ( player_t * ) >( 218 )( g.m_local );
	//if (state->m_ground) {
	//	if (state->m_land) {
	//		
	//	}
	//}

	//m_animstate->update( g.m_local->eye_angles( ) );
	if ( !g.m_real_bones )
		g.m_real_bones = static_cast< bone_array_t * >( g.m_interfaces->mem_alloc( )->alloc( sizeof( bone_array_t ) * 128 ) );
	g.m_interfaces->mdlcache()->begin_coarse_lock();
	g.m_interfaces->mdlcache()->begin_lock();
	g.m_bones_setup = g_bones.BuildBonesStripped( m_local, bone_used_by_anything, g.m_real_bones, &g.m_ipk );
	g.m_interfaces->mdlcache()->end_lock();
	g.m_interfaces->mdlcache()->end_coarse_lock();
	if( g.m_bones_setup ) {
		const auto abs_origin = g.m_local->origin( );
		for ( auto i = 0; i < 128; i++ ) {
			g.m_real_bones[i].mat_val[ 0 ][ 3 ] -= abs_origin.x;
			g.m_real_bones[i].mat_val[ 1 ][ 3 ] -= abs_origin.y;
			g.m_real_bones[i].mat_val[ 2 ][ 3 ] -= abs_origin.z;
		}
	}
	m_local->GetPoseParameters( m_poses );

	// store updated abs yaw.
	m_abs_yaw = state->m_goal_feet_yaw;// *(float*)(m_animstate + 112);

	// we landed.
	if (!m_ground && state->m_ground){// *(bool*)(m_animstate + 248)) {
		
		m_body = m_angle.y;
		m_body_pred = m_anim_time;
	}

	// walking, delay lby update by .22.
	else if(state->m_speed > 0.1f){ //if (*(float*)(m_animstate + 55) > 0.1f ) {
		if(state->m_ground)//if (*(bool*)(m_animstate + 248) )
			m_body = m_angle.y;

		m_body_pred = m_anim_time + 0.22f;
	}

	// standing update every 1.1s
	else if ( m_anim_time >= g.m_body_pred ) {
		m_body = m_angle.y;
		m_body_pred = m_anim_time + 1.1f;
	}

	// save updated data.
	m_rotation = m_local->m_angAbsRotation( );
	m_speed = state->m_speed;// *(float*)(m_animstate + 55);
	m_ground = state->m_ground;// *(bool*)(m_animstate + 248);
}

int c_g::time_to_ticks ( float time ) const {
	return static_cast< int >(time / m_interfaces->globals( )->m_interval_per_tick);
}

float c_g::ticks_to_time( int ticks) const {
	return static_cast< float >( ticks ) * m_interfaces->globals( )->m_interval_per_tick;
}

bool c_g::can_weapon_fire( ) const {
	// the player cant fire.
	if ( !m_player_fire )
		return false;

	if ( m_weapon_type == WEAPONTYPE_GRENADE )
		return false;

	// if we have no bullets, we cant shoot.
	if ( m_weapon_type == WEAPONTYPE_KNIFE || m_weapon->clip1_count( ) < 1 )
		return false;

	// yeez we have a normal gun.
	if ( ( static_cast< float >(g.m_local->tick_base( )) * g.m_interfaces->globals( )->m_interval_per_tick ) >= m_weapon->next_primary_attack( ) )
		return true;

	return false;
}
//
//void UTIL_FieldHighLowTickBase(int* high, int* low, int* ideal) {
//	auto* sv_clockcorrection_msecs = g.m_interfaces->console()->get_convar("sv_clockcorrection_msecs");
//	float flCorrectionSeconds = std::clamp(sv_clockcorrection_msecs->GetFloat() / 1000.0f, 0.0f, 1.0f);
//	int nCorrectionTicks = g.time_to_ticks(flCorrectionSeconds);
//
//	// Set the target tick flCorrectionSeconds (rounded to ticks) ahead in the future. this way the client can
//	//  alternate around this target tick without getting smaller than gpGlobals->tickcount.
//	// After running the commands simulation time should be equal or after current gpGlobals->tickcount, 
//	//  otherwise the simulation time drops out of the client side interpolated var history window.
//
//	int	nIdealFinalTick = g.m_interfaces->globals()->m_tickcount + nCorrectionTicks;
//	auto simulation_ticks = 1;
//	int nEstimatedFinalTick = g.m_local->tick_base() + simulation_ticks;
//
//	// If client gets ahead of this, we'll need to correct
//	int	 too_fast_limit = nIdealFinalTick + nCorrectionTicks;
//	// If client falls behind this, we'll also need to correct
//	int	 too_slow_limit = nIdealFinalTick - nCorrectionTicks;
//
//}
//
//void UTIL_EmplaceTickBaseShift(int command_num, int high, int low, int ideal, int ticks, int ticks2) {
//	if (nEstimatedFinalTick > high ||
//		nEstimatedFinalTick < low)
//	{
//		int nCorrectedTick = ideal - simulation_ticks + gpGlobals->simTicksThisFrame;
//
//		g.m_local->tick_base() = nCorrectedTick;
//	}
//}
//
void c_g::on_move ( float accumulated_extra_samples, bool bFinalTick, cl_move_t cl_move ) {

	static auto *sv_maxusrcmdprocessticks = g.m_interfaces->console( )->get_convar( "sv_maxusrcmdprocessticks" );
	const auto iProcessTicks = sv_maxusrcmdprocessticks->GetInt( );
	auto iMaxExtraTicks = 15;
	m_local = static_cast<player_t*>(m_interfaces->entity_list()->get_client_entity(m_interfaces->engine()->local_player_index()));
	if (!m_local || !m_local->alive()) {
		cl_move(accumulated_extra_samples, bFinalTick);
		return;
	}
	
	if ( iMaxExtraTicks > iProcessTicks - 1 ) {
		iMaxExtraTicks = iProcessTicks - 1;
	}
	
	if ( m_available_ticks < iMaxExtraTicks ) {
		m_available_ticks++;
		m_can_shift = false;
		m_shift = false;
		return;
	}
	
	m_can_shift = true;
	cl_move( accumulated_extra_samples, bFinalTick );
	m_can_shift = false;

	auto iExtraTicks = 0;
	static auto *frame_ticks = util::find( "engine.dll", "2B 05 ? ? ? ? 03 05 ? ? ? ? 83 CF ?" ) + 2;
	auto iTicksThisCommand = **reinterpret_cast<int**>(frame_ticks);
	if( m_shift )
		for ( auto i = 1; i < m_available_ticks; i++ ) {
			if ( iTicksThisCommand >= iProcessTicks ||
				 iTicksThisCommand > 15 ||
				 iExtraTicks >= 15 ) {
				break;
			}
			
			m_available_ticks--;
			iTicksThisCommand++;
			iExtraTicks++;
	
			m_shift = (iTicksThisCommand >= iProcessTicks ||
				iTicksThisCommand > 15 ||
				iExtraTicks >= 15);
	
			cl_move( accumulated_extra_samples, m_shift );
		}
	m_shift = false;
}

void c_g::UpdateIncomingSequences( i_net_channel *net ) {
	if ( m_sequences.empty( ) || net->m_in_seq > m_sequences.front( ).m_seq ) {
		// store new stuff.
		m_sequences.emplace_front( g.m_interfaces->globals(  )->m_curtime, net->m_in_rel_state, net->m_in_seq );
	}

	// do not save too many of these.
	while ( m_sequences.size( ) > 2048 )
		m_sequences.pop_back( );
}

void c_g::net_data_received ( ) const {
	if ( m_pEndData ) {
		CPredictionCopy CopyHelper( PC_EVERYTHING, m_pEndData, TD_OFFSET_PACKED, reinterpret_cast< byte * >(m_local), TD_OFFSET_NORMAL, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "net_data_received", m_local->index( ), m_local->GetPredDescMap( ) );
	}
}

bool c_g::start_move( cmd_t *cmd ) {
	should_stop = false;
	m_can_fire = false;
	m_weapon = static_cast< weapon_t * >( g.m_interfaces->entity_list( )->get_client_entity_handle( m_local->active_weapon( ) ) );
	m_origin = m_local->origin( );
	m_max_lag = ( m_local->flags( ) & fl_onground ) ? 16 : 15;


	prediction::start( cmd );
	m_speed = m_local->velocity().length();

	m_player_fire = ( g.m_local->tick_base( ) * g.m_interfaces->globals( )->m_interval_per_tick ) >= m_local->next_attack( ) && !( g.m_flags & fl_frozen );
	if ( m_weapon ) {
		m_weapon_info = g.m_interfaces->weapon_system( )->get_weapon_data( m_weapon->item_definition_index( ) );
		if ( m_weapon_info )
			m_weapon_type = m_weapon_info->m_weapon_type;
		m_can_fire = can_weapon_fire( );
	}

	m_force_strafe = false;
	const auto abs_origin = g.m_local->origin( );
	auto *bone_cache = &g.m_local->bone_cache( );
	bone_array_t *backup_cache = nullptr;
	if ( bone_cache && g.m_bones_setup ) {
		for ( auto i = 0; i < 128; i++ ) {
			g.m_real_bones[ i ].mat_val[ 0 ][ 3 ] += abs_origin.x;
			g.m_real_bones[ i ].mat_val[ 1 ][ 3 ] += abs_origin.y;
			g.m_real_bones[ i ].mat_val[ 2 ][ 3 ] += abs_origin.z;
		}
		backup_cache = bone_cache->m_pCachedBones;
		bone_cache->m_pCachedBones = g.m_real_bones;
	}
	m_local->get_eye_pos( &m_shoot_pos );
	if ( g.m_bones_setup && bone_cache ) {
		m_local->get_anim_state( )->ModifyEyePosition( g.m_real_bones, &m_shoot_pos );
		for ( auto i = 0; i < 128; i++ ) {
			g.m_real_bones[ i ].mat_val[ 0 ][ 3 ] -= abs_origin.x;
			g.m_real_bones[ i ].mat_val[ 1 ][ 3 ] -= abs_origin.y;
			g.m_real_bones[ i ].mat_val[ 2 ][ 3 ] -= abs_origin.z;
		}
		bone_cache->m_pCachedBones = backup_cache;
	}
	if ( m_weapon )
		if ( m_weapon_type != WEAPONTYPE_GRENADE )// ensure weapon spread values / etc are up to date.
			m_weapon->update_accuracy_penalty( );

	m_shot = false;

	auto *map = g.m_local->GetPredDescMap( );
	if ( m_pStartData && map ) {
		CPredictionCopy CopyHelper( PC_EVERYTHING, static_cast< byte * >(m_pPostPred), true, reinterpret_cast< const byte * >(g.m_local), false, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "PostPred", m_local->index( ), map );
	}
	
	return true;
}

void c_g::end_move ( cmd_t *cmd ) {
	cmd->m_forwardmove = std::clamp<float>( cmd->m_forwardmove, -450.f, 450.f );
	cmd->m_sidemove = std::clamp<float>( cmd->m_sidemove, -450.f, 450.f );
	auto *map = g.m_local->GetPredDescMap( );
	if ( m_pStartData && map ) {
		prediction::end( );
		CPredictionCopy CopyHelper( PC_EVERYTHING, ( byte * )g.m_local, false, static_cast< const byte * >(m_pStartData), true, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "CM_REPREDICT", m_local->index( ), map );
		prediction::start( g.m_cmd );
	}
	if ( *m_packet ) {
		g_hvh.m_step_switch = static_cast< bool >( g.random_int( 0, 1 ) );

		// we are sending a packet, so this will be reset soon.
		// store the old value.
		m_old_lag = m_lag;

		// get radar angles.
		m_radar = cmd->m_viewangles;

		// get current origin.
		auto cur = m_local->origin( );

		// get prevoius origin.
		const auto prev = m_net_pos.empty( ) ? cur : m_net_pos.front( ).m_pos;

		// check if we broke lagcomp.
		m_lagcomp = ( cur - prev ).length_sqr( ) > 4096.f;

		// save sent origin and time.
		m_net_pos.emplace_front( g.m_interfaces->globals( )->m_curtime, cur );
	}
	UpdateInformation( );
	if( g.m_shot && g.m_bones_setup ) {
		//g_aimbot.draw_hitboxes( g.m_local, g.m_real_bones );
	}
	prediction::end( );
	if ( m_pEndData && map ) {
		CPredictionCopy CopyHelper( PC_EVERYTHING, static_cast< byte * >(m_pEndData), true, ( const byte * )g.m_local, false, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "CM_END", m_local->index( ), map );
	}
	if ( m_pStartData && map ) {
		CPredictionCopy CopyHelper( PC_EVERYTHING, ( byte * )g.m_local, false, static_cast< const byte * >(m_pStartData), true, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "CM_RESTORE", m_local->index( ), map );
	}
	
	m_old_packet = *m_packet;
	m_old_shot = m_shot;
}