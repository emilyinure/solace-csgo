#include "aimbot.h"
#include "sdk.h"
#include "hooks.h"


unsigned long _stdcall initialize ( void *instance ) {
	while ( !GetModuleHandleA( "serverbrowser.dll" ) )
		Sleep( 200 );

	interfaces_t m_interfaces;
	g.m_interfaces = &m_interfaces;
	offsets_t m_offsets;
	g.m_offsets = &m_offsets;
	hooks_t m_hooks;
	g.m_hooks = &m_hooks;
	render_t m_render;
	g.m_render = &m_render;

	try {
		g.init_cheat( );
	}
	catch ( const std::runtime_error &error ) {
		MessageBoxA( nullptr, error.what( ), "major cheat error!", MB_OK | MB_ICONERROR );
		FreeLibraryAndExitThread( static_cast< HMODULE >(instance), 0 );
	}
	//CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr );
	//CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr );
	//CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr );
	//CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr );
	//if ( auto* const handle = CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr ) )
	//	CloseHandle( handle );
	//if ( auto* const handle = CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr ) )
	//	CloseHandle( handle );
	//if ( auto* const handle = CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr ) )
	//	CloseHandle( handle );
	//if ( auto* const handle = CreateThread( nullptr, NULL, hc_thread, instance, NULL, nullptr ) )
	//	CloseHandle( handle );

	while ( !GetAsyncKeyState( VK_END ) ) {
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}

	FreeLibraryAndExitThread( static_cast< HMODULE >(instance), 0 );
	return TRUE;
}

std::int32_t WINAPI DllMain ( const HMODULE instance, const unsigned long reason, [[maybe_unused]] const void *reserved ) {
	DisableThreadLibraryCalls( instance );

	switch ( reason ) {
	case DLL_PROCESS_ATTACH :
		if ( auto* const handle = CreateThread( nullptr, NULL, initialize, instance, NULL, nullptr ) )
			CloseHandle( handle );
		break;

	case DLL_PROCESS_DETACH :
		g.release( );
		break;

	}

	return true;
}
