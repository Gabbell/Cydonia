#pragma once

#include <future>

// Engine-level singleton implementation of a thread pool
namespace CYD::JobPool
{
void Initialize();
void Shutdown();
void IsInitialized();

template <typename F, typename... Args>
auto Submit( F&& f, Args&&... args ) -> std::future<decltype( f( args... ) )>;
}