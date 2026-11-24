/* config.h do FreeGLUT para compilacao com Visual Studio (MSVC).
 * Equivale ao arquivo que o CMake do FreeGLUT gera a partir de config.h.in
 * no Windows. Usado pelo projeto vs/freeglut.vcxproj. */

#define HAVE_SYS_TYPES_H
#define HAVE_FCNTL_H
#define HAVE_STDINT_H
#define HAVE_INTTYPES_H
#define NEED_XPARSEGEOMETRY_IMPL

/* Mostra avisos e erros do FreeGLUT no console */
#define FREEGLUT_PRINT_WARNINGS
#define FREEGLUT_PRINT_ERRORS
