// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <ctime>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_cal.h"
#include "nc_utils.h"
#include "vx_util.h"
#include "write_netcdf.h"
#include "grid_output.h"

///////////////////////////////////////////////////////////////////////////////

long hdrNum;
long obsNum;

int   obs_buf_size;
int   hdr_buf_size;
//int   processed_count;
int   cur_hdr_idx = 0;
int   cur_obs_idx = 0;

int   obs_data_idx;
int   obs_data_offset;
int   hdr_data_idx;
int   hdr_data_offset;
//bool  use_var_id;
//StringArray obs_names;

char   hdr_typ_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN_L];
char   hdr_sid_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
char   hdr_vld_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];
float  hdr_arr_buf[OBS_BUFFER_SIZE][HDR_ARRAY_LEN];
float obs_data_buf[OBS_BUFFER_SIZE][OBS_ARRAY_LEN];
char  qty_data_buf[OBS_BUFFER_SIZE][HEADER_STR_LEN];

static struct NcHeaderArrays hdr_arrays;

static const string err_msg_message_type =
      "error writing the message type string to the netCDF file\n\n";
static const string err_msg_station_id =
      "error writing the station id string to the netCDF file\n\n";
static const string err_msg_valid_time =
      "error writing the valid time to the netCDF file\n\n";
static const string err_msg_hdr_arr =
      "error writing the header array to the netCDF file\n\n";


///////////////////////////////////////////////////////////////////////////////

static void write_netcdf_latlon_1d(NcFile *, NcDim *, NcDim *, const Grid &);
static void write_netcdf_latlon_2d(NcFile *, NcDim *, NcDim *, const Grid &);

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_global(NcFile * f_out, const char *file_name,
                         const char *program_name, const char *model_name,
                         const char *obtype, const char *desc)
{
   int yr, mon, day, hr, min, sec;
   char attribute_str[PATH_MAX];
   char hostname_str[max_str_len];
   char time_str[max_str_len];
   
   unix_to_mdyhms(time(NULL), mon, day, yr, hr, min, sec);
   sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);
   gethostname(hostname_str, max_str_len);
   sprintf(attribute_str,
           "File %s generated %s UTC on host %s by the MET %s tool",
           file_name, time_str, hostname_str, program_name);
   f_out->putAtt("FileOrigins", attribute_str);
   f_out->putAtt("MET_version", met_version);
   f_out->putAtt("MET_tool", program_name);
   if(model_name) f_out->putAtt("model",  model_name);
   if(obtype)     f_out->putAtt("obtype", obtype);
   if(desc)       f_out->putAtt("desc",   desc);
   
   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_proj(NcFile * f_out, const Grid & grid)
{

const GridInfo info = grid.info();

grid_output(info, f_out);

return;

}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                         const Grid &grid) {

   // Write 1-dimensional arrays for lat/lon grids and 2-d for all others
   if(grid.info().ll != 0) {
      write_netcdf_latlon_1d(f_out, lat_dim, lon_dim, grid);
   }
   else {
      write_netcdf_latlon_2d(f_out, lat_dim, lon_dim, grid);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_1d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   int i;
   double lat, lon;
   //NcVar *lat_var  = (NcVar *) 0;
   //NcVar *lon_var  = (NcVar *) 0;
   NcVar lat_var  ;
   NcVar lon_var  ;
   float *lat_data = (float *) 0;
   float *lon_data = (float *) 0;

   // Define Variables
   lat_var = f_out->addVar("lat", ncFloat, *lat_dim);
   lon_var = f_out->addVar("lon", ncFloat, *lon_dim);
   
   //lat_var = &lat_var_T;
   //lon_var = &lon_var_T;

   // Add variable attributes
   add_att(&lat_var, "long_name", "latitude");
   add_att(&lat_var, "units", "degrees_north");
   add_att(&lat_var, "standard_name", "latitude");

   add_att(&lon_var, "long_name", "longitude");
   add_att(&lon_var, "units", "degrees_east");
   add_att(&lon_var, "standard_name", "longitude");

   // Allocate space for lat/lon values
   lat_data = new float [grid.ny()];
   lon_data = new float [grid.nx()];

   // Compute latitude values
   for(i=0; i<grid.ny(); i++) {
      grid.xy_to_latlon(0, i, lat, lon);
      lat_data[i] = (float) lat;
   }

   // Compute longitude values (convert degrees west to east)
   for(i=0; i<grid.nx(); i++) {
      grid.xy_to_latlon(i, 0, lat, lon);
      lon_data[i] = (float) -1.0*lon;
   }

   // Write the lat data
   put_nc_data(&lat_var, &lat_data[0], lat_dim->getSize(), 0);

   // Write the lon data
   put_nc_data(&lon_var, &lon_data[0], lon_dim->getSize(), 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_latlon_2d(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                            const Grid &grid) {
   int i, x, y;
   double lat, lon;
   //NcVar *lat_var  = (NcVar *) 0;
   //NcVar *lon_var  = (NcVar *) 0;
   NcVar lat_var  ;
   NcVar lon_var  ;
   float *lat_data = (float *) 0;
   float *lon_data = (float *) 0;
   std::vector<NcDim> dims;
   long  counts[2] = {grid.ny(), grid.nx()};
   long offsets[2] = {0 , 0};

   // Define Variables
   dims.push_back(*lat_dim);
   dims.push_back(*lon_dim);
   lat_var = add_var(f_out, "lat", ncFloat, dims);
   lon_var = add_var(f_out, "lon", ncFloat, dims);

   // Add variable attributes
   add_att(&lat_var, "long_name", "latitude");
   add_att(&lat_var, "units", "degrees_north");
   add_att(&lat_var, "standard_name", "latitude");

   add_att(&lon_var, "long_name", "longitude");
   add_att(&lon_var, "units", "degrees_east");
   add_att(&lon_var, "standard_name", "longitude");

   // Allocate space for lat/lon values
   lat_data = new float [grid.nx()*grid.ny()];
   lon_data = new float [grid.nx()*grid.ny()];

   // Compute lat/lon values
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);
         i = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         // Multiple by -1.0 to convert from degrees west to degrees east
         lat_data[i] = (float) lat;
         lon_data[i] = (float) -1.0*lon;
      }
   }

   // Write the lat data
   put_nc_data(&lat_var, &lat_data[0], counts, offsets);

   // Write the lon data
   put_nc_data(&lon_var, &lon_data[0], counts, offsets);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_grid_weight(NcFile *f_out, NcDim *lat_dim, NcDim *lon_dim,
                              const GridWeightType t, const DataPlane &wgt_dp) {
   int i, x, y;
   //NcVar *wgt_var  = (NcVar *) 0;
   NcVar wgt_var  ;
   float *wgt_data = (float *) 0;
   std::vector<NcDim> dims;
   std::vector<size_t> count;

   // Define Variables
   dims.push_back(*lat_dim);
   dims.push_back(*lon_dim);
   wgt_var = add_var(f_out, "grid_weight", ncFloat, dims);

   // Add variable attributes
   add_att(&wgt_var, "standard_name", "weight");

   switch(t) {

      case GridWeightType_Cos_Lat:
         add_att(&wgt_var, "long_name", "cosine latitude grid weight");
         add_att(&wgt_var, "units", "NA");
         break;

      case GridWeightType_Area:
         add_att(&wgt_var, "long_name", "true area grid weight");
         add_att(&wgt_var, "units", "km^2");
         break;

      default:
         add_att(&wgt_var, "long_name", "default grid weight");
         add_att(&wgt_var, "units", "NA");
         break;
   }

   // Allocate space for weight values
   wgt_data = new float [wgt_dp.nx()*wgt_dp.ny()];

   // Store weight values
   for(x=0; x<wgt_dp.nx(); x++) {
      for(y=0; y<wgt_dp.ny(); y++) {
         i = DefaultTO.two_to_one(wgt_dp.nx(), wgt_dp.ny(), x, y);
         wgt_data[i] = (float) wgt_dp(x, y);
      }
   }

   // Write the weights
   count.push_back(wgt_dp.ny());
   count.push_back(wgt_dp.nx());
   put_nc_data_with_dims(&wgt_var, &wgt_data[0], wgt_dp.ny(), wgt_dp.nx());

   // Clean up
   if(wgt_data) { delete [] wgt_data; wgt_data = (float *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_var_times(NcVar *var, const DataPlane &dp) {

   write_netcdf_var_times(var, dp.init(), dp.valid(), dp.accum());

   return;
}

///////////////////////////////////////////////////////////////////////////////

void write_netcdf_var_times(NcVar *var, const unixtime init_ut,
                            const unixtime valid_ut, const int accum_sec)

{

char time_str[max_str_len];
ConcatString s;

   // Init time
   unix_to_yyyymmdd_hhmmss(init_ut, time_str);
   add_att(var, "init_time", time_str);

   s = unixtime_to_string(init_ut);

   add_att(var, "init_time_ut", s.text());

   // Valid time
   unix_to_yyyymmdd_hhmmss(valid_ut, time_str);
   add_att(var, "valid_time", time_str);

   s = unixtime_to_string(valid_ut);

   add_att(var, "valid_time_ut", s.text());

   // Accumulation time
   if(accum_sec != 0) {
      sec_to_hhmmss(accum_sec, time_str);
      add_att(var, "accum_time", time_str);
      var->putAtt("accum_time_sec", ncInt, accum_sec);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void add_nc_header (const char *hdr_typ, const char *hdr_sid, const char *hdr_vld,
      const float hdr_lat, const float hdr_lon, const float hdr_elv)
{
   int hdr_str_len;
   
   hdr_arrays.typ_sa.add(hdr_typ);  // Message type
   hdr_arrays.sid_sa.add(hdr_sid);  // Station ID
   hdr_arrays.vld_sa.add(hdr_vld);  // Valid time
   hdr_arrays.lat_na.add(hdr_lat);  // Latitude
   hdr_arrays.lon_na.add(hdr_lon);  // Longitude
   hdr_arrays.elv_na.add(hdr_elv);  // Elevation
   
   cur_hdr_idx++;
}
      
///////////////////////////////////////////////////////////////////////////////

void add_and_write_nc_observation(const NetcdfObsVars &obsVars,
      const float obs_arr[OBS_ARRAY_LEN], const char *obs_qty)
{
   int qty_len = strlen(obs_qty);
   int qty_len2 = strlen(qty_data_buf[obs_data_idx]);
   if (qty_len2 < qty_len) qty_len2 = qty_len;
   strncpy(qty_data_buf[obs_data_idx], obs_qty, qty_len);
   for (int idx=qty_len; idx<qty_len2; idx++)
      qty_data_buf[obs_data_idx][idx] = bad_data_char;
      
   for (int idx=0; idx<OBS_ARRAY_LEN; idx++) {
      obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
   }
   obs_data_idx++;
   cur_obs_idx++;
   
   if (obs_data_idx >= OBS_BUFFER_SIZE) {
      write_nc_obs_buffer(obsVars, OBS_BUFFER_SIZE);
   }
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_dimensions(NetcdfObsVars &obsVars, NcFile *f_out) {
   // Define netCDF dimensions
   if (IS_INVALID_NC(obsVars.strl_dim))    obsVars.strl_dim    = add_dim(f_out, nc_dim_mxstr,   (long)HEADER_STR_LEN);
   //if (IS_INVALID_NC(obsVars.strl_dim_l) && obsVars.str_len > 0) {
   //   obsVars.strl_dim_l = add_dim(f_out, nc_dim_mxstr,   (long)obsVars.str_len);
   //}
   //if (IS_INVALID_NC(obsVars.strl_dim_l))  obsVars.strl_dim_l = add_dim(f_out, nc_dim_mxstrl,   (long)HEADER_STR_LEN_L);
   if (IS_INVALID_NC(obsVars.hdr_arr_dim)) obsVars.hdr_arr_dim = add_dim(f_out, nc_dim_hdr_arr, (long)HDR_ARRAY_LEN);
   if (IS_INVALID_NC(obsVars.obs_arr_dim)) obsVars.obs_arr_dim = add_dim(f_out, nc_dim_obs_arr, (long)OBS_ARRAY_LEN);
   if (IS_INVALID_NC(obsVars.hdr_dim) && obsVars.hdr_len > 0) {
      obsVars.hdr_dim = add_dim(f_out, nc_dim_nhdr, (long)obsVars.hdr_len);
   }
   if (IS_INVALID_NC(obsVars.obs_dim))     obsVars.obs_dim     = add_dim(f_out, nc_dim_nobs);   // unlimited dimension;
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_hdr_vars (NetcdfObsVars &obsVars, NcFile *f_out,
      const int hdr_count, const int deflate_level) {
   
   // Define netCDF dimensions
   create_nc_dimensions(obsVars, f_out);
   
   NcDim hdr_dim = !IS_INVALID_NC(obsVars.hdr_dim)
         ? obsVars.hdr_dim
         : (hdr_count > 0)
               ? add_dim(f_out, nc_dim_nhdr, (long)hdr_count)
               : add_dim(f_out, nc_dim_nhdr)    // unlimited dimension
         ;
   if (IS_INVALID_NC(obsVars.hdr_dim)) obsVars.hdr_dim = hdr_dim;

   // Define netCDF header variables
   obsVars.hdr_typ_var = add_var(f_out, "hdr_typ", ncChar,  hdr_dim, obsVars.strl_dim,    deflate_level);
   obsVars.hdr_sid_var = add_var(f_out, "hdr_sid", ncChar,  hdr_dim, obsVars.strl_dim,    deflate_level);
   obsVars.hdr_vld_var = add_var(f_out, "hdr_vld", ncChar,  hdr_dim, obsVars.strl_dim,    deflate_level);
   obsVars.hdr_arr_var = add_var(f_out, "hdr_arr", ncFloat, hdr_dim, obsVars.hdr_arr_dim, deflate_level);

   add_att(&obsVars.hdr_typ_var, "long_name", "message type");
   add_att(&obsVars.hdr_sid_var, "long_name", "station identification");
   add_att(&obsVars.hdr_vld_var, "long_name", "valid time");
   add_att(&obsVars.hdr_vld_var, "units", "YYYYMMDD_HHMMSS");

   add_att(&obsVars.hdr_arr_var, "long_name",
           "array of observation station header values");
   add_att(&obsVars.hdr_arr_var, "missing_value", FILL_VALUE);
   add_att(&obsVars.hdr_arr_var, "_FillValue",    FILL_VALUE);
   add_att(&obsVars.hdr_arr_var, "columns", "lat lon elv");
   add_att(&obsVars.hdr_arr_var, "lat_long_name", "latitude");
   add_att(&obsVars.hdr_arr_var, "lat_units", "degrees_north");
   add_att(&obsVars.hdr_arr_var, "lon_long_name", "longitude");
   add_att(&obsVars.hdr_arr_var, "lon_units", "degrees_east");
   add_att(&obsVars.hdr_arr_var, "elv_long_name", "elevation");
   add_att(&obsVars.hdr_arr_var, "elv_units", "meters above sea level (msl)");
}

///////////////////////////////////////////////////////////////////////////////

void create_nc_obs_vars (NetcdfObsVars &obsVars, NcFile *f_out, const int deflate_level, bool use_var_id) {
   // Define netCDF dimensions
   create_nc_dimensions(obsVars, f_out);
   
   // Define netCDF variables
   obsVars.obs_qty_var = add_var(f_out, "obs_qty", ncChar,  obsVars.obs_dim, obsVars.strl_dim,    deflate_level);
   obsVars.obs_arr_var = add_var(f_out, "obs_arr", ncFloat, obsVars.obs_dim, obsVars.obs_arr_dim, deflate_level);

   add_att(f_out, nc_att_use_var_id, (use_var_id ? "true" : "false"));

   // Add variable attributes
   add_att(&obsVars.obs_qty_var, "long_name", "quality flag");
   add_att(&obsVars.obs_arr_var, "long_name", "array of observation values");
   add_att(&obsVars.obs_arr_var, "missing_value", FILL_VALUE);
   add_att(&obsVars.obs_arr_var, "_FillValue",    FILL_VALUE);
   add_att(&obsVars.obs_arr_var, "hdr_id_long_name", "index of matching header data");
   if (use_var_id) {
      add_att(&obsVars.obs_arr_var, "columns", "hdr_id var_id lvl hgt ob");
      add_att(&obsVars.obs_arr_var, "var_id_long_name", "index of BUFR variable corresponding to the observation type");
   }
   else {
      add_att(&obsVars.obs_arr_var, "columns", "hdr_id gc lvl hgt ob");
      add_att(&obsVars.obs_arr_var, "gc_long_name", "grib code corresponding to the observation type");
   }
   add_att(&obsVars.obs_arr_var, "lvl_long_name", "pressure level (hPa) or accumulation interval (sec)");
   add_att(&obsVars.obs_arr_var, "hgt_long_name", "height in meters above sea level (msl)");
   add_att(&obsVars.obs_arr_var, "ob_long_name", "observation value");
}

///////////////////////////////////////////////////////////////////////////////

int get_nc_header_index() {
   return cur_hdr_idx;
}

///////////////////////////////////////////////////////////////////////////////

int get_nc_hdr_buf_count() {
   return hdr_data_idx;
}

///////////////////////////////////////////////////////////////////////////////

int get_nc_obs_buf_count() {
   return obs_data_idx;
}

///////////////////////////////////////////////////////////////////////////////

void nc_obs_initialize() {
   obs_data_idx    = 0;
   obs_data_offset = 0;
   hdr_data_idx    = 0;
   hdr_data_offset = 0;
   
   hdr_arrays.typ_sa.clear();
   hdr_arrays.sid_sa.clear();
   hdr_arrays.vld_sa.clear();
   hdr_arrays.lat_na.clear();
   hdr_arrays.lon_na.clear();
   hdr_arrays.elv_na.clear();

}

///////////////////////////////////////////////////////////////////////////////

void reset_header_buffer(int buf_size) {
   for (int i=0; i<buf_size; i++) {
      for (int j=0; j<HEADER_STR_LEN; j++) {
         hdr_typ_buf[i][j] = bad_data_char;
         hdr_sid_buf[i][j] = bad_data_char;
         hdr_vld_buf[i][j] = bad_data_char;
      }
      for (int j=0; j<HDR_ARRAY_LEN; j++) {
         hdr_arr_buf[i][j] = FILL_VALUE;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_headers(const NetcdfObsVars &obsVars)
{
   int hdr_str_len, hdr_str_len2;
   int buf_size = (cur_hdr_idx > OBS_BUFFER_SIZE) ? OBS_BUFFER_SIZE : cur_hdr_idx;

   hdr_data_idx = 0;
   for (int index=0; index<cur_hdr_idx; index++) {
      // PrepBufr Message type
      hdr_str_len  = strlen(hdr_arrays.typ_sa[index]);
      hdr_str_len2 = strlen(hdr_typ_buf[hdr_data_idx]);
      if (hdr_str_len > HEADER_STR_LEN) hdr_str_len = HEADER_STR_LEN;
      if (hdr_str_len2 < hdr_str_len) hdr_str_len2 = hdr_str_len;
      strncpy(hdr_typ_buf[hdr_data_idx], hdr_arrays.typ_sa[index], hdr_str_len);
      for (int idx=hdr_str_len; idx<hdr_str_len2; idx++)
         hdr_typ_buf[hdr_data_idx][idx] = bad_data_char;
      
      // Station ID
      hdr_str_len = strlen(hdr_arrays.sid_sa[index]);
      hdr_str_len2 = strlen(hdr_sid_buf[hdr_data_idx]);
      if (hdr_str_len > HEADER_STR_LEN) hdr_str_len = HEADER_STR_LEN;
      if (hdr_str_len2 < hdr_str_len) hdr_str_len2 = hdr_str_len;
      strncpy(hdr_sid_buf[hdr_data_idx], hdr_arrays.sid_sa[index], hdr_str_len);
      for (int idx=hdr_str_len; idx<hdr_str_len2; idx++)
         hdr_sid_buf[hdr_data_idx][idx] = bad_data_char;
      
      // Valid Time
      hdr_str_len = strlen(hdr_arrays.vld_sa[index]);
      hdr_str_len2 = strlen(hdr_vld_buf[hdr_data_idx]);
      if (hdr_str_len > HEADER_STR_LEN) hdr_str_len = HEADER_STR_LEN;
      if (hdr_str_len2 < hdr_str_len) hdr_str_len2 = hdr_str_len;
      strncpy(hdr_vld_buf[hdr_data_idx], hdr_arrays.vld_sa[index], hdr_str_len);
      for (int idx=hdr_str_len; idx<hdr_str_len2; idx++)
         hdr_vld_buf[hdr_data_idx][idx] = bad_data_char;
      
      // Write the header array which consists of the following:
      //    LAT LON ELV
      hdr_arr_buf[hdr_data_idx][0] = (float) hdr_arrays.lat_na[index];
      hdr_arr_buf[hdr_data_idx][1] = (float) hdr_arrays.lon_na[index];
      hdr_arr_buf[hdr_data_idx][2] = (float) hdr_arrays.elv_na[index];
      
      hdr_data_idx++;
      
      if (hdr_data_idx >= buf_size) {
         write_nc_header_buffer(obsVars, buf_size);
      }
   }

   if (hdr_data_idx > 0) {
      write_nc_header_buffer(obsVars, hdr_data_idx);
   }

}

///////////////////////////////////////////////////////////////////////////////

void write_nc_header_buffer(const NetcdfObsVars &obsVars, const int buf_size)
{
   long offsets[2] = { hdr_data_offset, 0 };
   long lengths[2] = { buf_size, HEADER_STR_LEN } ;
   
   lengths[1] = HEADER_STR_LEN;
   if(!put_nc_data((NcVar *)&obsVars.hdr_typ_var, (char *)hdr_typ_buf[0], lengths, offsets)) {
      mlog << Error << err_msg_message_type;
      exit(1);
   }
   
   // Station ID
   if(!put_nc_data((NcVar *)&obsVars.hdr_sid_var, (char *)hdr_sid_buf[0], lengths, offsets)) {
      mlog << Error << err_msg_station_id;
      exit(1);
   }
   // Valid Time
   if(!put_nc_data((NcVar *)&obsVars.hdr_vld_var, (char *)hdr_vld_buf[0], lengths, offsets)) {
      mlog << Error << err_msg_valid_time;
      exit(1);
   }
   
   // Write the header array which consists of the following:
   //    LAT LON ELV
   
   lengths[1] = HDR_ARRAY_LEN;
   if(!put_nc_data((NcVar *)&obsVars.hdr_arr_var, (float *)hdr_arr_buf[0], lengths, offsets)) {
      mlog << Error << err_msg_hdr_arr;
      exit(1);
   }
   
   //for(int hi=0; hi<buf_size; hi++) {
   //   for(int hj=0; hj<HEADER_STR_LEN; hj++) {
   //      hdr_typ_buf[hi][hj] = bad_data_char;
   //      hdr_sid_buf[hi][hj] = bad_data_char;
   //   }
   //}
   
   hdr_data_offset += buf_size;
   hdr_data_idx = 0;
}

///////////////////////////////////////////////////////////////////////////////

void write_nc_obs_buffer(const NetcdfObsVars &obsVars, const int buf_size)
{
   const string method_name = "write_nc_obs_buffer()";
   
   long offsets[2] = { obs_data_offset, 0 };
   long lengths[2] = { buf_size, 1} ;

   lengths[1] = HEADER_STR_LEN;
   if(!put_nc_data((NcVar *)&obsVars.obs_qty_var, (char*)qty_data_buf[0], lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the quality flag to the "
           << "netCDF file\n\n";
      exit(1);
   }
   lengths[1] = OBS_ARRAY_LEN;
   if(!put_nc_data((NcVar *)&obsVars.obs_arr_var, (float*)obs_data_buf[0], lengths, offsets)) {
      mlog << Error << "\n" << method_name << " -> "
           << "error writing the observation array to the "
           << "netCDF file\n\n";
      exit(1);
   }
   
   obs_data_offset += buf_size;
   obs_data_idx = 0;
}
      
///////////////////////////////////////////////////////////////////////////////

bool write_nc_observations(const NetcdfObsVars &obsVars,
                           const vector< Observation > observations)
{
   int prev_hdr_idx = -1;
   //float obs_arr[obs_arr_len];
   
   int obs_buf_size = observations.size();
   if (obs_buf_size > OBS_BUFFER_SIZE) obs_buf_size = OBS_BUFFER_SIZE;
   
   obs_data_idx = 0;
   obs_data_offset = 0;
   hdr_data_idx = 0;
   hdr_data_offset = 0;
   
   int processed_count =0;
   float obs_arr[OBS_ARRAY_LEN];
   for (vector< Observation >::const_iterator obs = observations.begin();
        obs != observations.end(); ++obs)
   {
      processed_count++;
      
      if (obs->getHeaderIndex() != prev_hdr_idx) {
         prev_hdr_idx = obs->getHeaderIndex();
         add_nc_header(obs->getHeaderType().c_str(),
                       obs->getStationId().c_str(),
                       obs->getValidTimeString().c_str(),
                       obs->getLatitude(),
                       obs->getLongitude(),
                       obs->getElevation());
      }
      
      obs_arr[0] = obs->getHeaderIndex();
      obs_arr[1] = obs->getVarCode();
      obs_arr[2] = obs->getPressureLevel();
      obs_arr[3] = obs->getHeight();
      obs_arr[4] = obs->getValue();
      add_and_write_nc_observation(obsVars, obs_arr, obs->getQualityFlag().c_str());
      
      if (obs_data_idx >= obs_buf_size) {
         write_nc_obs_buffer(obsVars, obs_buf_size);
      }
      
   } /* endfor - obs */
   
   if (obs_data_idx > 0) {
      write_nc_obs_buffer(obsVars, obs_data_idx);
   }

   write_nc_headers(obsVars);
   
   return true;
}
