#pragma once

#include <array>

namespace vk
{
class Device;
class CommandBuffer;

namespace DebugUtilsLabel
{
void Initialize( const Device& device );
void Begin( const CommandBuffer& cmdBuffer, const char* name, std::array<float, 4> color );
void End( const CommandBuffer& cmdBuffer );
void Insert( const CommandBuffer& cmdBuffer, const char* name, std::array<float, 4> color );
}
}