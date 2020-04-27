#ifndef __NETUDP_EXPORT_HPP__
#define __NETUDP_EXPORT_HPP__

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#ifdef WIN32
#    ifdef NETUDP_DLL_EXPORT
#        define NETUDP_API_ __declspec(dllexport)
#    elif NETUDP_STATIC
#        define NETUDP_API_
#    else
#        define NETUDP_API_ __declspec(dllimport)
#    endif
#else
#    define NETUDP_API_
#endif

#endif
