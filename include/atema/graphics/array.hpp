#ifndef ATEMA_GRAPHICS_ARRAY_HEADER
#define ATEMA_GRAPHICS_ARRAY_HEADER

#include <atema/context/opengl.hpp>
#include <atema/graphics/shared_object.hpp>

#include <vector>

namespace at
{
	template <typename T>
	class Array : public SharedObject
	{
		public:	
			enum class update : GLenum
			{
				static_mode		= GL_STATIC_DRAW;
				dynamic_mode	= GL_DYNAMIC_DRAW;
				stream_mode		= GL_STREAM_DRAW;
			};
			
		public:
			Array();
			Array(const T *elements, size_t elements_size, update update_mode = update.static_mode);
			Array(const Array<T>& Array);
			~Array() noexcept;
			
			void create(const T *elements, size_t elements_size, update update_mode = update.static_mode);
			void create(const Array<T>& Array);
			
			T* get() noexcept;
			const T* get() const noexcept;
			
			T& operator[](size_t index);
			const T& operator[](size_t index) const;
			
			size_t get_size() const;
			
			update get_update_mode() const;
			
			GLuint get_gl_id() const;
			
			void download();
			void upload();
			
		private:
			using data = struct
			{
				GLuint vbo;		
				std::vector elements;		
				size_t elements_size;		
				update update_mode;
				
				data() :
					vbo(0),
					elements(),
					elements_size(0),
					update_mode(update::static_mode)
				{
					
				}
				
				~data() noexcept
				{
					free();
				};
				
				void free() noexcept
				{
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glDeleteBuffers(1, &vbo);
					
					elements.clear();
					resize(0);
					
					elements_size = 0;
					
					vbo = 0;
				}
			};
			
			data m_data;
	};
}

#include "array.tpp"

#endif