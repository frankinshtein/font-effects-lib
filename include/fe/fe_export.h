
#ifndef FONT_EFFECT_EXPORT_H
#define FONT_EFFECT_EXPORT_H

#ifdef FONT_EFFECT_STATIC_DEFINE
#  define FONT_EFFECT_EXPORT
#  define FONT_EFFECT_NO_EXPORT
#else
#  ifndef FONT_EFFECT_EXPORT
#    ifdef font_effect_EXPORTS
/* We are building this library */
#ifdef _MSC_VER
#      define FONT_EFFECT_EXPORT extern "C"  __declspec(dllexport)
#elif APPLE
#      define FONT_EFFECT_EXPORT extern "C"  __attribute__((visibility("default")))
#else
#      define FONT_EFFECT_EXPORT extern "C"
#endif
#    else
/* We are using this library */
#      define FONT_EFFECT_EXPORT extern "C"
//__declspec(dllimport)
#    endif
#  endif

#  ifndef FONT_EFFECT_NO_EXPORT
#    define FONT_EFFECT_NO_EXPORT
#  endif
#endif

#ifndef FONT_EFFECT_DEPRECATED
#  define FONT_EFFECT_DEPRECATED __declspec(deprecated)
#endif

#ifndef FONT_EFFECT_DEPRECATED_EXPORT
#  define FONT_EFFECT_DEPRECATED_EXPORT FONT_EFFECT_EXPORT FONT_EFFECT_DEPRECATED
#endif

#ifndef FONT_EFFECT_DEPRECATED_NO_EXPORT
#  define FONT_EFFECT_DEPRECATED_NO_EXPORT FONT_EFFECT_NO_EXPORT FONT_EFFECT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FONT_EFFECT_NO_DEPRECATED
#    define FONT_EFFECT_NO_DEPRECATED
#  endif
#endif

#define FE_CC

#endif
