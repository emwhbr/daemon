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

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <memory>

#include "basicd_log.h"
#include "basicd.h"
#include "excep.h"

basicd_log* basicd_log::m_instance = NULL;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

basicd_log::~basicd_log(void)
{
  pthread_mutex_destroy(&m_write_mutex);
}

////////////////////////////////////////////////////////////////

basicd_log* basicd_log::instance(void)
{
  if (!m_instance) {
    m_instance = new basicd_log;
    static auto_ptr<basicd_log> m_auto = auto_ptr<basicd_log>(m_instance);
  }
  return m_instance;
}

////////////////////////////////////////////////////////////////

void basicd_log::initialize(string logfile)
{
  int rc;

  m_logfile = logfile;

  // Open logfile
  rc = open(m_logfile.c_str(), 
	    O_WRONLY | O_CREAT,
	    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  if (rc == -1) {
    THROW_EXP(BASICD_LINUX_ERROR, BASICD_FILE_OPERATION_FAILED,
	      "open failed, logfile (%s)", m_logfile.c_str());
  }
  m_fd = rc;

  // Move to end of file
  if ( lseek(m_fd, 0, SEEK_END) == -1 ) {
    THROW_EXP(BASICD_LINUX_ERROR, BASICD_FILE_OPERATION_FAILED,
	      "lseek failed, logfile (%s)", m_logfile.c_str());
  }  
}

////////////////////////////////////////////////////////////////

void basicd_log::finalize(void)
{
  int rc;

  // Close logfile
  rc = close(m_fd);
  if (rc == -1) {
    THROW_EXP(BASICD_LINUX_ERROR, BASICD_FILE_OPERATION_FAILED,
	      "close failed, logfile (%s)", m_logfile.c_str());
  }
}

////////////////////////////////////////////////////////////////

void basicd_log::writeln(string str)
{
  try {
    // Lockdown write operation
    pthread_mutex_lock(&m_write_mutex);

    // Decorate message with date and time prefix
    char prefix[40];
    get_date_time_prefix(prefix, sizeof(prefix));

    // Add newline
    string the_message = prefix + str + "\n";

    // Write message to file
    write_all(m_fd,
	      (uint8_t *)the_message.c_str(),
	      the_message.length());

    // Lockup write operation
    pthread_mutex_unlock(&m_write_mutex);
  }
  catch (...) {
    pthread_mutex_unlock(&m_write_mutex);
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

basicd_log::basicd_log(void)
{
  m_logfile = "";
  m_fd      = -1;

  pthread_mutex_init(&m_write_mutex, NULL); // Use default mutex attributes
}

////////////////////////////////////////////////////////////////

void basicd_log::get_date_time_prefix(char *buffer, unsigned len)
{
  time_t    now = time(NULL);
  struct tm *tstruct;

  // Get broken down time
  tstruct = localtime(&now);
  if ( tstruct == NULL ) {
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIME_ERROR,
	      "localtime failed");
  }

  // Current date/time, format is YYYY-MM-DD.HH:mm:ss
  if ( strftime(buffer,
		len,
		"[%Y-%m-%d.%X] ",
		tstruct) == 0 ) {
    
    THROW_EXP(BASICD_INTERNAL_ERROR, BASICD_TIME_ERROR,
	      "strftime failed");
  }
}

////////////////////////////////////////////////////////////////

void basicd_log::write_all(int fd,
			   const uint8_t *data,
			   unsigned nbytes)
{
  unsigned total = 0;           // How many bytes written
  unsigned bytes_left = nbytes; // How many bytes left to write
  int n = 0;

  while (total < nbytes) {
    n = write(fd, data+total, bytes_left);
    if (n == -1) {
      THROW_EXP(BASICD_LINUX_ERROR, BASICD_FILE_OPERATION_FAILED,
		"write failed, logfile (%s), bytes (%u) bytes left (%u)",
		m_logfile.c_str(), nbytes, bytes_left);
    }
    total += n;
    bytes_left -= n;
  }
}
