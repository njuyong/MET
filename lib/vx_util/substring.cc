// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util/substring.h"


////////////////////////////////////////////////////////////////////////


void substring(const char * text, char * out, int first, int last)

{

if ( first > last )  {

   cerr << "\n\n  substring() -> bad input values!\n\n";

   exit ( 1 );

}

int j, n;

n = last - first + 1;

for (j=0; j<n; ++j)  {

   out[j] = text[first + j];

}


out[n] = (char) 0;


return;

}


////////////////////////////////////////////////////////////////////////


