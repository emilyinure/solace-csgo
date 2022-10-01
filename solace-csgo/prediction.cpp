#include "prediction.h"
#include "includes.h"
#include "sdk.h"

void adjust_time( ) {
	static auto sv_clockcorrection_msecs = g.m_interfaces->console( )->get_convar( "sv_clockcorrection_msecs" );
	float flCorrectionSeconds = std::clamp( sv_clockcorrection_msecs->GetFloat() / 1000.0f, 0.0f, 1.0f );
	int nCorrectionTicks = g.time_to_ticks( flCorrectionSeconds );

	// Set the target tick flCorrectionSeconds (rounded to ticks) ahead in the future. this way the client can
	//  alternate around this target tick without getting smaller than gpGlobals->tickcount.
	// After running the commands simulation time should be equal or after current gpGlobals->tickcount, 
	//  otherwise the simulation time drops out of the client side interpolated var history window.

	auto* nci = g.m_interfaces->engine( )->get_net_channel_info( );
	int	nIdealFinalTick = g.m_interfaces->globals()->m_tickcount + nCorrectionTicks + nci->GetLatency( 2 );;

	int nEstimatedFinalTick = g.m_local->tick_base() + 1;

	// If client gets ahead of this, we'll need to correct
	int	 too_fast_limit = nIdealFinalTick + nCorrectionTicks;
	// If client falls behind this, we'll also need to correct
	int	 too_slow_limit = nIdealFinalTick - nCorrectionTicks;

	// See if we are too fast
	if ( nEstimatedFinalTick > too_fast_limit ||
		nEstimatedFinalTick < too_slow_limit ) {
		int nCorrectedTick = nIdealFinalTick - g.m_interfaces->client_state()->m_choked_commands;

		g.m_local->tick_base( ) = nCorrectedTick;
	}
}

void prediction::start( cmd_t *cmd ) {
	if ( !g.m_local )
		return;

	g.m_interfaces->prediction()->SuppressHostEvents(g.m_local);
	g.m_in_pred = true;
	static player_move_data data;
	if ( !prediction_random_seed )
		prediction_random_seed = address{ g.m_interfaces->prediction( ).hook( )->get_original( 19 ) }.add( 0x30 ).get< int * >( );
	if ( !prediction_player )
		prediction_player = address{ g.m_interfaces->prediction( ).hook( )->get_original( 19 ) }.add( 0x54 ).get< player_t * >( );

	*prediction_random_seed = cmd->m_randomseed;
	prediction_player = g.m_local;

	g.m_interfaces->move_helper( )->set_host( g.m_local );

	typedef char( __thiscall *start_command_t )( void *, void * );
	static auto start_command = reinterpret_cast< start_command_t >(util::find( "client.dll", "55 8B EC 8B 55 ? 3B CA" ));
	g.m_local->m_cmd( ) = cmd;
	start_command( &g.m_local->m_cmd_ukn( ), cmd );

	//pPlayer->SetLocalViewAngles(pCmd->viewangles);
	//g_cl.m_weapon->update_accuracy_penalty();

	old_cur_time = g.m_interfaces->globals( )->m_curtime;
	old_frame_time = g.m_interfaces->globals( )->m_frametime;

	g.m_interfaces->globals( )->m_curtime = g.ticks_to_time( g.m_local->tick_base( ));
	g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_interval_per_tick;

	g.m_interfaces->prediction( )->bIsFirstTimePredicted = false;
	g.m_interfaces->prediction( )->bInPrediction = true;
	
	g.m_interfaces->game_movement( )->start_track_prediction_errors( g.m_local );
	if ( cmd->m_weaponselect != 0 ) {

	}

	const auto backup_buttons = cmd->m_buttons;
	cmd->m_buttons |= *( int * )( ( uintptr_t )g.m_local + 0x3310 );

	g.m_local->UpdateButtons( g.m_local, cmd->m_buttons );
	g.m_local->set_angles( cmd->m_viewangles );

	cmd->m_buttons = backup_buttons;

	g.m_interfaces->prediction( )->check_moving_ground( g.m_local, g.m_interfaces->globals( )->m_frametime );

	typedef char( __thiscall *physics_run_think_t )( void *, int );
	static auto physics_run_think = reinterpret_cast< physics_run_think_t >(util::find( "client.dll",
	                                                                                    "55 8B EC 83 EC ? 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8 ?" ));
	if ( physics_run_think( g.m_local, 0 ) )
		g.m_local->PreThink( );

	typedef void( __thiscall *set_next_think_t )( void *, char );
	static auto set_next_think = reinterpret_cast< set_next_think_t >(util::find( "client.dll", "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6" ));
	const auto next_think = g.m_local->m_next_think( );
	if ( next_think > 0 && next_think <= *( int * )( g.m_local + 0x3430 ) ) {
		g.m_local->m_next_think( ) = -1;
		set_next_think( g.m_local, next_think );
		g.m_local->Think( );// pPlayer->Think();
	}

	memset( &data, 0, sizeof( data ) );

	g.m_interfaces->prediction(  )->setup_move( g.m_local, cmd, g.m_interfaces->move_helper(  ), &data );

	g.m_interfaces->game_movement(  )->process_movement( g.m_local, &data );

	g.m_interfaces->prediction( )->finish_move( g.m_local, cmd, &data );


	//g.m_local->PostThink( );
	g.m_interfaces->game_movement( )->finish_track_prediction_errors( g.m_local );

	g.m_interfaces->prediction( )->bIsFirstTimePredicted = false;
	adjust_time( );
	//g.m_local->tick_base()++;
	g.m_interfaces->prediction()->SuppressHostEvents(nullptr);
	g.m_in_pred = false;
}

void prediction::update( ) {
	auto valid{ g.m_interfaces->client_state(  )->m_delta_tick > 0 };
	//int         outgoing_command, current_command;
	//CUserCmd    *cmd;

	// render start was not called.
	if ( g.m_stage == FRAME_NET_UPDATE_END ) {
		/*outgoing_command = g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands;
		// this must be done before update ( update will mark the unpredicted commands as predicted ).
		for( int i{}; ; ++i ) {
			current_command = g_csgo.m_cl->m_last_command_ack + i;
			// caught up / invalid.
			if( current_command > outgoing_command || i >= MULTIPLAYER_BACKUP )
				break;
			// get command.
			cmd = g_csgo.m_input->GetUserCmd( current_command );
			if( !cmd )
				break;
			// cmd hasn't been predicted.
			// m_nTickBase is incremented inside RunCommand ( which is called frame by frame, we are running tick by tick here ) and prediction hasn't run yet,
			// so we must fix tickbase by incrementing it ourselves on non-predicted commands.
			if( !cmd->m_predicted )
				++g_cl.m_local->m_nTickBase( );
		}*/

		// EDIT; from what ive seen RunCommand is called when u call Prediction::Update
		// so the above code is not fucking needed.


		const auto start = g.m_interfaces->client_state( )->m_last_command_ack;
		const auto stop = g.m_interfaces->client_state( )->m_last_outgoing_command + g.m_interfaces->client_state( )->m_choked_commands;

		// call CPrediction::Update.
		g.m_interfaces->prediction(  )->update( g.m_interfaces->client_state( )->m_delta_tick, g.m_interfaces->client_state( )->m_delta_tick > 0, start, stop );
	}

	static auto unlocked_fakelag = false;
	if ( !unlocked_fakelag ) {
		auto *const cl_move_clamp = util::find( "engine.dll", "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ) + 1;
		unsigned long protect = 0;

		VirtualProtect( static_cast< void * >(cl_move_clamp), 4, PAGE_EXECUTE_READWRITE, &protect );
		*reinterpret_cast< std::uint32_t * >(cl_move_clamp) = 62;
		VirtualProtect( static_cast< void * >(cl_move_clamp), 4, protect, &protect );
		unlocked_fakelag = true;
	}
}
void prediction::end( ) {
	if ( !g.m_local )
		return;
	
	g.m_interfaces->prediction(  )->bInPrediction = false;

	g.m_interfaces->move_helper(  )->set_host( nullptr );
	g.m_local->m_cmd( ) = nullptr;

	*prediction_random_seed = -1;
	prediction_player = nullptr;

	
	g.m_interfaces->globals( )->m_curtime = old_cur_time;
	g.m_interfaces->globals( )->m_frametime = old_cur_time;
	//g.m_cmd->m_predicted = true;
}
void prediction::finish_partial_frame( player_t * player, cmd_t *cmd ) {
	if ( !g.m_local )
		return;

	g.m_in_pred = true;
	static player_move_data data;
	if ( !prediction_random_seed )
		prediction_random_seed = address{ g.m_interfaces->prediction( ).hook( )->get_original( 19 ) }.add( 0x30 ).get< int * >( );
	if ( !prediction_player )
		prediction_player = address{ g.m_interfaces->prediction( ).hook( )->get_original( 19 ) }.add( 0x54 ).get< player_t * >( );

	*prediction_random_seed = cmd->m_randomseed;
	prediction_player = g.m_local;

	g.m_interfaces->move_helper( )->set_host( g.m_local );

	typedef char( __thiscall *start_command_t )( void *, void * );
	static auto start_command = reinterpret_cast< start_command_t >( util::find( "client.dll", "55 8B EC 8B 55 ? 3B CA" ) );
	g.m_local->m_cmd( ) = cmd;
	start_command( &g.m_local->m_cmd_ukn( ), cmd );

	//pPlayer->SetLocalViewAngles(pCmd->viewangles);
	//g_cl.m_weapon->update_accuracy_penalty();

	old_cur_time = g.m_interfaces->globals( )->m_curtime;
	old_frame_time = g.m_interfaces->globals( )->m_frametime;

	g.m_interfaces->globals( )->m_curtime = g.ticks_to_time( g.m_local->tick_base( ) );
	g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_interval_per_tick;

	g.m_interfaces->prediction( )->bIsFirstTimePredicted = false;
	g.m_interfaces->prediction( )->bInPrediction = true;

	g.m_interfaces->prediction( )->bIsFirstTimePredicted = true;
	g.m_interfaces->prediction( )->bInPrediction = true;

	g.m_interfaces->game_movement( )->start_track_prediction_errors( g.m_local );
	if ( cmd->m_weaponselect != 0 ) {

	}

	const auto backup_buttons = cmd->m_buttons;
	cmd->m_buttons |= *reinterpret_cast< int * >(reinterpret_cast< uintptr_t >(g.m_local) + 0x3310);

	g.m_local->UpdateButtons( g.m_local, cmd->m_buttons );
	g.m_local->set_angles( cmd->m_viewangles );

	cmd->m_buttons = backup_buttons;

	g.m_interfaces->prediction( )->check_moving_ground( g.m_local, g.m_interfaces->globals( )->m_frametime );

	typedef char( __thiscall *physics_run_think_t )( void *, int );
	static auto physics_run_think = reinterpret_cast< physics_run_think_t >(util::find( "client.dll",
	                                                                                    "55 8B EC 83 EC ? 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8 ?" ));
	if ( physics_run_think( g.m_local, 0 ) )
		g.m_local->PreThink( );

	typedef void( __thiscall *set_next_think_t )( void *, char );
	static auto set_next_think = reinterpret_cast< set_next_think_t >(util::find( "client.dll", "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6" ));
	const auto next_think = g.m_local->m_next_think( );
	if ( next_think > 0 && next_think <= *( int * )( g.m_local + 0x3430 ) ) {
		g.m_local->m_next_think( ) = -1;
		set_next_think( g.m_local, next_think );
		g.m_local->Think( );// pPlayer->Think();
	}

	memset( &data, 0, sizeof( data ) );

	g.m_interfaces->prediction( )->setup_move( g.m_local, cmd, g.m_interfaces->move_helper( ), &data );

	g.m_interfaces->game_movement( )->process_movement( g.m_local, &data );

	g.m_interfaces->prediction( )->finish_move( g.m_local, cmd, &data );


	g.m_local->PostThink( );
	g.m_interfaces->game_movement( )->finish_track_prediction_errors( g.m_local );

	if ( g.m_interfaces->globals( )->m_frametime > 0.f ) {
		player->tick_base( )++;
	}

	prediction::end( );
	g.m_interfaces->prediction( )->bIsFirstTimePredicted = false;
	cmd->m_predicted = true;
	g.m_in_pred = false;
}
void prediction::re_predict ( cmd_t *cmd ) {
	if ( !g.m_local )
		return;

	g.m_in_pred = true;
	g.m_interfaces->prediction()->SuppressHostEvents(g.m_local);
	static player_move_data data;

	if ( !prediction_random_seed )
		prediction_random_seed = address{ util::get_virtual_function( g.m_interfaces->prediction( ), 19 ) }.add( 0x30 ).get< int * >( );
	if ( !prediction_player )
		prediction_player = address{ util::get_virtual_function( g.m_interfaces->prediction( ), 19 ) }.add( 0x54 ).get< player_t * >( );

	*prediction_random_seed = cmd->m_randomseed;
	prediction_player = g.m_local;

	g.m_interfaces->move_helper( )->set_host( g.m_local );

	typedef char( __thiscall *start_command_t )( void *, void * );
	static auto start_command = ( start_command_t )util::find( "client.dll", "55 8B EC 8B 55 ? 3B CA" );
	g.m_local->m_cmd( ) = cmd;
	start_command( &g.m_local->m_cmd_ukn( ), cmd );

	//pPlayer->SetLocalViewAngles(pCmd->viewangles);
	//g_cl.m_weapon->update_accuracy_penalty();

	old_cur_time = g.m_interfaces->globals( )->m_curtime;
	old_frame_time = g.m_interfaces->globals( )->m_frametime;

	g.m_interfaces->globals( )->m_curtime = ( g.m_local->tick_base( ) * g.m_interfaces->globals( )->m_interval_per_tick );
	g.m_interfaces->globals( )->m_frametime = g.m_interfaces->globals( )->m_interval_per_tick;

	g.m_interfaces->prediction( )->bIsFirstTimePredicted = false;
	g.m_interfaces->prediction( )->bInPrediction = true;

	g.m_interfaces->game_movement( )->start_track_prediction_errors( g.m_local );
	if ( cmd->m_weaponselect != 0 ) {

	}

	const auto backup_buttons = cmd->m_buttons;
	cmd->m_buttons |= *reinterpret_cast< int * >(reinterpret_cast< uintptr_t >(g.m_local) + 0x3310);
	
	g.m_local->UpdateButtons( g.m_local, cmd->m_buttons );
	g.m_local->set_angles( cmd->m_viewangles );

	cmd->m_buttons = backup_buttons;
	auto backup_ground = g.m_local->m_ground_entity(  );
	//g.m_interfaces->prediction( )->check_moving_ground( g.m_local, g.m_interfaces->globals( )->m_frametime );

	typedef char( __thiscall *physics_run_think_t )( void *, int );
	static auto physics_run_think = reinterpret_cast< physics_run_think_t >(util::find( "client.dll",
	                                                                                    "55 8B EC 83 EC ? 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8 ?" ));
	if ( physics_run_think( g.m_local, 0 ) )
		g.m_local->PreThink( );

	typedef void( __thiscall *set_next_think_t )( void *, char );
	//static auto set_next_think = ( set_next_think_t )util::find( "client.dll", "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6" );
	//int next_think = g.m_local->m_next_think( );
	//if ( next_think > 0 && next_think <= *( int * )( g.m_local + 0x3430 ) ) {
	//	g.m_local->m_next_think( ) = -1;
	//	set_next_think( g.m_local, next_think );
	//	g.m_local->Think( );// pPlayer->Think();
	//}

	memset( &data, 0, sizeof( data ) );
	g.m_interfaces->move_helper( )->set_host( g.m_local );

	g.m_interfaces->prediction( )->setup_move( g.m_local, cmd, g.m_interfaces->move_helper( ), &data );

	g.m_interfaces->game_movement( )->process_movement( g.m_local, &data );

	g.m_interfaces->prediction( )->finish_move( g.m_local, cmd, &data );

	g.m_interfaces->game_movement( )->finish_track_prediction_errors( g.m_local );

	g.m_local->PostThink( );
	
	g.m_interfaces->prediction( )->bIsFirstTimePredicted = false;	
	g.m_interfaces->prediction( )->bInPrediction = false;
	
	end( );
	g.m_interfaces->prediction()->SuppressHostEvents(nullptr);
	g.m_in_pred = false;
}
