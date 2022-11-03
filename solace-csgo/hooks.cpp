#include "hooks.h"
#include <future>
#include "detours.h"

#include <iostream>
#include <fstream>

#include "esp.h"
#include "hvh.h"
#include "movement.h"
#include "netvar_manager.h"
#include "prediction.h"
#include "predictioncopy.h"
#include "resolver.h"
#include "windowsx.h"
#include "input_helper/input_helper.hh"
#include "thread_handler.h"

void _stdcall paint_traverse( long panel, bool repaint, bool force ) {
	static long tools{}, zoom{};

	// cache CHudZoom panel once.
	if ( !zoom && fnv::hash( g.m_interfaces->panel()->GetName( panel ) ) == fnv::hash( "HudZoom" ) )
		zoom = panel;
	
	if ( panel == zoom && settings::visuals::weapons::noscope )
		return;

	using PaintTraverse_t = void( __thiscall * )( void *, long, bool, bool );
	g.m_interfaces->panel(  ).hook(  )->get_original< PaintTraverse_t >( panel_t::PAINTTRAVERSE )( g.m_interfaces->panel(  ).operator void *(  ), panel, repaint, force );
}

bool _stdcall create_move( float input_sample_frametime, cmd_t *cmd ) {
	g.m_interfaces->client_mode( ).hook( )->get_original<bool (_stdcall*)( float , cmd_t * )>(24)(input_sample_frametime, cmd );
	
	uintptr_t *frame_pointer;
	__asm mov frame_pointer, ebp;

	g.m_packet = reinterpret_cast< bool * >( *frame_pointer - 0x1C );
	g.m_final_packet = reinterpret_cast< bool * >( *frame_pointer - 0x1b );

	
	g.on_tick( cmd );
	
	//if ( *g.m_final_packet && *g.m_packet && cmd->m_command_number )
	//	g.m_cmds.push_back( cmd->m_command_number );

	//auto this_ptr = g.m_interfaces->engine( )->get_net_channel_info( );
	//if ( this_ptr && g.m_local && g.m_shot && settings::hvh::antiaim::lag_enable && settings::hvh::antiaim::lag_mode == 5 && this_ptr->m_choked_packets > 7 && !settings::hvh::antiaim::fakewalk ) {
	//	int nextcommandnr = g.m_interfaces->client_state( )->m_last_outgoing_command + 1;
	//
	//	cmd_t *cmdlist = *reinterpret_cast< cmd_t ** >( g.m_interfaces->input( ).operator address( ) + 0xEC );
	//	cmd_t *old_cmd = &cmdlist[ nextcommandnr % 150 ];
	//	verified_cmd_t *verifiedCmdList = *reinterpret_cast< verified_cmd_t ** >( g.m_interfaces->input( ).operator address( ) + 0xF0 );
	//	verified_cmd_t *verified = &verifiedCmdList[ nextcommandnr % 150 ];
	//	if ( old_cmd && old_cmd->m_command_number ) {
	//		for ( auto i : g.m_choked_logs ) {
	//			if ( i.commandnr == old_cmd->m_command_number ) {
	//				float backup_time = g.m_interfaces->globals( )->m_curtime;
	//				float backup_body_pred = g.m_body_pred;
	//				float backup_body = g.m_body;
	//				vec3_t backup_velocity = g.m_local->velocity( );
	//				vec3_t backup_origin = g.m_local->origin( );
	//				g.m_flags = i.flags;
	//				g.m_interfaces->globals( )->m_curtime = i.curtime;
	//				g.m_body_pred = i.m_body_pred;
	//				g.m_local->velocity( ) = i.m_velocity;
	//				g.m_local->origin( ) = i.origin;
	//				g.m_view_angles = i.view_angles;
	//				g_hvh.m_view_angle = i.view_angle;
	//				old_cmd->m_forwardmove = i.forwardmove;
	//				old_cmd->m_sidemove = i.sidemove;
	//				g.m_shoot_pos = i.shoot_pos;
	//				g.m_cmd = old_cmd;
	//
	//				g.m_cmds.push_back( i.commandnr );
	//				g.m_weapon = nullptr;
	//				g.m_can_fire = false;
	//				*g.m_packet = true;
	//				bool backup_final = *g.m_final_packet;
	//				*g.m_final_packet = true;
	//				g_hvh.AntiAim( );
	//				*g.m_final_packet = backup_final;
	//				math::correct_movement( old_cmd );
	//				g.m_interfaces->globals( )->m_curtime = backup_time;
	//				g.m_body_pred = backup_body_pred;
	//				g.m_body = backup_body;
	//				g.m_local->velocity( ) = backup_velocity;
	//				g.m_local->origin( ) = backup_origin;
	//				verified->m_cmd = *old_cmd;
	//				verified->m_crc = old_cmd->GetChecksum( );
	//				g.ran = true;
	//			}
	//		}
	//	}
	//	const auto current_choke = this_ptr->m_choked_packets;
	//	this_ptr->m_choked_packets -= 7;
	//	g.m_interfaces->client_state( )->m_choked_commands -= 7;
	//	
	//	//*reinterpret_cast< bool * >( *frame_pointer - 0x1C ) = true;
	//	g.restore_choke = ( current_choke - this_ptr->m_choked_packets );
	//}
	return g.m_force_view;
}

//void __stdcall draw_model_execute( uintptr_t ctx, const draw_model_state_t &state, const model_render_info_t &info, matrix3x4_t *bone ) {
//	// do chams.
//	g_cl.m_local = static_cast< player_t * >( interfaces::entity_list->get_client_entity( interfaces::engine->get_local_player( ) ) );
//	if ( g_chams.draw_model( info ) ) {
//		model_render_target.GetOldMethod< fn >( 21 )( interfaces::model_render, ctx, state, info, bone );
//	} else {
//
//	}
//
//	// disable material force for next call.
//	//g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
//}

void __stdcall scene_end( ) {

}

long __stdcall end_scene( IDirect3DDevice9 *pDevice ) {
	static auto end_scene_target = g.m_interfaces->device( ).hook( )->get_original<decltype( &end_scene )>( 42 );
	if ( !pDevice )
		return end_scene_target( pDevice );
	
	static std::uintptr_t gameoverlay_return_address = 0;
	//
	//MEMORY_BASIC_INFORMATION info;
	//VirtualQuery( _ReturnAddress( ), &info, sizeof( MEMORY_BASIC_INFORMATION ) );
	//
	//char mod[ MAX_PATH ];
	//GetModuleFileNameA( static_cast< HMODULE >( info.AllocationBase ), mod, MAX_PATH );
	//
	//if ( strstr( mod, "gameoverlay" ) )
	//	gameoverlay_return_address = reinterpret_cast< std::uintptr_t >( _ReturnAddress( ) );
	//
	//if ( gameoverlay_return_address != reinterpret_cast< std::uintptr_t >( _ReturnAddress( ) ) )
	//	 return end_scene_target( pDevice );
	
	g.on_render( pDevice );
	return end_scene_target( pDevice );
}

LRESULT __stdcall wndproc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	if ( uMsg == WM_MOUSEMOVE )
		input_helper.set_mouse_position( { static_cast< float >( GET_X_LPARAM( lParam ) ), static_cast< float >( GET_Y_LPARAM( lParam ) ) } );
	if ( uMsg == WM_MOUSEWHEEL )
		input_helper.set_scroll_position( static_cast< float >( GET_WHEEL_DELTA_WPARAM( wParam ) ) / static_cast< float >( WHEEL_DELTA ) );
	return CallWindowProc( g.m_old_window, hWnd, uMsg, wParam, lParam );
}

void __fastcall update_anims( player_t *player, uint32_t edx ) {
	auto *vmt = g.m_hooks->players_hook( player->index( ) - 1 );

	if ( player == g.m_local ) {
		if ( g.m_running_client )
			g.SetAngles( );
		else {
			vmt->get_original< void( __fastcall* ) ( player_t *, uint32_t ) >( 218 )( player, edx );
			//__asm {
			//	mov     ecx, player
			//	call original
			//}
		}
	} else if ( g_player_manager.m_animating ) {
		vmt->get_original< void( __fastcall* ) ( player_t *, uint32_t ) >( 218 )( player, edx );
		//__asm {
		//	mov     ecx, player
		//	call original
		//}
	} else if ( !g_player_manager.m_ents[ player->index(  ) - 1].m_valid ) {
		vmt->get_original< void( __fastcall* ) ( player_t *, uint32_t ) >( 218 )( player, edx );
		//__asm {
		//	mov     ecx, player
		//	call original
		//}
	}
}

bool __fastcall setupbones( animating_t *anim, uint32_t edx, matrix_t *out, int max, int mask, float time ) {
	auto player = reinterpret_cast< player_t * >(reinterpret_cast< uintptr_t >(anim) - 0x4);
	auto *vmt = g.m_hooks->players_hook( player->index( ) - 1 );
	auto *info = &g_player_manager.m_ents[ player->index( ) - 1 ];
	if ( info && info->m_valid && !info->m_teamate && !info->m_records.empty(  ) ) {
		auto &record = info->m_records.front( );
		if ( record && record->m_setup ) {
			memcpy( out, record->m_bones, sizeof( matrix_t ) );
			return true;
		}
	} 
	return vmt->get_original<bool( __thiscall * )( void *, matrix_t *, int, int, float )>( 13 )( player, out, max, mask, time );
	
}

bool __fastcall is_player( player_t *player, uint32_t edx ) {
	auto *vmt = g.m_hooks->players_hook( player->index( ) - 1 );
	static auto shouldskipanim = util::find( "client.dll", "84 C0 75 ? 5F C3 8B 0D ? ? ? ?" );
	if ( g_player_manager.m_animating && _ReturnAddress( ) == shouldskipanim )
		return false;
	return vmt->get_original<bool( __thiscall * )( player_t * )>( 152 )( player );
}

void __fastcall EstimateAbsVelocity( player_t *_this, uint32_t edx, vec3_t &vec ) {
	const auto velocity = _this->abs_vel( );
	vec.x = velocity.x;
	vec.y = velocity.y;
	vec.z = velocity.z;
}
void  __fastcall AllocateIntermediateData( player_t *_this, uint32_t edx ) {
	if ( _this != g.m_local )
		return;
	const auto map = g.m_local->GetPredDescMap( );
	if ( !map )
		return;
	const typedescription_t type_description{ FIELD_FLOAT, "m_velocityModifier", static_cast< int >( g.m_offsets->m_player.velocity_modifier ), 1, 0x100, "", sizeof( float ), 0.00390625f };
	const auto type_array = new typedescription_t[ map->m_num_fields + 1 ];
	memcpy( type_array, map->m_desc, sizeof( typedescription_t ) * map->m_num_fields );
	type_array[ map->m_num_fields ] = type_description;
	map->m_desc = type_array;
	map->m_pOptimizedDataMap = nullptr;
	map->m_num_fields++;
	map->m_packed_size = 0;
	g.m_map_setup = CPredictionCopy::PrepareDataMap( map );
	
}
void hooks_t::CustomEntityListener::OnEntityCreated ( entity_t *ent ) {
	if ( !ent || !ent->is_player( ) )
		return;
	auto *vmt = g.m_hooks->players_hook( ent->index( ) - 1 );
	if ( vmt ) {
		vmt->reset( );
		vmt->init( reinterpret_cast< uintptr_t >(ent) );
		vmt->add( update_anims, 218 );
		vmt->add( is_player, 152 );
		vmt->add( EstimateAbsVelocity, 141 );
		
	}
	//vmt = g.m_hooks->renderables_hook( ent->index( ) - 1 );
	//if ( vmt ) {
	//	vmt->reset( );
	//	vmt->init( reinterpret_cast< uintptr_t >( ent->animating(  ) ) );
	//	//vmt->add( setupbones, 13 );
	//}
}

void hooks_t::CustomEntityListener::OnEntityDeleted ( entity_t *ent ) {
	if ( ent && ent->index( ) >= 1 && ent->index( ) <= 64 ) {


		auto *vmt = g.m_hooks->players_hook( ent->index( ) - 1 );
		if ( vmt ) {
			vmt->reset( );
		}
	}
}

void __stdcall frame_stage_notify( client_frame_stage_t stage ) {
	if ( stage != FRAME_START )
		g.m_stage = stage;

	if ( stage == FRAME_RENDER_START ) {
		// apply local player animated angles.
		g.SetAngles( );

		// apply local player animation fix.
		g.UpdateAnimations( );
	}
	g.m_interfaces->client( ).hook( )->get_original<decltype( &frame_stage_notify )>( 36 )(stage);
	if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END ) {
		g_player_manager.update( );
	} 
	//else if ( stage == FRAME_NET_UPDATE_START ) {
	//	g_esp.NoSmoke( );
	//}
}

void __fastcall draw_model_execute( void *this_ptr, uint32_t edx, uintptr_t ctx, void *state,
												const model_render_info_t &info, matrix_t *bone ) {
	static auto original_draw_model = reinterpret_cast< decltype( &draw_model_execute ) >( g.m_interfaces->model_render( ).hook( )->get_original( 21 ));

	if ( !info.pRenderable )
		return original_draw_model( this_ptr, edx, ctx, state, info, bone );
	
	auto *const base_entity = static_cast< entity_t * >(g.m_interfaces->entity_list( )->get_client_entity( info.entity_index ));
	
	if ( !base_entity || !base_entity->is_player( ) )
		return original_draw_model( this_ptr, edx, ctx, state, info, bone );

	g_chams.player( reinterpret_cast< player_t * >(base_entity), ctx, state, info, bone );
}

void __stdcall override_view( view_setup_t *view ) {
	g.m_local = static_cast< player_t * >( g.m_interfaces->entity_list(  )->get_client_entity(
		g.m_interfaces->engine()->local_player_index( ) ) );
	const auto alive = g.m_local && g.m_local->alive( );
	if ( alive ) {
		if ( settings::misc::misc::thirdperson && !g.m_interfaces->input()->camera_is_third_person( ) )
			g.m_interfaces->input()->camera_to_third_person( );
		else if ( !settings::misc::misc::thirdperson && g.m_interfaces->input( )->camera_is_third_person( ) ) {
			g.m_interfaces->input( )->camera_to_first_person( );
			g.m_interfaces->input( )->m_camera_offset.z = 0.f;
		}
	}
	if ( g.m_interfaces->input( )->camera_is_third_person( ) ) {
		// get camera angles.
		ang_t ang;
		g.m_interfaces->engine()->get_view_angles( ang );
		auto offset = ang;

		// get our viewangle's forward directional vector.
		vec3_t forward;
		forward = ( offset.forward(  ) );

		// cam_idealdist convar.
		offset.z = 150.f;

		// start pos.
		auto origin = g.m_shoot_pos;

		// setup trace filter and trace.
		trace_filter filter;
		filter.skip = g.m_local;

		trace_t tr;

		{
			//std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex2 );
			g.m_interfaces->trace( )->trace_ray(
				ray_t( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
				MASK_NPCWORLDSTATIC,
				&filter,
				&tr
			);
		}

		// adapt distance to travel time.
		tr.flFraction = fminf( fmaxf( tr.flFraction, 0.f ), 1.f );
		offset.z *= tr.flFraction;

		// override camera angles.
		g.m_interfaces->input( )->m_camera_offset = { offset.x, offset.y, offset.z };

	}
	if ( settings::visuals::misc::fov != 0 )
		view->fov = settings::visuals::misc::fov;
	g.m_interfaces->client_mode( ).hook( )->get_original<decltype(&override_view)>( 18 )( view );
}

using cl_move_t = void( __cdecl * ) ( float, bool );
cl_move_t original_cl_move;

void cl_move(float accumulated_extra_samples, bool bFinalTick) {

	g.on_move(accumulated_extra_samples, bFinalTick, original_cl_move);
	//g.ran = false;
	//g.m_final_packet = &bFinalTick;
	//original_cl_move( accumulated_extra_samples, bFinalTick );
}
c_hook m_net_hook;
using LevelInitPostEntity_t = void( __stdcall * )( );
using ProcessPacket_t = void( __thiscall * )( void *, void *, bool );
using SendDatagram_t = int( __thiscall * )( void *, void * );

#define NET_FRAMES_BACKUP 64 // must be power of 2. 
#define NET_FRAMES_MASK ( NET_FRAMES_BACKUP - 1 )

int __fastcall SendDatagram( i_net_channel *this_ptr, uint32_t edx, void *data ) {
	const auto backup2 = this_ptr->m_in_seq;

	if ( settings::misc::misc::fake_latency ) {
		const auto ping = static_cast< int >(settings::misc::misc::fake_latency_amt);

		// the target latency.
		const auto correct = fmaxf( 0.f, ( ping / 1000.f ) - g.m_latency - g.m_lerp );
		
		this_ptr->m_in_seq += 2 * NET_FRAMES_MASK - static_cast< uint32_t >( NET_FRAMES_MASK * correct );
		//or ( const auto &sequence : g.m_sequences ) {
		//	if ( g.m_interfaces->globals(  )->m_curtime - sequence.m_time >= ping ) {
		//		this_ptr->in_reliable_state = sequence.m_state;
		//		this_ptr->in_sequence_nr = sequence.m_seq;
		//		break;
		//	}
		//
	}
	
	g_resolver.update_shot_timing( this_ptr->m_choked_packets );

	const auto ret = m_net_hook.get_original< SendDatagram_t >( 48 )( this_ptr, data );
;
	
	if ( !g.m_ran && g.m_valid_round) {
		g.m_cmds.push_back( ret );
	} else if ( !g.m_valid_round )
		g.m_cmds.clear( );

	this_ptr->m_in_seq = backup2;

	return ret;
}

void __fastcall ProcessPacket( i_net_channel *this_ptr, uint32_t edx, void *packet, bool header ) {
	m_net_hook.get_original< ProcessPacket_t >( 41 )( this_ptr, packet, header );
	
	g.UpdateIncomingSequences( this_ptr );

	// get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
	//for ( event_info_t *it{ g.m_interfaces->client_state(  )->m_events }; it != nullptr; it = it->m_next ) {
	//	if ( !it->m_client_class )
	//		continue;
	//
	//	// set all delays to instant.
	//	it->m_fire_delay = 0.f;
	//}
	//
	//// game events are actually fired in OnRenderStart which is WAY later after they are received
	//// effective delay by lerp time, now we call them right after theyre received (all receive proxies are invoked without delay).
	//g.m_interfaces->engine()->FireEvents( );
}

void __fastcall SetChoked( i_net_channel *this_ptr, uint32_t edx) {
	g.m_ran = true;
	using packet_start_t = void( __thiscall * )( i_net_channel * );
	//m_net_hook.get_original<packet_start_t>( 47 )( this_ptr );
	static auto ret = util::find( "engine.dll", "FF 90 ? ? ? ? FF 87 ? ? ? ?" ) + 6;

	const auto addr = _ReturnAddress( );
	auto local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	if( ret != addr || !local || !local->alive( ) || !g.m_valid_round || !g.m_interfaces->engine( )->is_connected( ) || !g.m_interfaces->engine( )->in_game( ) )
		return m_net_hook.get_original<packet_start_t>( 47 )( this_ptr );
	const auto current_choke = this_ptr->m_choked_packets;
	this_ptr->m_choked_packets = 0;
	this_ptr->SendDatagram( );
	this_ptr->m_choked_packets = current_choke + 1;
	g.m_ran = false;
	//using packet_start_t = void( __thiscall * )( i_net_channel * );
	//m_net_hook.get_original<packet_start_t>( 47 )( this_ptr );
}

void __stdcall LevelInitPostEntity( ) {
	g.m_sequences.clear( );
	g.m_datamap_updated = false;

	auto net = g.m_interfaces->engine( )->get_net_channel_info( );
	if ( net ) {
		m_net_hook.reset( );
		m_net_hook.init( reinterpret_cast< uintptr_t >(net) );
		m_net_hook.add( ProcessPacket, 41 );
		m_net_hook.add( SetChoked, 47 );
		m_net_hook.add( SendDatagram, 48 );
	}
	// invoke original method.
	g.m_interfaces->client(  ).hook()->get_original< LevelInitPostEntity_t >( 6 )( );
}

c_hook m_cl_hook;
using TempEntities_t = bool( __thiscall * )( void*, void * );

bool __fastcall TempEntities( client_state_t *this_ptr, uint32_t edx, void *msg ) {
	if ( !g.m_running_client ) {
		return m_cl_hook.get_original< TempEntities_t >( 36 )( this_ptr, msg );
	}
	
	const auto ret = m_cl_hook.get_original< TempEntities_t >( 36 )( this_ptr, msg );

	auto ei = g.m_interfaces->client_state(  )->m_events;
	event_info_t *next = nullptr;

	if ( !ei ) {
		return ret;
	}

	do {
		next = *reinterpret_cast< event_info_t ** >( reinterpret_cast< uintptr_t >( ei ) + 0x38 );

		const uint16_t classID = ei->m_class_id - 1;

		const auto m_pCreateEventFn = ei->m_client_class->m_pCreateEvent;
		if ( !m_pCreateEventFn ) {
			continue;
		}

		const auto pCE = m_pCreateEventFn( );
		if ( !pCE ) {
			continue;
		}

		if ( classID == 170 ) {
			ei->m_fire_delay = 0.0f;
		}
		ei = next;
	} while ( next != nullptr );

	return ret;
}

using RenderSmokeOverlay_t = void( __thiscall * )( void *, bool );
c_hook view_render;
void __fastcall RenderSmokeOverlay( i_render_view *this_ptr, uint32_t edx, bool unk ) {
	// do not render smoke overlay.
	//if ( !settings::visuals::world::wire_smoke )
	//	g.m_interfaces->viewrender( ).hook( )->get_original< RenderSmokeOverlay_t >( 40 )( this_ptr, unk );
}

recv_var_proxy_fn m_Body_original;
void Body_proxy( c_recv_proxy_data *data, address ptr, address out ) {
	address stack{ reinterpret_cast< uintptr_t >(_AddressOfReturnAddress( )) - sizeof( uintptr_t ) };

	static address RecvTable_Decode{ util::find( "engine.dll", "EB 0D FF 77 10" ) };

	// call from entity going into pvs.
	if ( (stack.get(2) + sizeof( uintptr_t )) != RecvTable_Decode ) {
		// convert to player.
		auto player = ptr.as< player_t * >( );

		// store data about the update.
		if ( player ) {
			auto *ent = &g_player_manager.m_ents[ player->index( ) - 1 ];
			if( ent )
				g_resolver.OnBodyUpdate( ent, data->m_value.m_Float );
		}
	}

	// call original proxy.
	if ( m_Body_original )
		m_Body_original( data, ptr, out );
}
c_hook net_show_fragments;
using GetBool_t = bool( __thiscall * )( void * );
bool __fastcall NetShowFragmentsGetBool( void *this_ptr, uint32_t edx ) {
	if ( !g.m_interfaces->engine(  )->in_game(  ) )
		return net_show_fragments.get_original< GetBool_t >( 13 )( this_ptr );

	static auto read_sub_channel_data_ret = static_cast< address >(util::find( "engine.dll", "85 C0 74 12 53 FF 75 0C 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C" )).as< uintptr_t *>( );
	static auto check_receiving_list_ret = static_cast< address >( util::find( "engine.dll", "8B 1D ? ? ? ? 85 C0 74 16 FF B6" )).as< uintptr_t *>( );

	static uint32_t last_fragment = 0;

	if ( _ReturnAddress( ) == reinterpret_cast< void * >( read_sub_channel_data_ret ) && last_fragment > 0 ) {
		const auto data = &reinterpret_cast< uint32_t * >( g.m_interfaces->client_state(  )->m_net_channel )[ 0x56 ];
		const auto bytes_fragments = reinterpret_cast< uint32_t * >( data )[ 0x43 ];

		if ( bytes_fragments == last_fragment ) {
			auto &buffer = reinterpret_cast< uint32_t * >( data )[ 0x42 ];
			buffer = 0;
		}
	}

	if ( _ReturnAddress( ) == check_receiving_list_ret ) {
		const auto data = &reinterpret_cast< uint32_t * >( g.m_interfaces->client_state( )->m_net_channel )[ 0x56 ];
		const auto bytes_fragments = reinterpret_cast< uint32_t * >( data )[ 0x43 ];

		last_fragment = bytes_fragments;
	}

	return net_show_fragments.get_original< GetBool_t >( 13 )( this_ptr );
}

void __stdcall run_command( player_t *player, cmd_t *cmd, player_move_helper *helper ) {
	g.m_local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	typedef void( __thiscall *o_run_command )( void *, player_t *, cmd_t *, player_move_helper * );
	//if ( player == g.m_local ) {
	//	prediction::finish_partial_frame( player, cmd );
	//}
	//else
		g.m_interfaces->prediction( ).hook( )->get_original<o_run_command>( 19 )( g.m_interfaces->prediction( ), player, cmd, helper );
}

bool __stdcall InPrediction() {
	typedef void(__thiscall* o_run_command)(void*);
	if (g.m_in_pred)
		return true;
	g.m_interfaces->prediction().hook()->get_original<o_run_command>(14)(g.m_interfaces->prediction());
}

void __stdcall PreEntityPacketReceived( int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed ) {
	g.m_local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	if ( g.m_local && g.m_local->alive( ) ) {
		g_pred_manager.pre_update( g.m_local );
	}
	typedef void( __thiscall *o_PostNetworkDataReceived )( void *, int,int,int );
	g.m_interfaces->prediction( ).hook( )->get_original<o_PostNetworkDataReceived>( 4 )( g.m_interfaces->prediction( ), commands_acknowledged, current_world_update_packet, server_ticks_elapsed );

}
void __stdcall PostNetworkDataReceived( int commands_acknowledged ) {
	g.m_local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	if ( g.m_local ) { 
		const auto map = g.m_local->GetPredDescMap( );
		if ( map ) {
			
			if ( !g.m_map_setup ) {
				const typedescription_t type_description{ FIELD_FLOAT, "m_velocityModifier", static_cast< int >( g.m_offsets->m_player.velocity_modifier ), 1, 0x100, "", sizeof( float ), 0.00390625f };
				const auto type_array = new typedescription_t[ map->m_num_fields + 1 ];
				memcpy( type_array, map->m_desc, sizeof( typedescription_t ) * map->m_num_fields );
				type_array[ map->m_num_fields ] = type_description;
				map->m_desc = type_array;
				map->m_pOptimizedDataMap = nullptr;
				map->m_num_fields++;
				map->m_packed_size = 0;
				g.m_map_setup = CPredictionCopy::PrepareDataMap( map );
				g_pred_manager.init( map );
			}
		}
	}
	if ( g.m_local && g.m_local->alive( ) ) {
		g_pred_manager.post_update( g.m_local );
	}
	typedef void( __thiscall *o_PostNetworkDataReceived )( void*, int );
	g.m_interfaces->prediction( ).hook( )->get_original<o_PostNetworkDataReceived>( 6 )( g.m_interfaces->prediction( ), commands_acknowledged );
	g.net_data_received( );
}


void __stdcall PostEntityPacketReceived() {
	typedef void(__thiscall* o_PostNetworkDataReceived)(void*);
	g.m_interfaces->prediction().hook()->get_original<o_PostNetworkDataReceived>(5)(g.m_interfaces->prediction());
	g.m_local = static_cast<player_t*>(g.m_interfaces->entity_list()->get_client_entity(g.m_interfaces->engine()->local_player_index()));
}

using FireEventFn = bool( __thiscall * )( void *, IGameEvent * );
bool __fastcall FireEventClientSide( void *ecx, void *edx, IGameEvent *pEvent ) {
	if( strcmp( pEvent->GetName(  ), "bullet_impact" ) )
		g_resolver.on_impact( pEvent );
	return g.m_interfaces->events( ).hook( )->get_original<FireEventFn>( 8 )( g.m_interfaces->events( ), pEvent );
}

void __stdcall SceneEnd( ) {
	g.m_interfaces->render_view(  ).hook(  )->get_original<  void( __thiscall * )( void * ) >( 9 )( g.m_interfaces->render_view( ) );
	g_chams.SceneEnd( );
}

void __fastcall PacketStart( client_state_t *this_ptr, void *edx, int incoming_sequence, int outgoing_acknowledged ) {
	auto local = static_cast< player_t * >( g.m_interfaces->entity_list( )->get_client_entity( g.m_interfaces->engine( )->local_player_index( ) ) );
	using packet_start_t = void( __thiscall * )( client_state_t *, int, int );
	if( !local || !local->alive(  ) || !g.m_valid_round || !g.m_interfaces->engine( )->is_connected( ) || !g.m_interfaces->engine( )->in_game( ) )
		return m_cl_hook.get_original<packet_start_t>( 5 )( this_ptr, incoming_sequence, outgoing_acknowledged );;
	//if( this_ptr->m_last_command_ack == this_ptr->m_last_outgoing_command )
		//m_cl_hook.get_original<packet_start_t>( 5 )( this_ptr, incoming_sequence, outgoing_acknowledged );;
		//return;
 	for ( auto it = g.m_cmds.begin( ); it != g.m_cmds.end( ); ++it )
		if ( *it == outgoing_acknowledged ) {
			m_cl_hook.get_original<packet_start_t>( 5 )( this_ptr, incoming_sequence, outgoing_acknowledged );
			break;
		}
	for( size_t i = 0; i < g.m_cmds.size(); i++ )
		if ( g.m_cmds[ i ] < outgoing_acknowledged ) {
			g.m_cmds.erase( g.m_cmds.begin( ) + i );
			i--;
		}
	//this_ptr->current_sequence = incoming_sequence;
}

float __fastcall Hook_GetScreenAspectRatio( void *pEcx, void *pEdx, int32_t iWidth, int32_t iHeight ) {
	if( settings::visuals::misc::aspectratio != 0 )
		iWidth *= ( settings::visuals::misc::aspectratio ) / 100.f;
	return g.m_interfaces->engine(  ).hook(  )->get_original<float(__thiscall*)( void *, int32_t, int32_t)>( 101 )( pEcx , iWidth , iHeight );
}
void __stdcall Hooked_ClientCmd(const char* str, bool force) {
	std::string name = str;
	if (name.length() > 5 && strcmp(name.substr(0, 5).c_str(), "save ") == 0) {
		name = name.substr(5, name.length() - 1) + ".cfg";
		remove(name.c_str());
		std::ofstream file(name.c_str());
		std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
		std::cout.rdbuf(file.rdbuf()); //redirect std::cin to in.txt!

		if (file.good()) {
			menu.save();
		}

		std::cout.rdbuf(coutbuf); //reset to standard output again

		file.close();
	}
	else if (name.length() > 5 && strcmp(name.substr(0, 5).c_str(), "load ") == 0) {
		name = name.substr(5, name.length() - 1) + ".cfg";
		std::ifstream in(name.c_str());
		std::streambuf* cinbuf = std::cin.rdbuf(); //save old buf
		std::cin.rdbuf(in.rdbuf()); //redirect std::cin to in.txt!

		menu.load();

		std::cin.rdbuf(cinbuf); //reset to standard output again
		in.close();
	}
	else g.m_interfaces->engine().hook()->get_original<void(__thiscall*)(void*, const char*, bool)>(114)(g.m_interfaces->engine(), str, force);
}

long __stdcall reset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* presentation_parameters) {
	g.m_render->on_lost_device();
	auto result = g.m_interfaces->device().hook()->get_original<decltype(&reset)>(16)(device, presentation_parameters);

	if (result == D3D_OK) {
		g.m_render->on_reset_device();
	}

	return result;
}
using setup_movment_t = int(__thiscall*) (anim_state*);
setup_movment_t o_setupmovement;
int __fastcall SetUpMovement(anim_state* _this, void* pEcx, void* pEdx) {
	if (_this->m_player == g.m_local) {


	}
	return o_setupmovement(_this);
}
hooks_t::hooks_t( ) {
	while ( !((g.m_window = FindWindowA( "Valve001", nullptr)) ) )
		Sleep( 100 );

	g_chams.create_materials( );

	m_custom_entity_listener.init( );
	//auto net_show_fragments_v = g.m_interfaces->console( )->get_convar( "net_showfragments" );
	//net_show_fragments.init( (uintptr_t)net_show_fragments_v );
	//net_show_fragments.add( static_cast< void * >( NetShowFragmentsGetBool ), 13 );

	g.m_old_window = reinterpret_cast< WNDPROC >(SetWindowLongPtr( g.m_window, GWL_WNDPROC, reinterpret_cast< LONG_PTR >(wndproc) ));
	g.m_interfaces->model_render( ).hook( )->add( draw_model_execute, 21 );
	g.m_interfaces->engine( ).hook( )->add( Hook_GetScreenAspectRatio, 101 );
	g.m_interfaces->engine().hook()->add(Hooked_ClientCmd, 114);
	//g.m_interfaces->events( ).hook( )->add( static_cast< void * >( FireEventClientSide ), 8 );
	g.m_interfaces->client( ).hook( )->add( frame_stage_notify, 36 );
	g.m_interfaces->client( ).hook( )->add( LevelInitPostEntity, 6 );
	//g.m_interfaces->viewrender( ).hook( )->add( static_cast< void * >( RenderSmokeOverlay ), 40 );

	g.m_interfaces->client_mode( ).hook( )->add( create_move, 24 );
	g.m_interfaces->client_mode( ).hook( )->add( override_view, 18 );
	g.m_interfaces->prediction( ).hook( )->add( run_command, 19 );
	g.m_interfaces->prediction( ).hook( )->add( PostNetworkDataReceived, 6 );
	g.m_interfaces->prediction( ).hook( )->add( PostEntityPacketReceived, 5 );
	g.m_interfaces->prediction().hook()->add(PreEntityPacketReceived, 4);
	g.m_interfaces->prediction().hook()->add(InPrediction, 14);

	g.m_interfaces->device().hook()->add(end_scene, 42);
	g.m_interfaces->device().hook()->add(reset, 16);
	g.m_interfaces->render_view( ).hook( )->add( SceneEnd, 9 );
	g.m_interfaces->panel( ).hook( )->add( paint_traverse, panel_t::PAINTTRAVERSE );
	m_cl_hook.init( reinterpret_cast< uintptr_t >(g.m_interfaces->hookable_client_state( )) );
	m_cl_hook.add( TempEntities, 36 );
	m_cl_hook.add( PacketStart, 5 );

	//o_setupmovement = reinterpret_cast<setup_movment_t>(DetourFunction( (PBYTE)util::find("client.dll", "55 8B EC 83 E4 ? 81 EC ? ? ? ? 56 57 8B 3D ? ? ? ? 8B F1"), (PBYTE)SetUpMovement));
	
	netvar_manager::set_proxy( fnv::hash("DT_CSPlayer"), fnv::hash( "m_flLowerBodyYawTarget"), Body_proxy, m_Body_original );

	
	original_cl_move = reinterpret_cast< decltype( &cl_move ) >( DetourFunction(
		util::find( "engine.dll", "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A F9" ),
		reinterpret_cast< PBYTE >( cl_move ) ) );

	//render_view_hook( )->hook( scene_end, 9 );

	//model_render_hook( )->hook( 21, &draw_model_execute );
}

hooks_t::~hooks_t( ) {
	for ( auto& p : m_players )
		p.reset( );
}
