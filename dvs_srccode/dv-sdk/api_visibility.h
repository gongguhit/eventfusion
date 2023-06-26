
#ifndef DVSDK_EXPORT_H
#define DVSDK_EXPORT_H

#ifdef DVSDK_STATIC_DEFINE
#  define DVSDK_EXPORT
#  define DVSDK_NO_EXPORT
#else
#  ifndef DVSDK_EXPORT
#    ifdef sdk_EXPORTS
        /* We are building this library */
#      define DVSDK_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define DVSDK_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef DVSDK_NO_EXPORT
#    define DVSDK_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef DVSDK_DEPRECATED
#  define DVSDK_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef DVSDK_DEPRECATED_EXPORT
#  define DVSDK_DEPRECATED_EXPORT DVSDK_EXPORT DVSDK_DEPRECATED
#endif

#ifndef DVSDK_DEPRECATED_NO_EXPORT
#  define DVSDK_DEPRECATED_NO_EXPORT DVSDK_NO_EXPORT DVSDK_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef DVSDK_NO_DEPRECATED
#    define DVSDK_NO_DEPRECATED
#  endif
#endif

#endif /* DVSDK_EXPORT_H */
