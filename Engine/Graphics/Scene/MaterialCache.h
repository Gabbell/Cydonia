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

   enum class State
   {
      UNINITIALIZED,
      LOADING_TO_RAM,
      LOADED_TO_RAM,  // RAM Resident
      LOADING_TO_VRAM,
      LOADED_TO_VRAM  // VRAM Resident
   };

   MaterialIndex getMaterialIndexByName( std::string_view name ) const;

   // Loads, unloads and binds GPU resources
   State progressLoad( CmdListHandle cmdList, MaterialIndex materialIdx );

   void bind( CmdListHandle cmdList, MaterialIndex index, uint8_t set ) const;
   void updateMaterial( MaterialIndex materialIdx, TextureSlot slot, TextureHandle texHandle );

  private:
   //  ============================================================================================
   // Material Entry
   /*
    * This is a reference to a material
    */

   struct TextureEntry
   {
      TextureDescription desc;
      std::string path;

      void* imageData = nullptr;
      TextureHandle texHandle;
   };

   struct Material
   {
      Material() = default;
      MOVABLE( Material );
      ~Material();

      std::array<TextureEntry, TextureSlot::COUNT> textures = {};
      std::array<BufferHandle, TextureSlot::COUNT> buffers  = {};

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
