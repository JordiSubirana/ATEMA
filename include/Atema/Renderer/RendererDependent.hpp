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

#ifndef ATEMA_RENDERER_RENDERERDEPENDENT_HPP
#define ATEMA_RENDERER_RENDERERDEPENDENT_HPP

#include <Atema/Renderer/Config.hpp>
#include <Atema/Core/Error.hpp>
#include <type_traits>

namespace at
{
	class RenderSystem;

	class ATEMA_RENDERER_API RendererDependent
	{
		friend class RenderSystem;

	public:
		RendererDependent() noexcept;
		virtual ~RendererDependent() noexcept;

		RenderSystem* getRenderSystem() const noexcept;

	protected:
		template <typename T, typename U>
		T* cast(U *obj)
		{
			static_assert(std::is_base_of<RendererDependent, U>::value, "RendererDependent objects can only cast other RendererDependents.");
			static_assert(std::is_base_of<RendererDependent, T>::value, "RendererDependent objects can only cast to RendererDependent objects.");

			if (m_system && m_system == static_cast<RendererDependent*>(obj)->m_system)
				return static_cast<T*>(obj);

			ATEMA_ERROR("The objects must share the same RenderSystem.");
		}

		template <typename T, typename U>
		const T* cast(const U *obj)
		{
			static_assert(std::is_base_of<RendererDependent, U>::value, "RendererDependent objects can only cast other RendererDependents.");
			static_assert(std::is_base_of<RendererDependent, T>::value, "RendererDependent objects can only cast to RendererDependent objects.");

			if (m_system && m_system == static_cast<const RendererDependent*>(obj)->m_system)
				return static_cast<const T*>(obj);

			ATEMA_ERROR("The objects must share the same RenderSystem.");
		}

	private:
		RenderSystem *m_system;
	};
}

#endif