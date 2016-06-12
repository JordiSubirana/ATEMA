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

#ifndef ATEMA_GRAPHICS_SHADER_VARIABLE_IMPLEMENTATION
#define ATEMA_GRAPHICS_SHADER_VARIABLE_IMPLEMENTATION

#include <atema/graphics/shader_variable.hpp>
#include <atema/core/error.hpp>
#include <atema/graphics/shader.hpp>

namespace at
{
	//Generic class
	template <typename T>
	ShaderVariable<T>::ShaderVariable() :
		m_shader(nullptr),
		m_location(-1)
	{
		
	}
	
	template <typename T>
	ShaderVariable<T>::ShaderVariable(Shader *shader, GLint location) :
		m_shader(shader),
		m_location(location)
	{
		
	}
	
	template <typename T>
	ShaderVariable<T>::~ShaderVariable()
	{
		
	}
	
	template <typename T>
	ShaderVariable<T>& ShaderVariable<T>::operator=(const T& arg)
	{
		ensure_validity();
		
		m_shader->set_parameter(m_location, arg);
		
		return (*this);
	}
}

#endif