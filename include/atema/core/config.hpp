#ifndef ATEMA_CORE_CONFIG
#define ATEMA_CORE_CONFIG

#include <atema/config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_CORE_API
	
#else
	
	#if defined(ATEMA_CORE_EXPORT)
		
		#define ATEMA_CORE_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_CORE_API ATEMA_IMPORT
		
	#endif

#endif

#endif