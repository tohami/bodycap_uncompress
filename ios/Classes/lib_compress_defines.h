/**
  ******************************************************************************
  * \file lib_compress_defines.h
  * \author S.Lejczyk
  * \copyright 2020 BodyCap S.A.S.
  * \brief Defines related to lib_compress.
  ******************************************************************************
  */
#ifndef _SRV_COMPRESS_DEFINES_H
#define _SRV_COMPRESS_DEFINES_H
//****************************************************************************
// Standard include files
//****************************************************************************

//****************************************************************************
// Project include files
//****************************************************************************

//****************************************************************************
// extern Defines and enum typedef
//****************************************************************************

// C9 parameters
#define C9_NB_CODE  (C9_DIFF_MAX - C9_DIFF_MIN + 1)  // number of coded values
#define C9_DIRECT_PREFIX_VALUE    0x7F
#define C9_DIRECT_PREFIX_NB_BITS  7
#define C9_DIRECT_MIN   0
#define C9_DIRECT_MAX   ((1<<C9_DIRECT_NB_BITS)-1)
#define C9_DIFF_MIN     -10
#define C9_DIFF_MAX     11
#define C9_DIRECT_NB_BITS   13

#define C9_INVALID_TEMPERATURE          C9_DIRECT_MAX

// Comp time parameters
#define CT_DIFF_MIN     -129
#define CT_DIFF_MAX     129
#define CT_NB_CODE  (CT_DIFF_MAX - CT_DIFF_MIN + 1) // number of coded values

#define CT_DIFF_MINUS_ONE_VALUE           0b1000
#define CT_DIFF_MINUS_ONE_NB_BITS         4
#define CT_DIFF_UNCHANGED_VALUE           0
#define CT_DIFF_UNCHANGED_NB_BITS         1
#define CT_DIFF_PLUS_ONE_VALUE            0b1001
#define CT_DIFF_PLUS_ONE_NB_BITS          4

#define CT_UNRECEIVED_PREFIX_VALUE      0b1100
#define CT_UNRECEIVED_PREFIX_NB_BITS    4
#define CT_UNRECEIVED_COUNTER_NB_BITS   6
#define CT_UNRECEIVED_COUNTER_MAX       ((1<<CT_UNRECEIVED_COUNTER_NB_BITS)-1)

#define CT_DIRECT_PREFIX_VALUE          0b1010    // prefix for a direct timestamp, following as a uint32_t
#define CT_DIRECT_PREFIX_NB_BITS        4
#define CT_DIRECT_NB_BITS   32

#define CT_INVALID_VALUE                0b1011    // invalid time (resync case)
#define CT_INVALID_NB_BITS              4

#define CT_NEW_PERIOD_PREFIX_VALUE      0b1101
#define CT_NEW_PERIOD_PREFIX_NB_BITS    4
#define CT_NEW_PERIOD_NB_BITS           16

#define CT_RESERVED_VALUE               0b1110
#define CT_RESERVED_NB_BITS             4

#ifndef INVALID_TEMPERATURE
#define INVALID_TEMPERATURE             0x7FFF  // the pill may send this value on internal sensor error
#endif

//****************************************************************************
// extern Structures typedef
//****************************************************************************

// Struct with raw data
typedef struct {
    uint32_t    time;
    int16_t     tempe;
} record_t;


//****************************************************************************
// extern Variables
//****************************************************************************

//****************************************************************************
// extern Functions prototypes
//****************************************************************************


#endif // _SRV_COMPRESS_DEFINES_H

