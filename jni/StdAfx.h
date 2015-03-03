// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef STDAFX
#define STDAFX

// Windows Header Files:
#ifdef _WINDOWS
//#define UNICODE
//#include <WTypes.h>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <string.h>
#include "wltypes.h"
#else
#include <wchar.h>
#include <climits>
#endif
#include <new>

#endif // !defined(STDAFX)
