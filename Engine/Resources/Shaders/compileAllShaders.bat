@echo ============== COMPILING ALL SHADERS ==============

@echo Compiling Default Shaders
glslc Raw/passthrough.vert -o Compiled/passthrough_vert.spv

glslc Raw/default.vert -o Compiled/default_vert.spv
glslc Raw/default.frag -o Compiled/default_frag.spv

glslc Raw/proteanclouds.frag -o Compiled/proteanclouds_frag.spv

@echo ============== COMPILING DONE ==============
pause