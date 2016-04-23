#ifndef ATEMA_CONTEXT_CONFIG
#define ATEMA_CONTEXT_CONFIG

#include <atema/config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_CONTEXT_API
	
#else
	
	#if defined(ATEMA_CONTEXT_EXPORT)
		
		#define ATEMA_CONTEXT_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_CONTEXT_API ATEMA_IMPORT
		
	#endif

#endif

#endif