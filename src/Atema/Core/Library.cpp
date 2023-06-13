/*
	Copyright 2022 Jordi SUBIRANA

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

#include <Atema/Core/Error.hpp>
#include <Atema/Core/Library.hpp>

using namespace at;

#ifdef ATEMA_SYSTEM_WINDOWS

#include <Atema/Core/Windows.hpp>

class Library::Implementation
{
public:
	Implementation() = delete;

	Implementation(const std::filesystem::path& filePath)
	{
		m_lib = LoadLibrary(filePath.string().c_str());
	}

	~Implementation()
	{
		if (isLoaded())
		{
			FreeLibrary(m_lib);
		}
	}

	bool isLoaded() const
	{
		return m_lib != nullptr;
	}

	Function getFunction(const std::string& name) const
	{
		ATEMA_ASSERT(isLoaded(), "Failed to get the function since the library could not be loaded");

		return reinterpret_cast<Function>(GetProcAddress(m_lib, name.c_str()));
	}

private:
	HINSTANCE m_lib;
};

#else

#error Library implementation for current OS does not exist

#endif

Library::Library(const std::filesystem::path& filePath)
{
	m_impl = std::make_unique<Implementation>(filePath);
}

Library::~Library()
{
}

bool Library::isLoaded() const
{
	return m_impl->isLoaded();
}

Library::Function Library::getFunction(const std::string& name) const
{
	return m_impl->getFunction(name);
}
