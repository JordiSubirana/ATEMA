# ATEMA

**ATEMA** is a modular C++17 library providing many tools for the development of 2D/3D applications.

This software is provided under MIT license. See [this website](https://opensource.org/licenses/MIT) or the file [LICENSE.txt](https://github.com/JordiSubirana/ATEMA/blob/master/LICENSE.txt) for more informations.

## Features

* 3D rendering with graphics API abstractions (only Vulkan for now)
* Shader representation using Abstract Syntax Trees
* Shader preprocessor (libraries management, uber shader instances)
* Custom shading language (atsl)
* Converters from atsl to other shading languages (SPIR-V and glsl for now)
* Customizable render pipelines (with pre-built deferred rendering and PBR)
* Multi-threading

## Developers

_**Jordi SUBIRANA**_  
Main developer, using this project to learn how rendering engines are made.
Contact: [jordi.subirana@yahoo.fr](mailto:jordi.subirana@yahoo.fr)

## Dependencies

As many dependencies as possible are header-only libraries. The exhaustive list is:  
* [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) : manage Vulkan memory allocations for buffers and images
* [stb_image](https://github.com/nothings/stb) : image loading
* [tinyobjloader](https://github.com/syoyo/tinyobjloader) : obj mesh loader

The rest is provided by [xmake](https://xmake.io) package dependencies:
* [glfw](https://github.com/glfw/glfw) : portable window library and event manager
* [glslang](https://github.com/KhronosGroup/glslang) : SPIR-V generation from glsl

## Platforms

This software aims to be portable and all the modules _should_ (one day) work on Windows, Linux and Mac OSX.

_For now, only Windows is fully supported._

## Build & Install

A [xmake](https://xmake.io) system allow you to retrieve all **ATEMA** dependencies and easily build the project. You only need to download xmake from the website, and have a compatible C++17 compiler.

### Generate an IDE project
You can generate a project for your favorite IDE either using a command line, or running one of the predefined scripts (for example _build_project_xxx.bat_ on Windows).

If you wish to use the command line, open a console in the project folder and run `xmake project --kind=vs` for Visual Studio. The full list of tools and parameters are available on the xmake website.

### Build
If you want to manually compile **ATEMA**, follow those steps :
* Configure the project if needed (for example `xmake config --mode=release --arch=x64`)
* Build all the targets `xmake build`

### Install
Finally, to retrieve **ATEMA**'s install files and dependencies for the current configuration :
* Install to the desired directory `xmake install --installdir=INSTALLDIR`

## Examples

_Some screenshots will appear there soon._

## Thanks

I'm working on **ATEMA** during my spare time to learn how rendering engines are made, and create my own implementation on various systems (yes, I enjoy reinventing the wheel sometimes). I've been inspired by other persons doing the same thing, and by some projects or papers I found online. Here are some of my sources of inspiration I'd like to thank:

* [Nazara Engine](https://github.com/NazaraEngine/NazaraEngine)
* [Banshee](https://github.com/ValtoGameEngines/Banshee-Engine)
* [Vulkan tutorial](https://vulkan-tutorial.com/)
* [Sascha Willems Vulkan examples](https://github.com/SaschaWillems/Vulkan)
* [Riccardo Loggini posts](https://logins.github.io/)
* [entt](https://github.com/skypjack/entt)
* All open source libraries I'm using (see [Dependencies](https://github.com/JordiSubirana/ATEMA#dependencies) section)
* And sooo much more!