// ************************************************************************
// *                                                                      *
// * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>

#include "basicd_core.h"
#include "basicd_log.h"
#include "basicd_cfg_file.h"
#include "daemon_utility.h"
#include "timer.h"
#include "delay.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PRODUCT_NUMBER   "BASICD"
#define RSTATE           "R1A01"

#ifndef BASICD_CFG_FILE
#define CFG_FILE "/tmp/"BASICD_NAME".cfg"
#endif

#define WORKER_THREAD_NAME           "BASICD_WT"
#define WORKER_THREAD_START_TIMEOUT    1.0 // Seconds
#define WORKER_THREAD_EXECUTE_TIMEOUT  0.5 // Seconds

#define MUTEX_LOCK(mutex) \
  ({ if (pthread_mutex_lock(&mutex)) { \
      return BASICD_MUTEX_FAILURE; \
    } })

#define MUTEX_UNLOCK(mutex) \
  ({ if (pthread_mutex_unlock(&mutex)) { \
      return BASICD_MUTEX_FAILURE; \
    } })

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

basicd_core::basicd_core(void)
{
  m_error_source    = BASICD_INTERNAL_ERROR;
  m_error_code      = BASICD_NO_ERROR;
  m_last_error_read = true;
  pthread_mutex_init(&m_error_mutex, NULL); // Use default mutex attributes

  m_initialized = false;
  pthread_mutex_init(&m_init_mutex, NULL); // Use default mutex attributes
}

/////////////////////////////////////////////////////////////////////////////

basicd_core::~basicd_core(void)
{
  pthread_mutex_destroy(&m_error_mutex);
  pthread_mutex_destroy(&m_init_mutex);
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::get_prod_info(BASICD_PROD_INFO *prod_info)
{
  try {
    // Do the actual work
    return internal_get_prod_info(prod_info);
  }
  catch (...) {
    return set_error(EXP(BASICD_INTERNAL_ERROR, BASICD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::get_config(BASICD_CONFIG *config)
{
  try {
    // Do the actual work
    return internal_get_config(config);
  }
  catch (excep &exp) {
    return set_error(exp);
  }
  catch (...) {
    return set_error(EXP(BASICD_INTERNAL_ERROR, BASICD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::get_last_error(BASICD_STATUS *status)
{
  try {
    MUTEX_LOCK(m_error_mutex);
    status->error_source = m_error_source;
    status->error_code   = m_error_code;
    
    // Clear internal error information
    m_error_source    = BASICD_INTERNAL_ERROR;
    m_error_code      = BASICD_NO_ERROR;
    m_last_error_read = true;
    MUTEX_UNLOCK(m_error_mutex);
    return BASICD_SUCCESS;
  }
  catch (...) {
    return set_error(EXP(BASICD_INTERNAL_ERROR, BASICD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::check_run_status(void)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    internal_check_run_status();

    // Check completed
    MUTEX_UNLOCK(m_init_mutex);

    return BASICD_SUCCESS;
  }
  catch (excep &exp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(exp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(EXP(BASICD_INTERNAL_ERROR, BASICD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::initialize(string logfile,
			     double worker_thread_frequency)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if already initialized
    if (m_initialized) {
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_ALREADY_INITIALIZED,
		"Already initialized");
    }

    // Check input values
    if (worker_thread_frequency < 0.0) {
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_BAD_ARGUMENT,
		"Illegal worker thread frequency (%f)",
		worker_thread_frequency);
    }

    // Do the actual initialization
    internal_initialize(logfile,
			worker_thread_frequency);

    // Initialization completed
    m_initialized = true;
    MUTEX_UNLOCK(m_init_mutex);

    return BASICD_SUCCESS;
  }
  catch (excep &exp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(exp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(EXP(BASICD_INTERNAL_ERROR, BASICD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::finalize(void)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual finalization
    internal_finalize();

    // Finalization completed
    m_initialized = false;
    MUTEX_UNLOCK(m_init_mutex);

    return BASICD_SUCCESS;
  }
  catch (excep &exp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(exp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(EXP(BASICD_INTERNAL_ERROR, BASICD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

long basicd_core::set_error(excep exp)
{
  // Get the stack trace
  STACK_FRAMES frames;
  exp.get_stack_frames(frames);

  ostringstream oss_msg;
  char buffer[18];

  // Note!!
  // To avoid problems with syslog multiline-messages (embedded '\n')
  // we use '\\n' as a newline separator.
  // This message can be decoded as a multiline message by examine
  // the error log using sed-command:
  // tail -f /var/log/errors.log | sed 's/\\n/\n/g'

  oss_msg << "\\n";
  oss_msg << "\tstack frames:" << (int) frames.active_frames << "\\n";

  for (unsigned i=0; i < frames.active_frames; i++) {
    sprintf(buffer, "0x%016lx", frames.frames[i]);
    oss_msg << "\tframe:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\\n";
  }

  // Get info from predefined macros
  oss_msg << "\tViolator: " << exp.get_file() 
	  << ":" << exp.get_line()
	  << ", " << exp.get_function() << "\\n";

  // Get the internal info
  oss_msg << "\tSource: " << exp.get_source()
	  << ", Code: " << exp.get_code() << "\\n";

  oss_msg << "\tInfo: " << exp.get_info() << "\\n";

  // Source of error (last multi-line, terminate with '\n')
  switch (exp.get_source()) {
  case BASICD_INTERNAL_ERROR:
    oss_msg << "\tBASICD INTERNAL ERROR\n";
    break;
  case BASICD_LINUX_ERROR:
    oss_msg << "\tBASICD LINUX ERROR - errno:" 
	    << errno << " => " << strerror(errno) << "\n";
    break;
  }
  
  // Print all info
  syslog_error(oss_msg.str().c_str());

  // Update internal error information
  return update_error(exp);
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::update_error(excep exp)
{
  MUTEX_LOCK(m_error_mutex);
  if (m_last_error_read) {
    m_error_source    = (BASICD_ERROR_SOURCE)exp.get_source();
    m_error_code      = exp.get_code();
    m_last_error_read = false; // Latch last error until read
  }
  MUTEX_UNLOCK(m_error_mutex);

  return BASICD_FAILURE;
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::internal_get_prod_info(BASICD_PROD_INFO *prod_info)
{
  long rc = BASICD_SUCCESS;
 
  strncpy(prod_info->prod_num, 
	  PRODUCT_NUMBER, 
	  sizeof(((BASICD_PROD_INFO *)0)->prod_num));

  strncpy(prod_info->rstate, 
	  RSTATE, 
	  sizeof(((BASICD_PROD_INFO *)0)->rstate));

  return rc;
}

/////////////////////////////////////////////////////////////////////////////

long basicd_core::internal_get_config(BASICD_CONFIG *config)
{
  long rc;
  basicd_cfg_file *cfg_f = new basicd_cfg_file(CFG_FILE);

  // Parse configuration file
  rc = cfg_f->parse();
  if ( (rc != CFG_FILE_SUCCESS) &&         // Use values from file
       (rc != CFG_FILE_FILE_NOT_FOUND) ) { // Use default values
    delete cfg_f;
    switch(rc) {
    case CFG_FILE_FILE_IO_ERROR:
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_FILE_OPERATION_FAILED,
		"I/O error parsing config file %s",
		CFG_FILE);
      break;
    case CFG_FILE_BAD_FILE_FORMAT:
    case CFG_FILE_BAD_VALUE_FORMAT:
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_BAD_FORMAT,
		"Bad format parsing config file %s",
		CFG_FILE);
      break;
    default:
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
		"Unexpected error(%ld) parsing config file %s",
		rc, CFG_FILE);
    }
  }

  // Get configuration values
  bool daemonize;
  rc = cfg_f->get_daemonize(daemonize);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_daemonize", rc);
  }
  string user;
  rc = cfg_f->get_user_name(user);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_user_name", rc);
  }
  string work_dir;
  rc = cfg_f->get_work_dir(work_dir);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_work_dir", rc);
  }
  string lock_file;
  rc = cfg_f->get_lock_file(lock_file);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_lock_file", rc);
  }
  string log_file;
  rc = cfg_f->get_log_file(log_file);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_log_file", rc);
  }
  double s_freq;
  rc = cfg_f->get_supervision_freq(s_freq);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_supervison_freq", rc);
  }
  double wt_freq;
  rc = cfg_f->get_worker_thread_freq(wt_freq);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_worker_thread_freq", rc);
  }
  
  // Copy configuration values to caller
  config->daemonize = daemonize;
  strncpy(config->user,      user.c_str(),      sizeof(BASICD_STRING));
  strncpy(config->work_dir,  work_dir.c_str(),  sizeof(BASICD_STRING));
  strncpy(config->lock_file, lock_file.c_str(), sizeof(BASICD_STRING));
  strncpy(config->log_file,  log_file.c_str(),  sizeof(BASICD_STRING));
  config->supervision_freq   = s_freq;
  config->worker_thread_freq = wt_freq;
  
  delete cfg_f;

  return BASICD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

void basicd_core::internal_check_run_status(void)
{
  // Check state and status of cyclic worker thread object
  
  if ( m_worker_thread_auto->get_state() != THREAD_STATE_EXECUTING ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_STATUS_NOT_OK,
	      "Cyclic worker thread not executing, status:0x%x, state:%u",
	      m_worker_thread_auto->get_status(),
	      m_worker_thread_auto->get_state());
  }

  if ( m_worker_thread_auto->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_STATUS_NOT_OK,
	      "Cyclic worker thread status not OK, status:0x%x, state:%u",
	      m_worker_thread_auto->get_status(),
	      m_worker_thread_auto->get_state());
  }
}

/////////////////////////////////////////////////////////////////////////////

void basicd_core::internal_initialize(string logfile,
				      double worker_thread_frequency)
{
  // Initialize the logfile singleton object
  basicd_log_initialize(logfile);

  // Create the cyclic worker thread object with garbage collector
  basicd_cyclic_thread *thread_ptr = 
    new basicd_cyclic_thread(WORKER_THREAD_NAME,
			     worker_thread_frequency);

  m_worker_thread_auto = auto_ptr<basicd_cyclic_thread>(thread_ptr);

  ////////////////////////////////////////////
  // Initialize cyclic worker thread object
  ////////////////////////////////////////////

  basicd_log_writeln("++++++++ About to start cyclic worker thread");

  // Step 1: Start thread
  if ( m_worker_thread_auto->start(NULL) != THREAD_SUCCESS ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_OPERATION_FAILED,
	      "Error start cyclic worker thread");
  }

  // Step 2: Wait for thread to complete setup
  timer thread_timer;
  bool thread_timeout = true;
  if ( thread_timer.reset() != TIMER_SUCCESS ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIME_ERROR,
	      "Error resetting cyclic worker thread timeout timer");
  }
  while ( thread_timer.get_elapsed_time() < 
	  WORKER_THREAD_START_TIMEOUT ) {
    if ( m_worker_thread_auto->get_state() == THREAD_STATE_SETUP_DONE ) {
      thread_timeout = false;
      break;
    }
    if ( delay(0.1) != DELAY_SUCCESS ) {
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIME_ERROR,
	      "Delay operation failed");
    }
  }
  if ( thread_timeout ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIMEOUT_OCCURRED,
	      "Timeout waiting for cyclic worker thread setup");
  }
  if ( m_worker_thread_auto->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_STATUS_NOT_OK,
	      "Cyclic worker thread status not OK, status:0x%x, state:%u",
	      m_worker_thread_auto->get_status(),
	      m_worker_thread_auto->get_state());
  }

  // Step 3: Release thread
  if ( m_worker_thread_auto->release() != THREAD_SUCCESS ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_OPERATION_FAILED,
	      "Error release cyclic worker thread");
  }

  // step 4: Wait for thread to start executing
  thread_timeout = true;
  if ( thread_timer.reset() != TIMER_SUCCESS ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIME_ERROR,
	      "Error resetting cyclic worker thread timeout timer");
  }
  while ( thread_timer.get_elapsed_time() < 
	  WORKER_THREAD_EXECUTE_TIMEOUT ) {
    if ( m_worker_thread_auto->get_state() == THREAD_STATE_EXECUTING ) {
      thread_timeout = false;
      break;
    }
    if ( delay(0.1) != DELAY_SUCCESS ) {
      THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIME_ERROR,
	      "Delay operation failed");
    }
  }
  if ( thread_timeout ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIMEOUT_OCCURRED,
	      "Timeout waiting for cyclic worker thread setup");
  }
  if ( m_worker_thread_auto->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_STATUS_NOT_OK,
	      "Cyclic worker thread status not OK, status:0x%x, state:%u",
	      m_worker_thread_auto->get_status(),
	      m_worker_thread_auto->get_state());
  }
}

/////////////////////////////////////////////////////////////////////////////

void basicd_core::internal_finalize(void)
{  
  /////////////////////////////////////////////
  // Finalize the cyclic worker thread object
  /////////////////////////////////////////////
  
  // Step 1: Stop thread
  if ( m_worker_thread_auto->stop() != THREAD_SUCCESS ) {    
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_OPERATION_FAILED,
	      "Error stop cyclic worker thread, status:0x%x, state:%u",
	      m_worker_thread_auto->get_status(),
	      m_worker_thread_auto->get_state());
  }

  // Step 2: Wait for thread to complete
  //         We must wait at least thread's periode time
  //         Add one extra second to get a safety factor
  const double thread_done_timeout =
    ( 1.0 / m_worker_thread_auto->get_frequency() ) + 1.0;
  if ( m_worker_thread_auto->wait_timed(thread_done_timeout)
       != THREAD_SUCCESS ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_THREAD_OPERATION_FAILED,
	      "Error wait_timed cyclic worker thread, status:0x%x, state:%u",
	      m_worker_thread_auto->get_status(),
	      m_worker_thread_auto->get_state());
  }

  // Step 3: Delete the cyclic worker thread object
  m_worker_thread_auto.reset();

  // Finalize the logfile singleton object
  basicd_log_finalize();
}
