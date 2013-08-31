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

#ifndef __BASICD_CORE_H__
#define __BASICD_CORE_H__

#include <pthread.h>
#include <memory>

#include "basicd.h"
#include "basicd_cyclic_thread.h"
#include "excep.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class basicd_core {

public:
  basicd_core(void);
  ~basicd_core(void);

  long get_prod_info(BASICD_PROD_INFO *prod_info);

  long get_config(BASICD_CONFIG *config);

  long get_last_error(BASICD_STATUS *status);

  long check_run_status(void);

  long initialize(string logfile,
		  double worker_thread_frequency);

  long finalize(void);

private:
  // Error handling information
  BASICD_ERROR_SOURCE  m_error_source;
  long                 m_error_code;
  bool                 m_last_error_read;
  pthread_mutex_t      m_error_mutex;

  // Keep track of initialization
  bool             m_initialized;
  pthread_mutex_t  m_init_mutex;

  // The cyclic worker thread object
  auto_ptr<basicd_cyclic_thread> m_worker_thread_auto;

  // Private member functions
  long set_error(excep exp);
  long update_error(excep exp);

  long internal_get_prod_info(BASICD_PROD_INFO *prod_info);

  long internal_get_config(BASICD_CONFIG *config);

  void internal_check_run_status(void);

  void internal_initialize(string logfile,
			   double worker_thread_frequency);

  void internal_finalize(void);  
};

#endif // __BASICD_CORE_H__
