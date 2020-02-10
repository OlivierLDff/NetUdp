#ifndef __NETUDP_EXPORT_HPP__
#define __NETUDP_EXPORT_HPP__

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#ifdef WIN32
    #ifdef NETUDP_DLL_EXPORT  // Shared build
        #define NETUDP_API_ __declspec(dllexport)
    #elif NETUDP_STATIC       // No decoration when building staticlly
        #define NETUDP_API_
    #else                       // Link to lib
        #define NETUDP_API_ __declspec(dllimport)
    #endif
#else
    #define NETUDP_API_
#endif // WIN32

#endif // __NETUDP_EXPORT_HPP__
