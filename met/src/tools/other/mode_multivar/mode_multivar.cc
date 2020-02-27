

////////////////////////////////////////////////////////////////////////


// static const char program [] = "(#1 || #2) && (!#3)";
static const char program [] = "#1 && #2 && #3";

static const char atts_out_filename [] = "atts.out";


////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "two_d_array.h"
#include "get_filenames.h"
#include "shapedata.h"
#include "interest.h"

#include "combine_boolplanes.h"
#include "objects_from_netcdf.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static const char mode_path [] = "/d3/personal/randy/github/feature_1184_dryline/MET/met/src/tools/core/mode/mode";   //  hardwired for now

static const char sep [] = "====================================================\n";

static const char tab [] = "   ";

static const bool do_clusters = false;


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


static void get_filename_list(const char * fof_name, StringArray &);

static void write(ostream &, const SingleFeature &);
static void write(ostream &, const   PairFeature &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc < 4 )  usage();

int j, k;
StringArray fcst_filenames;
StringArray  obs_filenames;
const char * const    fcst_fof = argv[1];
const char * const     obs_fof = argv[2];
const char * const config_file = argv[3];


get_filename_list(fcst_fof, fcst_filenames);
get_filename_list( obs_fof,  obs_filenames);


if ( fcst_filenames.n() != obs_filenames.n() )  {

   mlog << Error
        << "\n\n  " << program_name 
        << ": number of fcst and obs files should be the same!\n\n";

   exit ( 1 );

}

const int n_files = fcst_filenames.n();
int status;
ConcatString mode_args;
ConcatString command;
StringArray a, nc_files;
char dir [32];

mlog << Debug(1) << sep;

for (j=0; j<n_files; ++j)  {

   command << cs_erase
           << mode_path << ' '
           << fcst_filenames[j] << ' '
           <<  obs_filenames[j] << ' '
           << config_file;

   for (k=4; k<argc; ++k)  {

      command << ' ' << argv[k];

   }

   snprintf(dir, sizeof(dir), "%02d", j);

   command << " -outdir " << dir;

   command << " -field_index " << j;

   // cout << command.text() << '\n' << flush;

   status = system(command.text());

   if ( status )  {

      mlog << Error
           << "\n\n  " << program_name << ": "
           << "mode run failed!\n\n";

      exit ( 1 );

   }

   mlog << Debug(1) << sep;

   a = get_filenames_from_dir(dir, "mode_", ".nc");

   nc_files.add(a);

}   //  for j


// nc_files.dump(cout, 1);

BoolPlane * f_plane = new BoolPlane [n_files];
BoolPlane * o_plane = new BoolPlane [n_files];
BoolPlane f_result, o_result;
// char junk[256];
Pgm image;


for (j=0; j<n_files; ++j)  {

   objects_from_netcdf(nc_files[j].c_str(), do_clusters, f_plane[j], o_plane[j]);  

/*
   snprintf(junk, sizeof(junk), "f_%02d.pgm", j);

   boolplane_to_pgm(f_plane[j], image);

   image.write(junk);
  
   snprintf(junk, sizeof(junk), "o_%02d.pgm", j);

   boolplane_to_pgm(o_plane[j], image);

   image.write(junk);
*/

}


BoolCalc calc;
const int nx = f_plane[0].nx();
const int ny = f_plane[0].ny();

calc.set(program);

// calc.dump_program(cout);

f_result.set_size(nx, ny);
o_result.set_size(nx, ny);

combine_boolplanes(f_plane, n_files, calc, f_result);
combine_boolplanes(o_plane, n_files, calc, o_result);

boolplane_to_pgm(f_result, image);

image.write("f_result.pgm");

boolplane_to_pgm(o_result, image);

image.write("o_result.pgm");


   //
   //  bool plane -> data plane & shape data
   //


ShapeData f_shape, o_shape;

int x, y;
double value;
DataPlane & f_dp = f_shape.data;
DataPlane & o_dp = o_shape.data;

f_dp.set_size(nx, ny);
o_dp.set_size(nx, ny);

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      value = ( f_result(x, y) ? 1.0 : 0.0 );

      f_dp.set(value, x, y);

      value = ( o_result(x, y) ? 1.0 : 0.0 );

      o_dp.set(value, x, y);

   }   //  for ny

}   //  for nx

f_shape.calc_moments();
o_shape.calc_moments();

   //
   //  shape data -> single feature & pair feature
   //

SingleFeature f_single, o_single;
PairFeature pair;

      //
      //  We're not intrested in data values inside
      //    the super-objects, so we'll just pass
      //    the same shapedata for raw, thresh
      //    and mask
      //

const int perc                 =   50;    //  doesn't matter
const bool precip_flag         = true;    //  shouldn't matter
const double max_centroid_dist = 1.0e9;   //  want this to be large

f_single.set(f_shape, f_shape, f_shape, perc, precip_flag);
o_single.set(o_shape, o_shape, o_shape, perc, precip_flag);

pair.set(f_single, o_single, max_centroid_dist);

   //
   //  write attributes
   //

ofstream out;

out.open(atts_out_filename);

if ( ! out )  {

   mlog << Error
        << "\n\n  " << program_name << ": can't open attributes output file\n\n";

   exit ( 1 );

}

out << '\n'
    << "fcst single\n"
    << "===========\n";

write(out, f_single);

out << '\n'
    << "obs single\n"
    << "==========\n";

write(out, o_single);

out << '\n'
    << "pair\n"
    << "====\n";

write(out, pair);


out.close();

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

mlog << Error
     << "\n\n   usage:  " 
     << program_name
     << " fcst obs config "
     << "[ mode_options ]\n\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void get_filename_list(const char * fof_name, StringArray & a)

{

ifstream in;
char line[PATH_MAX + 10];

in.open(fof_name);

if ( ! in )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_filename_list() -> "
        << "unable to open input file \"" << fof_name << "\"\n\n";

   exit ( 1 );

}

while ( in.getline(line, sizeof(line)) )  {

   a.add((string) line);

}




return;

}


////////////////////////////////////////////////////////////////////////


void write(ostream & out, const SingleFeature & s)

{

out << "centroid_x"   << tab << s.centroid_x   << '\n';
out << "centroid_y"   << tab << s.centroid_y   << '\n';
out << "axis_ang"     << tab << s.axis_ang     << '\n';
out << "length"       << tab << s.length       << '\n';
out << "width"        << tab << s.width        << '\n';
out << "area"         << tab << s.area         << '\n';
out << "aspect_ratio" << tab << s.aspect_ratio << '\n';
out << "complexity"   << tab << s.complexity   << '\n';



// out << "curvature"    << tab << s.curvature    << '\n';
// out << "curvature_x"  << tab << s.curvature_x  << '\n';
// out << "curvature_y"  << tab << s.curvature_y  << '\n';


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void write(ostream & out, const PairFeature & p)

{

out << "centroid_dist"              << tab << p.centroid_dist              << '\n';
out << "boundary_dist"              << tab << p.boundary_dist              << '\n';
out << "convex_hull_dist"           << tab << p.convex_hull_dist           << '\n';
out << "angle_diff"                 << tab << p.angle_diff                 << '\n';
out << "aspect_diff"                << tab << p.aspect_diff                << '\n';
out << "area_ratio"                 << tab << p.area_ratio                 << '\n';
out << "intersection_area"          << tab << p.intersection_area          << '\n';
out << "union_area"                 << tab << p.union_area                 << '\n';
out << "symmetric_diff"             << tab << p.symmetric_diff             << '\n';
out << "intersection_over_area"     << tab << p.intersection_over_area     << '\n';
out << "curvature_ratio"            << tab << p.curvature_ratio            << '\n';
out << "complexity_ratio"           << tab << p.complexity_ratio           << '\n';


// out << "percentile_intensity_ratio" << tab << p.percentile_intensity_ratio << '\n';

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


