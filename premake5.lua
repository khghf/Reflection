workspace "Reflection"
	architecture"x64"
	configurations{
		"Debug",
		"Release",
	}
	language"C++"
	cppdialect "C++20"  
project"Mirror"
	kind"ConsoleApp"
	files{
		"./Mirror/include/**.h",
		"./Mirror/main.cpp",
		"./Mirror/mirror.h",
		"./Mirror/test.h",
	}
	includedirs{
		"./"
	}

	