#ifndef ATEMA_CONTEXT_IMPL_HEADER
#define ATEMA_CONTEXT_IMPL_HEADER

#include <atema/context/context.hpp>
#include <atema/context/opengl.hpp>

namespace at
{
	class context_impl
	{
		public:
			context_impl();
			virtual ~context_impl() noexcept;
			
			virtual void create(const context::gl_version& version) = 0;
			
			virtual void activate() = 0;
			virtual bool is_active() const noexcept = 0;
	};
}

#endif