#include <atema/core/error.hpp>

namespace at
{
	error::error(const char *description) noexcept :
		std::runtime_error(description)
	{
		
	}
	
	error::~error() noexcept
	{

	}
}