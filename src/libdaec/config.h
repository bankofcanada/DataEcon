#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef _WIN32
    #define DE_API __declspec(dllexport)
    // #pragma message("Setting DE_API to __declspec(dllexport)")
#else
    #define DE_API
    // #pragma message("Setting DE_API to empty")
#endif

#define DE_VERSION "0.4.0"
#define DE_VERNUM 0x0400
#define DE_VER_MAJOR 0
#define DE_VER_MINOR 4
#define DE_VER_REVISION 0
#define DE_VER_SUBREVISION 0

#define DE_MAX_AXES 5   /* maximum number of axes (dimensions) of Nd-arrays */

#endif
