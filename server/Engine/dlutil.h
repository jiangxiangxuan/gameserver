
#ifndef _DL_UTIL_H_
#define _DL_UTIL_H_

#include <dlfcn.h>

#define DLOPEN(lib) dlopen(lib, RTLD_LAZY);
#define DLSYM(h, type, func)  (type)dlsym(h, func);
#define DLCLOSE(h) dlclose(h);
#define DLERROR() dlerror()

#endif
