#ifndef ATEMA_CONTEXT_HEADER
#define ATEMA_CONTEXT_HEADER

#include <atema/context/config.hpp>

#include <atema/utility/non_copyable.hpp>
#include <atema/utility/pimpl.hpp>

namespace at
{
	class context_impl;
	
	class ATEMA_CONTEXT_API context : public non_copyable
	{
		public:
			using gl_version = struct
			{
				int major;
				int minor;
			};
		
		public:
			context();
			virtual ~context() noexcept;
			
			void create(const gl_version& version);
			
			void activate();
			bool is_active() const noexcept;
			
		protected:
			context(ATEMA_PIMPL(context_impl) pimpl);
			
		private:
			ATEMA_PIMPL(context_impl) m_pimpl;
	};
}

#endif