import os
import subprocess

directory = os.fsencode("GLSL")

for root, subFolder, files in os.walk(directory):
	for file in files:
		filename = os.fsdecode(file)
		shadername, ext = os.path.splitext(filename)
		if(ext == ".vert"):
			print("Vertex Shader: " + shadername)
			subprocess.run("glslc " + "GLSL/Vertex/" + filename + " -o " + "SPIR-V/" + shadername + "_VERT.spv")
		elif(ext == ".frag"):
			print("Fragment Shader: " + shadername)
			subprocess.run("glslc " + "GLSL/Fragment/" + filename + " -o " + "SPIR-V/" + shadername + "_FRAG.spv")
		elif(ext == ".comp"):
			print("Compute Shader: " + shadername)
			subprocess.run("glslc " + "GLSL/Compute/" + filename + " -o " + "SPIR-V/" + shadername + "_COMP.spv")