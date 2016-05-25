# ATEMA

**ATEMA** (_**A**dvanced **T**ools **E**ngine for **M**ultimedia **A**pplications_) is a portable and modular C++11 library providing many tools for the development of multimedia programs.  
This software is provided under GPL license. See the [GNU website](http://www.gnu.org/licenses/gpl.html) or the file [LICENSE.txt](https://github.com/JordiSubirana/ATEMA/blob/master/LICENSE.txt) for more informations.

## Ambitions

The goal of **ATEMA** is to provide a simple, intuitive and efficient API in order to allow developers to easily create multimedia programs, such as 2D/3D games, parallel computations on GPU, and more.  
**ATEMA** can be used as a static/dynamic library, or you can directly include the headers you need in your project and define a special directive in your file to use **ATEMA** as an header-only library.

## Dependencies

As many dependencies as possible are header-only libraries. The exhaustive list is :  
* Header-only dependencies
 * stb_image (see [nothings/stb](https://github.com/nothings/stb) on GitHub)
 * stb_image_write (see [nothings/stb](https://github.com/nothings/stb) on GitHub)
* Compiled dependencies
 * GLFW (see [GLFW website](http://www.glfw.org/))
 * glad (see [Dav1dde/glad](https://github.com/Dav1dde/glad) on GitHub)

## Modules

**ATEMA** is designed to be modular. This means you can use each module independently, based on what you exactly need. The different modules are :  
* [**Context**](https://github.com/JordiSubirana/ATEMA/tree/master/include/atema/context) : Creation of OpenGL contexts and use of OpenGL functions.
* [**Core**](https://github.com/JordiSubirana/ATEMA/tree/master/include/atema/core) : Miscellaneous tools for generic applications like _errors_, _strings_, _macros_, ...
* [**Graphics**](https://github.com/JordiSubirana/ATEMA/tree/master/include/atema/graphics) : OpenGL high-level abstraction to draw 2D/3D stuff, like _meshes_, _textures_, ...
* [**Math**](https://github.com/JordiSubirana/ATEMA/tree/master/include/atema/math) : Definition of _vectors_, _matrices_, ...
* [**Parallel**](https://github.com/JordiSubirana/ATEMA/tree/master/include/atema/parallel) : Use of GPU to do massively parallel computations.
* [**Window**](https://github.com/JordiSubirana/ATEMA/tree/master/include/atema/window) : Creation of _windows_, use of _mouse_, _keyboard_ and _joystick_.

## Platforms

This software is designed to be portable and all the modules should work on Windows, Linux and Mac OSX.

## Install

A _Makefile_ system allow you to easily build the library of each module and a global one. The Makefile also build the examples in the bin repository.  
The only prerequisite is a compatible C++11 compiler.

## Examples

You can find in the [examples repository](https://github.com/JordiSubirana/ATEMA/tree/master/examples) a lot of sources showing different ways to use **ATEMA**.  
_Some screenshots will appear there soon._
