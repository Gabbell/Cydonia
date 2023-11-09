#include <Multithreading/ThreadPool.h>

namespace EMP
{
ThreadPool::ThreadPool() : m_shutdown( false ) {}

void ThreadPool::init( int numberOfThreads )
{
   m_mainThread = std::this_thread::get_id();

   m_threads.resize( numberOfThreads );
   for( int i = 0; i < m_threads.size(); i++ )
   {
      m_threads[i] = std::thread( ThreadWorker( this, i ) );
   }
}

void ThreadPool::shutdown()
{
   m_shutdown = true;
   m_conditionalLock.notify_all();

   for( int i = 0; i < m_threads.size(); ++i )
   {
      if( m_threads[i].joinable() )
      {
         m_threads[i].join();
      }
   }
}

ThreadPool::~ThreadPool() { shutdown(); }

ThreadPool::ThreadWorker::ThreadWorker( ThreadPool* threadPool, const int threadIdx )
    : m_threadPool( threadPool ), m_threadIdx( threadIdx )
{
}

void ThreadPool::ThreadWorker::operator()()
{
   std::function<void()> function;
   bool dequeued;
   while( !m_threadPool->m_shutdown )
   {
      {
         std::unique_lock<std::mutex> lock( m_threadPool->m_conditionalMutex );
         if( m_threadPool->m_queue.empty() )
         {
            m_threadPool->m_conditionalLock.wait( lock );
         }
         dequeued = m_threadPool->m_queue.dequeue( function );
      }
      if( dequeued )
      {
         function();
      }
   }
}
}