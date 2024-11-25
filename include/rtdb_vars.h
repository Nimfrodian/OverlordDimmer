#ifndef RTDB_VARS_H
#define RTDB_VARS_H

#include<limits.h>
#include<stdint.h>
#include<float.h>

#define TU8_MIN (0)
#define TU8_MAX (UCHAR_MAX)
#define TU16_MIN (0)
#define TU16_MAX (USHRT_MAX)
#define TU32_MIN (0)
#define TU32_MAX (UINT_MAX)
#define TS8_MIN (SCHAR_MIN)
#define TS8_MAX (SCHAR_MAX)
#define TS16_MIN (SHRT_MIN)
#define TS16_MAX (SHRT_MAX)
#define TS32_MIN (INT_MIN)
#define TS32_MAX (INT_MAX)
#define TB_MIN (0)
#define TB_MAX (1)
#define TE_MIN (0)
#define TE_MAX (TU32_MAX)
#define TF32_MIN (FLT_MIN)
#define TF32_MAX (FLT_MAX)

typedef uint8_t tU8;
typedef uint16_t tU16;
typedef uint32_t tU32;
typedef int8_t tS8;
typedef int16_t tS16;
typedef int32_t tS32;
typedef tU8 tB;
typedef tU32 tE;
typedef float tF32;

// Variable units lookup table and enum
typedef enum
{
    VAR_UNIT_us = 0,
    VAR_UNIT_MAX_NUM
} unitEnumT;

// tU32S
enum
{
    TRGD_TI_US_ZEROCROSSTRIGGERDELAY_U32 = 0,

    NUM_OF_TU32S = 1
};

typedef enum
{
    SIGNAL_ERROR = 0,           ///< Value unreliable, use default
    SIGNAL_CAN_TIMEOUT = 194,   ///< Timeout on CAN signal
    SIGNAL_OK = 255
} signalStateT;

typedef enum
{
    OBJECT_STANDARD = 0,    ///< Value is derived from program
    OBJECT_OVERRIDDEN = 1,  ///< Value is derived from external source
    OBJECT_NOT_INIT = 255,
} objectStatusT;

typedef struct
{
    tU32 crc32;                         ///< currently unused. CRC of all object's variables
    signalStateT signalState : 8;       ///< state of the signal, i.e., its reliability
    objectStatusT objectStatus : 8;     ///< status of this object
    tU8 signalUnit;                     ///< unit of the signal, i.e., volt or RPM
    const char* signalCmnt;             ///< Signal comment
} rtdbT;

typedef struct
{
    rtdbT ss;
    tS8 tS8_val;
    tS8 tS8_min;
    tS8 tS8_def;
    tS8 tS8_max;
} tS8S;

typedef struct
{
    rtdbT ss;
    tS16 tS16_val;
    tS16 tS16_min;
    tS16 tS16_def;
    tS16 tS16_max;
} tS16S;

typedef struct
{
    rtdbT ss;
    tS32 tS32_val;
    tS32 tS32_min;
    tS32 tS32_def;
    tS32 tS32_max;
} tS32S;

typedef struct
{
    rtdbT ss;
    tU8 tU8_val;
    tU8 tU8_min;
    tU8 tU8_def;
    tU8 tU8_max;
} tU8S;

typedef struct
{
    rtdbT ss;
    tU16 tU16_val;
    tU16 tU16_min;
    tU16 tU16_def;
    tU16 tU16_max;
} tU16S;

typedef struct
{
    rtdbT ss;
    tU32 tU32_val;
    tU32 tU32_min;
    tU32 tU32_def;
    tU32 tU32_max;
} tU32S;

typedef struct
{
    rtdbT ss;
    tF32 tF32_val;
    tF32 tF32_min;
    tF32 tF32_def;
    tF32 tF32_max;
} tF32S;

typedef struct
{
    rtdbT ss;
    tU8 tB_val;
    tU8 tB_min;
    tU8 tB_def;
    tU8 tB_max;
} tBS;

typedef struct
{
    rtdbT ss;
    tU32 tE_val;
    tU32 tE_min;
    tU32 tE_def;
    tU32 tE_max;
} tES;

#endif
