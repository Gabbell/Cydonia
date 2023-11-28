<img align="left" src="CydoniaCropped.png"  width="150" height="150">

# Cydonia

*Experimental Engine*

<br/>
<br/>

*Note: I am using this repo as a way to learn rendering algorithms and game engine architecture. This is not meant to be used in any professional capacity.*

# Summary
Here is a summary of my experiments within this engine:
*  Built on a data-oriented architecture called Entity Component System (ECS) implemented mostly using template metaprogramming
*  Flexible rendering interface called GRIS with the current main implementation being in Vulkan. The D3D12 implementation is a WIP
*  Procedural terrain using compute-generated noise and Fractional Brownian Motion (FBM) where layers of noise (octaves) modulate each other
   * White noise
   * Simplex noise
   * Voronoi noise
   * Domain warped noise
*  Shadow mapping using Percentage Closer Filtering (PCF)
*  Physically-based rendering based on the Cook-Torrance BRDF
*  Physically-based sky using LUTs. Implemented by raymarching volumes at a planetary scale taking into consideration atmospheric scattering and absorption. Includes height fog that is modulated using FBM.
   * _Sun Transmittance LUT_: contains the color of the sun based on its direction relative to the atmosphere. Is only computed once or when scattering or atmosphere parameters change.
   * _Multiscattering LUT_: Simulates infinite light scattering by integrating rays in a sphere around a position. 
   * _Sky-view LUT_: A flat view of the sky with a non-linear mapping in the Y direction to have more resolution near the horizon where details are higher frequency.
   * _Aerial Perspective LUT_: A view-aligned 3D texture representing the atmospheric scattering present over long distances.
   * Hillaire, S. (2020). A scalable and production ready sky and atmosphere rendering technique. Computer Graphics Forum, 39(4), 13–22. https://doi.org/10.1111/cgf.14050
* An exponential distance and height fog fullscreen pass using compute
* Physically-based ocean simulation using the Phillips spectrum and the Inverse Fast-Fourier Transform (IFFT) in compute. Normals are generated using finite differences based on the resulting displacement map. The Jacobian determinant is also calculated to add foam that accumulates and exponentially decays over time. The ocean is seemlesly tiled, dynamically tessellated and uses instancing. Everything but the final rendering is done using compute.
   * Flügge, F. (2017). Realtime GPGPU FFT Ocean Water Simulation [Research Project Thesis, Hamburg
University of Technology]. https://doi.org/10.15480/882.1436
   * Tessendorf, J. (2004). Simulating Ocean Water.
* Parallax Occlusion Mapping (POM)

https://github.com/Gabbell/Cydonia/assets/10086598/83e1bf9d-cb3f-4497-9797-b60624ad2317

https://github.com/Gabbell/Cydonia/assets/10086598/0e4777c2-b8f1-431d-9d15-bca54951c2ad

# Notes

### Entity Component System (ECS) Architecture
In this pattern, components are raw data. This design is said to be data-oriented because data transformation, done through the use of systems, is done in linear memory. These components are allocated from pools which is how this memory is linear. In an ideal implementation, there is one pool per archetype (combination of components). This is because systems iterate over entities that have a specific archetypes and not over components. In the current Cydonia implementation however, there is one pool per component. Most of the implementation stands on top of template metaprogramming, which might not be ideal for compile times. Here are a few examples of components:

* `RenderableComponent`: Basic properties like visibility or shadow casting/receiving. Also contains some buffers containing instancing and tessellation data
* `ViewComponent:` Signals that this entity should have a scene view. For example, a player or a light that needs shadow mapping.
* `TransformComponent`: Position, scaling and rotation data.
* `MeshComponent`: Handle to mesh data from the mesh cache
* `TessellatedComponent`: An entity with a mesh can be dynamically tessellated using this component, effectively giving us granular control over its LOD.
* `InstancedComponent`: Signals that this entity contains instanced rendering and how many instances
* `MaterialComponent`: Handles to the pipeline and textures that will be used for final rendering

There are also `SharedComponents` which are basically singleton components that do not need multiple instances, i.e. they are not attached to one entity. Here are a few examples:

* `InputComponent`: Is responsible to track keyboard and mouse inputs to the application. Also manages window resizing.
* `SceneComponent`: Contains scene camera and light information. A bit of a catch all for things that don't fit or are not accessible anywhere else.

Here are a few examples of entities:

A player character that can move
```
   const EntityHandle player = m_ecs->createEntity( "Player" );
   m_ecs->assign<InputComponent>( player );
   m_ecs->assign<TransformComponent>( player, glm::vec3( 0.0f, 15.0f, 0.0f ) /*position*/ );
   m_ecs->assign<MotionComponent>( player );
   m_ecs->assign<ViewComponent>( player, "MAIN" );
```
A directional light
```
   const EntityHandle sun = m_ecs->createEntity( "Sun" );
   m_ecs->assign<TransformComponent>( sun, glm::vec3( 0.0f, 40.0f, 0.0f ) );
   m_ecs->assign<LightComponent>( sun, LightComponent::TYPE::Directional );
   m_ecs->assign<ViewComponent>( sun, "SUN", -71.0f, 71.0f, -71.0f, 71.0f, -200.0f, 200.0f ); // Define orthographic projection matrix
```

An FFT ocean
```
   const EntityHandle ocean = m_ecs->createEntity( "Ocean" );
   m_ecs->assign<RenderableComponent>( ocean, false /*shadowCasting*/, true /*shadowReceiving*/ );
   m_ecs->assign<TransformComponent>( ocean, glm::vec3( 0.0f, 1.0f, 0.0f ) /*position*/, glm::vec3( 1.0f ) /*scale*/ );
   m_ecs->assign<MeshComponent>( ocean, "GRID" );
   m_ecs->assign<TessellatedComponent>( ocean, 0.319f, 0.025f );
   m_ecs->assign<MaterialComponent>( ocean, "OCEAN" /*pipeline*/, "OCEAN" /*material*/ );
   m_ecs->assign<FFTOceanComponent>( ocean, 1024, 2000, 100.0f, 40.0f, 1.0f, 1.0f );
```
___

### Graphics Rendering Interface Subsystem (GRIS)
Basically just a rendering interface that can be implemented using different rendering APIs. Currently, Vulkan is my priority and is the only one fully implemented. Here are some notable features:
* Dynamically reload shaders for fast iteration times
* API resources are pool-allocated
* Some resources like command lists, buffers and textures are passed around as handles for safety and lightness
* Data-oriented pipeline and material descriptions in JSON
* A "render graph" where render pass order is explicitly described and command lists are chained together

#### To do
* D3D12 and/or Metal backend
