#include <Graphics/Vulkan/Instance.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>

#include <Window/GLFWWindow.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <set>

#if CYD_DEBUG
static VkBool32 errorCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackdata,
    void* /*pUserData*/ )
{
#ifdef _WIN32
   // Setting up Windows console to use ANSI escape color sequences. No need to do that on Unix.
   const HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
   DWORD mode            = 0;
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
   const auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
       instance, "vkCreateDebugUtilsMessengerEXT" );
   if( func != nullptr )
   {
      return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
   }

   return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator )
{
   const auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
       instance, "vkDestroyDebugUtilsMessengerEXT" );
   if( func != nullptr )
   {
      func( instance, debugMessenger, pAllocator );
   }
}
#endif

namespace vk
{
Instance::Instance( const CYD::Window& window ) : m_window( window )
{
   _createVKInstance();

#if CYD_DEBUG
   _createDebugMessenger();
#endif
}

static bool checkValidationLayerSupport( const std::vector<const char*>& desiredLayers )
{
   uint32_t layerCount;
   VKCALL( vkEnumerateInstanceLayerProperties( &layerCount, nullptr ) );

   std::vector<VkLayerProperties> supportedLayers( layerCount );
   VKCALL( vkEnumerateInstanceLayerProperties( &layerCount, supportedLayers.data() ) );

   std::set<std::string> requiredLayers( desiredLayers.begin(), desiredLayers.end() );
   for( const auto& layer : supportedLayers )
   {
      requiredLayers.erase( layer.layerName );
   }

   return requiredLayers.empty();
}

#if CYD_DEBUG
static void populateDebugInfo( VkDebugUtilsMessengerCreateInfoEXT& debugInfo )
{
   // Filling up debug info
   debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   debugInfo.pNext           = nullptr;
   debugInfo.flags           = 0;
   debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   debugInfo.pfnUserCallback = errorCallback;
   debugInfo.pUserData       = nullptr;
}

void Instance::_createDebugMessenger()
{
   // Create debug messenger
   VkDebugUtilsMessengerCreateInfoEXT debugInfo;
   populateDebugInfo( debugInfo );

   VKCALL( createDebugUtilsMessengerEXT( m_vkInstance, &debugInfo, nullptr, &m_debugMessenger ) );
}
#endif

void Instance::_createVKInstance()
{
   // General application info
   VkApplicationInfo appInfo  = {};
   appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pNext              = nullptr;
   appInfo.pApplicationName   = "Cydonia";
   appInfo.applicationVersion = 1;
   appInfo.pEngineName        = "Cydonia Engine";
   appInfo.engineVersion      = 1;
   appInfo.apiVersion         = VK_API_VERSION_1_2;

   // Use these validation layers if this is a debug build
#if CYD_DEBUG
   m_layers.push_back( "VK_LAYER_KHRONOS_validation" );
   CYDASSERT( checkValidationLayerSupport( m_layers ) );
#endif

   const std::vector<const char*>& extensions = m_window.getExtensionsFromGLFW();

   // Instance create info
   VkInstanceCreateInfo instInfo    = {};
   instInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   instInfo.flags                   = 0;
   instInfo.pApplicationInfo        = &appInfo;
   instInfo.enabledLayerCount       = static_cast<uint32_t>( m_layers.size() );
   instInfo.ppEnabledLayerNames     = m_layers.data();
   instInfo.enabledExtensionCount   = static_cast<uint32_t>( extensions.size() );
   instInfo.ppEnabledExtensionNames = extensions.data();

#if CYD_DEBUG
   VkDebugUtilsMessengerCreateInfoEXT debugInfo;
   populateDebugInfo( debugInfo );
   instInfo.pNext = &debugInfo;
#endif

   // Attempting to create an instance
   VKCALL( vkCreateInstance( &instInfo, nullptr, &m_vkInstance ) );
}

Instance::~Instance()
{
#if CYD_DEBUG
   destroyDebugUtilsMessengerEXT( m_vkInstance, m_debugMessenger, nullptr );
#endif
   
   vkDestroyInstance( m_vkInstance, nullptr );
}
}
