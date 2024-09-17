#pragma once

#include <Common/Assert.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/GraphicsTypes.h>

#define GRIS_RENDERBACKEND_IMP_ABSTRACT 0
#define GRIS_RENDERBACKEND_IMP_CONCRETE 1

#define GRIS_RENDERBACKEND_IMP GRIS_RENDERBACKEND_IMP_ABSTRACT

#include <Graphics/GRIS/Backends/RenderBackendMacrosBegin.h>
#include <Graphics/GRIS/Backends/RenderBackendMacros.h>

GRIS_RENDERBACKEND_DECLARATION( RenderBackend )

#include <Graphics/GRIS/Backends/RenderBackendMacrosEnd.h>