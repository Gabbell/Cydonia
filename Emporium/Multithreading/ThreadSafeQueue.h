#pragma once

#include <mutex>
#include <queue>

namespace CYD
{
template <typename T>
class ThreadSafeQueue
{
  public:
   ThreadSafeQueue() = default;

   ~ThreadSafeQueue() = default;

   bool empty()
   {
      std::unique_lock<std::mutex> lock( _mutex );
      return _queue.empty();
   }

   int size()
   {
      std::unique_lock<std::mutex> lock( _mutex );
      return _queue.size();
   }

   void enqueue( T& elem )
   {
      std::unique_lock<std::mutex> lock( _mutex );
      _queue.push( elem );
   }

   bool dequeue( T& elem )
   {
      std::unique_lock<std::mutex> lock( _mutex );

      if( _queue.empty() )
      {
         return false;
      }
      elem = std::move( _queue.front() );

      _queue.pop();
      return true;
   }

  private:
   std::mutex _mutex;
   std::queue<T> _queue;
};
}