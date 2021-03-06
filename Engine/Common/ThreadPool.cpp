#include <Common/ThreadPool.h>

#include <Multithreading/ThreadPool.h>

namespace CYD::ThreadPool
{
static EMP::ThreadPool s_threadPool;

void Initialize() { s_threadPool.init( std::thread::hardware_concurrency() ); }
void Shutdown() { s_threadPool.shutdown(); }

void IsInitialized() { s_threadPool.isInit(); }

template <typename F, typename... Args>
auto Submit( F&& f, Args&&... args ) -> std::future<decltype( f( args... ) )>
{
   s_threadPool.submit( f, std::forward( args ) );
}
}
