@echo ============== COMPILING ALL SHADERS ==============

@echo Compiling Default Shaders
glslc GLSL/PASSTHROUGH.vert -o Compiled/PASSTHROUGH_VERT.spv
glslc GLSL/PASSTHROUGH.frag -o Compiled/PASSTHROUGH_FRAG.spv

:: Render Pipelines
glslc GLSL/DEFAULT.vert -o Compiled/DEFAULT_VERT.spv
glslc GLSL/DEFAULT.frag -o Compiled/DEFAULT_FRAG.spv

glslc GLSL/DEFAULT_DISPLACEMENT.vert -o Compiled/DEFAULT_DISPLACEMENT_VERT.spv

glslc GLSL/DEFAULT_TEX.vert -o Compiled/DEFAULT_TEX_VERT.spv
glslc GLSL/DEFAULT_TEX.frag -o Compiled/DEFAULT_TEX_FRAG.spv

glslc GLSL/PHONG_TEX.vert -o Compiled/PHONG_TEX_VERT.spv
glslc GLSL/PHONG_TEX.frag -o Compiled/PHONG_TEX_FRAG.spv

glslc GLSL/PBR_TEX.vert -o Compiled/PBR_TEX_VERT.spv
glslc GLSL/PBR_TEX.frag -o Compiled/PBR_TEX_FRAG.spv

:: Compute
glslc GLSL/FFTOCEAN_SPECTRA.comp -o Compiled/FFTOCEAN_SPECTRA_COMP.spv
glslc GLSL/FFTOCEAN_FOURIERCOMPONENTS.comp -o Compiled/FFTOCEAN_FOURIERCOMPONENTS_COMP.spv
glslc GLSL/FFTOCEAN_BUTTERFLYTEX.comp -o Compiled/FFTOCEAN_BUTTERFLYTEX_COMP.spv
glslc GLSL/FFTOCEAN_BUTTERFLY.comp -o Compiled/FFTOCEAN_BUTTERFLY_COMP.spv
glslc GLSL/FFTOCEAN_INVERSIONPERMUTATION.comp -o Compiled/FFTOCEAN_INVERSIONPERMUTATION_COMP.spv

:: Others
glslc GLSL/PROTEANCLOUDS.frag -o Compiled/PROTEANCLOUDS_FRAG.spv

@echo ============== COMPILING DONE ==============
pause