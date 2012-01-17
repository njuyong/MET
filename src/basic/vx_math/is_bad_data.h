// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __IS_BAD_DATA_H__
#define  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////


#include <cmath>

#include "math_constants.h"


////////////////////////////////////////////////////////////////////////


inline int is_bad_data(int a)  {
   if(a == bad_data_int) return(1);
   else                  return(0);
}

inline int is_bad_data(double a) {
   if(abs((long double) (a - bad_data_double)) < default_tol) return(1);
   else                                                       return(0);
}

inline int is_bad_data(float a) {
   if(abs((long double) (a - bad_data_float)) < default_tol) return(1);
   else                                                      return(0);
}

inline int is_eq(double a, double b, double tol) {
   if(abs((long double) (a - b)) < tol) return(1);
   else                                 return(0);
}

inline int is_eq(double a, double b) {
   return(is_eq(a, b, default_tol));
}

////////////////////////////////////////////////////////////////////////


#endif   //  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////




