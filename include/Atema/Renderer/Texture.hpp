/*
	Copyright 2017 Jordi SUBIRANA

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

#ifndef ATEMA_RENDERER_TEXTURE_HPP
#define ATEMA_RENDERER_TEXTURE_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Color.hpp>
#include <Atema/Renderer/RendererDependent.hpp>
#include <vector>

namespace at
{
	class ATEMA_RENDERER_API Texture
	{
	public:
		class ATEMA_RENDERER_API Implementation : public RendererDependent
		{
		public:
			Implementation();
			virtual ~Implementation() = default;

			virtual void setData(const std::vector<Color>& pixels, unsigned width, unsigned height) = 0;
			virtual void getData(std::vector<Color>& pixels) const = 0;

			virtual void resize(unsigned width, unsigned height) = 0;
			virtual unsigned getWidth() const = 0;
			virtual unsigned getHeight() const = 0;
		};

		Texture();
		Texture(unsigned width, unsigned height);
		virtual ~Texture();

		Implementation* getImplementation();
		const Implementation* getImplementation() const;

		void loadFromFile(const std::string& filename);

		void setData(const std::vector<Color>& pixels, unsigned width, unsigned height);
		void getData(std::vector<Color>& pixels) const;

		void resize(unsigned width, unsigned height);
		unsigned getWidth() const;
		unsigned getHeight() const;

	private:
		Implementation *m_impl;
	};
}

#endif