#ifndef ATEMA_GRAPHICS_CONFIG
#define ATEMA_GRAPHICS_CONFIG

#include <atema/config.hpp>

#if defined(ATEMA_STATIC)
	
	#define ATEMA_GRAPHICS_API
	
#else
	
	#if defined(ATEMA_GRAPHICS_EXPORT)
		
		#define ATEMA_GRAPHICS_API ATEMA_EXPORT
		
	#else
		
		#define ATEMA_GRAPHICS_API ATEMA_IMPORT
		
	#endif

#endif

#endif