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

#include "basicd_cfg_file.h"
#include "basicd.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Valid item names
#define DAEMONIZE          "daemonize"
#define USER_NAME          "user_name"
#define WORK_DIR           "work_dir"
#define LOCK_FILE          "lock_file"
#define LOG_FILE           "log_file"
#define SUPERVISION_FREQ   "supervision_freq"
#define WORKER_THREAD_FREQ "worker_thread_freq"

// Default configuration values
#define DEF_DAEMONIZE           true
#define DEF_USER                "root"
#define DEF_WORK_DIR            "/"
#define DEF_LOCK_FILE           "/var/run/"BASICD_NAME".pid"
#define DEF_LOG_FILE            "/var/log/"BASICD_NAME".log"
#define DEF_SUPERVISION_FREQ    1.0 // Hz
#define DEF_WORKER_THREAD_FREQ  0.2 // Hz

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

basicd_cfg_file::basicd_cfg_file(string file_name) : cfg_file(file_name)
{
  // Define default values for all items that
  // can be found in a config file
  // Note!
  // - Using anything but 'dec' on double will lead to error
  // - Specifier on string is not used and has no effect.
  set_default_item_value(DAEMONIZE, bool(DEF_DAEMONIZE), boolalpha);
  set_default_item_value(USER_NAME, string(DEF_USER),      left);
  set_default_item_value(WORK_DIR,  string(DEF_WORK_DIR),  left);
  set_default_item_value(LOCK_FILE, string(DEF_LOCK_FILE), left);
  set_default_item_value(LOG_FILE,  string(DEF_LOG_FILE),  left);
  set_default_item_value(SUPERVISION_FREQ,   double(DEF_SUPERVISION_FREQ),   dec);
  set_default_item_value(WORKER_THREAD_FREQ, double(DEF_WORKER_THREAD_FREQ), dec);

  /*
    Example on how to use hex/dec integers   
  set_default_item_value("int_test_hex", int(0), hex);
  set_default_item_value("int_test_dec", int(0), dec);
  */
}

////////////////////////////////////////////////////////////////

basicd_cfg_file::~basicd_cfg_file(void)
{
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_daemonize(bool &value)
{
  return get_item_value(DAEMONIZE, value);
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_user_name(string &value)
{
  return get_item_value(USER_NAME, value);
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_work_dir(string &value)
{
  return get_item_value(WORK_DIR, value);
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_lock_file(string &value)
{
  return get_item_value(LOCK_FILE, value);
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_log_file(string &value)
{
  return get_item_value(LOG_FILE, value);
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_supervision_freq(double &value)
{
  return get_item_value(SUPERVISION_FREQ, value);
}

////////////////////////////////////////////////////////////////

long basicd_cfg_file::get_worker_thread_freq(double &value)
{
  return get_item_value(WORKER_THREAD_FREQ, value);
}
