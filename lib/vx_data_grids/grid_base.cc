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
#include <string.h>
#include <cmath>

#include <vx_data_grids/grid_base.h>

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GridInterface
   //


////////////////////////////////////////////////////////////////////////


GridInterface::~GridInterface()

{

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GridRep
   //


////////////////////////////////////////////////////////////////////////


GridRep::GridRep()

{

refCount = 0;

}


////////////////////////////////////////////////////////////////////////


GridRep::~GridRep() { }


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Grid
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid()

{

rep = (GridRep *) 0;

}


////////////////////////////////////////////////////////////////////////


Grid::~Grid()

{

detach();

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const Grid &g)

{

rep = (GridRep *) 0;

attach(g.rep);

}


////////////////////////////////////////////////////////////////////////


Grid & Grid::operator=(const Grid &g)

{

if ( this == &g )  return ( *this );

attach(g.rep);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void Grid::detach()

{

if ( !rep )  return;

if ( --(rep->refCount) <= 0 )  { delete rep;  rep = (GridRep *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::attach(GridRep *r)

{

detach();

rep = r;

if ( rep )  ++(rep->refCount);

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::latlon_to_xy(double lat, double lon, double &x, double &y) const

{

if ( !rep )  { x = y = 0.0;  return; }

rep->latlon_to_xy(lat, lon, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::xy_to_latlon(double x, double y, double &lat, double &lon) const

{

if ( !rep )  { lat = lon = 0.0;  return; }

rep->xy_to_latlon(x, y, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


double Grid::calc_area(int x, int y) const

{

if ( !rep )  return ( 0.0 );

return ( rep->calc_area(x, y) );

}


////////////////////////////////////////////////////////////////////////


double Grid::calc_area_ll(int x, int y) const

{

if ( !rep )  return ( 0.0 );

return ( rep->calc_area_ll(x, y) );

}


////////////////////////////////////////////////////////////////////////


int Grid::nx() const

{

if ( !rep )  return ( 0 );

return ( rep->nx() );

}


////////////////////////////////////////////////////////////////////////


int Grid::ny() const

{

if ( !rep )  return ( 0 );

return ( rep->ny() );

}


////////////////////////////////////////////////////////////////////////


double Grid::EarthRadiusKM() const

{

if ( !rep )  return ( 0.0 );

return ( rep->EarthRadiusKM() );

}


////////////////////////////////////////////////////////////////////////


const char * Grid::name() const

{

if ( !rep )  return ( "(no name)" );

return ( rep->name() );

}


////////////////////////////////////////////////////////////////////////


ProjType Grid::proj_type() const

{

if ( !rep )  return ( NoProj );

return ( rep->proj_type() );

}

////////////////////////////////////////////////////////////////////////


double Grid::rot_grid_to_earth(int x, int y) const

{

if ( !rep )  return ( 0.0 );

return ( rep->rot_grid_to_earth(x, y) );

}


////////////////////////////////////////////////////////////////////////


void Grid::grid_data(GridData &gdata) const
{

if ( !rep )  return;

return ( rep->grid_data(gdata) );

}

////////////////////////////////////////////////////////////////////////
