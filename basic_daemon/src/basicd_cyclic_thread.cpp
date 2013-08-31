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

#include "basicd_cyclic_thread.h"
#include "basicd_log.h"
#include "daemon_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

basicd_cyclic_thread::basicd_cyclic_thread(string thread_name,
					   double frequency) : cyclic_thread(thread_name,
									     frequency)
{
  init_members();
}

////////////////////////////////////////////////////////////////

basicd_cyclic_thread::~basicd_cyclic_thread(void)
{
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long basicd_cyclic_thread::setup(void)
{
  init_members();

  basicd_log_writeln(get_name() + " : setup");

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long basicd_cyclic_thread::cleanup(void)
{
  basicd_log_writeln(get_name() + " : cleanup");

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long basicd_cyclic_thread::cyclic_execute(void)
{
  basicd_log_writeln(get_name() + " : cyclic_execute");

  // return THREAD_INTERNAL_ERROR to signal error

  return THREAD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void basicd_cyclic_thread::init_members(void)
{
}
