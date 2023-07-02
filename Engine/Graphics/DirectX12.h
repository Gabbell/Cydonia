#pragma once

#include <Common/Assert.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#define D3D12CALL( EXPR ) CYD_ASSERT( !FAILED( EXPR ) && "A DirectX12 call failed" );