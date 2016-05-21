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
#include <atema/utility/non_copyable.hpp>
#include <atema/math/vector.hpp>
#include <atema/graphics/texture.hpp>
#include <atema/graphics/buffer_array.hpp>
#include <atema/graphics/color.hpp>

#include <map>

namespace at
{
	class ATEMA_GRAPHICS_API Shader : public ObjectGL, public NonCopyable
	{
		public:
			Shader();
			Shader(const char *entry_name, const char *vert_sh_filename, const char *frag_sh_filename);
			~Shader();
			
			void create_from_file(const char *entry_name, const char *vert_sh_filename, const char *frag_sh_filename);
			void create_from_memory(const char *entry_name, const char *vert_sh, const char *frag_sh);
			
			GLint get_gl_entry_location() const;
			GLuint get_gl_vao_id() const;
			
			void set_uniform(const char *name, const Texture& texture);
			// void set_uniform(const char *name, const Transform& transform);
			void set_uniform(const char *name, const Color& color);
			void set_uniform(const char *name, float arg);
			void set_uniform(const char *name, const Vector2f& arg);
			void set_uniform(const char *name, const Vector3f& arg);
			void set_uniform(const char *name, const Vector4f& arg);
			void set_varying(const char *name, const BufferArray<float>& array);
			void set_varying(const char *name, const BufferArray<Vector2f>& array);
			void set_varying(const char *name, const BufferArray<Vector3f>& array);
			void set_varying(const char *name, const BufferArray<Vector4f>& array);
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
			
			bool m_valid;
			
			GLuint m_program;
			
			GLuint m_vert;
			GLuint m_frag;
			
			GLint m_entry;
			
			size_t m_tex_units;
			
			std::map<std::string, GLint> m_locations;
			
			std::map< GLint, std::pair<int, const Texture*> > m_texs;
			
			/*
			std::map<GLint, const BufferArray<float>*> m_arr_float;
			std::map<GLint, const BufferArray<Vector2f>*> m_arr_vec2;
			std::map<GLint, const BufferArray<Vector3f>*> m_arr_vec3;
			#warning create Vector4f container
			//*/
			
			GLuint m_vao;
			bool m_vao_ok;
	};
}

#endif