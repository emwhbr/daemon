/************************************************************************
 *                                                                      *
 * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 ************************************************************************/

#ifndef __BASICD_H__
#define __BASICD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * BASICD daemon name
 */
#define BASICD_NAME "basicd"

/*
 * BASICD Return codes
 */
#define BASICD_SUCCESS         0
#define BASICD_FAILURE        -1
#define BASICD_MUTEX_FAILURE  -2

/*
 * BASICD Internal error codes
 */
#define BASICD_NO_ERROR                   0
#define BASICD_NOT_INITIALIZED            1
#define BASICD_ALREADY_INITIALIZED        2
#define BASICD_BAD_ARGUMENT               3
#define BASICD_CFG_FILE_BAD_FORMAT        4
#define BASICD_CFG_FILE_UNEXCPECTED_ERROR 5
#define BASICD_TIMEOUT_OCCURRED           6
#define BASICD_TIME_ERROR                 7
#define BASICD_FILE_OPERATION_FAILED      8
#define BASICD_THREAD_OPERATION_FAILED    9
#define BASICD_THREAD_STATUS_NOT_OK       10
#define BASICD_UNEXPECTED_EXCEPTION       11

/*
 * Error source values
 */
typedef enum {BASICD_INTERNAL_ERROR, 
	      BASICD_LINUX_ERROR} BASICD_ERROR_SOURCE;

/*
 * API types
 */
typedef char BASICD_STRING[256];

typedef struct {
  char prod_num[20];
  char rstate[10];
} BASICD_PROD_INFO;

typedef struct {
  BASICD_ERROR_SOURCE error_source;
  long                error_code;
} BASICD_STATUS;

typedef struct {
  bool          daemonize;
  BASICD_STRING user;
  BASICD_STRING work_dir;
  BASICD_STRING lock_file;
  BASICD_STRING log_file;
  double        supervision_freq;
  double        worker_thread_freq;
} BASICD_CONFIG;

/****************************************************************************
*
* Name basicd_prod_info
*
* Description Returns the product number and the RState of BASICD.
*
* Parameters prod_info  IN/OUT  Pointer to a buffer to hold the product
*                               number and the RState.
*
* Error handling Returns always BASICD_SUCCESS.
*
****************************************************************************/
extern long basicd_get_prod_info(BASICD_PROD_INFO *prod_info);

/****************************************************************************
*
* Name basicd_get_config
*
* Description Returns the configuration of BASICD.
*
* Parameters config  IN/OUT  Pointer to a buffer to hold the configuration
*
* Error handling Returns BASICD_SUCCESS if successful
*                otherwise BASICD_FAILURE or BASICD_MUTEX_FAILURE
*
****************************************************************************/
extern long basicd_get_config(BASICD_CONFIG *config);

/****************************************************************************
*
* Name basicd_get_last_error
*
* Description Returns the error information held by BASICD, when a call
*             returns unsuccessful completion. 
*             BASICD clears its internal error information after it has
*             been read by the calling application.
*
* Parameters status  IN/OUT  pointer to a buffer to hold the error information
*
* Error handling Returns BASICD_SUCCESS if successful
*                otherwise BASICD_FAILURE or BASICD_MUTEX_FAILURE
*
****************************************************************************/
extern long basicd_get_last_error(BASICD_STATUS *status);

/****************************************************************************
*
* Name basicd_check_run_status
*
* Description Check the running status of BASICD, including all threads.
*
* Parameters None
*
* Error handling Returns BASICD_SUCCESS if successful
*                otherwise BASICD_FAILURE or BASICD_MUTEX_FAILURE
*
****************************************************************************/
extern long basicd_check_run_status(void);

/****************************************************************************
*
* Name basicd_initialize
*
* Description Allocates system resources and performs operations that are
*             necessary to start BASICD.
*
* Parameters worker_thread_frequency  IN  Frequency (Hz) of worker thread
*
* Error handling Returns BASICD_SUCCESS if successful
*                otherwise BASICD_FAILURE or BASICD_MUTEX_FAILURE
*
****************************************************************************/
extern long basicd_initialize(const char *logfile,
			      double worker_thread_frequency);

/****************************************************************************
*
* Name basicd_finalize
*
* Description Releases any resources that were claimed during initialization.
*
* Parameters None 
*
* Error handling Returns BASICD_SUCCESS if successful
*                otherwise BASICD_FAILURE or BASICD_MUTEX_FAILURE
*
****************************************************************************/
extern long basicd_finalize(void);

#ifdef  __cplusplus
}
#endif

#endif /* __BASICD_H__ */
