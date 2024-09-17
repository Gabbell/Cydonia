#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class PostProcessVolumeComponent final : public BaseComponent
{
  public:
   PostProcessVolumeComponent() = default;
   COPIABLE( PostProcessVolumeComponent );
   virtual ~PostProcessVolumeComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::POST_PROCESS_VOLUME;
};
}
