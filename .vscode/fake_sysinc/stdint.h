#ifndef _FAKE_STDINT_H_
#define _FAKE_STDINT_H_

/* Minimal stdint.h replacement for IntelliSense without a system compiler */

typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;

typedef signed int          int_fast16_t;
typedef unsigned int        uint_fast16_t;
typedef signed int          int_fast32_t;
typedef unsigned int        uint_fast32_t;

typedef signed char         int_least8_t;
typedef unsigned char       uint_least8_t;
typedef signed short        int_least16_t;
typedef unsigned short      uint_least16_t;
typedef signed int          int_least32_t;
typedef unsigned int        uint_least32_t;
typedef signed long long    int_least64_t;
typedef unsigned long long  uint_least64_t;

typedef signed long long    intmax_t;
typedef unsigned long long  uintmax_t;

typedef signed int          intptr_t;
typedef unsigned int        uintptr_t;

#define INT8_MIN     (-0x80 - 1)
#define INT16_MIN    (-0x8000 - 1)
#define INT32_MIN    (-0x80000000 - 1)
#define INT64_MIN    (-0x8000000000000000LL - 1)

#define INT8_MAX     0x7F
#define INT16_MAX    0x7FFF
#define INT32_MAX    0x7FFFFFFF
#define INT64_MAX    0x7FFFFFFFFFFFFFFFLL

#define UINT8_MAX    0xFF
#define UINT16_MAX   0xFFFF
#define UINT32_MAX   0xFFFFFFFF
#define UINT64_MAX   0xFFFFFFFFFFFFFFFFULL

#define INTPTR_MIN   INT32_MIN
#define INTPTR_MAX   INT32_MAX
#define UINTPTR_MAX  UINT32_MAX

#define INTMAX_MIN   INT64_MIN
#define INTMAX_MAX   INT64_MAX
#define UINTMAX_MAX  UINT64_MAX

#define INT8_C(x)     (x)
#define INT16_C(x)    (x)
#define INT32_C(x)    (x ## L)
#define INT64_C(x)    (x ## LL)
#define UINT8_C(x)     (x ## U)
#define UINT16_C(x)    (x ## U)
#define UINT32_C(x)    (x ## UL)
#define UINT64_C(x)    (x ## ULL)

#endif /* _FAKE_STDINT_H_ */