#ifndef ATEMA_ERROR_HEADER
#define ATEMA_ERROR_HEADER

#include <atema/core/config.hpp>

#include <exception>
#include <stdexcept>

#define ATEMA_ERROR(desc) throw at::Error(desc);

namespace at
{
	class ATEMA_CORE_API Error : public std::runtime_error
	{
		public:
			Error(const char *description) noexcept;
			virtual ~Error() noexcept;
	};
}

#endif