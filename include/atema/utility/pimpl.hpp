#ifndef ATEMA_PIMPL_HEADER
#define ATEMA_PIMPL_HEADER

// Ensure we have std::unique_ptr
#include <memory>

#define ATEMA_PIMPL(class_name)								\
			std::shared_ptr<class_name>

#define ATEMA_PIMPL_CTOR(class_name)						\
			std::make_shared<class_name>
			
#define ATEMA_PIMPL_SHARE(new_class_name)					\
			std::dynamic_pointer_cast<new_class_name>

#endif