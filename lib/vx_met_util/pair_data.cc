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
#include <string.h>
#include <cmath>

#include "vx_met_util/pair_data.h"
#include "vx_met_util/compute_ci.h"
#include "vx_met_util/constants.h"
#include "vx_met_util/conversions.h"
#include "vx_util/vx_util.h"
#include "vx_grib_classes/grib_strings.h"
#include "vx_wrfdata/vx_wrfdata.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class GCInfo
//
////////////////////////////////////////////////////////////////////////

GCInfo::GCInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GCInfo::~GCInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GCInfo::GCInfo(const GCInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

GCInfo & GCInfo::operator=(const GCInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

int GCInfo::operator==(const GCInfo &c) {
   int match = 0;

   if(code     == c.code &&
      lvl_type == c.lvl_type &&
      lvl_1    == c.lvl_1 &&
      lvl_2    == c.lvl_2 &&
      vflag    == c.vflag) match = 1;

   return(match);
}


////////////////////////////////////////////////////////////////////////

void GCInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::clear() {

   abbr_str.clear();
   lvl_str.clear();
   info_str.clear();

   code     = 0;
   lvl_type = NoLevel;
   lvl_1    = 0;
   lvl_2    = 0;
   vflag    = 0;
   pflag    = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::assign(const GCInfo &c) {

   clear();

   abbr_str = c.abbr_str;
   lvl_str  = c.lvl_str;
   info_str = c.info_str;

   code     = c.code;

   lvl_type = c.lvl_type;
   lvl_1    = c.lvl_1;
   lvl_2    = c.lvl_2;
   vflag    = c.vflag;
   pflag    = c.pflag;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_gcinfo(const char *c, int ptv) {
   char tmp_str[max_str_len], tmp2_str[max_str_len];
   char *ptr, *ptr2;
   int j;

   // Initialize
   clear();

   // Initialize the temp string
   strcpy(tmp_str, c);

   // Retreive the GRIB code value
   if((ptr = strtok(tmp_str, "/")) == NULL) {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "bad GRIB code specified \""
           << c << "\".\n\n" << flush;
      exit(1);
   }

   // Store the code value
   code = str_to_grib_code(ptr);

   // Retrieve the level value
   if((ptr = strtok(NULL, "/")) == NULL) {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "each GRIB code specified must be followed by an "
           << "accumulation, level, or presssure level indicator \""
           << c << "\".\n\n" << flush;
      exit(1);
   }

   // Check the level indicator type
   if(*ptr != 'A' && *ptr != 'Z' &&
      *ptr != 'P' && *ptr != 'R' &&
      *ptr != 'L') {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "each GRIB code specified (" << c
           << ") must be followed by level information "
           << "that begins with:\n"
           << "\t\'A\' for an accumulation interval\n"
           << "\t\'Z\' for a vertical level\n"
           << "\t\'P\' for a pressure level\n"
           << "\t\'R\' for a record number\n"
           << "\t\'L\' for a generic level\n\n"
           << flush;
      exit(1);
   }

   // Set the level type
   if(      *ptr == 'A') lvl_type = AccumLevel;
   else if (*ptr == 'Z') lvl_type = VertLevel;
   else if (*ptr == 'P') lvl_type = PresLevel;
   else if (*ptr == 'R') lvl_type = RecNumber;
   else if (*ptr == 'L') lvl_type = NoLevel;
   else                  lvl_type = NoLevel;

   // Store the level string
   set_lvl_str(ptr);

   // Advance the pointer past the 'A', 'Z', 'P', 'R', or 'L'
   ptr++;
   lvl_1 = atoi(ptr);

   // Look for a '-' and a second level indicator
   ptr2 = strchr(ptr, '-');
   if(ptr2 != NULL) lvl_2 = atoi(++ptr2);
   else             lvl_2 = lvl_1;

   // Only allow ranges for PresLevel and VertLevel
   if(lvl_type != PresLevel &&
      lvl_type != VertLevel &&
      lvl_1    != lvl_2) {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "ranges of levels are only supported for pressure levels "
           << "(P) and vertical levels (Z).\n"
           << flush;
      exit(1);
   }

   // For pressure levels, check the order of lvl_1 and lvl_2
   // and define lvl_1 < lvl_2
   if(lvl_type == PresLevel) {

      // If the levels are the same, reset the lvl_str
      if(lvl_1 == lvl_2) {
         sprintf(tmp2_str, "P%i", lvl_1);
         set_lvl_str(tmp2_str);
      }
      // Switch lvl_1 and lvl_2
      else if(lvl_1 > lvl_2) {
         j = lvl_1;
         lvl_1 = lvl_2;
         lvl_2 = j;
      }
      // Reset the lvl_str to be high - low
      else {
         sprintf(tmp2_str, "P%i-%i", lvl_2, lvl_1);
         set_lvl_str(tmp2_str);
      }
   }

   // Check for "/PROB" to indicate a probability forecast
   if((ptr = strtok(NULL, "/")) != NULL) {

      if(strncasecmp(ptr, "PROB", strlen("PROB")) == 0) pflag = 1;
      else {
         cout << "WARNING: GCInfo::set_gcinfo() -> "
              << "unrecognized flag value \"" << ptr
              << "\" for GRIB code \"" << c << "\".\n" << flush;
      }
   }

   // Get the GRIB code abbreviation string
   get_grib_code_abbr(code, ptv, tmp_str);

   // For a non-zero accumulation interval, append _HH to the
   // abbreviation string
   if(lvl_type == AccumLevel && lvl_1 > 0) {
      sprintf(tmp2_str, "%s_%.2i", tmp_str, lvl_1);
      strcpy(tmp_str, tmp2_str);
   }

   // Set the abbr_str
   set_abbr_str(tmp_str);

   // Set the info_str
   sprintf(tmp_str, "%s/%s", abbr_str.text(), lvl_str.text());
   set_info_str(tmp_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_abbr_str(const char *c) {

   abbr_str.clear();

   abbr_str = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_lvl_str(const char *c) {

   lvl_str.clear();

   lvl_str = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_info_str(const char *c) {

   info_str.clear();

   info_str = c;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PairData
//
////////////////////////////////////////////////////////////////////////

PairData::PairData() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairData::~PairData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PairData::PairData(const PairData &pd) {

   init_from_scratch();

   assign(pd);
}

////////////////////////////////////////////////////////////////////////

PairData & PairData::operator=(const PairData &pd) {

   if(this == &pd) return(*this);

   assign(pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PairData::init_from_scratch() {

   mask_wd_ptr = (WrfData *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::clear() {

   msg_typ.clear();
   mask_name.clear();

   mask_wd_ptr = (WrfData *) 0;  // Not allocated

   interp_mthd = im_na;
   interp_wdth = bad_data_int;

   f_na.clear();
   c_na.clear();
   o_na.clear();

   lat_na.clear();
   lon_na.clear();
   lvl_na.clear();
   elv_na.clear();

   n_pair  = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::assign(const PairData &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name);
   set_mask_wd_ptr(pd.mask_wd_ptr);

   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_wdth(pd.interp_wdth);

   for(i=0; i<pd.n_pair; i++) {
      add_pair(pd.lat_na[i], pd.lon_na[i],
               pd.lvl_na[i], pd.elv_na[i],
               pd.f_na[i], pd.c_na[i], pd.o_na[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_mask_name(const char *c) {

   mask_name = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_mask_wd_ptr(WrfData *wd_ptr) {

   mask_wd_ptr = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_msg_typ(const char *c) {

   msg_typ = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_interp_mthd(const char *str) {

   interp_mthd = string_to_interpmthd(str);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_interp_mthd(InterpMthd m) {

   interp_mthd = m;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_interp_wdth(int n) {

   interp_wdth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::add_pair(double lat, double lon,
                        double lvl, double elv,
                        double f,   double c,   double o) {
   lat_na.add(lat);
   lon_na.add(lon);
   lvl_na.add(lvl);
   elv_na.add(elv);

   f_na.add(f);
   c_na.add(c);
   o_na.add(o);

   // Increment the number of pairs
   n_pair += 1;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class GCPairData
//
////////////////////////////////////////////////////////////////////////

GCPairData::GCPairData() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GCPairData::~GCPairData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GCPairData::GCPairData(const GCPairData &gc_pd) {

   init_from_scratch();

   assign(gc_pd);
}

////////////////////////////////////////////////////////////////////////

GCPairData & GCPairData::operator=(const GCPairData &gc_pd) {

   if(this == &gc_pd) return(*this);

   assign(gc_pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GCPairData::init_from_scratch() {

   fcst_wd_ptr  = (WrfData **) 0;
   climo_wd_ptr = (WrfData **) 0;
   pd           = (PairData ***) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::clear() {
   int i, j, k;

   fcst_gci.clear();
   obs_gci.clear();

   beg_ut        = (unixtime) 0;
   end_ut        = (unixtime) 0;

   interp_thresh = 0;
   n_fcst        = 0;
   n_climo       = 0;
   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

   fcst_lvl.clear();
   climo_lvl.clear();

   for(i=0; i<n_fcst; i++)  fcst_wd_ptr[i]  = (WrfData *) 0;
   for(i=0; i<n_climo; i++) climo_wd_ptr[i] = (WrfData *) 0;

   fcst_wd_ptr  = (WrfData **) 0;
   climo_wd_ptr = (WrfData **) 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::assign(const GCPairData &gc_pd) {
   int i, j, k;

   clear();

   set_fcst_gci(gc_pd.fcst_gci);
   set_obs_gci(gc_pd.obs_gci);

   beg_ut  = gc_pd.beg_ut;
   end_ut  = gc_pd.end_ut;

   interp_thresh = gc_pd.interp_thresh;

   set_n_fcst(gc_pd.n_fcst);
   for(i=0; i<gc_pd.n_fcst; i++) {
      set_fcst_lvl(i, fcst_lvl[i]);
      set_fcst_wd_ptr(i, fcst_wd_ptr[i]);
   }

   set_n_climo(gc_pd.n_climo);
   for(i=0; i<gc_pd.n_climo; i++) {
      set_climo_lvl(i, climo_lvl[i]);
      set_climo_wd_ptr(i, climo_wd_ptr[i]);
   }

   set_pd_size(gc_pd.n_msg_typ, gc_pd.n_mask, gc_pd.n_interp);

   for(i=0; i<gc_pd.n_msg_typ; i++) {
      for(j=0; j<gc_pd.n_mask; j++) {
         for(k=0; k<gc_pd.n_interp; k++) {

            pd[i][j][k] = gc_pd.pd[i][j][k];
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_fcst_gci(const GCInfo &gci) {

   fcst_gci = gci;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_obs_gci(const GCInfo &gci) {

   obs_gci = gci;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_n_fcst(int n) {
   int i;

   n_fcst = n;

   fcst_wd_ptr  = new WrfData * [n_fcst];

   for(i=0; i<n_fcst; i++) {
      fcst_lvl.add(bad_data_double);
      fcst_wd_ptr[i] = (WrfData *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_fcst_lvl(int i, double lvl) {

   fcst_lvl.set(i, lvl);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_fcst_wd_ptr(int i, WrfData *wd_ptr) {

   fcst_wd_ptr[i] = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_n_climo(int n) {
   int i;

   n_climo = n;

   climo_wd_ptr  = new WrfData * [n_climo];

   for(i=0; i<n_climo; i++) {
      climo_lvl.add(bad_data_double);
      climo_wd_ptr[i] = (WrfData *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_climo_lvl(int i, double lvl) {

   climo_lvl.set(i, lvl);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_climo_wd_ptr(int i, WrfData *wd_ptr) {

   climo_wd_ptr[i] = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_pd_size(int types, int masks, int interps) {
   int i, j;

   // Store the dimensions for the PairData array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;

   // Allocate space for the PairData array
   pd = new PairData ** [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i] = new PairData * [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j] = new PairData [n_interp];
      }
   }

   return;
}


////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_ens_size() {
   int i, j, k;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].set_size();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_msg_typ(int i_msg_typ, const char *name) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_mask_wd(int i_mask, const char *name,
                             WrfData *wd_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_wd_ptr(wd_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_interp(int i_interp, const char *interp_mthd_str,
                            int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_wdth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_interp(int i_interp, InterpMthd mthd, int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_wdth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::add_obs(float *hdr_arr,     char *hdr_typ_str,
                         char  *hdr_sid_str, unixtime hdr_ut,
                         float *obs_arr,     Grid &gr) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_hgt;
   double fcst_v, climo_v, obs_v;
   int fcst_lvl_below, fcst_lvl_above;
   int climo_lvl_below, climo_lvl_above;

   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_gci.code != nint(obs_arr[1])) return;

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) return;

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];

   obs_lvl = obs_arr[2];
   obs_hgt = obs_arr[3];
   obs_v   = obs_arr[4];

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) return;

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(x < 0 || x >= gr.nx() ||
      y < 0 || y >= gr.ny()) return;

   // For pressure levels, check if the observation pressure level
   // falls in the requsted range.
   if(obs_gci.lvl_type == PresLevel) {

      if(obs_lvl < obs_gci.lvl_1 ||
         obs_lvl > obs_gci.lvl_2) return;
   }
   // For accumulations, check if the observation accumulation interval
   // matches the requested interval.
   else if(obs_gci.lvl_type == AccumLevel) {

      if(obs_lvl < obs_gci.lvl_1 ||
         obs_lvl > obs_gci.lvl_2) return;
   }
   // For vertical levels, check for a surface message type or if the
   // observation height falls within the requested range.
   else if(obs_gci.lvl_type == VertLevel) {

      if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL &&
         (obs_hgt < obs_gci.lvl_1 ||
          obs_hgt > obs_gci.lvl_2)) return;
   }
   // For all other level types (RecNumber, NoLevel), check
   // if the observation height falls within the requested range.
   else {
      if(obs_hgt < obs_gci.lvl_1 ||
         obs_hgt > obs_gci.lvl_2) return;
   }

   // For a single forecast field
   if(n_fcst == 1) {
      fcst_lvl_below = 0;
      fcst_lvl_above = 0;
   }
   // For multiple forecast fields, find the levels above and below
   // the observation point.
   else {

      // Interpolate using the pressure value
      if(fcst_gci.lvl_type == PresLevel) {
         find_vert_lvl(1, obs_lvl, fcst_lvl_below, fcst_lvl_above);
      }
      // Interpolate using the height value
      else {
         find_vert_lvl(1, obs_hgt, fcst_lvl_below, fcst_lvl_above);
      }
   }

   // For a single climatology field
   if(n_climo == 1) {
      climo_lvl_below = 0;
      climo_lvl_above = 0;
   }
   // For multiple climatology fields, find the levels above and below
   // the observation point.
   else {

      // Interpolate using the pressure value
      if(fcst_gci.lvl_type == PresLevel) {
         find_vert_lvl(0, obs_lvl, climo_lvl_below, climo_lvl_above);
      }
      // Interpolate using the height value
      else {
         find_vert_lvl(0, obs_hgt, climo_lvl_below, climo_lvl_above);
      }
   }

   // Look through all of the PairData objects to see if the observation
   // should be added.

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //

      // Handle ANYAIR
      if(strcmp(anyair_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anyair_msg_typ_str, hdr_typ_str) == NULL ) continue;
      }

      // Handle ANYSFC
      else if(strcmp(anysfc_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anysfc_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle ONLYSF
      else if(strcmp(onlysf_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle all other message types
      else {
         if(strcmp(hdr_typ_str, pd[i][0][0].msg_typ) != 0) continue;
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_wd_ptr != (WrfData *) 0) {
            if(!pd[i][j][0].mask_wd_ptr->s_is_on(x, y)) continue;
         }
         // Otherwise, check for the obs Station ID matching the
         // masking SID
         else {
            if(strcmp(hdr_sid_str, pd[i][j][0].mask_name) != 0)
               continue;
         }

         // Compute the interpolated values
         for(k=0; k<n_interp; k++) {

            // Compute the interpolated forecast value
            fcst_v = compute_interp(1, obs_x, obs_y, k,
                        obs_lvl, fcst_lvl_below, fcst_lvl_above);

            if(is_bad_data(fcst_v)) continue;

            // Compute the interpolated climotological value
            climo_v = compute_interp(0, obs_x, obs_y, k,
                         obs_lvl, climo_lvl_below, climo_lvl_above);

            // Add the forecast, climatological, and observation data
            pd[i][j][k].add_pair(hdr_lat, hdr_lon,
                                 obs_lvl, obs_hgt,
                                 fcst_v, climo_v, obs_v);

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::find_vert_lvl(int fcst_flag, double obs_lvl,
                               int &i_below, int &i_above) {
   int i, n;
   NumArray *lvl_na;
   double dist, dist_below, dist_above;

   // Check for the forecast or climo fields
   if(fcst_flag) { n = n_fcst;  lvl_na = &fcst_lvl;  }
   else          { n = n_climo; lvl_na = &climo_lvl; }

   if(n==0) {
      i_below = i_above = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_below = dist_above = 1.0e30;
   for(i=0; i<n; i++) {

      dist = obs_lvl - (*lvl_na)[i];

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && abs((long double) dist) < dist_below) {
         dist_below = abs((long double) dist);
         i_below = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && abs((long double) dist) < dist_above) {
         dist_above = abs((long double) dist);
         i_above = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_below, 1.0e30) && !is_eq(dist_above, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_below = i_above;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_above = i_below;
   }
   // Check if an error occurred
   else if(is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      cerr << "\n\nERROR: GCPairData::find_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n"
           << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int GCPairData::get_n_pair() {
   int n, i, j, k;

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            n += pd[i][j][k].n_pair;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

double GCPairData::compute_interp(int fcst_flag,
                                  double obs_x, double obs_y,
                                  int i_interp, double to_lvl,
                                  int i_below, int i_above) {
   int n;
   NumArray *lvl_na;
   WrfData **wd_ptr;
   double v, v_below, v_above, t;

   // Check for the forecast or climo fields
   if(fcst_flag) {
      n       = n_fcst;
      lvl_na  = &fcst_lvl;
      wd_ptr  = fcst_wd_ptr;
   }
   else {
      n       = n_climo;
      lvl_na  = &climo_lvl;
      wd_ptr  = climo_wd_ptr;
   }

   if(n==0) return(bad_data_double);

   v_below = compute_horz_interp(wd_ptr[i_below], obs_x, obs_y,
                                 pd[0][0][i_interp].interp_mthd,
                                 pd[0][0][i_interp].interp_wdth);

   if(i_below == i_above) {
      v = v_below;
   }
   else {
      v_above = compute_horz_interp(wd_ptr[i_above], obs_x, obs_y,
                                    pd[0][0][i_interp].interp_mthd,
                                    pd[0][0][i_interp].interp_wdth);

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_gci.code == spfh_grib_code &&
         obs_gci.code  == spfh_grib_code) {
         t = compute_vert_pinterp(log(v_below), (*lvl_na)[i_below],
                                  log(v_above), (*lvl_na)[i_above],
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_gci.lvl_type == PresLevel) {
         v = compute_vert_pinterp(v_below, (*lvl_na)[i_below],
                                  v_above, (*lvl_na)[i_above],
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_below, (*lvl_na)[i_below],
                                  v_above, (*lvl_na)[i_above],
                                  to_lvl);
      }
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double GCPairData::compute_horz_interp(WrfData *wd_ptr,
                                       double obs_x, double obs_y,
                                       int mthd, int wdth) {
   double v;
   int x_ll, y_ll;

   // The neighborhood width is odd, find the lower-left corner of
   // the neighborhood
   if(wdth%2 == 1) {
      x_ll = nint(obs_x) - (wdth - 1)/2;
      y_ll = nint(obs_y) - (wdth - 1)/2;
   }
   // The neighborhood width is even, find the lower-left corner of
   // the neighborhood
   else {
      x_ll = nint(floor(obs_x) - (wdth/2 - 1));
      y_ll = nint(floor(obs_y) - (wdth/2 - 1));
   }

   // Compute the interpolated value for the fields above and below
   switch(mthd) {

      case(im_min):     // Minimum
         v = interp_min(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_max):     // Maximum
         v = interp_max(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_median):  // Median
         v = interp_median(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_uw_mean): // Unweighted Mean
         v = interp_uw_mean(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_dw_mean): // Distance-Weighted Mean
         v = interp_dw_mean(*wd_ptr, x_ll, y_ll, wdth, obs_x, obs_y,
                            dw_mean_pow, interp_thresh);
         break;

      case(im_ls_fit):  // Least-squares fit
         v = interp_ls_fit(*wd_ptr, x_ll, y_ll, wdth, obs_x, obs_y,
                           interp_thresh);
         break;

      default:
         cerr << "\n\nERROR: GCPairData::compute_horz_interp() -> "
              << "unexpected interpolation method encountered: "
              << mthd << "\n\n" << flush;
         exit(1);
         break;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Interpolate lineary in the log of pressure between values "v1" and
// "v2" at pressure levels "prs1" and "prs2" to pressure level "to_prs".
//
////////////////////////////////////////////////////////////////////////

double GCPairData::compute_vert_pinterp(double v1, double prs1,
                                        double v2, double prs2,
                                        double to_prs) {
   double v_interp;

   if(prs1 <= 0.0 || prs2 <= 0.0 || to_prs <= 0.0) {
      cerr << "\n\nERROR: GCPairData::compute_vert_pinterp() -> "
           << "pressure shouldn't be <= zero!\n\n" << flush;
      exit(1);
   }

   v_interp = v1 + ((v2-v1)*log(prs1/to_prs)/log(prs1/prs2));

   return(v_interp);
}

////////////////////////////////////////////////////////////////////////
//
// Linearly interpolate between values "v1" and "v2" at height levels
// "lvl1" and "lvl2" to height level "to_lvl".
//
////////////////////////////////////////////////////////////////////////

double GCPairData::compute_vert_zinterp(double v1, double lvl1,
                                        double v2, double lvl2,
                                        double to_lvl) {
   double d1, d2, v_interp;

   if(lvl1 <= 0.0 || lvl2 <= 0.0 || to_lvl <= 0.0) {
      cerr << "\n\nERROR: GCPairData::compute_vert_zinterp() -> "
           << "level shouldn't be <= zero!\n\n" << flush;
      exit(1);
   }

   d1 = abs(lvl1 - to_lvl);
   d2 = abs(lvl2 - to_lvl);

   // Linearly interpolate betwen lvl_1 and lvl_2
   v_interp = v1*d1/(d1+d2) + v2*d2/(d1+d2);

   return(v_interp);
}

////////////////////////////////////////////////////////////////////////
//
// Code for class EnsPairData
//
////////////////////////////////////////////////////////////////////////

EnsPairData::EnsPairData() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

EnsPairData::~EnsPairData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

EnsPairData::EnsPairData(const EnsPairData &pd) {

   init_from_scratch();

   assign(pd);
}

////////////////////////////////////////////////////////////////////////

EnsPairData & EnsPairData::operator=(const EnsPairData &pd) {

   if(this == &pd) return(*this);

   assign(pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::init_from_scratch() {

   mask_wd_ptr = (WrfData * ) 0;
   e_na        = (NumArray *) 0;
   n_pair      = 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::clear() {
   int i;

   msg_typ.clear();
   mask_name.clear();

   mask_wd_ptr = (WrfData *) 0;  // Not allocated

   interp_mthd = im_na;
   interp_wdth = bad_data_int;

   for(i=0; i<n_pair; i++) e_na[i].clear();

   if(e_na) { delete [] e_na; e_na = (NumArray *) 0; }

   o_na.clear();
   v_na.clear();
   r_na.clear();

   sid_sa.clear();
   lat_na.clear();
   lon_na.clear();
     x_na.clear();
     y_na.clear();
   lvl_na.clear();
   elv_na.clear();

   n_pair = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::assign(const EnsPairData &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name);
   set_mask_wd_ptr(pd.mask_wd_ptr);

   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_wdth(pd.interp_wdth);

   sid_sa = pd.sid_sa;
   lat_na = pd.lat_na;
   lon_na = pd.lon_na;
     x_na = pd.x_na;
     y_na = pd.y_na;
   lvl_na = pd.lvl_na;
   elv_na = pd.elv_na;
   n_pair = pd.n_pair;
   o_na   = pd.o_na;
   v_na   = pd.v_na;
   r_na   = pd.r_na;

   set_size();

   for(i=0; i<pd.n_pair; i++) e_na[i] = pd.e_na[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_mask_name(const char *c) {

   mask_name = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_mask_wd_ptr(WrfData *wd_ptr) {

   mask_wd_ptr = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_msg_typ(const char *c) {

   msg_typ = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_interp_mthd(const char *str) {

   interp_mthd = string_to_interpmthd(str);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_interp_mthd(InterpMthd m) {

   interp_mthd = m;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_interp_wdth(int n) {

   interp_wdth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::add_obs(const char *sid, double lat, double lon,
                          double x, double y, double lvl, double elv,
                          double o) {
   sid_sa.add(sid);
   lat_na.add(lat);
   lon_na.add(lon);
     x_na.add(x);
     y_na.add(y);
   lvl_na.add(lvl);
   elv_na.add(elv);
     o_na.add(o);

   // Increment the number of pairs
   n_pair += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::add_obs(int x, int y, double o) {

   x_na.add(x);
   y_na.add(y);
   o_na.add(o);

   // Increment the number of pairs
   n_pair += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::add_ens(int i, double v) {

   e_na[i].add(v);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::set_size() {

   // Allocate a NumArray to store ensemble values for each observation
   e_na = new NumArray [n_pair];

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::compute_rank(int n_vld_ens) {
   int i, j, n_vld, n_bel;

   // Compute the rank for each observation
   for(i=0; i<o_na.n_elements(); i++) {

      // Compute the number of ensemble values less than the observation
      for(j=0, n_vld=0, n_bel=0; j<e_na[i].n_elements(); j++) {

         // Skip bad data
         if(!is_bad_data(e_na[i][j])) {

            // Increment the valid count
            n_vld++;

            // Increment the number of ensembles < observation
            if(e_na[i][j] < o_na[i]) n_bel++;
         }
      } // end for j

      // Store the number of valid ensemble values
      v_na.add(n_vld);

      // Store the observation rank only when the number of vaild
      // values matches the number of valid ensembles
      if(n_vld == n_vld_ens) r_na.add(n_bel+1);
      else                   r_na.add(bad_data_int);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsPairData::compute_rhist(int n_vld_ens, NumArray &rhist_na) {
   int i, rank;

   // Clear the input ranked histogram
   rhist_na.clear();

   // Initialize the histogram counts to 0
   for(i=0; i<=n_vld_ens; i++) rhist_na.add(0);

   // The compute_rank() routine should have already been called.
   // Loop through the ranks and populate the histogram.
   for(i=0; i<r_na.n_elements(); i++) {

      // Get the current rank
      rank = r_na[i];

      // Increment the histogram counts
      if(!is_bad_data(rank)) rhist_na.set(rank-1, rhist_na[rank-1]+1);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class GCEnsPairData
//
////////////////////////////////////////////////////////////////////////

GCEnsPairData::GCEnsPairData() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GCEnsPairData::~GCEnsPairData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GCEnsPairData::GCEnsPairData(const GCEnsPairData &gc_pd) {

   init_from_scratch();

   assign(gc_pd);
}

////////////////////////////////////////////////////////////////////////

GCEnsPairData & GCEnsPairData::operator=(const GCEnsPairData &gc_pd) {

   if(this == &gc_pd) return(*this);

   assign(gc_pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::init_from_scratch() {

   fcst_wd_ptr = (WrfData **) 0;
   pd          = (EnsPairData ***) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::clear() {
   int i, j, k;

   fcst_gci.clear();
   obs_gci.clear();

   beg_ut        = (unixtime) 0;
   end_ut        = (unixtime) 0;

   interp_thresh = 0;
   n_fcst        = 0;
   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

   fcst_lvl.clear();

   for(i=0; i<n_fcst; i++)  fcst_wd_ptr[i]  = (WrfData *) 0;

   fcst_wd_ptr  = (WrfData **) 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::assign(const GCEnsPairData &gc_pd) {
   int i, j, k;

   clear();

   set_fcst_gci(gc_pd.fcst_gci);
   set_obs_gci(gc_pd.obs_gci);

   beg_ut  = gc_pd.beg_ut;
   end_ut  = gc_pd.end_ut;

   interp_thresh = gc_pd.interp_thresh;

   set_n_fcst(gc_pd.n_fcst);
   for(i=0; i<gc_pd.n_fcst; i++) {
      set_fcst_lvl(i, fcst_lvl[i]);
      set_fcst_wd_ptr(i, fcst_wd_ptr[i]);
   }

   set_pd_size(gc_pd.n_msg_typ, gc_pd.n_mask, gc_pd.n_interp);

   for(i=0; i<gc_pd.n_msg_typ; i++) {
      for(j=0; j<gc_pd.n_mask; j++) {
         for(k=0; k<gc_pd.n_interp; k++) {

            pd[i][j][k] = gc_pd.pd[i][j][k];
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_fcst_gci(const GCInfo &gci) {

   fcst_gci = gci;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_obs_gci(const GCInfo &gci) {

   obs_gci = gci;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_n_fcst(int n) {
   int i;

   n_fcst = n;

   fcst_wd_ptr  = new WrfData * [n_fcst];

   for(i=0; i<n_fcst; i++) {
      fcst_lvl.add(bad_data_double);
      fcst_wd_ptr[i] = (WrfData *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_fcst_lvl(int i, double lvl) {

   fcst_lvl.set(i, lvl);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_fcst_wd_ptr(int i, WrfData *wd_ptr) {

   fcst_wd_ptr[i] = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_pd_size(int types, int masks, int interps) {
   int i, j;

   // Store the dimensions for the PairData array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;

   // Allocate space for the PairData array
   pd = new EnsPairData ** [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i] = new EnsPairData * [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j] = new EnsPairData [n_interp];
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_msg_typ(int i_msg_typ, const char *name) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_mask_wd(int i_mask, const char *name,
                             WrfData *wd_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_wd_ptr(wd_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_interp(int i_interp, const char *interp_mthd_str,
                            int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_wdth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::set_interp(int i_interp, InterpMthd mthd, int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_wdth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::add_obs(float *hdr_arr, const char *hdr_typ_str,
                            const char  *hdr_sid_str, unixtime hdr_ut,
                            float *obs_arr, Grid &gr) {


   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_hgt;
   double obs_v;

   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_gci.code != nint(obs_arr[1])) return;

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) return;

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];

   obs_lvl = obs_arr[2];
   obs_hgt = obs_arr[3];
   obs_v   = obs_arr[4];

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) return;

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(x < 0 || x >= gr.nx() ||
      y < 0 || y >= gr.ny()) return;

   // For pressure levels, check if the observation pressure level
   // falls in the requsted range.
   if(obs_gci.lvl_type == PresLevel) {

      if(obs_lvl < obs_gci.lvl_1 ||
         obs_lvl > obs_gci.lvl_2) return;
   }
   // For accumulations, check if the observation accumulation interval
   // matches the requested interval.
   else if(obs_gci.lvl_type == AccumLevel) {

      if(obs_lvl < obs_gci.lvl_1 ||
         obs_lvl > obs_gci.lvl_2) return;
   }
   // For vertical levels, check for a surface message type or if the
   // observation height falls within the requested range.
   else if(obs_gci.lvl_type == VertLevel) {

      if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL &&
         (obs_hgt < obs_gci.lvl_1 ||
          obs_hgt > obs_gci.lvl_2)) return;
   }
   // For all other level types (RecNumber, NoLevel), check
   // if the observation height falls within the requested range.
   else {
      if(obs_hgt < obs_gci.lvl_1 ||
         obs_hgt > obs_gci.lvl_2) return;
   }

   // Look through all of the PairData objects to see if the observation
   // should be added.

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //

      // Handle ANYAIR
      if(strcmp(anyair_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anyair_msg_typ_str, hdr_typ_str) == NULL ) continue;
      }

      // Handle ANYSFC
      else if(strcmp(anysfc_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anysfc_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle ONLYSF
      else if(strcmp(onlysf_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle all other message types
      else {
         if(strcmp(hdr_typ_str, pd[i][0][0].msg_typ) != 0) continue;
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_wd_ptr != (WrfData *) 0) {
            if(!pd[i][j][0].mask_wd_ptr->s_is_on(x, y)) continue;
         }
         // Otherwise, check for the obs Station ID matching the
         // masking SID
         else {
            if(strcmp(hdr_sid_str, pd[i][j][0].mask_name) != 0)
               continue;
         }

         // Add the observation for each interpolation method
         for(k=0; k<n_interp; k++) {

            // Add the observation value
            pd[i][j][k].add_obs(hdr_sid_str, hdr_lat, hdr_lon,
                                obs_x, obs_y, obs_lvl, obs_hgt,
                                obs_v);

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::add_ens() {
   int i, j, k, l;
   int fcst_lvl_below, fcst_lvl_above;
   double fcst_v;

   // Loop through all the EnsPairData objects and interpolate
   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            // Process each of the observations
            for(l=0; l<pd[i][j][k].n_pair; l++) { 

               // For a single forecast field
               if(n_fcst == 1) {
                  fcst_lvl_below = 0;
                  fcst_lvl_above = 0;
               }
               // For multiple forecast fields, find the levels above and below
               // the observation point.
               else {

                  // Interpolate using the pressure value
                  if(fcst_gci.lvl_type == PresLevel) {
                     find_vert_lvl(pd[i][j][k].lvl_na[l], fcst_lvl_below, fcst_lvl_above);
                  }
                  // Interpolate using the height value
                  else {
                     find_vert_lvl(pd[i][j][k].elv_na[l], fcst_lvl_below, fcst_lvl_above);
                 }
               }

               // Compute the interpolated ensemble value
               fcst_v = compute_interp(pd[i][j][k].x_na[l], pd[i][j][k].y_na[l], k,
                                       pd[i][j][k].lvl_na[l], fcst_lvl_below, fcst_lvl_above);

               // Add the ensemble value, even if it's bad data
               pd[i][j][k].add_ens(l, fcst_v);

            }
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::add_miss() {
   int i, j, k, l;

   // Loop through all the EnsPairData objects
   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            for(l=0; l<pd[i][j][k].n_pair; l++) { 

               // Add bad data as a placeholder for missing file
               pd[i][j][k].add_ens(l, bad_data_double);

            } // end for l
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void GCEnsPairData::find_vert_lvl(double obs_lvl,
                                  int &i_below, int &i_above) {
   int i, n;
   NumArray *lvl_na;
   double dist, dist_below, dist_above;

   n = n_fcst;
   lvl_na = &fcst_lvl;

   if(n==0) {
      i_below = i_above = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_below = dist_above = 1.0e30;
   for(i=0; i<n; i++) {

      dist = obs_lvl - (*lvl_na)[i];

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && abs((long double) dist) < dist_below) {
         dist_below = abs((long double) dist);
         i_below = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && abs((long double) dist) < dist_above) {
         dist_above = abs((long double) dist);
         i_above = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_below, 1.0e30) && !is_eq(dist_above, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_below = i_above;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_above = i_below;
   }
   // Check if an error occurred
   else if(is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      cerr << "\n\nERROR: GCEnsPairData::find_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n"
           << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int GCEnsPairData::get_n_pair() {
   int n, i, j, k;

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            n += pd[i][j][k].n_pair;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

double GCEnsPairData::compute_interp(double obs_x, double obs_y,
                                     int i_interp, double to_lvl,
                                     int i_below, int i_above) {
   int n;
   NumArray *lvl_na;
   WrfData **wd_ptr;
   double v, v_below, v_above, t;

   n      = n_fcst;
   lvl_na = &fcst_lvl;
   wd_ptr = fcst_wd_ptr;

   if(n==0) return(bad_data_double);

   v_below = compute_horz_interp(wd_ptr[i_below], obs_x, obs_y,
                                 pd[0][0][i_interp].interp_mthd,
                                 pd[0][0][i_interp].interp_wdth);

   if(i_below == i_above) {
      v = v_below;
   }
   else {
      v_above = compute_horz_interp(wd_ptr[i_above], obs_x, obs_y,
                                    pd[0][0][i_interp].interp_mthd,
                                    pd[0][0][i_interp].interp_wdth);

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_gci.code == spfh_grib_code &&
         obs_gci.code  == spfh_grib_code) {
         t = compute_vert_pinterp(log(v_below), (*lvl_na)[i_below],
                                  log(v_above), (*lvl_na)[i_above],
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_gci.lvl_type == PresLevel) {
         v = compute_vert_pinterp(v_below, (*lvl_na)[i_below],
                                  v_above, (*lvl_na)[i_above],
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_below, (*lvl_na)[i_below],
                                  v_above, (*lvl_na)[i_above],
                                  to_lvl);
      }
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double GCEnsPairData::compute_horz_interp(WrfData *wd_ptr,
                                       double obs_x, double obs_y,
                                       int mthd, int wdth) {
   double v;
   int x_ll, y_ll;

   // The neighborhood width is odd, find the lower-left corner of
   // the neighborhood
   if(wdth%2 == 1) {
      x_ll = nint(obs_x) - (wdth - 1)/2;
      y_ll = nint(obs_y) - (wdth - 1)/2;
   }
   // The neighborhood width is even, find the lower-left corner of
   // the neighborhood
   else {
      x_ll = nint(floor(obs_x) - (wdth/2 - 1));
      y_ll = nint(floor(obs_y) - (wdth/2 - 1));
   }

   // Compute the interpolated value for the fields above and below
   switch(mthd) {

      case(im_min):     // Minimum
         v = interp_min(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_max):     // Maximum
         v = interp_max(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_median):  // Median
         v = interp_median(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_uw_mean): // Unweighted Mean
         v = interp_uw_mean(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_dw_mean): // Distance-Weighted Mean
         v = interp_dw_mean(*wd_ptr, x_ll, y_ll, wdth, obs_x, obs_y,
                            dw_mean_pow, interp_thresh);
         break;

      case(im_ls_fit):  // Least-squares fit
         v = interp_ls_fit(*wd_ptr, x_ll, y_ll, wdth, obs_x, obs_y,
                           interp_thresh);
         break;

      default:
         cerr << "\n\nERROR: GCEnsPairData::compute_horz_interp() -> "
              << "unexpected interpolation method encountered: "
              << mthd << "\n\n" << flush;
         exit(1);
         break;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Interpolate lineary in the log of pressure between values "v1" and
// "v2" at pressure levels "prs1" and "prs2" to pressure level "to_prs".
//
////////////////////////////////////////////////////////////////////////

double GCEnsPairData::compute_vert_pinterp(double v1, double prs1,
                                        double v2, double prs2,
                                        double to_prs) {
   double v_interp;

   if(prs1 <= 0.0 || prs2 <= 0.0 || to_prs <= 0.0) {
      cerr << "\n\nERROR: GCEnsPairData::compute_vert_pinterp() -> "
           << "pressure shouldn't be <= zero!\n\n" << flush;
      exit(1);
   }

   v_interp = v1 + ((v2-v1)*log(prs1/to_prs)/log(prs1/prs2));

   return(v_interp);
}

////////////////////////////////////////////////////////////////////////
//
// Linearly interpolate between values "v1" and "v2" at height levels
// "lvl1" and "lvl2" to height level "to_lvl".
//
////////////////////////////////////////////////////////////////////////

double GCEnsPairData::compute_vert_zinterp(double v1, double lvl1,
                                        double v2, double lvl2,
                                        double to_lvl) {
   double d1, d2, v_interp;

   if(lvl1 <= 0.0 || lvl2 <= 0.0 || to_lvl <= 0.0) {
      cerr << "\n\nERROR: GCEnsPairData::compute_vert_zinterp() -> "
           << "level shouldn't be <= zero!\n\n" << flush;
      exit(1);
   }

   d1 = abs(lvl1 - to_lvl);
   d2 = abs(lvl2 - to_lvl);

   // Linearly interpolate betwen lvl_1 and lvl_2
   v_interp = v1*d1/(d1+d2) + v2*d2/(d1+d2);

   return(v_interp);
}

////////////////////////////////////////////////////////////////////////
