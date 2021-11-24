#pragma once

#include <Common/Assert.h>

#include <vulkan/vulkan.h>

#define VKCALL( EXPR )                                                               \
   if( EXPR != VK_SUCCESS )                                                          \
   {                                                                                 \
      fprintf(                                                                       \
          stderr,                                                                    \
          "Vulkan Call Failed->\n \tFile: %s\n \tLine: %d\n \tPrecondition: (%s)\n", \
          __FILE__,                                                                  \
          __LINE__,                                                                  \
          #EXPR );                                                                   \
      __debugbreak();                                                                \
   }