//
// Created by spaske on 22.10.20..
//

#ifndef PROJECT_BASE_ERROR_H
#define PROJECT_BASE_ERROR_H

#include <iostream>

#define LOG(stream) stream << "[" << __FILE__ << ", " << __func__ << ", " << __LINE__ << "]\n"
#define BREAK_IF_FALSE(x) if (!(x)) __builtin_trap()
#define ASSERT(x, msg) do { if (!(x)) { std::cerr << msg << '\n'; BREAK_IF_FALSE(false); } } while(0)
#define GLCALL(x) \
do{ rg::clearAllOpenGlErrors(); x; BREAK_IF_FALSE(rg::wasPreviousOpenGLCallSuccessful(__FILE__, __LINE__, #x)); } while (0)

namespace rg {

void clearAllOpenGlErrors();
const char* openGLErrorToString(GLenum error);
bool wasPreviousOpenGLCallSuccessful(const char* file, int line, const char* call);

};
#endif //PROJECT_BASE_ERROR_H
