@echo ============== COMPILING ALL SHADERS ==============

@echo Compiling Default Shaders
glslc -fshader-stage=vert GLSL/passthrough_vert.glsl -o Compiled/passthrough_vert.spv

glslc -fshader-stage=vert GLSL/default_vert.glsl -o Compiled/default_vert.spv
glslc -fshader-stage=frag GLSL/default_frag.glsl -o Compiled/default_frag.spv

glslc -fshader-stage=vert GLSL/defaultTex_vert.glsl -o Compiled/defaultTex_vert.spv
glslc -fshader-stage=frag GLSL/defaultTex_frag.glsl -o Compiled/defaultTex_frag.spv

glslc -fshader-stage=frag GLSL/proteanclouds_frag.glsl -o Compiled/proteanclouds_frag.spv

@echo ============== COMPILING DONE ==============
pause