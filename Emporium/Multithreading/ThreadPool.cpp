#include <Multithreading/ThreadPool.h>

namespace CYD
{
ThreadPool::ThreadPool() : _shutdown( false ) {}

void ThreadPool::init( int numberOfThreads )
{
   _threads.resize( numberOfThreads );
   for( int i = 0; i < _threads.size(); i++ )
   {
      _threads[i] = std::thread( ThreadWorker( this, i ) );
   }
}

void ThreadPool::shutdown()
{
   _shutdown = true;
   _conditionalLock.notify_all();

   for( int i = 0; i < _threads.size(); ++i )
   {
      if( _threads[i].joinable() )
      {
         _threads[i].join();
      }
   }
}

ThreadPool::~ThreadPool() { shutdown(); }

ThreadPool::ThreadWorker::ThreadWorker( ThreadPool* threadPool, const int threadIdx )
    : _threadPool( threadPool ), _threadIdx( threadIdx )
{
}

void ThreadPool::ThreadWorker::operator()()
{
   std::function<void()> function;
   bool dequeued;
   while( !_threadPool->_shutdown )
   {
      {
         std::unique_lock<std::mutex> lock( _threadPool->_conditionalMutex );
         if( _threadPool->_queue.empty() )
         {
            _threadPool->_conditionalLock.wait( lock );
         }
         dequeued = _threadPool->_queue.dequeue( function );
      }
      if( dequeued )
      {
         function();
      }
   }
}
}