#ifndef ATEMA_GRAPHICS_SHARED_OBJECT_HEADER
#define ATEMA_GRAPHICS_SHARED_OBJECT_HEADER

class shared_object
{
	public:
		shared_object() = default;
		virtual ~shared_object() = default;

#warning je propose cpy2gpu() et cpy2cpu pour les nom de ces fonctions
		virtual void download() = 0;
		virtual void upload() = 0;
};

#endif