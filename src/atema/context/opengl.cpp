#include <atema/context/opengl.hpp>

// PFNGLBINDBUFFERPROC at_glBindBuffer;

void at_glBindBuffer(GLenum target, GLuint buffer)
{
	glad_glBindBuffer(target, buffer);
}