#pragma once
#include <thread>
#include <vector>
#include <shared_mutex>
#include <condition_variable>
#include <algorithm>
#include <queue>


class ThreadHandler {
public:
	std::vector<std::thread> thread_pool;
	ThreadHandler( ) {
	}
	mutable std::mutex queue_mutex;                  // Prevents data races to the job queue
	mutable std::mutex queue_mutex2;                  // Prevents data races to the job queue
	
	bool busy( ) {
		bool poolbusy = false;
		{
			std::unique_lock<std::mutex> lock( queue_mutex2 );
			poolbusy = !jobs.empty( );
		}
		return poolbusy;
	}

	std::queue<std::function<void( )>> jobs;

	static void ThreadLoop( );

	void QueueJob( const std::function<void( )>& job ) {
		{
			std::unique_lock<std::mutex> lock( queue_mutex2 );
			jobs.push( job );
		}
		mutex_condition.notify_one( );
	}
	bool should_terminate = false;
	void stop( ) {
		{
			std::unique_lock<std::mutex> lock( queue_mutex2 );
			should_terminate = true;
		}
		mutex_condition.notify_all( );
		for ( std::thread& active_thread : thread_pool ) {
			active_thread.join( );
		}
		thread_pool.clear( );
	}

	void start( ) {
		should_terminate = false;
		const uint32_t num_threads = std::thread::hardware_concurrency( ); // Max # of threads the system supports
		thread_pool.resize( num_threads );
		for ( uint32_t i = 0; i < num_threads; i++ ) {
			thread_pool.at( i ) = std::thread( ThreadLoop );
		}
	}
	std::condition_variable mutex_condition;
} inline g_thread_handler;
