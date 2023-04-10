#pragma once
#include <vcruntime.h>


#if _HAS_CXX20 && defined __cpp_modules && defined _BUILD_UTILS_MODULE && !defined __SANITIZE_ADDRESS__
#define _USE_MODULES

#else
#undef _USE_MODULES

#endif


#if _HAS_CXX20 && defined(_BUILD_UTILS_MODULE)
#define _EXPORT_UTILS export

#else
#define _EXPORT_UTILS

#endif