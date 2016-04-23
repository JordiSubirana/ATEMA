#ifndef ATEMA_NON_COPYABLE_HEADER
#define ATEMA_NON_COPYABLE_HEADER

#include <atema/utility/config.hpp>

namespace at
{
	class ATEMA_UTILITY_API non_copyable
	{
		public:
			non_copyable() = default;
			non_copyable(const non_copyable& object) = delete;
			virtual ~non_copyable() = default;
			
			non_copyable& operator=(const non_copyable& object) = delete;
	};
}

#endif