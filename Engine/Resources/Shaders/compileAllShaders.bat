@echo ============== COMPILING ALL SHADERS ==============

@echo Compiling Default Shaders
glslc Raw/default.vert -o Compiled/default_vert.spv
glslc Raw/default.frag -o Compiled/default_frag.spv

@echo ============== COMPILING DONE ==============
pause