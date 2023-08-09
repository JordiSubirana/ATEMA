/*
	Copyright 2023 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_GRAPHICS_PRIMITIVE_HPP
#define ATEMA_GRAPHICS_PRIMITIVE_HPP

#include <Atema/Graphics/Config.hpp>
#include <Atema/Graphics/Mesh.hpp>
#include <Atema/Graphics/Loaders/ModelLoader.hpp>

namespace at
{
	class ATEMA_GRAPHICS_API Primitive
	{
	public:
		Primitive(const Primitive& other) = default;
		Primitive(Primitive&& other) noexcept = default;
		~Primitive() = default;

		static Ptr<Mesh> createBox(const ModelLoader::Settings& settings, float width, float height, float depth, size_t widthSubdivisions, size_t heightSubdivisions, size_t depthSubdivisions);
		static Ptr<Mesh> createConeFromRadius(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float radius, size_t verticalSubdivisions, size_t horizontalSubdivisions);
		static Ptr<Mesh> createConeFromAngle(const ModelLoader::Settings& settings, const Vector3f& direction, float range, float angle, size_t verticalSubdivisions, size_t horizontalSubdivisions);
		static Ptr<Mesh> createUVSphere(const ModelLoader::Settings& settings, float radius, size_t verticalSliceCount, size_t horizontalSliceCount);

		Primitive& operator=(const Primitive& other) = default;
		Primitive& operator=(Primitive&& other) noexcept = default;

	private:
		Primitive() = default;
	};
}

#endif
