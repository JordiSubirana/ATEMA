#ifndef ATEMA_ERROR_HEADER
#define ATEMA_ERROR_HEADER

#include <atema/core/config.hpp>

#include <exception>
#include <stdexcept>

#define ATEMA_ERROR(desc) throw at::error(desc);

namespace at
{
	class ATEMA_CORE_API error : public std::runtime_error
	{
		public:
			error(const char *description) noexcept;
			virtual ~error() noexcept;
	};
}

#endif