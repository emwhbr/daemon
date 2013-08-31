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

#include "basicd.h"
#include "basicd_core.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static basicd_core g_object;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long basicd_get_prod_info(BASICD_PROD_INFO *prod_info)
{
  return g_object.get_prod_info(prod_info);
}

////////////////////////////////////////////////////////////////

long basicd_get_config(BASICD_CONFIG *config)
{
  return g_object.get_config(config);
}

////////////////////////////////////////////////////////////////

long basicd_get_last_error(BASICD_STATUS *status)
{
  return g_object.get_last_error(status);
}

////////////////////////////////////////////////////////////////

long basicd_check_run_status(void)
{
  return g_object.check_run_status();
}

////////////////////////////////////////////////////////////////

long basicd_initialize(const char *logfile,
		       double worker_thread_frequency)
{
  return g_object.initialize(logfile,
			     worker_thread_frequency);
}

////////////////////////////////////////////////////////////////

long basicd_finalize(void)
{
  return g_object.finalize();
}
