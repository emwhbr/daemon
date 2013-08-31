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

#ifndef __BASICD_CYCLIC_THREAD_H__
#define __BASICD_CYCLIC_THREAD_H__

#include "cyclic_thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class basicd_cyclic_thread : public cyclic_thread {

 public:
  basicd_cyclic_thread(string thread_name,
		       double frequency);
  ~basicd_cyclic_thread(void);

 protected:
  virtual long setup(void);   // Implements pure virtual function from base class
  virtual long cleanup(void); // Implements pure virtual function from base class

  virtual long cyclic_execute(void); // Implements pure virtual function from base class
    
 private:
  void init_members(void);
};

#endif // __BASICD_CYCLIC_THREAD_H__
