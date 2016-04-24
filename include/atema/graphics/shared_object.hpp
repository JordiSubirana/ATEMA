#ifndef ATEMA_GRAPHICS_SHARED_OBJECT_HEADER
#define ATEMA_GRAPHICS_SHARED_OBJECT_HEADER

class SharedObject
{
	public:
		SharedObject() = default;
		virtual ~SharedObject() = default;

#warning je propose cpy2gpu() et cpy2cpu pour les nom de ces fonctions
		virtual void download() = 0;
		virtual void upload() = 0;
};

#endif