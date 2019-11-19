#include <Core/Graphics/Vulkan/Instance.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Window/GLFWWindow.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/Surface.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <algorithm>
#include <set>

static VkBool32 errorCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackdata,
    void* /*pUserData*/ )
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

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger )
{
   auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
       instance, "vkCreateDebugUtilsMessengerEXT" );
   if( func != nullptr )
   {
      return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
   }
   else
   {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
   }
}

void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator )
{
   auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
       instance, "vkDestroyDebugUtilsMessengerEXT" );
   if( func != nullptr )
   {
      func( instance, debugMessenger, pAllocator );
   }
}

cyd::Instance::Instance( const Window& window ) : _window( window )
{
   _createVKInstance();
   _createDebugMessenger();
}

static bool checkValidationLayerSupport( const std::vector<const char*>& desiredLayers )
{
   uint32_t layerCount;
   vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

   std::vector<VkLayerProperties> supportedLayers( layerCount );
   VkResult result = vkEnumerateInstanceLayerProperties( &layerCount, supportedLayers.data() );

   CYDASSERT( result == VK_SUCCESS && "Instance: Could not enumerate instance layer properties" );

   std::set<std::string> requiredLayers( desiredLayers.begin(), desiredLayers.end() );
   for( const auto& layer : supportedLayers )
   {
      requiredLayers.erase( layer.layerName );
   }

   return requiredLayers.empty();
}

static void populateDebugInfo( VkDebugUtilsMessengerCreateInfoEXT& debugInfo )
{
   // Filling up debug info
   debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   debugInfo.pNext           = nullptr;
   debugInfo.flags           = 0;
   debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   debugInfo.pfnUserCallback = errorCallback;
   debugInfo.pUserData       = nullptr;
}

void cyd::Instance::_createVKInstance()
{
   // General application info
   VkApplicationInfo appInfo  = {};
   appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pNext              = nullptr;
   appInfo.pApplicationName   = "Cydonia";
   appInfo.applicationVersion = 1;
   appInfo.pEngineName        = "VK";
   appInfo.engineVersion      = 1;
   appInfo.apiVersion         = VK_API_VERSION_1_1;

   // Use validation layers if this is a debug build
#if defined( _DEBUG )
   _layers.push_back( "VK_LAYER_LUNARG_standard_validation" );
   CYDASSERT( checkValidationLayerSupport( _layers ) );
#endif

   const std::vector<const char*>& extensions = _window.getExtensionsFromGLFW();

   // Instance create info
   VkInstanceCreateInfo instInfo    = {};
   instInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   instInfo.flags                   = 0;
   instInfo.pApplicationInfo        = &appInfo;
   instInfo.enabledLayerCount       = static_cast<uint32_t>( _layers.size() );
   instInfo.ppEnabledLayerNames     = _layers.data();
   instInfo.enabledExtensionCount   = static_cast<uint32_t>( extensions.size() );
   instInfo.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
   VkDebugUtilsMessengerCreateInfoEXT debugInfo;
   populateDebugInfo( debugInfo );
   instInfo.pNext = &debugInfo;
#endif

   // Attempting to create an instance
   VkResult instanceResult = vkCreateInstance( &instInfo, nullptr, &_vkInstance );
   CYDASSERT( instanceResult == VK_SUCCESS && "Instance: Vulkan instance creation failed" );
}

void cyd::Instance::_createDebugMessenger()
{
#ifdef _DEBUG
   // Create debug messenger
   VkDebugUtilsMessengerCreateInfoEXT debugInfo;
   populateDebugInfo( debugInfo );

   VkResult debugResult =
       createDebugUtilsMessengerEXT( _vkInstance, &debugInfo, nullptr, &_debugMessenger );

   CYDASSERT(
       debugResult == VK_SUCCESS && "cyd::Instance:: Debug utils messenger creation failed" );
#endif
}

cyd::Instance::~Instance()
{
#ifdef _DEBUG
   destroyDebugUtilsMessengerEXT( _vkInstance, _debugMessenger, nullptr );
#endif
   vkDestroyInstance( _vkInstance, nullptr );
}
