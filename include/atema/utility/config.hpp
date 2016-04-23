#ifndef ATEMA_UTILITY_CONFIG
#define ATEMA_UTILITY_CONFIG

#include <atema/config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_UTILITY_API
	
#else
	
	#if defined(ATEMA_UTILITY_EXPORT)
		
		#define ATEMA_UTILITY_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_UTILITY_API ATEMA_IMPORT
		
	#endif

#endif

#endif