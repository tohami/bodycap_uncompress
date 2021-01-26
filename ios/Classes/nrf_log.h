/**
  ******************************************************************************
  * \file    	nrf_log.h
  * \author 	S.Lejczyk
  * \copyright	2020 BodyCap S.A.S.
  * \brief  	Remplace nrf_log.h en natif
  ******************************************************************************
  */
#undef NRF_LOG_MODULE_NAME
#ifndef _NRF_LOG_H
#define _NRF_LOG_H

#define PRINTF printf

#ifndef LOGLEVEL
#   define NRF_LOG_ERROR(...)
#   define NRF_LOG_WARNING(...)
#   define NRF_LOG_INFO(...)
#   define NRF_LOG_DEBUG(...)

#ifdef NRF_LOG_LEVEL
#define LOGLEVEL NRF_LOG_LEVEL
#else
#define LOGLEVEL LOGINFO
#endif
#else   // LOGLEVEL is defined

#   define NRF_LOG_ERROR(...) \
if (LOGLEVEL >= LOGERROR) {\
	print_level(LOGERROR,	__VA_ARGS__);\
}
#   define NRF_LOG_WARNING(...) \
if (LOGLEVEL >= LOGWARNING) {\
	print_level(LOGWARNING,	__VA_ARGS__);\
}
#   define NRF_LOG_INFO(...) \
if (LOGLEVEL >= LOGINFO) {\
	print_level(LOGINFO,	 __VA_ARGS__);\
}
#   define NRF_LOG_DEBUG(...) \
if (LOGLEVEL >= LOGDEBUG) {\
	print_level(LOGDEBUG,	__VA_ARGS__);\
}
/*#  define LOG(level, ...) \
if (LOGLEVEL >= level) {\
	print_level(level,	__VA_ARGS__);\
}*/

#define LOG_ERROR   NRF_LOG_ERROR
#define LOG_WARNING NRF_LOG_WARNING
#define LOG_INFO    NRF_LOG_INFO
#define LOG_DEBUG   NRF_LOG_DEBUG

#define DEBUG_MAX_STRING_SIZE 500

enum log_levels {
    LOGERROR   = 1, /**< Error which can stop the system */
    LOGWARNING = 2, /**< Anormal/strange behaviour message */
    LOGINFO    = 3, /**< High level information */
    LOGDEBUG   = 4  /**< Internal information for component debug */
};

void print_level(int level, const char *format, ...);
void log_displayBuffer(const char *s8p_mess, const uint8_t *u8p_buffer, int32_t s32_len);
void log_display_binary(uint8_t *buffer, uint16_t nbBits);

#endif
#define NRF_LOG_MODULE_REGISTER()
#define ASSERT  assert


#endif
