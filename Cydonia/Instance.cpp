#include "Instance.h"

#include "Assert.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <SDL2/SDL.h>
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
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
         // Yellow for warnings
         fprintf( stderr, "\x1B[93mValidation Layers-> \033[0m" );
         break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
         // Red for errors
         fprintf( stderr, "\x1B[91mValidation Layers-> \033[0m" );
         break;
      default:
         // Cyan for everything else
         fprintf( stderr, "\x1B[96mValidation Layers-> \033[0m" );
         break;
   }

   // Print actual message
   fprintf( stderr, "%s\n", pCallbackdata->pMessage );

   return VK_FALSE;
}

cyd::Instance::Instance()
{
   _createVKInstance();
   _createDebugMessenger();
}

void cyd::Instance::_createVKInstance()
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
   _populateExtensions();
   CYDASSERT( _checkExtensionSupport( _extensions ) );

   // Instance create info
   vk::InstanceCreateInfo instInfo =
       vk::InstanceCreateInfo()
           .setFlags( vk::InstanceCreateFlags() )
           .setPApplicationInfo( &appInfo )
           .setEnabledExtensionCount( static_cast<uint32_t>( _extensions.size() ) )
           .setPpEnabledExtensionNames( _extensions.data() )
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
   _vkInstance = instanceResult.value;

   // Create dynamic loader
   _dld = vk::DispatchLoaderDynamic( _vkInstance );
}

void cyd::Instance::_createDebugMessenger()
{
#ifdef _DEBUG
   // Create debug messenger
   vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
   _populateDebugInfo( debugInfo );
   auto debugResult = _vkInstance.createDebugUtilsMessengerEXT( debugInfo, nullptr, _dld );
   CYDASSERT(
       debugResult.result == vk::Result::eSuccess &&
       "cyd::Instance:: Debug utils messenger creation failed" );
   _debugMessenger = debugResult.value;
#endif
}

void cyd::Instance::_populateExtensions()
{
   SDL_Window* dummy = SDL_CreateWindow(
       "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_VULKAN );

   uint32_t extensionsCount = 0;
   CYDASSERT( SDL_Vulkan_GetInstanceExtensions( dummy, &extensionsCount, nullptr ) );

   _extensions.reserve( extensionsCount );
   CYDASSERT( SDL_Vulkan_GetInstanceExtensions( dummy, &extensionsCount, _extensions.data() ) );

#if _DEBUG
   _extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif
}

void cyd::Instance::_populateDebugInfo( vk::DebugUtilsMessengerCreateInfoEXT& debugInfo )
{
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
   _vkInstance.destroyDebugUtilsMessengerEXT( _debugMessenger, nullptr, _dld );
#endif

   _vkInstance.destroy();
}
