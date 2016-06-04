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

#ifndef ATEMA_CORE_FILE_HEADER
#define ATEMA_CORE_FILE_HEADER

#include <atema/core/config.hpp>
#include <atema/core/error.hpp>
#include <atema/core/flags.hpp>

#include <fstream>
#include <string>

namespace at
{
	class ATEMA_CORE_API File
	{
		public:
			enum class options : at::Flags
			{
				read		= 0x0001,
				write		= 0x0002,
				binary		= 0x0004,
				at_end		= 0x0008,
				append		= 0x0010,
				truncate	= 0x0020
			};
			
		public:
			File();
			File(const File& file) = delete;
			File(const char *filename, Flags opts = static_cast<Flags>(options::read) | static_cast<Flags>(options::write));
			virtual ~File();
			
			void open(const char *filename, Flags opts = static_cast<Flags>(options::read) | static_cast<Flags>(options::write));
			void close();
			
			template <typename T>
			void read(T& arg);
			
			template <typename T>
			void write(const T& arg);
			
			void write(const char *_format, std::size_t max_size, ...);
			
			operator bool() const noexcept;
			bool end_of_file() const noexcept;
			
			std::string get_line(int count = -1);
			int get_current_line_index();
			
		private:
			Flags process_options(Flags opts) const noexcept;
			
			std::fstream m_file;
			
			bool m_valid;
			
			Flags m_opts;
			
			int m_current_line;
			
			bool m_eof;
	};
}

ATEMA_ENUM_FLAGS(at::File::options)

namespace at
{
	// INLINE
	template <typename T>
	void File::read(T& _arg)
	{
		if (!m_valid)
			ATEMA_ERROR("Invalid file.")
		
		if (!(m_opts & options::read))
			ATEMA_ERROR("Read mode not set.")
		
		try
		{
			m_file >> _arg;
		}
		catch (...)
		{
			ATEMA_ERROR("An error occurred.")
		}
	}

	template <typename T>
	void File::write(const T& _arg)
	{
		if (!m_valid)
			ATEMA_ERROR("Invalid file.")
		
		if (!(m_opts & options::write))
			ATEMA_ERROR("Write mode not set.")
		
		try
		{
			m_file <<_arg;
		}
		catch (...)
		{
			ATEMA_ERROR("An error occurred.")
		}
	}
}

#endif