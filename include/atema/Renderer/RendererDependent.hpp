// ----------------------------------------------------------------------
// Copyright (C) 2016 Jordi SUBIRANA
//
// This file is part of ATEMA.
//
// ATEMA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATEMA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATEMA.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------

#ifndef ATEMA_RENDERER_RENDERER_DEPENDENT_HEADER
#define ATEMA_RENDERER_RENDERER_DEPENDENT_HEADER

#include <Atema/Renderer/Config.hpp>
#include <Atema/Renderer/Renderer.hpp>
#include <Atema/Core/Ref.hpp>

namespace at
{
	class ATEMA_RENDERER_API RendererDependent
	{
		public:
			virtual ~RendererDependent() = 0;
			
			Renderer* get_renderer();
			const Renderer* get_renderer() const;
			
		protected:
			RendererDependent();
			
			void set_renderer(Renderer *renderer);
			
			template <typename D, typename B>
			Ref<D> cast(const Ref<B>& ref) const;
			
		private:
			Renderer *m_renderer;
	};
}

#include <Atema/Renderer/RendererDependent.tpp>

#endif