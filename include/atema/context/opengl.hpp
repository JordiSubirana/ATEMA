#ifndef ATEMA_OPENGL_HEADER
#define ATEMA_OPENGL_HEADER

#include <glad/glad.h>

// glBindBuffer
#undef glBindBuffer
ATEMA_API extern void at_glBindBuffer(GLenum target, GLuint buffer);
#define glBindBuffer at_glBindBuffer

#endif