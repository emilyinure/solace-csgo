#pragma once
#include <thread>
#include <vector>
#include <shared_mutex>

class baseThreadObject {
public:
	bool handling = false;
	bool finished = false;
	virtual void run( ) = 0;
};

class ThreadHandler {
public:
	std::vector<std::thread> thread_pool;
	ThreadHandler( ) {
	}
	static void thread_fn( );
	mutable std::shared_mutex queue_mutex;                  // Prevents data races to the job queue
	[[nodiscard]] bool object_available( ) const {
		std::unique_lock lock( queue_mutex );
		for ( const auto object : objects ) {
			if ( !object->handling && !object->finished ) {
				return true;
			}
		}
		return false;
	}
	bool busy( ) {
		bool poolbusy;
		{
			poolbusy = object_available();
		}
		return poolbusy;
	}
	void start( ) {
		const uint32_t num_threads = std::thread::hardware_concurrency( ); // Max # of threads the system supports
		thread_pool.resize( num_threads );
		for ( auto& i : thread_pool ) {
			i = std::thread( thread_fn );
		}
	}
	void wait( ) {
		for ( auto& i : thread_pool ) {
			if ( i.joinable( ) )
				i.join( );
		}
		thread_pool.clear( );
	}
	std::vector<baseThreadObject*> objects = {};
	bool haulted = true;

	[[nodiscard]] baseThreadObject* assign_object( ) const {
		std::unique_lock lock( queue_mutex );
		for ( const auto object : objects ) {
			if ( !object->handling && !object->finished ) {
				object->handling = true;
				return object;
			}
		}

		return nullptr;
	}
} inline g_thread_handler;
