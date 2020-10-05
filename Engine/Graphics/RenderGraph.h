#pragma once

#include <Common/Include.h>

#include <Graph/NodeGraph.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace CYD
{
class RenderGraph final : public EMP::NodeGraph
{
  public:
   RenderGraph();
   MOVABLE( RenderGraph );
   virtual ~RenderGraph();

   void reset() override;

   void add3DRenderable(
       const glm::mat4& modelMatrix,
       StaticPipelines::Type pipType,
       std::string_view materialPath,
       std::string_view meshPath );

   static constexpr std::string_view MAIN_VIEW_STRING = "MAIN";
   void addView(
       std::string_view name,
       const glm::vec4& position,
       const glm::mat4& view,
       const glm::mat4& projection );

   void addLight( const glm::vec4& enabled, const glm::vec4& direction, const glm::vec4& color );

   // Dynamic State
   void setViewport( float offsetX, float offsetY, float width, float height );
   void setScissor( int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height );

   // Transforms the graph into an optimized tree and perform validations. Here are the operations:
   // * Getting all resources
   // * TODO Checking that the graph is acyclic (no directed cycles)
   // * TODO "Touching" used resources as a hint to keep them alive longer
   // ...
   bool compile();

   // Goes through the graph and render to swapchain
   bool execute() const;

  private:
   enum class State : uint8_t
   {
      INITIAL,    // Empty or has just been reset
      SETUP,      // Being populated
      COMPILING,  // Doing its optimization strategies
      READY,      // Ready to be rendered
      EXECUTING,  // Rendering
      ERROR,      // Something bad happened
   } m_currentState = State::INITIAL;

   void _updateState( State desiredState );

   // Load resource functions called during compile time
   bool _loadMesh( CmdListHandle transferList, std::string_view meshPath );
   bool
   _loadMaterial( CmdListHandle transferList, uint32_t pipType, std::string_view materialPath );

   // Viewport
   // =============================================================================================
   Viewport m_viewport;
   Rectangle m_scissor;

   // Renderables to draw
   // =============================================================================================
   struct Renderable3D
   {
      glm::mat4 modelMatrix = glm::mat4( 1.0f );
      std::string_view meshPath;
      std::string_view materialPath;
   };
   static constexpr uint32_t MAX_NUMBER_RENDERABLES = 512;

   using Renderable3DArray = std::array<Renderable3D, MAX_NUMBER_RENDERABLES>;
   using Renderables       = std::array<Renderable3DArray, (size_t)StaticPipelines::Type::COUNT>;

   std::array<uint32_t, (size_t)StaticPipelines::Type::COUNT> m_renderableCounts = {};
   Renderables m_renderables                                                     = {};

   // Lights
   // =============================================================================================
   struct Light
   {
      glm::vec4 enabled;
      glm::vec4 direction;
      glm::vec4 color;
   } m_light;

   BufferHandle m_lightBuffer;

   // Views
   // =============================================================================================
   struct View
   {
      glm::vec4 position;
      glm::mat4 viewMatrix;
      glm::mat4 projectionMatrix;
   };

   // All the available views as view and projection matrices
   std::unordered_map<std::string_view, View> m_views;

   BufferHandle m_viewBuffer;

   // Resources
   // =============================================================================================
   static constexpr uint32_t INITIAL_AMOUNT_RESOURCES = 128;

   struct Mesh
   {
      VertexBufferHandle vertexBuffer;
      IndexBufferHandle indexBuffer;
      uint32_t vertexCount = 0;
      uint32_t indexCount  = 0;
   };

   struct Material
   {
      TextureHandle albedo;     // Diffuse/Albedo color map
      TextureHandle normal;     // Normal map
      TextureHandle height;     // Height map
      TextureHandle metalness;  // Metallic/Specular map
      TextureHandle roughness;  // Roughness map
      TextureHandle ao;         // Ambient occlusion map
   };

   std::unordered_map<std::string_view, Mesh> m_meshes;
   std::unordered_map<std::string_view, Material> m_materials;
   std::unordered_map<std::string_view, BufferHandle> m_buffers;
};
}
