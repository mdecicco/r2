#define r2_debug
#include <limits.h>
#include <float.h>
#include <stdint.h>

#define _CRT_NO_VA_START_VALIDATION

namespace r2 {
    typedef uintptr_t           Ptr;

    typedef int8_t              i8;
    typedef int16_t             i16;
    typedef int32_t             i32;
    typedef int64_t             i64;

    typedef uint8_t             u8;
    typedef uint16_t            u16;
    typedef uint32_t            u32;
    typedef uint64_t            u64;

    typedef signed char         s8;
    typedef signed short        s16;
    typedef signed int          s32;
    typedef signed long long    s64;

    typedef float               f32;
    typedef double              f64;

    typedef char                Byte;
    typedef unsigned char       uByte;

    typedef const char*         Literal;
    typedef char*               CString;
    typedef f32                 Scalar;
    typedef bool                Flag;

    #ifdef _USE_32BIT_TIME_
    typedef f32                 Time;
    #else
    typedef f64                 Time;
    #endif

    #ifdef __gnu_linux__
    #endif

    #ifdef _USE_32BIT_INDICES_
        typedef i32             Index;
        #define INDEX_NULL      INT32_MAX
    #else
        typedef i16             Index;
        #define INDEX_NULL      INT16_MAX
    #endif

    #ifdef _USE_32BIT_REF_COUNTS_
        typedef i32             RefCount;
    #else
        typedef i16             RefCount;
    #endif

    #ifdef _USE_64BIT_UIDS_
        typedef u64             UID;
    #else
        typedef u32             UID;
    #endif
}
