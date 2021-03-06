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

#ifndef __BASICD_CFG_FILE_H__
#define __BASICD_CFG_FILE_H__

#include "cfg_file.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class basicd_cfg_file : public cfg_file {

 public:
  basicd_cfg_file(string file_name);
  ~basicd_cfg_file(void);

  long get_daemonize(bool &value);
  long get_user_name(string &value);
  long get_work_dir(string &value);
  long get_lock_file(string &value);
  long get_log_file(string &value);
  long get_supervision_freq(double &value);
  long get_worker_thread_freq(double &value);
};

#endif // __BASICD_CFG_FILE_H__
