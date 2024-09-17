#pragma once

namespace CYD
{
// Assuming std140
static constexpr size_t CYD_SHADER_ALIGNMENT = 16;

#define CYD_SHADER_ALIGN alignas( CYD_SHADER_ALIGNMENT )

}