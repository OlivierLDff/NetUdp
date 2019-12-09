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

#ifdef NETUDP_USE_NAMESPACE

    #ifndef NETUDP_NAMESPACE
        #define NETUDP_NAMESPACE NetUdp
    #endif // ifndef NETUDP_NAMESPACE

    #define NETUDP_NAMESPACE_START namespace NETUDP_NAMESPACE {
    #define NETUDP_NAMESPACE_END }
    #define NETUDP_USING_NAMESPACE using namespace NETUDP_NAMESPACE;

#else // NETUDP_USE_NAMESPACE

    #undef NETUDP_NAMESPACE
    #define NETUDP_NAMESPACE
    #define NETUDP_NAMESPACE_START
    #define NETUDP_NAMESPACE_END
    #define NETUDP_USING_NAMESPACE

#endif // NETUDP_USE_NAMESPACE

#endif // __NETUDP_EXPORT_HPP__
