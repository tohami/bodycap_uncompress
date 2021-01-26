/**
  ******************************************************************************
  * \file lib_uncompress.h
  * \author S.Lejczyk
  * \copyright 2020 BodyCap S.A.S.
  * \brief Interface to uncompress data.
  ******************************************************************************
  */
#ifndef _LIB_UNCOMPRESS_H
#define _LIB_UNCOMPRESS_H
//****************************************************************************
// Standard include files
//****************************************************************************

//****************************************************************************
// Project include files
//****************************************************************************
#include "lib_compress_defines.h"

//****************************************************************************
// extern Defines and enum typedef
//****************************************************************************
#define SRV_UNCOMPRESS_NB_MAX_SAMPLES   200000

//****************************************************************************
// extern Structures typedef
//****************************************************************************
typedef struct {
    uint32_t    nbSamples;
    record_t    samples[SRV_UNCOMPRESS_NB_MAX_SAMPLES];
} samples_t;

//****************************************************************************
// extern Variables
//****************************************************************************

//****************************************************************************
// extern Functions prototypes
//****************************************************************************

//****************************************************************************
/**
 * \brief Uncompress time/temperature samples compressed by lib_compress.
 * \param[in] buffer the buffer contains the compressed data.
 * \param[in] len len of data, in bytes.
 * \param[out] records a list to store uncompressed samples.
 * \retval len len of uncompressed data
 */
uint16_t uncompress_data(uint8_t *buffer,uint16_t size,record_t *records);
//****************************************************************************
/**
 * \brief Uncompress time/temperature samples compressed by lib_compress.
 * \param[in] buffer the buffer contains the compressed data.
 * \param[in] len len of data, in bytes.
 * \param[out] p_samples a struct where to store uncompressed samples.
 * \retval 1 on success, 0 on error
 * Warning: unreceived samples may generate many samples in a single frame (unreceived samples are highly compressed).
 */
uint8_t lib_uncompress_data(uint8_t *buffer, uint8_t len, samples_t *p_samples);

#endif // _LIB_UNCOMPRESS_H
