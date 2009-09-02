// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_PXM_UTILS_H__
#define  __VX_PXM_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


extern int   parse_number     (istream &);

extern void  skip_whitespace  (istream &);

extern void  get_comment      (istream &, char *);


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_PXM_UTILS_H__


////////////////////////////////////////////////////////////////////////


