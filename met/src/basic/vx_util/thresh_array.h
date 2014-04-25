// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __THRESH_ARRAY_H__
#define  __THRESH_ARRAY_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "concat_string.h"
#include "threshold.h"

////////////////////////////////////////////////////////////////////////

class ThreshArray {

   public:

      void init_from_scratch();
      void assign(const ThreshArray &);

      SingleThresh *t;

      int Nelements;
      int Nalloc;

   public:

      ThreshArray();
     ~ThreshArray();
      ThreshArray(const ThreshArray &);
      ThreshArray & operator=(const ThreshArray &);

      void clear();

      void extend(int);

      void dump(ostream &, int depth = 0) const;

      SingleThresh operator[](int) const;

      void add(const SingleThresh &);
      void add(const double, const ThreshType);
      void add(const char *);
      void add(const ThreshArray &);
      void add_css(const char *);

      void parse_thresh_str(const char *);

      int n_elements() const;

      int has(const SingleThresh &) const;
      int has(const SingleThresh &, int & index) const;
      
      ConcatString get_str(const char *, int precision = thresh_default_precision) const;
      void         get_str(const char *, char *, int precision = thresh_default_precision) const;
      ConcatString get_abbr_str(const char *, int precision = thresh_default_precision) const;
      void         get_abbr_str(const char *, char *, int precision = thresh_default_precision) const;

      int  check(double) const;
};

////////////////////////////////////////////////////////////////////////

inline int ThreshArray::n_elements() const { return ( Nelements ); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __THRESH_ARRAY_H__  */

////////////////////////////////////////////////////////////////////////
