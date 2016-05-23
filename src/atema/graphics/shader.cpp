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

#include <atema/graphics/shader.hpp>
#include <atema/core/error.hpp>

#include <vector>
#include <fstream>
#include <iostream>

namespace at
{
	//TODO: check parameter type from glsl shader

	//PRIVATE
	GLuint Shader::load_from_file(const char *filename, GLenum shader_type)
	{
		const char* buffer = nullptr;
		
		std::ifstream shader_stream;
		
		try
		{
			std::string shader_code;
			std::string line;
			
			glGetError();
			
			if (!filename)
				ATEMA_ERROR("No filename given.")
			
			shader_stream.open(filename, std::ios::in);
			
			if(!shader_stream.is_open())
				ATEMA_ERROR("Invalid filename.")
			
			line = "";
			
			while(getline(shader_stream, line))
			{
				shader_code += "" + line;
			}
			
			shader_stream.close();
			
			buffer = shader_code.c_str();
			
			return (load_from_memory(buffer, shader_type));
		}
		catch (const Error& e)
		{
			if(!shader_stream.is_open())
			{
				try {shader_stream.close();}
				catch (...) {}
			}
			
			throw;
		}
		
		return (0);
	}

	GLuint Shader::load_from_memory(const char *data, GLenum shader_type)
	{
		GLuint shader_id = 0;
		
		try
		{
			GLint result = GL_FALSE;
			
			glGetError();
			
			if (!data)
				ATEMA_ERROR("No data given.")
			
			glGetError();
			
			shader_id = glCreateShader(shader_type);
			
			if ((glGetError() != GL_NO_ERROR) || (shader_id == 0))
				ATEMA_ERROR("OpenGL internal error: Shader creation failed.")
			
			glShaderSource(shader_id, 1, &(data), nullptr);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL internal error: Shader allocation failed.")
			
			glCompileShader(shader_id);
			
			//TODO: r�cup�rer les cha�nes d'erreur de la compilation OpenGL.
			glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
			
			// /*
			int InfoLogLength = 0;
			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if ( InfoLogLength > 1 ){
				std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
				glGetShaderInfoLog(shader_id, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
				printf("%s", &FragmentShaderErrorMessage[0]);
			}
			// */
			
			if ((glGetError() != GL_NO_ERROR) || (result == GL_FALSE))
				ATEMA_ERROR("OpenGL internal error: Shader compilation failed.")
			
			return (shader_id);
		}
		catch (const Error& e)
		{
			if (glIsShader(shader_id))
				glDeleteShader(shader_id);
			
			shader_id = 0;
			
			throw;
		}
		
		return (0);
	}

	GLuint Shader::load(GLuint vert, GLuint frag)
	{
		GLuint prog = 0;
		
		try
		{
			GLint result = GL_FALSE;
			
			glGetError();
			
			prog = glCreateProgram();
			
			if ((glGetError() != GL_NO_ERROR) || (prog == 0))
				ATEMA_ERROR("OpenGL internal error: Shader creation failed.")
			
			if (vert)
				glAttachShader(prog, vert);
			if (frag)
				glAttachShader(prog, frag);
			
			glLinkProgram(prog);
			
			//TODO: Gestion erreur linkage
			glGetProgramiv(prog, GL_LINK_STATUS, &result);
			
			// /*
			int InfoLogLength = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if ( InfoLogLength > 1 ){
				std::vector<char> ProgramErrorMessage(InfoLogLength+1);
				glGetProgramInfoLog(prog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
				printf("%s", &ProgramErrorMessage[0]);
			}
			// */
			
			if ((glGetError() != GL_NO_ERROR) || (result == GL_FALSE))
				ATEMA_ERROR("OpenGL internal error: Shader linking failed.")
			
			return (prog);
		}
		catch (const Error& e)
		{
			if (glIsProgram(prog))
				glDeleteProgram(prog);
			
			throw;
		}
		
		return (0);
	}

	GLint Shader::get_location(const char *name)
	{
		bool exists = false;
		GLint location;
		
		if (!name)
			ATEMA_ERROR("No variable name given.")
		
		exists = (m_locations.find(std::string(name)) != m_locations.end());
		
		if (exists)
		{
			location = m_locations[std::string(name)];
		}
		else
		{
			location = glGetUniformLocation(m_program, name);
			
			if (location < 0)
			{
				location = glGetAttribLocation(m_program, name);
				
				if (location < 0)
					ATEMA_ERROR("Invalid variable name.")
			}
			
			m_locations[std::string(name)] = location;
		}
		
		return (location);
	}

	void Shader::clear()
	{
		m_tex_units = 0;
		
		m_locations.clear();
		
		m_texs.clear();
		
		/*
		m_arr_float.clear();
		m_arr_vec2.clear();
		m_arr_vec3.clear();
		//*/
	}
	
	void Shader::ensure_vao()
	{
		if (m_vao_ok)
			return;
		
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		
		if (glIsVertexArray(m_vao) == GL_FALSE)
		{
			glBindVertexArray(0);
			ATEMA_ERROR("OpenGL internal error: VAO creation failed.")
		}
		
		m_vao_ok = true;
		
		glBindVertexArray(0);
	}

	//----------
	//PUBLIC
	Shader::Shader() :
		m_valid(false),
		m_program(0),
		m_vert(0),
		m_frag(0),
		m_entry(0),
		m_tex_units(0),
		m_locations(),
		m_texs(),
		m_vao(0),
		m_vao_ok(false)
	{
		
	}

	Shader::Shader(const char *entry_name, const char *vert_sh_filename, const char *frag_sh_filename) :
		Shader()
	{
		try
		{
			create_from_file(vert_sh_filename, frag_sh_filename, entry_name);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	Shader::~Shader()
	{
		clear();
		
		glUseProgram(0);

		if (glIsProgram(m_program))
			glDeleteProgram(m_program);
		
		if (glIsShader(m_vert))
			glDeleteShader(m_vert);
		
		if (glIsShader(m_frag))
			glDeleteShader(m_frag);
		
		if (glIsVertexArray(m_vao) == GL_TRUE)
			glDeleteVertexArrays(1, &m_vao);
		
		m_program = 0;
		
		m_vert = 0;
		m_frag = 0;
		
		m_vao = 0;
	}

	void Shader::create_from_file(const char *entry_name, const char *vert_sh_filename, const char *frag_sh_filename)
	{
		GLuint vert = 0;
		GLuint frag = 0;
		GLuint prog = 0;
		
		try
		{
			m_valid = false;
			
			clear();
			
			if (!entry_name)
				ATEMA_ERROR("No mesh location given.")
			
			if (vert_sh_filename)
				vert = load_from_file(vert_sh_filename, GL_VERTEX_SHADER);
			
			if (frag_sh_filename)
				frag = load_from_file(frag_sh_filename, GL_FRAGMENT_SHADER);
			
			if (!vert && !frag)
				ATEMA_ERROR("No filenames given.")
			
			prog = load(vert, frag);
			
			m_vert = vert;
			m_frag = frag;
			
			m_program = prog;
			
			if (std::string(entry_name).compare("gl_Position") == 0)
				m_entry = 0;
			else
				m_entry = get_location(entry_name);
			
			if (m_entry < 0)
				ATEMA_ERROR("Invalid mesh location.")
			
			m_valid = true;
		}
		catch (const Error& e)
		{
			if (glIsProgram(prog))
				glDeleteProgram(prog);
			
			if (glIsShader(vert))
				glDeleteShader(vert);
			
			if (glIsShader(frag))
					glDeleteShader(frag);
			
			throw;
		}
	}

	void Shader::create_from_memory(const char *entry_name, const char *vert_sh, const char *frag_sh)
	{
		GLuint vert = 0;
		GLuint frag = 0;
		GLuint prog = 0;
		
		try
		{
			m_valid = false;
			
			clear();
			
			if (!entry_name)
				ATEMA_ERROR("No mesh location given.")
			
			if (vert_sh)
				vert = load_from_memory(vert_sh, GL_VERTEX_SHADER);
			
			if (frag_sh)
				frag = load_from_memory(frag_sh, GL_FRAGMENT_SHADER);
			
			if (!vert && !frag)
				ATEMA_ERROR("No filenames given.")
			
			prog = load(vert, frag);
			
			m_vert = vert;
			m_frag = frag;
			
			m_program = prog;
			
			if (std::string(entry_name).compare("gl_Position") == 0)
				m_entry = 0;
			else
				m_entry = get_location(entry_name);
			
			if (m_entry < 0)
				ATEMA_ERROR("Invalid mesh location.")
			
			m_valid = true;
		}
		catch (const Error& e)
		{
			if (glIsProgram(prog))
				glDeleteProgram(prog);
			
			if (glIsShader(vert))
				glDeleteShader(vert);
			
			if (glIsShader(frag))
					glDeleteShader(frag);
			
			throw;
		}
	}

	GLint Shader::get_gl_entry_location() const
	{
		return (m_entry);
	}
	
	GLuint Shader::get_gl_vao_id() const
	{
		const_cast<Shader*>(this)->ensure_vao();
		
		return (m_vao);
	}

	void Shader::set_uniform(const char *name, const Texture& texture)
	{
		try
		{
			GLint location;
			bool exists = false;
			GLenum tex_unit = GL_TEXTURE0;
			
			if (!name)
				ATEMA_ERROR("No variable name given.")
			
			exists = (m_locations.find(std::string(name)) != m_locations.end());
			
			if (exists)
			{
				location = m_locations[std::string(name)];
				
				m_texs[location].second = &texture;
			}
			else
			{
				if ((get_max_texture_units() - m_tex_units == 0) && !exists) // on depasse le nombre max de texture units
					ATEMA_ERROR("Too much texture units allocated.")
				
				location = glGetUniformLocation(m_program, name);
				
				if (location < 0)
					ATEMA_ERROR("Invalid variable name.")
				
				m_locations[std::string(name)] = location;
				
				m_texs[location].first = static_cast<int>(m_tex_units);
				m_texs[location].second = &texture;
				
				m_tex_units++;
			}
			
			tex_unit += static_cast<GLenum>(m_texs[location].first);
			
			glUseProgram(m_program);
			
			glUniform1i(location, m_texs[location].first);
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	/*
	void Shader::set_uniform(const char *name, const Transform& transform)
	{
		try
		{
			GLint location;
			
			location = get_location(name);
			
			glUseProgram(m_program);
			
			glUniformMatrix4fv(location, 1, GL_FALSE, transform);
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	//*/
	void Shader::set_uniform(const char *name, const Color& color)
	{
		try
		{
			GLint location;
			
			location = get_location(name);
			
			glUseProgram(m_program);
			
			glUniform4fv(location, 1, reinterpret_cast<const float*>(&color));
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::set_uniform(const char *name, float arg)
	{
		try
		{
			GLint location;
			
			location = get_location(name);
			
			glUseProgram(m_program);
			
			glUniform1f(location, arg);
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::set_uniform(const char *name, const Vector2f& arg)
	{
		try
		{
			GLint location;
			
			location = get_location(name);
			
			glUseProgram(m_program);
			
			glUniform2f(location, arg.data[0], arg.data[1]);
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::set_uniform(const char *name, const Vector3f& arg)
	{
		try
		{
			GLint location;
			
			location = get_location(name);
			
			glUseProgram(m_program);
			
			glUniform3f(location, arg.data[0], arg.data[1], arg.data[2]);
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	void Shader::set_uniform(const char *name, const Vector4f& arg)
	{
		try
		{
			GLint location;
			
			location = get_location(name);
			
			glUseProgram(m_program);
			
			glUniform4f(location, arg.data[0], arg.data[1], arg.data[2], arg.data[3]);
			
			glUseProgram(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::set_varying(const char *name, const Buffer<float>& array)
	{
		try
		{
			GLint location;
			
			ensure_vao(); //At this point VAO is created or an exception is thrown
			
			location = get_location(name);
			
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE, 0, 0);
			glBindVertexArray(0);
			
			// m_arr_float[location] = &array;
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::set_varying(const char *name, const Buffer<Vector2f>& array)
	{
		try
		{
			GLint location;
			
			ensure_vao(); //At this point VAO is created or an exception is thrown
			
			location = get_location(name);
			
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glBindVertexArray(0);
			
			// m_arr_vec2[location] = &array;
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::set_varying(const char *name, const Buffer<Vector3f>& array)
	{
		try
		{
			GLint location;
			
			ensure_vao(); //At this point VAO is created or an exception is thrown
			
			location = get_location(name);
			
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindVertexArray(0);
			
			// m_arr_vec3[location] = &array;
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	void Shader::set_varying(const char *name, const Buffer<Vector4f>& array)
	{
		try
		{
			GLint location;
			
			ensure_vao(); //At this point VAO is created or an exception is thrown
			
			location = get_location(name);
			
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBindVertexArray(0);
			
			// m_arr_vec4[location] = &array;
		}
		catch (const Error& e)
		{
			throw;
		}
	}
	
	void Shader::delete_varying(const char *name)
	{
		try
		{
			GLint location;
			
			if (!m_vao_ok)
				return;
			
			location = get_location(name);
			
			glBindVertexArray(m_vao);
			glDisableVertexAttribArray(location);
			glBindVertexArray(0);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	void Shader::bind() const
	{
		if (!is_valid())
			ATEMA_ERROR("Invalid shader.")
		
		glUseProgram(m_program);
		
		// /*
		for (auto& pair : m_texs)
		{
			if (!pair.second.second->is_valid())
				ATEMA_ERROR("Invalid texture.")
			
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(pair.second.first));
			
			glBindTexture(GL_TEXTURE_2D, pair.second.second->get_gl_id());
		}
		//*/
		
		/*
		for (auto it = m_arr_float.begin(); it != m_arr_float.end(); it++)
		{
			if (!it->second->is_valid())
				ATEMA_ERROR("Invalid buffer array.")
			
			glBindBuffer(GL_ARRAY_BUFFER, it->second->get_gl_id());
			glEnableVertexAttribArray(it->first);
			glVertexAttribPointer(it->first, 1, GL_FLOAT, GL_FALSE, 0, 0);
		}
		
		for (auto it = m_arr_vec2.begin(); it != m_arr_vec2.end(); it++)
		{
			if (!it->second->is_valid())
				ATEMA_ERROR("Invalid buffer array.")
			
			glBindBuffer(GL_ARRAY_BUFFER, it->second->get_gl_id());
			glEnableVertexAttribArray(it->first);
			glVertexAttribPointer(it->first, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
		
		for (auto it = m_arr_vec3.begin(); it != m_arr_vec3.end(); it++)
		{
			if (!it->second->is_valid())
				ATEMA_ERROR("Invalid buffer array.")
			
			glBindBuffer(GL_ARRAY_BUFFER, it->second->get_gl_id());
			glEnableVertexAttribArray(it->first);
			glVertexAttribPointer(it->first, 3, GL_FLOAT, GL_FALSE, 0, 0);
			// std::cout << glGetError() << std::endl;
			// if (glGetError() != GL_NO_ERROR)
				// ATEMA_ERROR("FUCK YOUUU")
		}
		//*/
	}

	void Shader::unbind() const
	{
		glUseProgram(0);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	size_t Shader::get_max_texture_units() noexcept
	{
		return (GL_MAX_TEXTURE_COORDS > GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ? GL_MAX_TEXTURE_COORDS : GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	}
	
	GLuint Shader::get_gl_id() const
	{
		return (m_program);
	}
	
	bool Shader::is_valid() const noexcept
	{
		return (m_valid);
	}
}