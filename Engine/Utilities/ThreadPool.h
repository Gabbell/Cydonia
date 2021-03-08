#pragma once

#include <future>

namespace CYD::ThreadPool
{
void Initialize();
void Shutdown();
void IsInitialized();

template <typename F, typename... Args>
auto Submit( F&& f, Args&&... args ) -> std::future<decltype( f( args... ) )>;
}