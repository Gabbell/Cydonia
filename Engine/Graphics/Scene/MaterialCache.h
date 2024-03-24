#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/GraphicsTypes.h>

#include <Common/ObjectPool.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace EMP
{
class ThreadPool;
}

// ================================================================================================
// Definition
// ================================================================================================
/*
 * This is a cache of material references
 */
namespace CYD
{
class MaterialCache final
{
  public:
   MaterialCache( EMP::ThreadPool& threadPool );
   NON_COPIABLE( MaterialCache );
   ~MaterialCache() = default;

   enum TextureSlot
   {
      ALBEDO            = 0,
      DIFFUSE           = 0,
      NORMAL            = 1,
      METALNESS         = 2,
      ROUGHNESS         = 3,
      GLOSSINESS        = 3,
      AMBIENT_OCCLUSION = 4,
      DISPLACEMENT      = 5,
      COUNT
   };

   enum TextureFallback
   {
      PINK,
      BLACK,
      WHITE
   };

   enum class State
   {
      UNINITIALIZED,
      LOADING_TO_RAM,
      LOADED_TO_RAM,  // RAM Resident
      LOADING_TO_VRAM,
      LOADED_TO_VRAM  // VRAM Resident
   };

   MaterialIndex findMaterial( std::string_view name ) const;
   MaterialIndex addMaterial( std::string_view name );

   // Loads, unloads and binds GPU resources
   State progressLoad( CmdListHandle cmdList, MaterialIndex materialIdx );

   void bindSlot(
       CmdListHandle cmdList,
       MaterialIndex index,
       TextureSlot slot,
       uint8_t binding,
       uint8_t set = 0 ) const;
   void bind( CmdListHandle cmdList, MaterialIndex index, uint8_t set = 1 ) const;
   void updateMaterial(
       MaterialIndex materialIdx,
       TextureSlot slot,
       TextureHandle texHandle,
       const SamplerInfo& sampler = {} );

  private:
   //  ============================================================================================
   // Material Entry
   /*
    * This is a reference to a material
    */

   struct TextureEntry
   {
      SamplerInfo sampler;
      TextureDescription desc;
      TextureSlot slot;
      TextureFallback fallback;

      std::string path;

      void* imageData = nullptr;
      TextureHandle texHandle;

      bool useFallback = false;  // Formally use the fallback as the texture itself
   };

   struct Material
   {
      Material() = default;
      MOVABLE( Material );
      ~Material();

      std::array<TextureEntry, TextureSlot::COUNT> textures = {};
      std::array<BufferHandle, TextureSlot::COUNT> buffers  = {};

      bool needsLoadFromStorage = false;  // If the material must be loaded from disk to VRAM

      State currentState = State::UNINITIALIZED;
   };

   //  ============================================================================================

   void _initializeStaticMaterials();
   static void loadToRAM( Material& material );
   static void loadToVRAM( CmdListHandle cmdList, Material& material );

   //  ============================================================================================

   EMP::ThreadPool& m_threadPool;
   EMP::ObjectPool<Material> m_materials;
   std::unordered_map<std::string, MaterialIndex> m_materialNames;
};
}
