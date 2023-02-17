#include "threading.h"
#include <thread>
static LList<struct Job> jobs;
int Threading::threadID = -1;

uint64_t Threading::_QueueJob( JobFn function, void* data, bool ref, bool priority ) {
   Job job;
   job.args = data;
   job.function = function;
   job.ref = ref;
   uint64_t ret = jobs.Enqueue( job, priority );
   return ret;
}

static void RunJob( struct Job& job ) {
   job.function( job.args );
   if ( !job.ref )
	  free( job.args );
}

static auto allocate_thread_id =
    reinterpret_cast<void (*)()>(GetProcAddress(GetModuleHandleA("tier0.dll"), "AllocateThreadID"));
static auto free_thread_id =
    reinterpret_cast<void (*)()>(GetProcAddress(GetModuleHandleA("tier0.dll"), "FreeThreadID"));

static void* __stdcall ThreadLoop(void* t)
{
   allocate_thread_id();
   struct JobThread* thread = ( struct JobThread* )t;
   Threading::threadID = thread->id;

   struct Job job;
   thread->isRunning = true;
   while ( !thread->shouldQuit ) {
	  if ( job.id ^ ~0ull ) {
		 thread->queueEmpty = false;
		 RunJob( job );
	  } else
		 thread->queueEmpty = true;
	  struct LList<struct Job>* tJobs = thread->jobs;
	  thread->jLock->unlock( );
	  job = tJobs->PopFront( thread->jLock );
   }
   thread->isRunning = false;
   free_thread_id();
   return nullptr;
}

unsigned int Threading::numThreads = 0;
static struct JobThread* threads = nullptr;

static void InitThread( struct JobThread* thread, int id ) {
   thread->id = id;
   thread->jLock = new Mutex( );
   thread->jobs = &jobs;
   thread_t handle = Threading::StartThread( ThreadLoop, thread, false );
   thread->handle = malloc( sizeof( thread_t ) );
   *( thread_t* ) thread->handle = handle;
}

void Threading::InitThreads( ) {
   numThreads = std::thread::hardware_concurrency( );
   numThreads = std::clamp<unsigned int>( numThreads, 2, 16 );

   threads = ( struct JobThread* )calloc( numThreads, sizeof( struct JobThread ) );

   for ( unsigned int i = 0; i < numThreads; i++ )
	  InitThread( threads + i, i );
}

int Threading::EndThreads( ) {
   int ret = 0;

   if ( !threads )
	  return ret;

   for ( unsigned int i = 0; i < numThreads; i++ )
	  threads[ i ].shouldQuit = true;

   for ( unsigned int i = 0; i < numThreads; i++ )
	  threads[ i ].jobs->quit = true;

   for ( int o = 0; o < 4; o++ )
	  for ( unsigned int i = 0; i < numThreads; i++ )
		 threads[ i ].jobs->sem.Post( );

   for ( size_t i = 0; i < numThreads; i++ ) {
   #if defined(__linux__) || defined(__APPLE__)
	  void* ret2 = nullptr;
	  pthread_join( *( pthread_t* ) threads[ i ].handle, &ret2 );
   #else
	  ResumeThread( *( HANDLE* ) threads[ i ].handle );
	  if ( WaitForSingleObject( *( HANDLE* ) threads[ i ].handle, 100 ) == WAIT_TIMEOUT && threads[ i ].isRunning )
		 ;
   #endif
	  delete threads[ i ].jLock;
	  threads[ i ].jLock = nullptr;
	  free( threads[ i ].handle );
   }
   free( threads );
   threads = nullptr;

   return ret;
}

void Threading::FinishQueue( bool executeJobs ) {
   if ( !threads )
	  return;

   if ( executeJobs ) {
	  for ( unsigned int i = 0; i < numThreads; i++ ) {
		 auto jobList = &jobs;
		 if ( threads[ i ].jobs )
			jobList = threads[ i ].jobs;
		 while ( 1 ) {
			struct Job job = jobList->TryPopFront( );
			if ( job.id == ~0ull )
			   break;
			RunJob( job );
		 }
	  }
   }

   for ( unsigned int i = 0; i < numThreads; i++ ) {
	  if ( threads[ i ].jobs )
		 while ( !threads[ i ].jobs->IsEmpty( ) );

	  threads[ i ].jLock->lock( );
	  threads[ i ].jLock->unlock( );
   }
}

JobThread* Threading::BindThread( LList<struct Job>* jobsQueue ) {
   for ( size_t i = 0; i < numThreads; i++ ) {
	  if ( threads[ i ].jobs == &jobs || !threads[ i ].jobs ) {
		 threads[ i ].jobs = jobsQueue;
		 for ( size_t o = 0; o < numThreads; o++ )
			jobs.sem.Post( );
		 return threads + i;
	  }
   }
   return nullptr;
}

void Threading::UnbindThread( LList<struct Job>* jobsQueue ) {
   for ( size_t i = 0; i < numThreads; i++ ) {
	  threads[ i ].jLock->lock( );
	  if ( threads[ i ].jobs == jobsQueue )
		 threads[ i ].jobs = &jobs;
	  threads[ i ].jLock->unlock( );
   }
}

thread_t Threading::StartThread( threadFn start, void* arg, bool detached, thread_t* thread ) {
#ifdef _WIN32
   auto hThread = CreateThread( nullptr, 0, ( LPTHREAD_START_ROUTINE ) start, arg, 0, thread );
   SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );

   if( hThread )
	   CloseHandle( hThread );
#else
   pthread_attr_t* attr = nullptr;
   pthread_attr_t tAttr;
   if ( detached ) {
	  pthread_attr_init( &tAttr );
	  pthread_attr_setdetachstate( &tAttr, PTHREAD_CREATE_DETACHED );
	  attr = &tAttr;
   }
   pthread_create( thread, attr, start, arg );
#endif
   return *thread;
}

thread_t Threading::StartThread( threadFn start, void* arg, bool detached ) {
   thread_t thread;
   return StartThread( start, arg, detached, &thread );
}

void Threading::JoinThread( thread_t thread, void** returnVal ) {
#ifdef __posix__
   pthread_join( thread, returnVal );
#else
   WaitForSingleObject( ( void* ) thread, INFINITE );
#endif
}

