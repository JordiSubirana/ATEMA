#ifndef ATEMA_GRAPHICS_SHARED_OBJECT_HEADER
#define ATEMA_GRAPHICS_SHARED_OBJECT_HEADER

class shared_object
{
	public:
		shared_object() = default;
		virtual ~shared_object() = default;
		
		virtual void download() = 0;
		virtual void upload() = 0;
};

#endif