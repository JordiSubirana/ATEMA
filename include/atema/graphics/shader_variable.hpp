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

#ifndef ATEMA_GRAPHICS_SHADER_VARIABLE_HEADER
#define ATEMA_GRAPHICS_SHADER_VARIABLE_HEADER

#include <atema/graphics/config.hpp>
#include <atema/context/opengl.hpp>

namespace at
{
	class Shader;
	
	class ATEMA_GRAPHICS_API ShaderAbstractVariable
	{
		friend class at::Shader;
		
		public:
			ShaderAbstractVariable();
			virtual ~ShaderAbstractVariable() = 0;
			
			bool is_valid() const noexcept;
			
		protected:
			void ensure_validity() const;
			
		private:
			bool m_valid;
	};
	
	template <typename T>
	class ShaderVariable : public ShaderAbstractVariable
	{
		friend class at::Shader;
		
		public:
			virtual ~ShaderVariable();
			
			ShaderVariable<T>& operator=(const T& arg);
			
		private:
			ShaderVariable();
			ShaderVariable(Shader *shader, GLint location);
			
			Shader *m_shader;
			GLint m_location;
	};
}

#include <atema/graphics/shader_variable.tpp>

#endif