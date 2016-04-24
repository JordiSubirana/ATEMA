#include <atema/core/error.hpp>

namespace at
{
	Error::Error(const char *description) noexcept :
		std::runtime_error(description)
	{
		
	}
	
	Error::~Error() noexcept
	{

	}
}