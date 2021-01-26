/**
  ******************************************************************************
  * \file    	lib_bitStream.c
  * \author 	Sylvain Lejczyk
  * \copyright	2020 BodyCap S.A.S.
  * \brief  	This library provides some functions to handle bit streams.
  ******************************************************************************
  */

//****************************************************************************
// Standard include files
//****************************************************************************
#include <string.h>

//****************************************************************************
// Project include files
//****************************************************************************
#include "project.h"
#include "lib_bitStream.h"
#include "assert.h"

// Define specific color INFO traces in this module
#define NRF_LOG_INFO_COLOR  6   // magenta
//#define NRF_LOG_DEBUG_COLOR
#undef NRF_LOG_LEVEL
#define NRF_LOG_LEVEL 3         // set to 4 to display DEBUG LOGs, also set NRF_LOG_DEFAULT_LEVEL to 4
#define NRF_LOG_MODULE_NAME BS
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


//****************************************************************************
// static Defines and enum typedef
//****************************************************************************
#define BYTES2BITS(val) (val << 3)
#define BITS2BYTES(val) (val >> 3)

//****************************************************************************
// static Structures typedef
//****************************************************************************

//****************************************************************************
// static Functions prototypes
//****************************************************************************

//****************************************************************************
// static Variables
//****************************************************************************

//****************************************************************************
// Functions
//****************************************************************************

//****************************************************************************
void lib_bitStream_create(def_bitStream_t *bs, uint8_t* dataBuffer, uint16_t sizeBytes, uint8_t setStartBit)
{
    ASSERT(bs);             // Always called with staticaly assigned buffers
    ASSERT(dataBuffer);     // Always called with staticaly assigned buffers
    ASSERT(sizeBytes>0);    // Would trig in debug configuration. Anyway, if size is not enough, this is handled in lib_bitStream_set_bits.
    memset(bs, 0, sizeof(def_bitStream_t));
    memset(dataBuffer, 0, sizeBytes);
    bs->data = dataBuffer;
    bs->dataSize = BYTES2BITS(sizeBytes);
    if (setStartBit) {
        bs->data[0] |= 0x80;   // set start bit
        bs->currentIdx = bs->confirmedIdx = 1;    // one bit is already used
    }
    NRF_LOG_DEBUG("created BS with %u bytes", sizeBytes);
}

//****************************************************************************
void lib_bitStream_define(def_bitStream_t *bs, uint8_t* dataBuffer, uint16_t sizeBytes)
{
    if (!bs) {
        return;
    }
    memset(bs, 0, sizeof(def_bitStream_t));
    if (!dataBuffer || !sizeBytes) {
        return;
    }
    bs->data = dataBuffer;
    bs->dataSize = BYTES2BITS(sizeBytes);
    bs->confirmedIdx = bs->dataSize;
    NRF_LOG_DEBUG("defined BS from %u bytes of data", sizeBytes);
#ifdef UNIT_TEST
    log_displayBuffer(":          ", dataBuffer, sizeBytes);
#endif // UNIT_TEST
}

//****************************************************************************
uint8_t lib_bitStream_set_bits(def_bitStream_t *bs, uint32_t value, uint8_t nbBits)
{
    uint8_t ret = 0;
    ASSERT(bs);     // Always called with staticaly assigned buffers

    if (bs->currentIdx+nbBits <= bs->dataSize) {
        NRF_LOG_DEBUG("  adding value %02x (%u bits)", value, nbBits);
        // enough space, we can add that
        uint16_t i,offsetBit;
        offsetBit = bs->currentIdx+nbBits-1;
        for(i=0;i<nbBits;i++) {
            if (value & 1) {
                // check that offsetBit >> 3 is within the buffer
                bs->data[offsetBit >> 3] |= (1 << (7-((offsetBit)&0x7)));
            }
            value = value >> 1; // shift bits for next loop turn
            offsetBit--;
        }
        bs->currentIdx += nbBits;
        ret = 1;
//        NRF_LOG_DEBUG("  added value %02x (%u bits); current now @ %u", value, nbBits, bs->currentIdx);
#ifdef UNIT_TEST_DEBUG
        log_display_binary(bs->data, bs->currentIdx);
#endif // UNIT_TEST_DEBUG
    }
    return(ret);
}

//****************************************************************************
void lib_bitStream_completeLastByte(def_bitStream_t *bs)
{
    ASSERT(bs);     // Always called with staticaly assigned buffers

    uint8_t nb = (bs->confirmedIdx) & 0x07;   // if the number of bits  % 8 is 0, then we have only full bytes
    if (nb) {
        nb = 8 - nb;    // number of bits to set to 1
        NRF_LOG_DEBUG("Adding %u 1-bits at the end", nb);
        uint8_t nb2 = (1<<nb) -1;      // mask to apply to the last byte
        bs->data[bs->confirmedIdx >> 3] |= nb2;
        bs->confirmedIdx += nb;
#ifdef UNIT_TEST_DEBUG
        log_display_binary(bs->data, bs->confirmedIdx);
#endif
    }
}

//****************************************************************************
void lib_bitStream_validate(def_bitStream_t *bs, uint32_t nbSamplesAdded)
{
    ASSERT(bs);     // Always called with staticaly assigned buffers
    bs->confirmedIdx = bs->currentIdx;
    bs->nbSamples += nbSamplesAdded;
    NRF_LOG_DEBUG("Validated %u bits, %u samples", bs->confirmedIdx, bs->nbSamples);
}

//****************************************************************************
uint16_t lib_bitStream_get_len(def_bitStream_t *bs)
{
    ASSERT(bs);     // Always called with staticaly assigned buffers
    return((bs->confirmedIdx+7)/8);
}

//****************************************************************************
uint8_t lib_bitStream_get_bits(def_bitStream_t *bs, uint8_t nb, uint32_t *output)
{
    uint8_t st = 1;
    uint8_t cp_nb = nb;
    uint32_t ret = 0;

    if ((!bs) || (!output)) {
        return 0;   // error
    }

    while ( (bs->currentIdx < bs->confirmedIdx) && (nb) ) {
        ret = ret << 1; // shift to the left
        //printf("    checking data offset %u bit mask %02x\n", BUF_RX_CMD9_DATA((offset>>3)), (1<<(7-(offset & 0x7))));
        uint16_t offsetByte = (bs->currentIdx >> 3);
        uint8_t offsetBit = bs->currentIdx & 0x7;
        if (bs->data[offsetByte] & (1<<(7-offsetBit))) {
            ret |= 1;   // set the LSb
        }
        bs->currentIdx++;
        nb--;
    }
    if (nb > 0) {
        st = 0; // there was not enough data to get all the requested bits
    } else {
        NRF_LOG_DEBUG("     read %u bits, val %02x (remaining %u bits)", cp_nb, ret, bs->confirmedIdx-bs->currentIdx);
        *output = ret;
    }
    return(st);
}
