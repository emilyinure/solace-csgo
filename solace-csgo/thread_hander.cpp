#include "aimbot.h"
#include "thread_handler.h"

static auto allocate_thread_id = reinterpret_cast< void(*) ( ) >(GetProcAddress( GetModuleHandleA( "tier0.dll" ), "AllocateThreadID" ));
static auto free_thread_id = reinterpret_cast< void(*) ( ) >(GetProcAddress( GetModuleHandleA( "tier0.dll" ), "FreeThreadID" ));

void ThreadHandler::ThreadLoop( ) {
	allocate_thread_id( );
	while ( true ) {
		std::function<void( )> job;
		{
			std::unique_lock<std::mutex> lock( g_thread_handler.queue_mutex2 );
			g_thread_handler.mutex_condition.wait( lock, [] {
				return !g_thread_handler.jobs.empty( ) || g_thread_handler.should_terminate;
				} );

			if ( g_thread_handler.should_terminate )
				break;

			job = g_thread_handler.jobs.front( );
			g_thread_handler.jobs.pop( );
		}
		job( );
	}
	free_thread_id( );
}
