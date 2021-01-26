/**
  ******************************************************************************
  * \file    	lib_bitStream.h
  * \author 	Sylvain Lejczyk
  * \copyright	2020 BodyCap S.A.S.
  * \brief  	This library provides some functions to handle bit streams.
  ******************************************************************************
  */

#ifndef _LIB_BITSTREAM_H
#define _LIB_BITSTREAM_H

//****************************************************************************
// Standard include files
//****************************************************************************
#include <stdint.h>

//****************************************************************************
// Project include files
//****************************************************************************

//****************************************************************************
// extern Defines and enum typedef
//****************************************************************************

//****************************************************************************
// extern Structures typedef
//****************************************************************************

// Generic bit stream stuct
typedef struct {
    uint16_t currentIdx;    // incremented as soon as data are added
    uint16_t confirmedIdx;  // incremented if we checked that there was enough space for all data in the buffer
    uint16_t dataSize;      // number of bits allocated in data field, filled by lib_bitStream_create.
    uint16_t nbSamples;     // number of samples successfully added within the bitStream, incremented when validating currentIdx to confirmedIdx
    uint8_t *data;          // contains the data, space varies, depending on usage, allocated by caller of lib_bitStream_create.
} def_bitStream_t;

//****************************************************************************
// extern Variables
//****************************************************************************

//****************************************************************************
// extern Functions prototypes 
//****************************************************************************

//****************************************************************************
/**
 * \brief   Initialize a bit stream.
 * \param[in] bs an allocated bitStream struct.
 * \param[in] dataBuffer the memory where the bit stream will be stored, allocated by caller.
 * \param[in] sizeBytes the space allocated by caller for the data field
 * \param[in] setStartBit set the start bit at the beginning of the buffer
 *
 * This function reset all data, and initialize an empty bit stream,
 * all data are set to 0 and the indexes are at the beginning.
 */
void lib_bitStream_create(def_bitStream_t *bs, uint8_t* dataBuffer, uint16_t sizeBytes, uint8_t setStartBit);

//****************************************************************************
/**
 * \brief   Set bits in a bit stream.
 * \param[in] bs the reference bitStream struct.
 * \param[in] value contains the bits to set in the stream, bit are right justified in the parameter
 *            (MSB bits are not used if nbBits<32)
 * \param[in] nbBits number of bit to use in the value, within 1 to 32.
 * \return  1 if the bits were successfully added, 0 otherwise (not enough remaining space).
 * This function was named "frames_set_bits" in P022.
 */
uint8_t lib_bitStream_set_bits(def_bitStream_t *bs, uint32_t value, uint8_t nbBits);

//****************************************************************************
/**
 * \brief   Validate the current data in the bit stream.
 * \param[in] bs the reference bitStream struct.
 * \param[in] nbSamplesAdded the number of samples validated
 * The previously added bits by lib_bitStream_set_bits are validated, the pointer confirmedIdx is moved to currentIdx.
 * The number of samples is increased by nbSamplesAdded.
 */
void lib_bitStream_validate(def_bitStream_t *bs, uint32_t nbSamplesAdded);

//****************************************************************************
/**
 * \brief   Add some bit stuffing in the last byte.
 * \param[in] bs the reference bitStream struct.
 * If the last byte is not full, fill it with bit set to 1, and adjust the confirmedIdx.
 * Otherwise the BS is unchanged.
 */
void lib_bitStream_completeLastByte(def_bitStream_t *bs);

//****************************************************************************
/**
 * \brief   Returns the current len of the bit stream, in bytes.
 * \param[in] bs the reference bitStream struct.
 * \return the confirmed len of the bit streams, rounded to the next byte.
 */
uint16_t lib_bitStream_get_len(def_bitStream_t *bs);

//****************************************************************************
/**
 * \brief   Define a bit stream from existing data.
 * \param[in] bs an allocated bitStream struct.
 * \param[in] dataBuffer the memory where the data are already stored.
 * \param[in] sizeBytes the space allocated by caller for the data field
 * The currentIdx is set to 0 so that the bit stream is ready to deliver data from lib_bitStream_get_bits.
 * This is used by lib_uncompress as an easy way to access data.
 */
void lib_bitStream_define(def_bitStream_t *bs, uint8_t* dataBuffer, uint16_t sizeBytes);

//****************************************************************************
/**
 * \brief Get the "nb" next bits from the bit stream.
 * \param[in] bs the reference bitStream struct.
 * \param[in] nb the number of bits to return. Should be <= 32 (the return value is an uint32_t).
 * \param[out] output the requested bits will be stored there.
 * \return 1 if there were enough data, 0 otherwise (not enough for the number of bits requested) or error in parameters.
 *           If return 1, then output is filled with the expected data
 */
uint8_t lib_bitStream_get_bits(def_bitStream_t *bs, uint8_t nb, uint32_t *output);

#endif // _LIB_BITSTREAM_H
