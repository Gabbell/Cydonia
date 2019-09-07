#include "Instance.h"

#include "Assert.h"
#include "Window.h"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

static VkBool32 errorCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackdata,
    void* pUserData )
{
#ifdef _WIN32
   // Setting up Windows console to use ANSI escape color sequences. No need to do that on Unix.
   HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
   DWORD mode      = 0;
   GetConsoleMode( hConsole, &mode );
   SetConsoleMode( hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING );
#endif

   // Severity colors
   switch( messageSeverity )
   {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
         // Cyan for verbose
         fprintf( stderr, "\x1B[96mValidation Layers-> \033[0m" );
         break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
         // Yellow for warnings
         fprintf( stderr, "\x1B[93mValidation Layers-> \033[0m" );
         break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
         // Red for errors
         fprintf( stderr, "\x1B[91mValidation Layers-> \033[0m" );
         break;
      default:
         // White for everything else
         fprintf( stderr, "\x1B[37mValidation Layers-> \033[0m" );
         break;
   }

   // Print actual message
   fprintf( stderr, "%s\n", pCallbackdata->pMessage );

   // If the severity is error, we want to assert immediately
   if( messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
   {
      CYDASSERT( !"Fix validation layers" );
   }

   return VK_FALSE;
}

cyd::Instance::Instance( const Window* window )
{
   _createVKInstance( window );
   _createDebugMessenger();
}

void cyd::Instance::_createVKInstance( const Window* window )
{
   // General application info
   vk::ApplicationInfo appInfo = vk::ApplicationInfo()
                                     .setPApplicationName( "Cydonia" )
                                     .setApplicationVersion( 1 )
                                     .setPEngineName( "VK" )
                                     .setEngineVersion( 1 )
                                     .setApiVersion( VK_API_VERSION_1_0 );

   // Use validation layers if this is a debug build
#if defined( _DEBUG )
   _layers.push_back( "VK_LAYER_LUNARG_standard_validation" );
#endif
   CYDASSERT( _checkValidationLayerSupport( _layers ) );

   // Get necessary extensions
   std::vector<const char*> extensions;
   if( window )
   {
      extensions = window->getExtensions();
      CYDASSERT( _checkExtensionSupport( extensions ) );
   }

   // Instance create info
   vk::InstanceCreateInfo instInfo =
       vk::InstanceCreateInfo()
           .setFlags( vk::InstanceCreateFlags() )
           .setPApplicationInfo( &appInfo )
           .setEnabledExtensionCount( static_cast<uint32_t>( extensions.size() ) )
           .setPpEnabledExtensionNames( extensions.data() )
           .setEnabledLayerCount( static_cast<uint32_t>( _layers.size() ) )
           .setPpEnabledLayerNames( _layers.data() );

   vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
#ifdef _DEBUG
   _populateDebugInfo( debugInfo );
   instInfo.setPNext( &debugInfo );
#endif

   // Attempting to create an instance
   auto instanceResult = vk::createInstance( instInfo );
   CYDASSERT(
       instanceResult.result == vk::Result::eSuccess &&
       "Instance: Vulkan instance creation failed" );
   _vkInstance = std::make_unique<vk::Instance>( std::move( instanceResult.value ) );

   // Creating dispatch loader dynamic
   _dld = std::make_unique<vk::DispatchLoaderDynamic>( *_vkInstance );
}

void cyd::Instance::_createDebugMessenger()
{
#ifdef _DEBUG
   // Create debug messenger
   vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
   _populateDebugInfo( debugInfo );

   auto debugResult = _vkInstance->createDebugUtilsMessengerEXT( debugInfo, nullptr, *_dld );

   CYDASSERT(
       debugResult.result == vk::Result::eSuccess &&
       "cyd::Instance:: Debug utils messenger creation failed" );
   _debugMessenger = std::make_unique<vk::DebugUtilsMessengerEXT>( std::move( debugResult.value ) );
#endif
}

void cyd::Instance::_populateDebugInfo( vk::DebugUtilsMessengerCreateInfoEXT& debugInfo )
{
   debugInfo.operator VkDebugUtilsMessengerCreateInfoEXT&().sType =
       VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

   // Filling up debug info
   debugInfo.setFlags( vk::DebugUtilsMessengerCreateFlagsEXT() )
       .setMessageSeverity(
           vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
           vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError )
       .setMessageType(
           vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance )
       .setPfnUserCallback( errorCallback );
}

bool cyd::Instance::_checkValidationLayerSupport( const std::vector<const char*>& desiredLayers )
{
   auto result = vk::enumerateInstanceLayerProperties();
   CYDASSERT(
       result.result == vk::Result::eSuccess &&
       "Instance: Could not enumerate instance layer properties" );

   const std::vector<vk::LayerProperties>& supportedLayers = result.value;

   bool found = true;
   for( const char* layerName : desiredLayers )
   {
      auto it = std::find_if(
          supportedLayers.begin(), supportedLayers.end(), [&]( const vk::LayerProperties& layer ) {
             return strcmp( layerName, layer.layerName ) == 0;
          } );
      if( it == supportedLayers.end() )
      {
         found = false;
         break;
      }
   }
   return found;
}

bool cyd::Instance::_checkExtensionSupport( const std::vector<const char*>& desiredExtensions )
{
   auto result = vk::enumerateInstanceExtensionProperties();
   CYDASSERT(
       result.result == vk::Result::eSuccess &&
       "Instance: Could not enumerate extension properties" );

   const std::vector<vk::ExtensionProperties>& supportedLayers = result.value;

   bool found = true;
   for( const char* extensionName : desiredExtensions )
   {
      auto it = std::find_if(
          supportedLayers.begin(),
          supportedLayers.end(),
          [&]( const vk::ExtensionProperties& extension ) {
             return strcmp( extensionName, extension.extensionName ) == 0;
          } );
      if( it == supportedLayers.end() )
      {
         found = false;
         break;
      }
   }
   return found;
}

cyd::Instance::~Instance()
{
#ifdef _DEBUG
   _vkInstance->destroyDebugUtilsMessengerEXT( *_debugMessenger, nullptr, *_dld );
#endif

   _vkInstance->destroy();
}
