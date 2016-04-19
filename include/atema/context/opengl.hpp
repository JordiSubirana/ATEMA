#ifndef ATEMA_OPENGL_HEADER
#define ATEMA_OPENGL_HEADER

#include <glad/glad.h>

// glBindBuffer
#undef glBindBuffer
GLAPI PFNGLBINDBUFFERPROC at_glBindBuffer;
#define glBindBuffer at_glBindBuffer

#endif