@echo ============== COMPILING ALL SHADERS ==============

@echo Compiling Default Shaders
glslc HLSL/passthrough_vert.hlsl -o Compiled/passthrough_vert.spv

glslc HLSL/default_vert.hlsl -o Compiled/default_vert.spv
glslc HLSL/default_frag.hlsl -o Compiled/default_frag.spv

glslc HLSL/defaultTex_vert.hlsl -o Compiled/defaultTex_vert.spv
glslc HLSL/defaultTex_frag.hlsl -o Compiled/defaultTex_frag.spv

@echo ============== COMPILING DONE ==============
pause