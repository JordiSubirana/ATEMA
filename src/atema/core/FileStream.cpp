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

#include <Atema/Core/FileStream.hpp>

#include <cstdarg>
#include <cstdio>

namespace at
{
	//PRIVATE
	Flags FileStream::process_options(Flags opts) const noexcept
	{
		Flags buffer = opts;
		
		if ( ((opts & Option::Append) || (opts & Option::Truncate)) && !(opts & Option::Write) )
			buffer &= ~(Option::Append | Option::Truncate);
		
		if ((opts & Option::Append) && (opts & Option::Truncate))
			buffer &= ~Option::Truncate;
		
		return (buffer);
	}

	//PUBLIC
	FileStream::FileStream()
	try :
		m_file(),
		m_valid(false),
		m_opts(0),
		m_current_line(0),
		m_eof(false)
	{

	}
	catch (...)
	{
		ATEMA_ERROR("Construction failed.")
	}

	FileStream::FileStream(const char *filename, Flags opts)
	try :
		m_file(),
		m_valid(false),
		m_opts(0),
		m_current_line(0),
		m_eof(false)
	{
		open(filename, opts);
	}
	catch (const std::exception& e)
	{
		throw;
	}

	FileStream::~FileStream()
	{
		try
		{
			close();
		}
		catch (...)
		{
		
		}
	}

	void FileStream::open(const char *filename, Flags b_opts)
	{
		try
		{
			std::ios_base::openmode openmode = std::ios::binary;
			Flags opts = process_options(b_opts);
			
			if (!filename)
			{
				ATEMA_ERROR("No filename given.")
			}
			
			close();
			
			if (opts & Option::Binary)
				openmode |= std::ios::binary;
			else
				openmode &= ~std::ios::binary;
			
			if (opts & Option::Read)
				openmode |= std::ios::in;
			
			if (opts & Option::Write)
				openmode |= std::ios::out;
			
			if (opts & Option::AtEnd)
				openmode |= std::ios::ate;
			
			if (opts & Option::Append)
				openmode |= std::ios::app;
			
			if (opts & Option::Truncate)
				openmode |= std::ios::trunc;
			
			m_file.open(filename, openmode);
			
			if (!m_file.is_open())
			{
				m_file.open(filename, std::ios::out); //Ensure the file exists
				
				close();
				
				m_file.open(filename, openmode);
			}
			
			if (m_file.is_open())
			{
				m_valid = true;
				m_opts = opts;
				m_current_line = 0;
				m_eof = false;
			}
			else
			{
				ATEMA_ERROR("FileStream could not be open.")
			}
		}
		catch (const std::exception& e)
		{
			throw;
		}
	}

	void FileStream::close()
	{
		try
		{
			if (m_file.is_open())
				m_file.close();
			
			m_eof = false;
			m_valid = false;
			
			m_file.clear(); //Clear error bits
		}
		catch (const std::exception& e)
		{
			throw;
		}
	}

	void FileStream::write(const char *format, std::size_t max_size, ...)
	{
		va_list args;
		char buffer[max_size];
		
		if (!format)
			ATEMA_ERROR("No format string given.")
		
		if (!m_valid)
			ATEMA_ERROR("Invalid file.")
		
		if (!(m_opts & Option::Write))
			ATEMA_ERROR("Write mode not set.")
		
		va_start(args, max_size);	
		
		vsnprintf(buffer, max_size, format, args);
		
		try
		{
			m_file << buffer;
		}
		catch (...)
		{
			va_end(args);
			ATEMA_ERROR("An error occurred.")
		}
		
		va_end(args);
	}

	FileStream::operator bool() const noexcept
	{
		return (m_valid);
	}
	
	bool FileStream::end_of_file() const noexcept
	{
		return (m_eof);
	}
	
	std::string FileStream::get_line(int count)
	{
		std::string tmp;
		
		if (!m_valid)
			ATEMA_ERROR("Invalid file.")
		
		if (count < 0)
		{
			if (m_eof)
				ATEMA_ERROR("End of file already reached.")
			else
				std::getline(m_file, tmp);
			
			m_current_line++;
			
			if (m_file.eof())
				m_eof = true;
		}
		else
		{
			int begin = 0;
			int length = count;
			
			m_eof = false;
			
			if (count > m_current_line)
			{
				begin = m_current_line;
				length = count;
			}
			else
			{
				m_file.seekg(0, m_file.beg);
			}
			
			for (int i = begin; i <= length; i++)
			{
				std::getline(m_file, tmp);
				
				if (m_file.eof())
				{
					m_eof = true;
					
					m_current_line = i+1;
					
					if (i < length)
						ATEMA_ERROR("Index is greater than line number")
				}
			}
			
			m_current_line = length+1;
		}
		
		return (tmp);
	}
	
	int FileStream::get_current_line_index()
	{
		return (m_current_line);
	}
}
