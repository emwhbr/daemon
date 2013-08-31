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

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <exception>

#include "basicd.h"
#include "daemon_utility.h"
#include "delay.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

static void daemon_terminate(void);
static void daemon_signal_handler(int sig);
static void daemon_exit_on_error(int fd_lock_file);
static void daemon_report_prod_info(void);
static int  daemon_get_config(BASICD_CONFIG *config);
static int  daemon_check_status(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( daemon_terminate );
  }
};
// Install terminate function (in case of emergency)
// Install as soon as possible, efore main starts
static the_terminate_handler g_terminate_handler;

static volatile sig_atomic_t g_received_sighup  = 0;
static volatile sig_atomic_t g_received_sigterm = 0;

static BASICD_CONFIG g_config;

////////////////////////////////////////////////////////////////

static void daemon_terminate(void)
{
  // Only log this event, no further actions for now
  syslog_error("Unhandled exception, termination handler activated");
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static void daemon_signal_handler(int sig)
{
  switch (sig) {
  case SIGHUP:
    g_received_sighup = 1;
    break;
  case SIGTERM:
    g_received_sigterm = 1;
    break;
  default:
    ;
  }
}

////////////////////////////////////////////////////////////////

static void daemon_exit_on_error(int fd_lock_file)
{
  syslog_info("Terminated bad");
  syslog_close();

  if (fd_lock_file != DAEMON_BAD_FD_LOCK_FILE) {
    close(fd_lock_file);
    unlink(g_config.lock_file);
  }

  exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////

static void daemon_report_prod_info(void)
{
  BASICD_PROD_INFO prod_info;
  basicd_get_prod_info(&prod_info);

  syslog_info("%s - %s",
	      prod_info.prod_num,
	      prod_info.rstate);
}

////////////////////////////////////////////////////////////////

static int daemon_get_config(BASICD_CONFIG *config)
{
  if (basicd_get_config(config) != BASICD_SUCCESS) {
    BASICD_STATUS status;
    basicd_get_last_error(&status);
    syslog_error("Configuration file error, source:%d, code:%ld\n",
		 status.error_source, status.error_code);
    return 0;
  }
  
  // Note!!
  // To avoid problems with syslog multiline-messages (embedded '\n')
  // we use '\\n' as a newline separator.
  // This message can be decoded as a multiline message by examine
  // the messages log using sed-command:
  // tail -f /var/log/messages.log | sed 's/\\n/\n/g'

  ostringstream oss_msg;
  oss_msg << "Configuration:" << "\\n";
  oss_msg << "\tdaemonize:" << config->daemonize << "\\n";
  oss_msg << "\tuser     :" << config->user << "\\n";
  oss_msg << "\twork_dir :" << config->work_dir << "\\n";
  oss_msg << "\tlock_file:" << config->lock_file  << "\\n";
  oss_msg << "\tlog_file :" << config->log_file  << "\\n";
  oss_msg << "\tsup_freq :" << config->supervision_freq << "\\n";
  oss_msg << "\twt_freq  :" << config->worker_thread_freq << "\n";

  // Print all info
  syslog_info(oss_msg.str().c_str());

  return 1;
}

////////////////////////////////////////////////////////////////

static int daemon_check_status(void)
{
  BASICD_STATUS status;

  // Check for errors in daemon
  if (basicd_get_last_error(&status) == BASICD_SUCCESS) {
    if (status.error_code != BASICD_NO_ERROR) {
      syslog_error("Daemon status not OK, source:%d, code:%ld\n",
		   status.error_source, status.error_code);
      return 0;
    }
  }
  else {
    syslog_error("Can't get daemon status");
    return 0;
  }

  // Force a check of daemon status, ignore result now
  // Any errors will be detected next time this function is called
  basicd_check_run_status();

  return 1;
}

////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  long rc;
  int fd_lock_file = DAEMON_BAD_FD_LOCK_FILE;

  // Initialize handling of messages sent to system logger
  syslog_open(BASICD_NAME);

  // Report product number and the RState
  daemon_report_prod_info();

  // Read configuration file
  if (!daemon_get_config(&g_config)) {
    daemon_exit_on_error(fd_lock_file);
  }

  // Define handler for SIGHUP to tell daemon to restart.
  // The daemon will reread its configuration file and
  // reopen its log file
  rc = define_signal_handler(SIGHUP, daemon_signal_handler);
  if (rc != DAEMON_SUCCESS) {
    daemon_exit_on_error(fd_lock_file);
  }

  // Define handler for SIGTERM to tell daemon to terminate.  
  rc = define_signal_handler(SIGTERM, daemon_signal_handler);
  if (rc != DAEMON_SUCCESS) {
    daemon_exit_on_error(fd_lock_file);
  }
  
  // Go through the steps in becoming a daemon (or not)
  if (g_config.daemonize) {
    rc = become_daemon(g_config.user,
		       g_config.work_dir,
		       g_config.lock_file,		     
		       &fd_lock_file);
    if (rc != DAEMON_SUCCESS) {
      daemon_exit_on_error(fd_lock_file);
    }
  }
  
  // We are now running as a daemon (or not)
  syslog_info("Started");

  // Initialize daemon
  if (basicd_initialize(g_config.log_file,
			g_config.worker_thread_freq) != BASICD_SUCCESS) {
    daemon_exit_on_error(fd_lock_file);
  }

  // Daemon main supervision and control loop
  for (;;) {
    // Check if time to restart
    if (g_received_sighup) {
      syslog_info("Got SIGHUP, restarting");
      g_received_sighup = 0;
      // Finalize
      if (basicd_finalize() != BASICD_SUCCESS) {
	daemon_exit_on_error(fd_lock_file);
      }
      // Read configuration file
      if (!daemon_get_config(&g_config)) {
	daemon_exit_on_error(fd_lock_file);
      }
      // Initialize
      if (basicd_initialize(g_config.log_file,
			    g_config.worker_thread_freq) != BASICD_SUCCESS) {
	daemon_exit_on_error(fd_lock_file);
      }
    }
    // Check if time to quit
    if (g_received_sigterm) {
      syslog_info("Got SIGTERM, terminating");
      g_received_sigterm = 0;
      if (basicd_finalize() != BASICD_SUCCESS) {
	daemon_exit_on_error(fd_lock_file);
      }
      break;
    }
    // Check daemon status
    if (!daemon_check_status()) {
      syslog_info("Daemon status not OK, terminating");
      basicd_finalize();
      daemon_exit_on_error(fd_lock_file);
    }
    // Take it easy
    if ( delay(1.0/g_config.supervision_freq) != DELAY_SUCCESS ) {
      syslog_info("Error when delay, terminating");
      basicd_finalize();
      daemon_exit_on_error(fd_lock_file);
    }
  }
  
  // Cleanup and exit
  syslog_info("Terminated ok");
  syslog_close();
  
  if (fd_lock_file != DAEMON_BAD_FD_LOCK_FILE) {
    close(fd_lock_file);
    unlink(g_config.lock_file);
  }

  exit(EXIT_SUCCESS);
}
