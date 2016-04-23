#ifndef ATEMA_WINDOW_CONFIG
#define ATEMA_WINDOW_CONFIG

#include <atema/config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_WINDOW_API
	
#else
	
	#if defined(ATEMA_WINDOW_EXPORT)
		
		#define ATEMA_WINDOW_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_WINDOW_API ATEMA_IMPORT
		
	#endif

#endif

#endif