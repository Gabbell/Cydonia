#pragma once

#include <Multithreading/ThreadSafeQueue.h>

#include <thread>
#include <functional>
#include <future>

namespace EMP
{
class ThreadPool
{
  public:
   ThreadPool();

   ThreadPool( const ThreadPool& ) = delete;
   ThreadPool( ThreadPool&& )      = delete;
   ThreadPool& operator=( const ThreadPool& ) = delete;
   ThreadPool& operator=( ThreadPool&& ) = delete;
   ~ThreadPool();

   // Initialize or shutdown the threadpool
   void init( int numberOfThreads );
   void shutdown();

   bool isInit() const { return !m_threads.empty(); }

   // Submit work to the threadpool
   template <typename F, typename... Args>
   auto submit( F&& f, Args&&... args ) -> std::future<decltype( f( args... ) )>
   {
      std::function<decltype( f( args... ) )()> func =
          std::bind( std::forward<F>( f ), std::forward<Args>( args )... );

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

   bool m_shutdown;
   std::condition_variable m_conditionalLock;
   std::mutex m_conditionalMutex;
   ThreadSafeQueue<std::function<void()>> m_queue;
   std::vector<std::thread> m_threads;
};
}