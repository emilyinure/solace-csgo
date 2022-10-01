#include "aimbot.h"
#include "thread_handler.h"

static auto allocate_thread_id = reinterpret_cast< void(*) ( ) >(GetProcAddress( GetModuleHandleA( "tier0.dll" ), "AllocateThreadID" ));
static auto free_thread_id = reinterpret_cast< void(*) ( ) >(GetProcAddress( GetModuleHandleA( "tier0.dll" ), "FreeThreadID" ));

void ThreadHandler::thread_fn( ) {
	allocate_thread_id( );
	while ( true ) {
		const auto object = g_thread_handler.assign_object( );
		if ( object ) {
			object->run( );
			object->handling = false;
		}
		else break;
	}
	free_thread_id( );
}
