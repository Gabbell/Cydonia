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

   bool isInit() const { return !_threads.empty(); }

   // Submit work to the threadpool
   template <typename F, typename... Args>
   auto submit( F&& f, Args&&... args ) -> std::future<decltype( f( args... ) )>
   {
      std::function<decltype( f( args... ) )()> func =
          std::bind( std::forward<F>( f ), std::forward<Args>( args )... );

      auto taskPtr = std::make_shared<std::packaged_task<decltype( f( args... ) )()>>( func );

      std::function<void()> voidFunc = [taskPtr]() { ( *taskPtr )(); };

      _queue.enqueue( voidFunc );

      _conditionalLock.notify_one();

      return taskPtr->get_future();
   }

  private:
   class ThreadWorker
   {
     public:
      ThreadWorker( ThreadPool* threadPool, const int threadIdx );
      void operator()();

     private:
      int _threadIdx;
      ThreadPool* _threadPool;
   };

   bool _shutdown;
   std::condition_variable _conditionalLock;
   std::mutex _conditionalMutex;
   ThreadSafeQueue<std::function<void()>> _queue;
   std::vector<std::thread> _threads;
};
}