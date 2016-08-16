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
#include <atema/core/file_stream.hpp>
#include <atema/core/error.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

namespace at
{
	//TODO: check parameter type from glsl shader

	//PRIVATE
	GLuint Shader::load_from_file(const char *filename, GLenum shader_type)
	{
		std::string shader_code;
		FileStream file;
		
		if (!filename)
			ATEMA_ERROR("No filename given.")
		
		file.open(filename, static_cast<Flags>(FileStream::options::read));
		
		if(!file)
			ATEMA_ERROR("Error while opening file.")
		
		while(!file.end_of_file())
			shader_code += file.get_line() + "\n";
		
		file.close();
		
		return (load_from_memory(shader_code.c_str(), shader_type));
	}

	GLuint Shader::load_from_memory(const char *data, GLenum shader_type)
	{
		GLuint shader_id = 0;
		std::string shader_code(data);
		
		try
		{
			GLint result = GL_FALSE;
			
			glGetError();
			
			if (!data)
				ATEMA_ERROR("No data given.")
			
			complete_shader(shader_code, shader_type);
			
			shader_id = glCreateShader(shader_type);
			
			if ((glGetError() != GL_NO_ERROR) || (shader_id == 0))
				ATEMA_ERROR("OpenGL internal error: Shader creation failed.")
			
			const char *tmp_data = shader_code.c_str();
			
			if (shader_type == GL_VERTEX_SHADER)
			{
				FileStream file;
				file.open("test.vert", static_cast<Flags>(FileStream::options::write));
				file.write(tmp_data);
			}
			
			glShaderSource(shader_id, 1, &(tmp_data), nullptr);
			
			if (glGetError() != GL_NO_ERROR)
				ATEMA_ERROR("OpenGL internal error: Shader allocation failed.")
			
			glCompileShader(shader_id);
			
			//TODO: get the OpenGL compilation errors.
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
	
	void Shader::complete_shader(std::string& shader, GLenum shader_type)
	{
		std::stringstream program(shader.c_str());
		std::string line;
		std::string new_shader;
		
		while (std::getline(program, line, '\n'))
		{
			if (line.compare(0, 8, "#version", 8) == 0)
			{
				new_shader += line + "\n";
				
				new_shader += std::string(get_glsl_header()) + "\n";
				
				continue;
			}
			
			if (line.compare(0, 23, "ATEMA_MODEL_VERTEX_DATA", 23) == 0)
			{
				size_t name_begin = line.find_first_of(" ");
				size_t name_end = line.find_first_of(";");
				
				std::string basename = line.substr(name_begin+1, name_end-name_begin-1);
				
				new_shader += std::string("in vec3 at_") + basename + "_position;\n";
				new_shader += std::string("in vec3 at_") + basename + "_normal;\n";
				new_shader += std::string("in vec2 at_") + basename + "_tex_coords;\n";
				new_shader += std::string("AtemaModel ") + basename + " = { at_" + basename + "_position, at_" + basename + "_normal, at_" + basename + "_tex_coords };\n";
				
				line = line.substr(name_end+1, std::string::npos); //If something is after the ';'
				
				continue;
			}
			
			if (line.compare(0, 25, "ATEMA_MODEL_FRAGMENT_DATA", 25) == 0)
			{
				size_t name_begin = line.find_first_of(" ");
				size_t name_end = line.find_first_of(";");
				
				std::string basename = line.substr(name_begin+1, name_end-name_begin-1);
				
				new_shader += std::string("uniform AtemaMaterial ") + basename + ";\n";
				
				line = line.substr(name_end+1, std::string::npos); //If something is after the ';'
				
				continue;
			}
			
			new_shader += line + "\n";
		}
		
		shader = new_shader;
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

	void Shader::clear()
	{
		m_tex_units = 0;
		
		m_uniform_locations.clear();
		m_varying_locations.clear();
		
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
		m_entry_name(),
		m_entry(0),
		m_tex_units(0),
		m_uniform_locations(),
		m_varying_locations(),
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
			create_from_file(entry_name, vert_sh_filename, frag_sh_filename);
		}
		catch (const Error& e)
		{
			throw;
		}
	}

	Shader::~Shader()
	{
		clear();
		
		// glUseProgram(0);

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
			
			m_entry_name = entry_name;
			
			prog = load(vert, frag);
			
			m_vert = vert;
			m_frag = frag;
			
			m_program = prog;
			
			if (m_entry_name.compare("gl_Position") == 0)
			{
				m_entry = 0;
			}
			else
			{
				try
				{
					try
					{
						m_entry = get_varying_location(entry_name);
					}
					catch(...)
					{
						m_entry = get_varying_location((std::string("at_") + entry_name + "_position").c_str());
						// m_entry_name = std::string("at_") + entry_name + "_position";
					}
				}
				catch(...)
				{
					ATEMA_ERROR("Invalid mesh location.")
				}
			}
			
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
			
			m_entry_name = entry_name;
			
			prog = load(vert, frag);
			
			m_vert = vert;
			m_frag = frag;
			
			m_program = prog;
			
			if (m_entry_name.compare("gl_Position") == 0)
			{
				m_entry = 0;
			}
			else
			{
				try
				{
					try
					{
						m_entry = get_varying_location(entry_name);
					}
					catch(...)
					{
						m_entry = get_varying_location((std::string("at_") + entry_name + "_position").c_str());
						// m_entry_name = std::string("at_") + entry_name + "_position";
					}
				}
				catch(...)
				{
					ATEMA_ERROR("Invalid mesh location.")
				}
			}
			
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
	
	GLint Shader::get_uniform_location(const char *name)
	{
		bool exists = false;
		GLint location;
		
		if (!name)
			ATEMA_ERROR("No variable name given.")
		
		exists = (m_uniform_locations.find(std::string(name)) != m_uniform_locations.end());
		
		if (exists)
		{
			location = m_uniform_locations[std::string(name)];
		}
		else
		{
			location = glGetUniformLocation(m_program, name);
			
			if (location < 0)
				ATEMA_ERROR("Invalid variable name.")
			
			m_uniform_locations[std::string(name)] = location;
		}
		
		return (location);
	}
	
	GLint Shader::get_varying_location(const char *name)
	{
		bool exists = false;
		GLint location;
		
		if (!name)
			ATEMA_ERROR("No variable name given.")
		
		exists = (m_varying_locations.find(std::string(name)) != m_varying_locations.end());
		
		if (exists)
		{
			location = m_varying_locations[std::string(name)];
		}
		else
		{
			location = glGetAttribLocation(m_program, name);
			
			if (location < 0)
				ATEMA_ERROR("Invalid variable name.")
			
			m_varying_locations[std::string(name)] = location;
		}
		
		return (location);
	}
	
	const char* Shader::get_entry_name() const
	{
		return (m_entry_name.c_str());
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
		GLint location;
		bool exists = false;
		GLenum tex_unit = GL_TEXTURE0;
		
		if (!name)
			ATEMA_ERROR("No variable name given.")
		
		exists = (m_uniform_locations.find(std::string(name)) != m_uniform_locations.end());
		
		if (exists)
		{
			location = m_uniform_locations[std::string(name)];
			
			m_texs[location].second = &texture;
		}
		else
		{
			if ((get_max_texture_units() - m_tex_units == 0) && !exists) // on depasse le nombre max de texture units
				ATEMA_ERROR("Too much texture units allocated.")
			
			location = glGetUniformLocation(m_program, name);
			
			if (location < 0)
				ATEMA_ERROR("Invalid variable name.")
			
			m_uniform_locations[std::string(name)] = location;
			
			m_texs[location].first = static_cast<int>(m_tex_units);
			m_texs[location].second = &texture;
			
			m_tex_units++;
		}
		
		tex_unit += static_cast<GLenum>(m_texs[location].first);
		
		set_parameter(location, texture);
	}
	
	void Shader::set_uniform(const char *name, const Color& color)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_uniform_location(name), color);
	}

	void Shader::set_uniform(const char *name, float arg)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_uniform_location(name), arg);
	}

	void Shader::set_uniform(const char *name, const Vector2f& arg)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_uniform_location(name), arg);
	}

	void Shader::set_uniform(const char *name, const Vector3f& arg)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_uniform_location(name), arg);
	}
	
	void Shader::set_uniform(const char *name, const Vector4f& arg)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_uniform_location(name), arg);
	}
	
	void Shader::set_uniform(const char *name, const Matrix4f& arg)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_uniform_location(name), arg);
	}
	
	void Shader::set_uniform(const char *name, const Material& arg)
	{
		std::string tmp(name);
		
		if (tmp.empty())
			ATEMA_ERROR("Invalid variable name.")
		
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_uniform((tmp + ".ambient_color").c_str(), arg.ambient_color);
		set_uniform((tmp + ".diffuse_color").c_str(), arg.diffuse_color);
		set_uniform((tmp + ".specular_color").c_str(), arg.specular_color);
		set_uniform((tmp + ".shininess").c_str(), arg.shininess);
		set_uniform((tmp + ".ambient_texture").c_str(), arg.ambient_texture);
		set_uniform((tmp + ".diffuse_texture").c_str(), arg.diffuse_texture);
		set_uniform((tmp + ".specular_texture").c_str(), arg.specular_texture);
	}
	void Shader::set_uniform(const char *name, const PointLight& arg)
	{
		std::string tmp(name);
		
		if (tmp.empty())
			ATEMA_ERROR("Invalid variable name.")
		
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_uniform((tmp + ".position").c_str(), arg.position);
		set_uniform((tmp + ".color").c_str(), arg.color);
		set_uniform((tmp + ".radius").c_str(), arg.radius);
		set_uniform((tmp + ".intensity").c_str(), arg.intensity);
	}

	void Shader::set_varying(const char *name, const Buffer<float>& array)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_varying_location(name), array);
	}

	void Shader::set_varying(const char *name, const Buffer<Vector2f>& array)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_varying_location(name), array);
	}

	void Shader::set_varying(const char *name, const Buffer<Vector3f>& array)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_varying_location(name), array);
	}
	
	void Shader::set_varying(const char *name, const Buffer<Vector4f>& array)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_varying_location(name), array);
	}
	
	void Shader::set_varying(const char *name, const Buffer<Color>& array)
	{
		ensure_vao(); //At this point VAO is created or an exception is thrown
		
		set_parameter(get_varying_location(name), array);
	}
	
	void Shader::delete_varying(const char *name)
	{
		try
		{
			GLint location;
			
			if (!m_vao_ok)
				return;
			
			location = get_varying_location(name);
			
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
		
		for (auto& pair : m_texs)
		{
			// if (!pair.second.second || !pair.second.second->is_valid())
				// ATEMA_ERROR("Invalid texture.")
			
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(pair.second.first));
			
			glBindTexture(GL_TEXTURE_2D, pair.second.second->get_gl_id());
		}
	}

	void Shader::unbind() const
	{
		// glUseProgram(0);
		
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
	
	//--------------------
	//PARAMETERS
	void Shader::set_parameter(GLint location, const Texture& texture)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniform1i(location, m_texs[location].first);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}
	
	void Shader::set_parameter(GLint location, const Color& color)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniform4f(location, color.r, color.g, color.b, color.a);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}

	void Shader::set_parameter(GLint location, float arg)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniform1f(location, arg);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}

	void Shader::set_parameter(GLint location, const Vector2f& arg)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniform2f(location, arg.data[0], arg.data[1]);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}

	void Shader::set_parameter(GLint location, const Vector3f& arg)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniform3f(location, arg.data[0], arg.data[1], arg.data[2]);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}
	
	void Shader::set_parameter(GLint location, const Vector4f& arg)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniform4f(location, arg.data[0], arg.data[1], arg.data[2], arg.data[3]);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}
	
	void Shader::set_parameter(GLint location, const Matrix4f& arg)
	{
		//TODO: Optimize this ugly stuff
		GLint current_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(m_program);
		
		glUniformMatrix4fv(location, 1, GL_FALSE, arg.get());
		
		if (m_program != static_cast<GLuint>(current_program))
			glUseProgram(static_cast<GLuint>(current_program));
	}

	void Shader::set_parameter(GLint location, const Buffer<float>& array)
	{
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
	}

	void Shader::set_parameter(GLint location, const Buffer<Vector2f>& array)
	{
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
	}

	void Shader::set_parameter(GLint location, const Buffer<Vector3f>& array)
	{
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
	}
	
	void Shader::set_parameter(GLint location, const Buffer<Vector4f>& array)
	{
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
	}
	
	void Shader::set_parameter(GLint location, const Buffer<Color>& array)
	{
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, array.get_gl_id());
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);
	}
	
	//SHADER VARIABLE SPECIFIC
	void Shader::set_parameter(GLint location, const Mesh& arg)
	{
		glBindVertexArray(m_vao);
		
		if ((location == m_entry) && (arg.indices.is_valid()))
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arg.indices.get_gl_id());
		glBindBuffer(GL_ARRAY_BUFFER, arg.elements.get_gl_id());
		
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindVertexArray(0);
		glDisableVertexAttribArray(location);
	}
	//--------------------
	
	const char* Shader::get_glsl_header()
	{
		static const char * header = "\n"
			"//ATEMA types definition\n"
			"struct AtemaMaterial\n"
			"{\n"
				"\tvec4 ambient_color;\n"
				"\tvec4 diffuse_color;\n"
				"\tvec4 specular_color;\n"
				"\t\n"
				"\tfloat shininess;\n"
				"\t\n"
				"\tsampler2D ambient_texture;\n"
				"\tsampler2D diffuse_texture;\n"
				"\tsampler2D specular_texture;\n"
			"};\n"
			"\n"
			"struct AtemaModel\n"
			"{\n"
				"\tvec3 position;\n"
				"\tvec3 normal;\n"
				"\tvec2 tex_coords;\n"
			"};\n"
			"\n"
			"struct AtemaPointLight\n"
			"{\n"
				"\tvec3 position;\n"
				"\tvec4 color;\n"
				"\tfloat radius;\n"
				"\tfloat intensity;\n"
			"};\n"
			"//End of ATEMA types definition\n"
		"";
		
		return (header);
	}
}