#pragma once

#include <Common/Include.h>

#include <Graph/NodeGraph.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/RenderPipelines.h>
#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/AssetManager.h>

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

   // Render Graph interactions
   // ==============================================================================================

   // Transforms the graph into an optimized tree and perform validations. Here are the operations:
   // * Getting all resources
   // * TODO Checking that the graph is acyclic (no directed cycles)
   // * TODO "Touching" used resources as a hint to keep them alive longer
   // ...
   bool compile();

   // Goes through the graph and render to swapchain
   bool execute();

   // Dynamic State
   // ==============================================================================================

   // Set the current viewport with the possibility of flipping it in on the Y-axis
   void
   setViewport( float offsetX, float offsetY, float width, float height, bool flippedY = false );

   void setScissor( int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height );

   // Scene components that can be added. These components are destroyed when resetScene is called.
   // ==============================================================================================

   static constexpr std::string_view MAIN_VIEW_STRING = "MAIN";

   void addSceneView(
       std::string_view name,
       const glm::vec4& position,
       const glm::mat4& view,
       const glm::mat4& projection );

   void
   addSceneLight( const glm::vec4& enabled, const glm::vec4& direction, const glm::vec4& color );

   void add3DRenderable(
       const glm::mat4& modelMatrix,
       std::string_view pipName,
       std::string_view materialPath,
       std::string_view meshPath );

   void resetScene();

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

   // Reference to node handles based on pass name
   std::unordered_map<std::string_view, NodeHandle> m_passToHandle;

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
   using Renderables       = std::unordered_map<std::string_view, Renderable3DArray>;

   // Number of renderables per pipeline
   std::unordered_map<std::string_view, uint32_t> m_renderableCounts;

   // All renderables per pipeline
   Renderables m_renderables;

   // Manager for assets to allow reuse for read-only assets
   AssetManager m_assets;

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
};
}