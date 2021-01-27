/**
  ******************************************************************************
  * \file lib_uncompress.c
  * \author S.Lejczyk
  * \copyright 2020 BodyCap S.A.S.
  * \brief Implement decompression API for timestamp and temperature data.
  *       Details are given in P110SAL001.
  *       lib_bitStream is used to manipulate bit data.
  */
//****************************************************************************
// Standard include files
//****************************************************************************
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//****************************************************************************
// Project include files
//****************************************************************************
#include "project.h"
#include "lib_uncompress.h"
#include "lib_compress_defines.h"
#include "lib_bitStream.h"
#include "assert.h"

// Define specific color INFO traces in this module
#define NRF_LOG_INFO_COLOR  6   // magenta
//#define NRF_LOG_DEBUG_COLOR
#undef NRF_LOG_LEVEL
#define NRF_LOG_LEVEL 3         // set to 4 to display DEBUG LOGs
#define NRF_LOG_MODULE_NAME uncompress
#include "nrf_log.h"
#include <android/log.h>

#define  LOG_TAG    "testjni"
#define  ALOG(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

NRF_LOG_MODULE_REGISTER();

//****************************************************************************
// static Defines and enum typedef
//****************************************************************************
#define C_DIFF_MAX_NB      256
#define C_HANDLERS_MAX_NB   7
enum {
    // Timestamp part
    CT_START_DEC_1,   //    1 bit: only differential value 0
    CT_DEC_3,   // 1+3 bits: diff +/-1, resync, unreceveid, direct, invalid timestamp, new period
    CT_DEC_8,   // 4+8 bits: differential on 8 bits
    // Temperature part
    C9_START_DEC_3,
    C9_DEC_4,
    C9_DEC_DIRECT,
    C_NB_DEC
};
#define MAX_VALUE(n)                    ((1<<n)-1)
#define NUMBER_BITS_AND_MAX_VALUE(n)     n, MAX_VALUE(n)

//****************************************************************************
// static Structures typedef
//****************************************************************************

// Structure to describe each timestamp/temperature decoder for CT/C9 compressed data
typedef struct {
    uint8_t nbBits;      // number of bits to use
    uint8_t max_value;   // the high value (using all the bits at 1), telling that the next decoder should be used (current bits are consumed)
    uint8_t (*handler)(samples_t *p_samples, uint32_t parameter);  // handler to call if we get there

    // If diff is 1, it's a diff value, then decode values through diff_values, otherwise we will try handlers.
    uint16_t nbDiff;
    int16_t diff_values[C_DIFF_MAX_NB];

    struct {
        uint8_t (*handler)(samples_t *p_samples, uint32_t parameter);    // Function to call to handle this value. Return the next index to handle
        uint8_t nbBitsParam;                                            // mandatory nb bits for the parameter of the handler, to be read from stream by called of handler
                                                                        // handler won't be called if these bits are not available from the stream
    } handlers[C_HANDLERS_MAX_NB];
} def_C_decode_t;


//****************************************************************************
// static Functions prototypes
//****************************************************************************
static void ct_handler_add_value(samples_t *p_samples, uint32_t value, uint8_t addPeriod, uint8_t invalidValue);
static uint8_t ct_handler_differential(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_minus_one(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_plus_one(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_unexpected(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_unreceived(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_direct(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_invalid(samples_t *p_samples, uint32_t parameter);
static uint8_t ct_handler_new_period(samples_t *p_samples, uint32_t parameter);

static void c9_handler_add_value(samples_t *p_samples, uint32_t value);
static uint8_t c9_handler_differential(samples_t *p_samples, uint32_t parameter);
static uint8_t c9_handler_direct(samples_t *p_samples, uint32_t parameter);


//****************************************************************************
// static Variables
//****************************************************************************
static const def_C_decode_t C_dec[C_NB_DEC] = {
    // Array to decode Timestamp
    // Decoder 1 bit, value 0 for differential 0, 1 otherwise
    {
        NUMBER_BITS_AND_MAX_VALUE(CT_DIFF_UNCHANGED_NB_BITS), &ct_handler_differential,
        1, { CT_DIFF_UNCHANGED_VALUE },   // this is a diff value, and the diff value is 0
        { NULL },   // no handler there
    },
    // Decoder with 3 bits more
    {
        NUMBER_BITS_AND_MAX_VALUE(3), NULL, // no generic handler
        0, { 0 },   // this is not a diff value (only two are, handled by a function
        {
            [ CT_DIFF_MINUS_ONE_VALUE    & MAX_VALUE(3) ] = { &ct_handler_minus_one,    0, },
            [ CT_DIFF_PLUS_ONE_VALUE     & MAX_VALUE(3) ] = { &ct_handler_plus_one,     0, },
            [ CT_UNRECEIVED_PREFIX_VALUE & MAX_VALUE(3) ] = { &ct_handler_unreceived,   CT_UNRECEIVED_COUNTER_NB_BITS, },
            [ CT_DIRECT_PREFIX_VALUE     & MAX_VALUE(3) ] = { &ct_handler_direct,       CT_DIRECT_NB_BITS, },
            [ CT_INVALID_VALUE           & MAX_VALUE(3) ] = { &ct_handler_invalid,      0, },
            [ CT_NEW_PERIOD_PREFIX_VALUE & MAX_VALUE(3) ] = { &ct_handler_new_period,   CT_NEW_PERIOD_NB_BITS, },
            [ CT_RESERVED_VALUE          & MAX_VALUE(3) ] = { &ct_handler_unexpected,   0, },
        },
    },
    // 8 bits more, diffential value also
    {
        8, 0, &ct_handler_differential, // here the max_value is set to 0 since the highest value available for 8 bits is used here to code the index 255, which is +129
        256, {
            -129,-128,-127,-126,-125,-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-114,-113,-112,-111,-110,-109,-108,-107,-106,-105,-104,-103,-102,-101,-100,
            -99,-98,-97,-96,-95,-94,-93,-92,-91,-90,-89,-88,-87,-86,-85,-84,-83,-82,-81,-80,-79,-78,-77,-76,-75,-74,-73,-72,-71,-70,
            -69,-68,-67,-66,-65,-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,
            -39,-38,-37,-36,-35,-34,-33,-32,-31,-30,-29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10,
            -9,-8,-7,-6,-5,-4,-3,-2,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
            20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,
            50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
            80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,
            110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129
        },
        { NULL },   // no handler there
    },

    // Array to decode C9 compressed frames
    // Decoder 3 bits to decode a differential value. If all at 1, consume them and go to next decoder
    {   NUMBER_BITS_AND_MAX_VALUE(3),  &c9_handler_differential,   // values < to the max value are given in the array below
        7, { -3, -2, -1, 0, 1, 2, 3 },
        { NULL },   // no handler there
    },
    // Decoder 4 bits to decode a differential value. If all at 1, consume them and go to next decoder
    {   NUMBER_BITS_AND_MAX_VALUE(4),  &c9_handler_differential,   // values < to the max value are given in the array below
        15, { -10, -9, -8, -7, -6, -5, -4,
             4, 5, 6, 7, 8, 9, 10, 11 },
        { NULL },   // no handler there
    },
    // Last chance, it's a direct value on 13 bits, coding 0 to 81.91 values.
    // This entry is also used for the first value; there is no previous decoder in this case, so this direct value is not prefixed by 1 bits.
    {   C9_DIRECT_NB_BITS,  0, &c9_handler_direct,
        0, { 0 },
        { NULL },   // no handler there
    }
};

static uint32_t lastValidTime = UINT32_MAX;
static uint16_t currentPeriod = UINT16_MAX;
static uint16_t nbPeriodToAdd = 0;

//****************************************************************************
// Functions
//****************************************************************************

//****************************************************************************
static void ct_handler_add_value(samples_t *p_samples, uint32_t value, uint8_t addPeriod, uint8_t invalidValue)
{
    NRF_LOG_DEBUG("  ct_handler_add_value %u, addPeriod %u", value, addPeriod);
    p_samples->samples[p_samples->nbSamples].time = value;
    if (!invalidValue) {
        if (addPeriod && (currentPeriod != UINT16_MAX)) {
            NRF_LOG_DEBUG("      Adding %u periods %u to ref %u", nbPeriodToAdd+1, currentPeriod, p_samples->samples[p_samples->nbSamples].time);
            p_samples->samples[p_samples->nbSamples].time += (nbPeriodToAdd+1) * currentPeriod;
            nbPeriodToAdd = 0;
        }
        lastValidTime = p_samples->samples[p_samples->nbSamples].time;
        NRF_LOG_DEBUG("      New time ref is %u", lastValidTime);
    } else {
        nbPeriodToAdd++;
    }
}

//****************************************************************************
static uint8_t ct_handler_minus_one(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_minus_one %u", parameter);
    ct_handler_add_value(p_samples, lastValidTime-1, 1, 0);
    return C9_START_DEC_3;
}

//****************************************************************************
static uint8_t ct_handler_plus_one(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_plus_one %u", parameter);
    ct_handler_add_value(p_samples, lastValidTime+1, 1, 0);
    return C9_START_DEC_3;
}

//****************************************************************************
static uint8_t ct_handler_unexpected(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_WARNING("  ct_handler_unexpected %u", parameter);
    // Here we got a code which is not expected in the frame. Only ignore it
    return CT_START_DEC_1;  // keep timestamp uncoding
}

//****************************************************************************
static uint8_t ct_handler_unreceived(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_unreceived %u", parameter);
    // we need the next 6 bits to add the coded unreceived samples in the output data
    for(uint8_t i=0;i<parameter;i++) {
        ct_handler_add_value(p_samples, UINT32_MAX, 0, 1);
        // do not use c9_handler_add_value to store temperature, the value -1 would be used as next reference.
        p_samples->samples[p_samples->nbSamples].tempe = UINT16_MAX;
        NRF_LOG_DEBUG("    SAMPLE #%u: unreceived", p_samples->nbSamples+1);
        p_samples->nbSamples++;
        //c9_handler_add_value(p_samples, UINT16_MAX);
    }

    return CT_START_DEC_1;
}

//****************************************************************************
static uint8_t ct_handler_direct(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_direct %u", parameter);
    // we need the next 32 bits to add the raw timestamp
    ct_handler_add_value(p_samples, parameter, 0, 0); // we may have called this handler directly
    nbPeriodToAdd = 0; // ignore previous added periods for a direct value
    return C9_START_DEC_3;
}

//****************************************************************************
static uint8_t ct_handler_differential(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_differential %d", parameter);
    // we need the next 32 bits to add the raw timestamp
    ct_handler_add_value(p_samples, lastValidTime+parameter, 1, 0);
    return C9_START_DEC_3;
}

//****************************************************************************
static uint8_t ct_handler_invalid(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_invalid %u", parameter);
    // Add an invalid timestamp
    ct_handler_add_value(p_samples, UINT32_MAX, 0, 1);
    // the temperature follows, do not update nbSamples
    return C9_START_DEC_3;
}

//****************************************************************************
static uint8_t ct_handler_new_period(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  ct_handler_new_period %u", parameter);
    // we need the next 16 bits to set the new period
    currentPeriod = (uint16_t)parameter;
    return CT_START_DEC_1;  // next data are timestamp
}

static int16_t  lastValidTempe = INVALID_TEMPERATURE;

//****************************************************************************
static void c9_handler_add_value(samples_t *p_samples, uint32_t value)
{
    NRF_LOG_DEBUG("  c9_handler_add_value %d", (int16_t)value);
    if (value == C9_INVALID_TEMPERATURE) {
        value = INVALID_TEMPERATURE;
    } else {
        lastValidTempe = value;
        NRF_LOG_DEBUG("      New tempe ref is %d", lastValidTempe);
    }
    p_samples->samples[p_samples->nbSamples].tempe = (int16_t)value;
    NRF_LOG_DEBUG("    SAMPLE #%u: %u, %d", p_samples->nbSamples+1, p_samples->samples[p_samples->nbSamples].time, p_samples->samples[p_samples->nbSamples].tempe);
    p_samples->nbSamples++;
}

//****************************************************************************
static uint8_t c9_handler_differential(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  c9_handler_differential %d", parameter);
    if (lastValidTempe == INVALID_TEMPERATURE) {
        // error we have no reference
        NRF_LOG_WARNING("    Trying to add a differential temperature but previous temperature is invalid!");
    } else {
        c9_handler_add_value(p_samples, lastValidTempe+parameter);
    }
    return CT_START_DEC_1;
}

//****************************************************************************
static uint8_t c9_handler_direct(samples_t *p_samples, uint32_t parameter)
{
    NRF_LOG_DEBUG("  c9_handler_direct %u", parameter);
    // we need the next 13 bits to add the raw temperature
        c9_handler_add_value(p_samples, parameter);
    return CT_START_DEC_1;
}

uint16_t uncompress_data(uint8_t *buffer,uint16_t size,record_t *records){

    samples_t data;

    int ret = lib_uncompress_data(buffer, size, &data);
    if (ret != 0) {
        ALOG("%u uncompressed samples", data.nbSamples);
        for(uint32_t i=0;i<data.nbSamples;i++) {
            records[i] = data.samples[i] ;
            ALOG("%d move", i);
        }
        ALOG("%u uncompressed samples end", data.nbSamples);
        return data.nbSamples ;
    }else{
        return 0 ;
    }
}
//****************************************************************************
/**
* XXX data are currently uncompressed in a local buffer; we may want to use a callback because we do not know how many samples we will have.
*/
uint8_t lib_uncompress_data(uint8_t *buffer, uint8_t len, samples_t *p_samples)
{
    ASSERT(buffer);
    ASSERT(p_samples);
    uint32_t val;         // current bits value, read from the frame
    uint32_t param;
    uint8_t dec_index;   // the number of the decoder to use in C9_dec array
    uint8_t more_data = 1; // when not set in the while loop, the while will stop
    uint8_t ret = 0;  // by default error

    def_bitStream_t bs;
    lib_bitStream_define(&bs, buffer, len);

    // reset some internal data
    lastValidTime = UINT32_MAX;
    currentPeriod = UINT16_MAX;
    nbPeriodToAdd = 0;
  ALOG("This message comes from uncompress at line %d.", __LINE__);

    // First step is to know where the start bit is
    //offsetBit = 0;
    //len <<= 8;
/* 1.  read the bits required by nbBits in current entry
   2.  if the read value is = max_value, move the next entry and go back to step 1. (NEXT_ENTRY)
   3b. if the flag diff is not set, call the generic handler if it is set, with the value as parameter (CT_START_DEC_1 / C9_START_DEC_3)
   3c. otherwise, check the entry in handlers. If it is not NULL, read nbBitsParam bits (if not 0) from the stream,
       and call this handler. (CT_START_DEC_1 / C9_START_DEC_3)
   3d. otherwise ignore the value, display a warning (CT_START_DEC_1 / C9_START_DEC_3)
   4.  return to 1, while more bits to handle ion bit stream
 */
    ALOG("This message comes from memset at line %d.", p_samples);
    p_samples->nbSamples = 0 ;
    //memset(p_samples, 0, sizeof(samples_t));
    dec_index = CT_START_DEC_1;
    ALOG("This message comes from memset at line %d.", __LINE__);
    while((more_data)) {
        ALOG("  * dec_index = %u", dec_index);
        ASSERT(dec_index < C_NB_DEC);
        // Within this loop we parse a group of bits
        // step 1.
        more_data = lib_bitStream_get_bits(&bs, C_dec[dec_index].nbBits, &val);
        if (more_data) {          // check if we have enough bits for current decoder
            if ((C_dec[dec_index].max_value != 0) && (C_dec[dec_index].max_value == val)) {
                // Step 2. move to next entry
                dec_index++;
            } else {
                // Step 3.
                if (C_dec[dec_index].handler) {
                    // Step 3b. call generic handler
                    if (C_dec[dec_index].nbDiff && (val < C_dec[dec_index].nbDiff)) {
                        param = C_dec[dec_index].diff_values[val];
                    } else {
                        param = val;
                    }
                    dec_index = (*C_dec[dec_index].handler)(p_samples, param);   // call handler with supposed diff value as parameter
                } else {
                    // Step 3c.
                    if (C_dec[dec_index].handlers[val].handler) {
                        // Step 3d. call specific handler
                        ALOG(" checking handlers, dec_index=%u, val=%u, nbBitsParam=%u", dec_index, val, C_dec[dec_index].handlers[val].nbBitsParam);
                        param = 0;
                        if (C_dec[dec_index].handlers[val].nbBitsParam) {
                            // read the required data
                            more_data = lib_bitStream_get_bits(&bs, C_dec[dec_index].handlers[val].nbBitsParam, &param);
                        }
                        if (more_data) {
                            dec_index = (*C_dec[dec_index].handlers[val].handler)(p_samples, param);
                        }
                    } else {
                        // Step 3d. ignore entry but move to next entry
                        dec_index = CT_START_DEC_1;
                        ALOG("Ignored value %u, dec_index=%u", val ,dec_index);
                    }
                }
            }
        }
    }   // main while
    if (p_samples->nbSamples) {
        ret = 1;
    }

    return (ret);
}
