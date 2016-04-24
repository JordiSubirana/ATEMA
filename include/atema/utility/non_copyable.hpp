#ifndef ATEMA_NON_COPYABLE_HEADER
#define ATEMA_NON_COPYABLE_HEADER

#include <atema/utility/config.hpp>

namespace at
{
	class NonCopyable
	{
		public:
			NonCopyable() = default;
			NonCopyable(const NonCopyable& object) = delete;
			virtual ~NonCopyable() = default;
			
			NonCopyable& operator=(const NonCopyable& object) = delete;
	};
}

#endif