# ATEMA

**ATEMA** (_**A**dvanced **T**ools **E**ngine for **M**ultimedia **A**pplications_) is a portable and modular C++17 library providing many tools for the development of multimedia programs.

The goal of **ATEMA** is to offer a simple, intuitive and efficient API to allow developers to easily create multimedia programs, such as 2D/3D games, parallel computations on GPU, and more. 

This software is provided under MIT license. See [this website](https://opensource.org/licenses/MIT) or the file [LICENSE.txt](https://github.com/JordiSubirana/ATEMA/blob/master/LICENSE.txt) for more informations.

## Features

* 3D rendering based on an abstraction of multiple back-ends (Vulkan back-end is implemented, DirectX/Metal/OpenGL may follow)
* Custom shading language (atsl) and AST representation
* Converters from atsl to other shading languages (SPIR-V and glsl for now)

## Developers

_**Jordi SUBIRANA**_  
Main developer, using this project to learn how rendering engines are made.
Contact: [jordi.subirana@yahoo.fr](mailto:jordi.subirana@yahoo.fr)

## Dependencies

As many dependencies as possible are header-only libraries. The exhaustive list is :  
* [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) : manage Vulkan memory allocations for buffers and images
* [stb_image](https://github.com/nothings/stb) : image loading
* [tinyobjloader](https://github.com/syoyo/tinyobjloader) : obj mesh loader

The rest is provided by [xmake](https://xmake.io) package dependencies :
* [glfw](https://github.com/glfw/glfw) : portable window library and event manager
* [glslang](https://github.com/KhronosGroup/glslang) : SPIR-V generation from glsl

## Platforms

This software aims to be portable and all the modules _should_ (one day) work on Windows, Linux and Mac OSX.

_For now, only Windows is fully supported._

## Install

A [xmake](https://xmake.io) system allow you to retrieve all **ATEMA** dependencies and easily build the project. You only need to download xmake from the website, and have a compatible C++17 compiler.

Once this is done, you can generate a project for your favorite IDE either using a command line, or running one of the predefined scripts (for example _build_project_xxx.bat_ on Windows).

If you wish to use the command line, open a console in the project folder and run `xmake project -k vs` for Visual Studio. The full list of tools and parameters are available on the xmake website.

## Examples

_Some screenshots will appear there soon._

## Thanks

I'm working on **ATEMA** during my spare time to learn how rendering engines are made, and create my own implementation on various systems (yes, I enjoy reinventing the wheel sometimes). I've been inspired by other persons doing the same thing, and by some projects or papers I found online. Here is some of my inspiration sources I'd like to thank :

* [Nazara Engine](https://github.com/NazaraEngine/NazaraEngine)
* [Banshee](https://github.com/ValtoGameEngines/Banshee-Engine)
* [Vulkan tutorial](https://vulkan-tutorial.com/)
* [Sascha Willems Vulkan examples](https://github.com/SaschaWillems/Vulkan)
* [Riccardo Loggini posts](https://logins.github.io/)
* [entt](https://github.com/skypjack/entt)
* All open source libraries I&#39;m using (see [Dependencies](https://github.com/JordiSubirana/ATEMA#dependencies) section)
* And sooo much more