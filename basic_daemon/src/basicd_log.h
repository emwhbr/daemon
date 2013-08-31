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

#ifndef __BASICD_LOG_H__
#define __BASICD_LOG_H__

#include <pthread.h>
#include <stdint.h>
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define basicd_log_initialize basicd_log::instance()->initialize
#define basicd_log_finalize   basicd_log::instance()->finalize
#define basicd_log_writeln    basicd_log::instance()->writeln

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class basicd_log {

 public:
  ~basicd_log(void);
  static basicd_log* instance(void);

  void initialize(string logfile);
  void finalize(void);

  void writeln(string str);

 private:
  static basicd_log *m_instance;
  string            m_logfile;
  int               m_fd;
  pthread_mutex_t   m_write_mutex;

  basicd_log(void); // Private constructor
                    // so it can't be called

  void get_date_time_prefix(char *buffer, unsigned len);

  void write_all(int fd,
		 const uint8_t *data,
		 unsigned nbytes);
};

#endif // __BASICD_LOG_H__
