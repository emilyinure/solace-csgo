#pragma once

#include <d3d9.h>
#include "vmt.h"

class interfaces_t {
public:
	template<typename t = address>
	class c_base_interface {
		const char *lib = "";
		const char *ver = "";
		t *ptr = nullptr;
		c_hook *m_hook;
	public:
		~c_base_interface( ) { if( m_hook ) delete m_hook; }
		c_base_interface( const char *lib, const char *ver, bool hook = true ) : lib( lib ), ver( ver ) { 
			ptr = util::capture_interface< t *>( lib, ver ); 
			if ( hook ) m_hook = new c_hook( reinterpret_cast< uintptr_t >( ptr ) ); 
		}
		c_base_interface( void *ptr_, bool hook = true ) { 
			ptr = static_cast< t * >( ptr_ ); 
			if ( hook ) m_hook = new c_hook( reinterpret_cast< uintptr_t >( ptr ) ); 
		}
		__forceinline t *operator->( ) { return ptr; }
		__forceinline operator t *( ) { return ptr; }
		explicit __forceinline operator bool( ) const { return ptr; }
		explicit __forceinline operator address( ) const { return address( ptr ); }
		explicit __forceinline operator void *( ) const { return ptr; }
		__forceinline c_hook *&hook( ) { return m_hook; }
	};
protected:


	c_base_interface<c_base_client> m_client{ "client.dll", "VClient018" };
	c_base_interface<c_global_vars> m_globals{ static_cast< void * >( *address( m_client ).to< address * >( )[ 11 ].to< address * >( 10 ) ), false };
	c_base_interface<c_engine_client> m_engine{ "engine.dll", "VEngineClient014" };
	c_base_interface<c_engine_trace> m_trace{ "engine.dll", "EngineTraceClient004" };
	c_base_interface<debug_overlay_t> m_debug_overlay{ "engine.dll", "VDebugOverlay004" };
	c_base_interface<c_entity_list> m_entity_list{ "client.dll", "VClientEntityList003" };
	c_base_interface<i_render_view> m_render_view{ "engine.dll", "VEngineRenderView014" };
	c_base_interface<i_material_system> m_material_system{ "materialsystem.dll", "VMaterialSystem080" };
	c_base_interface<iv_model_render> m_model_render{ "engine.dll", "VEngineModel016" };
	c_base_interface<i_console> m_console{ "vstdlib.dll", "VEngineCvar007" };
	c_base_interface<model_info_t> m_model_info{ "engine.dll", "VModelInfoClient004" };
	c_base_interface<phys_surface_props_t> m_phys_surface{ "vphysics.dll", "VPhysicsSurfaceProps001" };
	c_base_interface<i_mdl_cache> m_mdlcache{ "datacache.dll", "MDLCache004", false };
	c_base_interface<address> m_studio_render{ "studiorender.dll", "VStudioRender026" };
	c_base_interface<player_prediction> m_prediction{ "client.dll", "VClientPrediction001" };
	c_base_interface<player_game_movement> m_game_movement{ "client.dll", "GameMovement001" };
	c_base_interface<IGameEventManager2> m_event{ "engine.dll", "GAMEEVENTSMANAGER002" };
	c_base_interface<panel_t> m_panel{ "vgui2.dll", "VGUI_Panel009" };
	c_base_interface<i_weapon_system> m_weapon_system{ *reinterpret_cast< void ** >( util::find( "client.dll", "8B 35 ? ? ? ? FF 10 0F B7 C0" ) + 2 ), false };
	c_base_interface<mem_alloc_t> m_mem_alloc{ *reinterpret_cast< void ** >( GetProcAddress( GetModuleHandleA( "tier0.dll" ), "g_pMemAlloc" ) ), false };
	c_base_interface<IDirect3DDevice9> m_device{ **reinterpret_cast< void *** >( util::find( "shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C" ) + 0x1 ) };
	//c_base_interface<void> m_viewrender{ **reinterpret_cast< void *** >( util::find( "client.dll",  "8B 0D ? ? ? ? 8B 01 FF 50 4C 8B 06" ) + 2 ), false };
	c_base_interface<client_state_t> m_client_state{ **reinterpret_cast< client_state_t *** > ( ( *reinterpret_cast< uintptr_t ** > ( m_engine.operator c_engine_client *(  ) ) )[ 12 ] + 0x10 ), false };
	c_base_interface<player_move_helper> m_move_helper{ **reinterpret_cast< void *** >( util::find( "client.dll", "8B 0D ? ? ? ? 8B 46 08 68" ) + 2 ) };
	c_base_interface<i_input> m_input{ *reinterpret_cast< i_input ** >( util::find( "client.dll", "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10" ) + 1 ) };
	c_base_interface<void> m_client_mode{ address( util::get_virtual_function<uintptr_t>( m_client, 10 ) + 0x5  ).get<void*>(2) };
	void *m_hookable_state = reinterpret_cast< void * >( reinterpret_cast< uintptr_t * >( reinterpret_cast< uintptr_t >( m_client_state.operator client_state_t * ( ) ) + 0x8 ) );

public:


	interfaces_t( ) = default;

	[[nodiscard]] decltype( m_render_view ) &render_view( ) {
		return m_render_view;
	}
	
	[[nodiscard]] decltype( m_panel ) &panel( ) {
		return m_panel;
	}
	
	[[nodiscard]] decltype( m_mdlcache ) &mdlcache( ) {
		return m_mdlcache;
	}
	
	[[nodiscard]] decltype( m_input ) &input( ) {
		return m_input;
	}

	[[nodiscard]] decltype( m_material_system ) &material_system( ) {
		return m_material_system;
	}
	[[nodiscard]] decltype( m_event ) &events( ) {
		return m_event;
	}
	
	[[nodiscard]] decltype( m_game_movement ) &game_movement( ) {
		return m_game_movement;
	}
	
	[[nodiscard]] decltype( m_move_helper ) &move_helper( ) {
		return m_move_helper;
	}
	
	[[nodiscard]] decltype( m_prediction ) &prediction( ) {
		return m_prediction;
	}
	
	[[nodiscard]] decltype( m_studio_render ) &studio_render( ) {
		return m_studio_render;
	}

	[[nodiscard]] decltype( m_weapon_system ) &weapon_system( ) {
		return m_weapon_system;
	}
	
	[[nodiscard]] decltype( m_debug_overlay ) &debug_overlay( ) {
		return m_debug_overlay;
	}
	
	[[nodiscard]] decltype( m_phys_surface ) &phys_surface( ) {
		return m_phys_surface;
	}

	[[nodiscard]] decltype( m_model_render ) &model_render( ) {
		return m_model_render;
	}

	[[nodiscard]] decltype( m_console ) &console( ) {
		return m_console;
	}
	
	[[nodiscard]] decltype( m_model_info ) &model_info( ) {
		return m_model_info;
	}

	[[nodiscard]] decltype( m_globals ) &globals( ) {
		return m_globals;
	}

	[[nodiscard]] decltype( m_client ) &client( ) {
		return m_client;
	}

	[[nodiscard]] decltype( m_engine ) &engine( ) {
		return m_engine;
	}

	[[nodiscard]] decltype( m_trace ) &trace( ) {
		return m_trace;
	}

	[[nodiscard]] decltype( m_client_mode ) &client_mode( ) {
		return m_client_mode;
	}

	[[nodiscard]] decltype( m_entity_list ) &entity_list( ) {
		return m_entity_list;
	}

	[[nodiscard]] decltype( m_device ) &device( ) {
		return m_device;
	}

	[[nodiscard]] decltype( m_mem_alloc ) &mem_alloc( ) {
		return m_mem_alloc;
	}

	[[nodiscard]] decltype( m_client_state ) &client_state( ) {
		return m_client_state;
	}
	
	[[nodiscard]] void *hookable_client_state( ) const {
		return m_hookable_state;
	}
};
