#pragma once

#include <mutex>
#include <queue>

namespace EMP
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
      return m_queue.empty();
   }

   int size()
   {
      std::unique_lock<std::mutex> lock( _mutex );
      return m_queue.size();
   }

   void enqueue( T& elem )
   {
      std::unique_lock<std::mutex> lock( _mutex );
      m_queue.push( elem );
   }

   bool dequeue( T& elem )
   {
      std::unique_lock<std::mutex> lock( _mutex );

      if( m_queue.empty() )
      {
         return false;
      }
      elem = std::move( m_queue.front() );

      m_queue.pop();
      return true;
   }

  private:
   std::mutex _mutex;
   std::queue<T> m_queue;
};
}