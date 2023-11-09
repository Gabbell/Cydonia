#pragma once

#include <Multithreading/ThreadSafeQueue.h>

#include <cassert>
#include <thread>
#include <functional>
#include <future>

namespace EMP
{
class ThreadPool
{
  public:
   ThreadPool();

   ThreadPool( const ThreadPool& )            = delete;
   ThreadPool( ThreadPool&& )                 = delete;
   ThreadPool& operator=( const ThreadPool& ) = delete;
   ThreadPool& operator=( ThreadPool&& )      = delete;
   ~ThreadPool();

   // Initialize or shutdown the threadpool
   void init( int numberOfThreads );
   void shutdown();

   bool isInit() const { return !m_threads.empty(); }

   // Maybe wrap references or perfect forwarding
   template <class T>
   std::reference_wrapper<T> maybe_wrap( T& val )
   {
      return std::ref( val );
   }

   template <class T>
   T&& maybe_wrap( T&& val )
   {
      return std::forward<T>( val );
   }

   // Submit work to the threadpool
   template <typename F, typename... Args>
   auto submit( F&& f, Args&&... args ) -> std::future<decltype( f( args... ) )>
   {
      // Can only submit jobs from the main thread
      assert( std::this_thread::get_id() == m_mainThread );

      std::function<decltype( f( args... ) )()> func =
          std::bind( std::forward<F>( f ), maybe_wrap( std::forward<Args>( args ) )... );

      auto taskPtr = std::make_shared<std::packaged_task<decltype( f( args... ) )()>>( func );

      std::function<void()> voidFunc = [taskPtr]() { ( *taskPtr )(); };

      m_queue.enqueue( voidFunc );

      m_conditionalLock.notify_one();

      return taskPtr->get_future();
   }

  private:
   class ThreadWorker
   {
     public:
      ThreadWorker( ThreadPool* threadPool, const int threadIdx );
      void operator()();

     private:
      int m_threadIdx;
      ThreadPool* m_threadPool;
   };

   std::thread::id m_mainThread;

   bool m_shutdown;
   std::condition_variable m_conditionalLock;
   std::mutex m_conditionalMutex;
   ThreadSafeQueue<std::function<void()>> m_queue;
   std::vector<std::thread> m_threads;
};
}