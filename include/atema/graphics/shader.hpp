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

#ifndef ATEMA_GRAPHICS_SHADER_HEADER
#define ATEMA_GRAPHICS_SHADER_HEADER

#include <atema/graphics/config.hpp>
#include <atema/context/opengl.hpp>
#include <atema/context/object_gl.hpp>
#include <atema/core/non_copyable.hpp>
#include <atema/math/vector.hpp>
#include <atema/graphics/texture.hpp>
#include <atema/graphics/buffer.hpp>
#include <atema/graphics/color.hpp>
#include <atema/graphics/mesh.hpp>
#include <atema/math/matrix.hpp>

#include <map>

namespace at
{
	class ShaderAbstractVariable;
	
	template <typename T>
	class ShaderVariable;
	
	class ATEMA_GRAPHICS_API Shader : public ObjectGL, public NonCopyable
	{
		template <typename T>
		friend class at::ShaderVariable;
		
		public:
			Shader();
			Shader(const char *entry_name, const char *vert_sh_filename, const char *frag_sh_filename);
			~Shader();
			
			void create_from_file(const char *entry_name, const char *vert_sh_filename, const char *frag_sh_filename);
			void create_from_memory(const char *entry_name, const char *vert_sh, const char *frag_sh);
			
			GLint get_gl_entry_location() const;
			GLuint get_gl_vao_id() const;
			
			template <typename T>
			ShaderVariable<T> get_variable(const char *name);
			
			void set_uniform(const char *name, const Texture& texture);
			void set_uniform(const char *name, const Color& color);
			void set_uniform(const char *name, float arg);
			void set_uniform(const char *name, const Vector2f& arg);
			void set_uniform(const char *name, const Vector3f& arg);
			void set_uniform(const char *name, const Vector4f& arg);
			void set_uniform(const char *name, const Matrix4f& arg);
			void set_varying(const char *name, const Buffer<float>& array);
			void set_varying(const char *name, const Buffer<Vector2f>& array);
			void set_varying(const char *name, const Buffer<Vector3f>& array);
			void set_varying(const char *name, const Buffer<Vector4f>& array);
			void set_varying(const char *name, const Buffer<Color>& array);
			void delete_varying(const char *name);
			
			void bind() const;
			void unbind() const;
			
			static size_t get_max_texture_units() noexcept;
			
			//ObjectGL
			GLuint get_gl_id() const;
			bool is_valid() const noexcept;
			
		private:
			GLuint load_from_file(const char *filename, GLenum shader_type);
			GLuint load_from_memory(const char *data, GLenum shader_type);
			GLuint load(GLuint vert, GLuint frag);
			GLint get_location(const char *name);
			void clear();
			void ensure_vao();
			
			//---PARAMETERS
			void set_parameter(GLint location, const Texture& texture);
			void set_parameter(GLint location, const Color& color);
			void set_parameter(GLint location, float arg);
			void set_parameter(GLint location, const Vector2f& arg);
			void set_parameter(GLint location, const Vector3f& arg);
			void set_parameter(GLint location, const Vector4f& arg);
			void set_parameter(GLint location, const Matrix4f& arg);
			void set_parameter(GLint location, const Buffer<float>& array);
			void set_parameter(GLint location, const Buffer<Vector2f>& array);
			void set_parameter(GLint location, const Buffer<Vector3f>& array);
			void set_parameter(GLint location, const Buffer<Vector4f>& array);
			void set_parameter(GLint location, const Buffer<Color>& array);
			//ShaderVariable specific
			void set_parameter(GLint location, const Mesh& mesh);
			//---
			
			static const char* get_glsl_header();
			
			bool m_valid;
			
			GLuint m_program;
			
			GLuint m_vert;
			GLuint m_frag;
			
			GLint m_entry;
			
			size_t m_tex_units;
			
			std::map<std::string, GLint> m_locations;
			
			std::map< GLint, std::pair<int, const Texture*> > m_texs;
			
			std::map<GLint, ShaderAbstractVariable*> m_variables;
			
			GLuint m_vao;
			bool m_vao_ok;
	};
}

#include <atema/graphics/shader_variable.hpp>

//Inline definitions
namespace at
{
	template <typename T>
	ShaderVariable<T> Shader::get_variable(const char *name)
	{
		ShaderVariable<T> *ret = nullptr;
		GLint location = get_location(name);
		auto it = m_variables.find(location);
		
		if (it != m_variables.end())
		{
			ret = dynamic_cast< ShaderVariable<T>* >(it->second);
			
			if (!ret)
				ATEMA_ERROR("Invalid type requested for the variable.")
		}
		else
		{		
			ShaderVariable<T> tmp(this, location);
			tmp.m_valid = true;
			
			ret = &tmp;
			m_variables[location] = static_cast< ShaderAbstractVariable* >(ret);
		}
		
		return (*ret);
	}
}

#endif