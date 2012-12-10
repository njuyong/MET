// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "engine.h"
#include "mode_columns.h"
#include "vx_util.h"

///////////////////////////////////////////////////////////////////////

static inline int max(int a, int b) { return((a > b) ? a : b); }

static inline double area_ratio_conf(double t) { return(t); }

///////////////////////////////////////////////////////////////////////
//
// Code for class ModeFuzzyEngine
//
///////////////////////////////////////////////////////////////////////

ModeFuzzyEngine::ModeFuzzyEngine() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////

ModeFuzzyEngine::~ModeFuzzyEngine() {

   clear_features();

   //
   // Clear single features
   //
   if(fcst_single) {
      delete [] fcst_single;
      fcst_single = (SingleFeature *) 0;
   }
   if(obs_single) {
      delete [] obs_single;
      obs_single = (SingleFeature *) 0;
   }
   if(fcst_clus) {
      delete [] fcst_clus;
      fcst_clus = (SingleFeature *) 0;
   }
   if(obs_clus) {
      delete [] obs_clus;
      obs_clus = (SingleFeature *) 0;
   }

   //
   // Clear pair features
   //
   if(pair) {
      delete [] pair;
      pair = (PairFeature *) 0;
   }
   if(pair_clus) {
      delete [] pair_clus;
      pair_clus = (PairFeature *) 0;
   }

   //
   // Clear fcst ShapeData objects
   //
   if(fcst_raw) {
      delete fcst_raw;
      fcst_raw = (ShapeData *) 0;
   }
   if(fcst_filter) {
      delete fcst_filter;
      fcst_filter = (ShapeData *) 0;
   }
   if(fcst_thresh) {
      delete fcst_thresh;
      fcst_thresh = (ShapeData *) 0;
   }
   if(fcst_conv) {
      delete fcst_conv;
      fcst_conv = (ShapeData *) 0;
   }
   if(fcst_mask) {
      delete fcst_mask;
      fcst_mask = (ShapeData *) 0;
   }
   if(fcst_split) {
      delete fcst_split;
      fcst_split = (ShapeData *) 0;
   }
   if(fcst_clus_split) {
      delete fcst_clus_split;
      fcst_clus_split = (ShapeData *) 0;
   }

   //
   // Clear obs ShapeData objects
   //
   if(obs_raw) {
      delete obs_raw;
      obs_raw = (ShapeData *) 0;
   }
   if(obs_filter) {
      delete obs_filter;
      obs_filter = (ShapeData *) 0;
   }
   if(obs_thresh) {
      delete obs_thresh;
      obs_thresh = (ShapeData *) 0;
   }
   if(obs_conv) {
      delete obs_conv;
      obs_conv = (ShapeData *) 0;
   }
   if(obs_mask) {
      delete obs_mask;
      obs_mask = (ShapeData *) 0;
   }
   if(obs_split) {
      delete obs_split;
      obs_split = (ShapeData *) 0;
   }
   if(obs_clus_split) {
      delete obs_clus_split;
      obs_clus_split = (ShapeData *) 0;
   }

   //
   // Clear fcst and obs engines
   //
   if(fcst_engine) {
      delete fcst_engine;
      fcst_engine = (ModeFuzzyEngine *) 0;
   }
   if(obs_engine) {
      delete obs_engine;
      obs_engine = (ModeFuzzyEngine *) 0;
   }
}

///////////////////////////////////////////////////////////////////////

ModeFuzzyEngine::ModeFuzzyEngine(const ModeFuzzyEngine &eng) {

   mlog << Error << "\nModeFuzzyEngine::ModeFuzzyEngine(const ModeFuzzyEngine &) -> "
        << "should never be called!\n\n";
   exit(1);
}

///////////////////////////////////////////////////////////////////////

ModeFuzzyEngine & ModeFuzzyEngine::operator=(const ModeFuzzyEngine & eng) {

   mlog << Error << "\nModeFuzzyEngine::operator=(const ModeFuzzyEngine &) -> "
        << "should never be called\n\n";
   exit(1);
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::init_from_scratch() {

   //
   // Reset all fcst and obs processing flags to initial state
   //
   need_fcst_filter     = true;
   need_fcst_conv       = true;
   need_fcst_thresh     = true;
   need_fcst_split      = true;
   need_fcst_merge      = true;
   need_fcst_clus_split = true;

   need_obs_filter      = true;
   need_obs_conv        = true;
   need_obs_thresh      = true;
   need_obs_split       = true;
   need_obs_merge       = true;
   need_obs_clus_split  = true;

   need_match           = true;

   fcst_raw         = new ShapeData;
   fcst_filter      = new ShapeData;
   fcst_thresh      = new ShapeData;
   fcst_conv        = new ShapeData;
   fcst_mask        = new ShapeData;
   fcst_split       = new ShapeData;
   fcst_clus_split  = new ShapeData;

   obs_raw          = new ShapeData;
   obs_filter       = new ShapeData;
   obs_thresh       = new ShapeData;
   obs_conv         = new ShapeData;
   obs_mask         = new ShapeData;
   obs_split        = new ShapeData;
   obs_clus_split   = new ShapeData;

   fcst_engine      = (ModeFuzzyEngine *) 0;
   obs_engine       = (ModeFuzzyEngine *) 0;

   n_fcst           = 0;
   n_obs            = 0;
   n_clus           = 0;

   fcst_single      = (SingleFeature *) 0;
   obs_single       = (SingleFeature *) 0;
   pair             = (PairFeature *)   0;

   fcst_clus        = (SingleFeature *) 0;
   obs_clus         = (SingleFeature *) 0;
   pair_clus        = (PairFeature *)   0;

   fcst_single      = new SingleFeature [max_singles];
   obs_single       = new SingleFeature [max_singles];
   pair             = new PairFeature   [max_singles*max_singles];

   fcst_clus        = new SingleFeature [max_singles];
   obs_clus         = new SingleFeature [max_singles];
   pair_clus        = new PairFeature   [max_singles];

   if(!fcst_single || !obs_single || !pair ||
      !fcst_clus   || !obs_clus   || !pair_clus) {
      mlog << Error << "\nModeFuzzyEngine::init_from_scratch() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   collection.clear();

   clear_features();

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::clear_features() {
   int j;

   for(j=0; j<max_singles; j++) {
      fcst_single[j].clear();
      obs_single[j].clear();
      fcst_clus[j].clear();
      obs_clus[j].clear();
      pair_clus[j].clear();
   }

   for(j=0; j<(max_singles*max_singles); j++) {
      pair[j].clear();
   }

   n_fcst = 0;
   n_obs  = 0;
   n_clus = 0;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::clear_colors() {
   int j;

   for(j=0; j<max_singles; j++) {
      fcst_color[j].clear();
      obs_color[j].clear();
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::set(const ShapeData &fcst_wd, const ShapeData &obs_wd) 

{

   ConcatString path;

   clear_features();
   clear_colors();
   ctable.clear();

   set_fcst(fcst_wd);
   set_obs(obs_wd);

   path = replace_path(conf_info.object_pi.color_table);

   ctable.read(path);

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::set_fcst(const ShapeData &fcst_wd) {

   *fcst_raw = fcst_wd;

   need_fcst_filter     = true;
   need_fcst_conv       = true;
   need_fcst_thresh     = true;
   need_fcst_split      = true;
   need_fcst_merge      = true;
   need_fcst_clus_split = true;
   need_match           = true;

   int k = conf_info.zero_border_size;
   fcst_raw->zero_border(k, bad_data_double);

   do_fcst_filter();
   do_fcst_convolution();
   do_fcst_thresholding();
   do_fcst_splitting();

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::set_obs(const ShapeData &obs_wd) {

   *obs_raw = obs_wd;

   need_obs_filter     = true;
   need_obs_conv       = true;
   need_obs_thresh     = true;
   need_obs_split      = true;
   need_obs_merge      = true;
   need_obs_clus_split = true;
   need_match          = true;

   int k = conf_info.zero_border_size;
   obs_raw->zero_border(k, bad_data_double);

   do_obs_filter();
   do_obs_convolution();
   do_obs_thresholding();
   do_obs_splitting();

   return;
}


///////////////////////////////////////////////////////////////////////


int ModeFuzzyEngine::two_to_one(int n_f, int n_o) const {
   int n;

   n = n_o*n_fcst + n_f;

   return(n);
}

///////////////////////////////////////////////////////////////////////


void ModeFuzzyEngine::do_fcst_filter() {
   SingleThresh st;

   if(!need_fcst_filter) return;

   *fcst_filter = *fcst_raw;

   //
   // Filter out the data which doesn't meet the fcst_raw_thresh
   //
   fcst_filter->filter(conf_info.fcst_raw_thresh);

   //
   // Threshold the fcst_filter field applying the fcst_conv_thresh
   //
   *fcst_thresh = *fcst_filter;
   fcst_thresh->threshold(conf_info.fcst_conv_thresh);

   need_fcst_filter     = false;
   need_fcst_conv       = true;
   need_fcst_thresh     = true;
   need_fcst_split      = true;
   need_fcst_merge      = true;
   need_fcst_clus_split = true;
   need_match           = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_filter() {
   SingleThresh st;

   if(!need_obs_filter) return;

   *obs_filter = *obs_raw;

   //
   // Filter out the data which doesn't meet the obs_raw_thresh
   //
   obs_filter->filter(conf_info.obs_raw_thresh);

   //
   // Threshold the obs_filter field applying the obs_conv_thresh
   //
   *obs_thresh = *obs_filter;
   obs_thresh->threshold(conf_info.obs_conv_thresh);

   need_obs_filter     = false;
   need_obs_conv       = true;
   need_obs_thresh     = true;
   need_obs_split      = true;
   need_obs_merge      = true;
   need_obs_clus_split = true;
   need_match          = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_convolution() {
   int r;

   if(need_fcst_filter) do_fcst_filter();

   if(!need_fcst_conv) return;

   r = conf_info.fcst_conv_radius;

   *fcst_conv = *fcst_filter;

   //
   // Apply a circular convolution to the filtered field
   //
   if(r > 0) fcst_conv->conv_filter_circ(2*r + 1, conf_info.fcst_vld_thresh);

   fcst_conv->zero_border(conf_info.zero_border_size, bad_data_double);

   need_fcst_conv       = false;
   need_fcst_thresh     = true;
   need_fcst_split      = true;
   need_fcst_merge      = true;
   need_fcst_clus_split = true;
   need_match           = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_convolution() {
   int r;

   if(need_obs_filter) do_obs_filter();

   if(!need_obs_conv) return;

   r = conf_info.obs_conv_radius;

   *obs_conv = *obs_filter;

   //
   // Apply a circular convolution to the filtered field
   //
   if(r > 0) obs_conv->conv_filter_circ(2*r + 1, conf_info.obs_vld_thresh);

   obs_conv->zero_border(conf_info.zero_border_size, bad_data_double);

   need_obs_conv       = false;
   need_obs_thresh     = true;
   need_obs_split      = true;
   need_obs_merge      = true;
   need_obs_clus_split = true;
   need_match          = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_thresholding() {

   if(need_fcst_conv) do_fcst_convolution();

   if(!need_fcst_thresh) return;

   *fcst_mask = *fcst_conv;

   //
   // Threshold the convolved field
   //
   fcst_mask->threshold(conf_info.fcst_conv_thresh);

   //
   // Apply the area threshold
   //
   fcst_mask->threshold_area(conf_info.fcst_area_thresh);

   //
   // Apply the intensity threshold
   //
   fcst_mask->threshold_intensity(fcst_filter,
                                  conf_info.fcst_inten_perc_value,
                                  conf_info.fcst_inten_perc_thresh);

   need_fcst_thresh     = false;
   need_fcst_split      = true;
   need_fcst_merge      = true;
   need_fcst_clus_split = true;
   need_match           = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_thresholding() {

   if(need_obs_conv) do_obs_convolution();

   if(!need_obs_thresh) return;

   *obs_mask = *obs_conv;

   //
   // Threshold the convolved field
   //
   obs_mask->threshold(conf_info.obs_conv_thresh);

   //
   // Apply the area threshold
   //
   obs_mask->threshold_area(conf_info.obs_area_thresh);

   //
   // Apply the intensity threshold
   //
   obs_mask->threshold_intensity(obs_filter,
                                 conf_info.obs_inten_perc_value,
                                 conf_info.obs_inten_perc_thresh);

   need_obs_thresh     = false;
   need_obs_split      = true;
   need_obs_merge      = true;
   need_obs_clus_split = true;
   need_match          = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_splitting() {

   if(need_fcst_thresh) do_fcst_thresholding();

   if(!need_fcst_split) return;

   *fcst_split = split(*fcst_mask, n_fcst);

   need_fcst_split      = false;
   need_fcst_merge      = true;
   need_fcst_clus_split = true;
   need_match           = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_splitting() {

   if(need_obs_thresh) do_obs_thresholding();

   if(!need_obs_split) return;

   *obs_split = split(*obs_mask, n_obs);

   need_obs_split      = false;
   need_obs_merge      = true;
   need_obs_clus_split = true;
   need_match          = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_merging() {

   do_fcst_merging("", "");

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_merging() {

   do_obs_merging("", "");

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_merging(const char *default_config,
                             const char *merge_config) {

   if(need_fcst_thresh) do_fcst_thresholding();

   if(!need_fcst_merge) return;

   if(conf_info.fcst_merge_flag == MergeType_Both ||
      conf_info.fcst_merge_flag == MergeType_Thresh)
      do_fcst_merge_thresh();

   if(conf_info.fcst_merge_flag == MergeType_Both ||
      conf_info.fcst_merge_flag == MergeType_Engine)
      do_fcst_merge_engine(default_config, merge_config);

   //
   // Done
   //

   need_fcst_merge      = false;
   need_fcst_clus_split = true;
   need_match           = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_merging(const char *default_config,
                            const char *merge_config) {

   if(need_obs_thresh) do_obs_thresholding();

   if(!need_obs_merge) return;

   if(conf_info.obs_merge_flag == MergeType_Both ||
      conf_info.obs_merge_flag == MergeType_Thresh)
      do_obs_merge_thresh();

   if(conf_info.obs_merge_flag == MergeType_Both ||
      conf_info.obs_merge_flag == MergeType_Engine)
      do_obs_merge_engine(default_config, merge_config);

   //
   // Done
   //

   need_obs_merge      = false;
   need_obs_clus_split = true;
   need_match          = true;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_matching() {

   if(!need_match) return;

   if(conf_info.match_flag == MatchType_None) {
      mlog << Warning << "\nModeFuzzyEngine::do_matching() -> "
           << "no matching requested in configuration file\n";
      do_no_match();
   }
   else if(conf_info.match_flag == MatchType_MergeBoth) {
      do_match_merge();
   }
   else if(conf_info.match_flag == MatchType_MergeFcst) {
      do_match_fcst_merge();
   }
   else if(conf_info.match_flag == MatchType_NoMerge) {
      do_match_only();
   }
   else {
      mlog << Error << "\nModeFuzzyEngine::do_matching() -> "
           << "invalid match_flag value specified.  match_flag must be "
           << "between 0 and 3.\n\n";
      exit(1);
   }

   //
   // Generate the fcst and obs composite split fields
   //
   do_fcst_clus_splitting();
   do_obs_clus_splitting();

   //
   // Compute the cluster single and pair features
   //
   do_cluster_features();

   //
   // Done
   //

   need_match           = false;
   need_fcst_clus_split = false;
   need_obs_clus_split  = false;

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Perform no matching, but still define the single features.
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_no_match() {
   int j;
   ShapeData * fcst_shape = (ShapeData *) 0;
   ShapeData * obs_shape = (ShapeData *) 0;

   do_fcst_splitting();
   do_obs_splitting();

   if((n_fcst >= max_singles) || (n_obs >= max_singles)) {

      mlog << Error << "\nModeFuzzyEngine::do_no_match() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << max(n_fcst, n_obs) << "\n\n";
      exit(1);
   }

   clear_colors();

   fcst_shape = new ShapeData [n_fcst];
   obs_shape  = new ShapeData [n_obs];

   if(!fcst_shape || !obs_shape) {

      mlog << Error << "\nModeFuzzyEngine::do_no_match() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Do the single features
   //

   for(j=0; j<n_fcst; j++) {
      fcst_shape[j] = select(*fcst_split, j+1);
      fcst_single[j].set(*fcst_filter, *fcst_thresh, fcst_shape[j],
                         conf_info.inten_perc_value);
      fcst_single[j].object_number = j+1;
   }

   for(j=0; j<n_obs; j++) {
      obs_shape[j] = select(*obs_split, j+1);
      obs_single[j].set(*obs_filter, *obs_thresh, obs_shape[j],
                        conf_info.inten_perc_value);
      obs_single[j].object_number = j+1;
   }

   //
   // Clear out any empty sets
   //
   collection.clear_empty_sets();

   //
   // Done
   //
   delete [] fcst_shape; fcst_shape = (ShapeData *) 0;
   delete [] obs_shape;  obs_shape  = (ShapeData *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_match_merge() {
   int j, k, n;
   InterestInfo junkinfo;
   ShapeData * fcst_shape = (ShapeData *) 0;
   ShapeData * obs_shape = (ShapeData *) 0;

   do_fcst_splitting();
   do_obs_splitting();

   if((n_fcst >= max_singles) || (n_obs >= max_singles)) {

      mlog << Error << "\nModeFuzzyEngine::do_match_merge() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << max(n_fcst, n_obs) << "\n\n";
      exit(1);
   }

   clear_colors();

   fcst_shape = new ShapeData [n_fcst];
   obs_shape = new ShapeData [n_obs];

   if(!fcst_shape || !obs_shape) {

      mlog << Error << "\nModeFuzzyEngine::do_match_merge() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Do the single features
   //

   for(j=0; j<n_fcst; j++) {
      fcst_shape[j] = select(*fcst_split, j+1);
      fcst_single[j].set(*fcst_filter, *fcst_thresh, fcst_shape[j],
                         conf_info.inten_perc_value);
      fcst_single[j].object_number = j+1;
   }

   for(j=0; j<n_obs; j++) {
      obs_shape[j] = select(*obs_split, j+1);
      obs_single[j].set(*obs_filter, *obs_thresh, obs_shape[j],
                        conf_info.inten_perc_value);
      obs_single[j].object_number = j+1;
   }

   //
   // Do the pair features
   //
   for(j=0; j<n_fcst; j++) {
      for(k=0; k<n_obs; k++) {

         n = two_to_one(j, k);

         pair[n].set(fcst_single[j], obs_single[k],
                     conf_info.max_centroid_dist);
         pair[n].pair_number = n;
      }
   }

   //
   // Calculate the interest values
   //
   for(j=0; j<n_fcst; j++) {
      for(k=0; k<n_obs; k++) {

         n = two_to_one(j, k);

         info[n].fcst_number    = (j+1);
         info[n].obs_number     = (k+1);
         info[n].pair_number    = n;
         info[n].interest_value = total_interest(conf_info, 1, pair[n]);
      }
   }

   //
   // Sort the interest values in decreasing order
   //
   n = n_fcst*n_obs;

   for(j=0; j<(n-1); j++) {
      for(k=(j+1); k<n; k++) {

         if(info[j].interest_value < info[k].interest_value) {

            junkinfo = info[j];
            info[j] = info[k];
            info[k] = junkinfo;
         }
      }
   }

   //
   // Form the sets
   //
   n = n_fcst*n_obs;

   for(j=0; j<n; j++) {

      if(info[j].interest_value < conf_info.total_interest_thresh)
         continue;

      collection.add_pair(info[j].fcst_number, info[j].obs_number);
   }

   //
   // Clear out any empty sets
   //
   collection.clear_empty_sets();

   //
   // Assign the colors
   //
   if(collection.n_sets > ctable.n_entries()) {

      mlog << Error << "\nModeFuzzyEngine::do_match_merge() -> "
           << "not enough colors ... need at least " << (collection.n_sets)
           << "\n\n";
      exit(1);
   }

   for(j=0; j<n_fcst; j++) {
      fcst_color[j] = unmatched_color;

      for(k=0; k<(collection.n_sets); k++) {

         if(collection.set[k].has_fcst(j+1)) {
            fcst_color[j] = ctable.nearest(k+1);
            break;
         }
      }
   }

   for(j=0; j<n_obs; j++) {
      obs_color[j] = unmatched_color;

      for(k=0; k<(collection.n_sets); k++) {

         if(collection.set[k].has_obs(j+1)) {
            obs_color[j] = ctable.nearest(k+1);
            break;
         }
      }
  }

   //
   // Done
   //

   delete [] fcst_shape; fcst_shape = (ShapeData *) 0;
   delete [] obs_shape;  obs_shape = (ShapeData *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////
//
// perform merging of the forecast field based on a different convolution
// threshold.  It is the user's responsibility to choose a threshold type and
// value to define objects which are suitable for merging.
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_merge_thresh() {
   int j, k, x, y;
   int n_fcst_merge, intersection;
   int count, first_k;
   ShapeData fcst_merge_mask, fcst_merge_split;
   ShapeData * fcst_shape = (ShapeData *) 0;
   ShapeData * fcst_merge_shape = (ShapeData *) 0;

   do_fcst_splitting();

   if(n_fcst >= max_singles) {

      mlog << Error << "\nModeFuzzyEngine::do_fcst_merge_thresh() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << n_fcst << "\n\n";
      exit(1);
   }

   //
   // Define the forecast merge field by applying the specified threshold
   // to the convolved field
   //
   fcst_merge_mask = *fcst_conv;

   //
   // Threshold the forecast merge field
   //
   fcst_merge_mask.threshold(conf_info.fcst_merge_thresh);

   //
   // Split up the forecast merge field
   //
   fcst_merge_split = split(fcst_merge_mask, n_fcst_merge);

   //
   // Allocate space for all of the simple forecast shapes and
   // forecast merge shapes
   //
   fcst_shape       = new ShapeData [n_fcst];
   fcst_merge_shape = new ShapeData [n_fcst_merge];

   if(!fcst_shape || !fcst_merge_shape) {

      mlog << Error << "\nModeFuzzyEngine::do_fcst_merge_thresh() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Select all of the simple forecast shapes and
   // forecast merge shapes
   //
   for(j=0; j<n_fcst; j++) {
      fcst_shape[j] = select(*fcst_split, j+1);
   }

   for(j=0; j<n_fcst_merge; j++) {
      fcst_merge_shape[j] = select(fcst_merge_split, j+1);
   }

   //
   // Calculate the composite object sets
   //
   for(j=0; j<n_fcst_merge; j++) {

      count = first_k = 0;

      for(k=0; k<n_fcst; k++) {

         //
         // Calculate intersection area
         //
         intersection = 0;

         for(x=0; x<fcst_merge_mask.data.nx(); x++) {
            for(y=0; y<fcst_merge_mask.data.ny(); y++) {

               if(fcst_merge_shape[j].f_is_on(x, y) &&
                  fcst_shape[k].f_is_on(x, y)) intersection++;
            }
         }

         //
         // Add to set collection only if the fcst object is
         // completely contained in the merge object.  Meaning,
         // intersection area >= fcst area
         //

         if(intersection >= fcst_shape[k].area()) {

            //
            // Keep track of the first embedded shape.  You only want to
            // create a composite if there are more than 1 shapes in it.
            //
            if(count == 0) first_k = k+1;

            else if(count == 1) {
               collection.set[collection.n_sets].add_pair(first_k, -1);
               collection.set[collection.n_sets].add_pair(k+1, -1);
            }
            else {
               collection.set[collection.n_sets].add_pair(k+1, -1);
            }

            count++;
         }
      }  // end for k

      if(count > 0) collection.n_sets++;
   }  // end for j

   //
   // Done
   //

   delete [] fcst_shape;       fcst_shape = (ShapeData *) 0;
   delete [] fcst_merge_shape; fcst_merge_shape = (ShapeData *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////
//
// perform merging of the observation field based on a different convolution
// threshold.  It is the user's responsibility to choose a threshold type and
// value to define objects which are suitable for merging.
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_merge_thresh() {
   int j, k, x, y;
   int n_obs_merge, intersection;
   int count, first_k;
   ShapeData obs_merge_mask, obs_merge_split;
   ShapeData * obs_shape = (ShapeData *) 0;
   ShapeData * obs_merge_shape = (ShapeData *) 0;

   do_obs_splitting();

   if(n_obs >= max_singles) {

      mlog << Error << "\nModeFuzzyEngine::do_obs_merge_thresh() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << n_obs << "\n\n";
      exit(1);
   }

   //
   // Define the forecast merge field by applying the specified threshold
   // to the convolved field
   //
   obs_merge_mask = *obs_conv;

   //
   // Threshold the forecast merge field
   //
   obs_merge_mask.threshold(conf_info.obs_merge_thresh);

   //
   // Split up the forecast merge field
   //
   obs_merge_split = split(obs_merge_mask, n_obs_merge);

   //
   // Allocate space for all of the simple observation shapes and
   // observation merge shapes
   //
   obs_shape       = new ShapeData [n_obs];
   obs_merge_shape = new ShapeData [n_obs_merge];

   if(!obs_shape || !obs_merge_shape) {

      mlog << Error << "\nModeFuzzyEngine::do_obs_merge_thresh() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Select all of the simple observation shapes and
   // simple merge shapes
   //
   for(j=0; j<n_obs; j++) {
      obs_shape[j] = select(*obs_split, j+1);
   }

   for(j=0; j<n_obs_merge; j++) {
      obs_merge_shape[j] = select(obs_merge_split, j+1);
   }

   //
   // Calculate the composite object sets
   //
   for(j=0; j<n_obs_merge; j++) {

      count = first_k = 0;

      for(k=0; k<n_obs; k++) {

         //
         // Calculate intersection area
         //
         intersection = 0;

         for(x=0; x<obs_merge_mask.data.nx(); x++) {
            for(y=0; y<obs_merge_mask.data.ny(); y++) {

               if(obs_merge_shape[j].f_is_on(x, y) &&
                  obs_shape[k].f_is_on(x, y)) intersection++;
            }
         }

         //
         // Add to set collection only if the obs object is
         // completely contained in the merge object.  Meaning,
         // intersection area >= obs area
         //

         if(intersection >= obs_shape[k].area()) {

            //
            // Keep track of the first embedded shape.  You only want to
            // create a composite if there are more than 1 shapes in it.
            //
            if(count == 0) first_k = k+1;

            else if(count == 1) {
               collection.set[collection.n_sets].add_pair(-1, first_k);
               collection.set[collection.n_sets].add_pair(-1, k+1);
            }
            else {
               collection.set[collection.n_sets].add_pair(-1, k+1);
            }

            count++;
         }
      }  // end for k

      if(count > 0) collection.n_sets++;
   }  // end for j

   //
   // Done
   //

   delete [] obs_shape;       obs_shape = (ShapeData *) 0;
   delete [] obs_merge_shape; obs_merge_shape = (ShapeData *) 0;

   return;
}

   ///////////////////////////////////////////////////////////////////////
   //
   // Perform merging of the forecast field using a fuzzy engine matching
   // approach
   //
   ///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_merge_engine(const char *default_config, const char *merge_config)

{

   int i, j;
   ShapeData fcst_merge_split;
   ConcatString path;

   do_fcst_splitting();

   if(n_fcst >= max_singles) {
      mlog << Error << "\nModeFuzzyEngine::do_fcst_merge_engine() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << n_fcst << "\n\n";
      exit(1);
   }

   //
   // Will be deleted by destructor if allocated
   //
   fcst_engine = new ModeFuzzyEngine;

   if(!fcst_engine) {
      mlog << Error << "\nModeFuzzyEngine::do_fcst_merge_engine() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Specify the configuration for the forecast merging fuzzy engine
   //
   fcst_engine->ctable = ctable;
   if(default_config && merge_config) {
      fcst_engine->conf_info.read_config(default_config, merge_config);
      fcst_engine->conf_info.process_config(conf_info.fcst_info->file_type(),
                                            conf_info.obs_info->file_type());
      path = replace_path(fcst_engine->conf_info.object_pi.color_table);
      fcst_engine->ctable.read(path);
   }

   //
   // Copy over the forecast threshold values
   //
   fcst_engine->conf_info.fcst_raw_thresh        = conf_info.fcst_raw_thresh;
   fcst_engine->conf_info.obs_raw_thresh         = conf_info.fcst_raw_thresh;

   fcst_engine->conf_info.fcst_conv_thresh       = conf_info.fcst_conv_thresh;
   fcst_engine->conf_info.obs_conv_thresh        = conf_info.fcst_conv_thresh;

   fcst_engine->conf_info.fcst_area_thresh       = conf_info.fcst_area_thresh;
   fcst_engine->conf_info.obs_area_thresh        = conf_info.fcst_area_thresh;

   fcst_engine->conf_info.fcst_inten_perc_thresh = conf_info.fcst_inten_perc_thresh;
   fcst_engine->conf_info.obs_inten_perc_thresh  = conf_info.fcst_inten_perc_thresh;

   fcst_engine->conf_info.fcst_merge_thresh      = conf_info.fcst_merge_thresh;
   fcst_engine->conf_info.obs_merge_thresh       = conf_info.fcst_merge_thresh;

   //
   // Copy the previously defined fuzzy engine fields
   // setting the current forecast field to both the
   // forecast and forecast fields in the fcst_engine.
   // this will capture any previous merging performed
   // in the current forecast field.
   //
   *(fcst_engine->fcst_raw)    = *(fcst_raw);
   *(fcst_engine->obs_raw)     = *(fcst_raw);

   *(fcst_engine->fcst_filter) = *(fcst_filter);
   *(fcst_engine->obs_filter)  = *(fcst_filter);

   *(fcst_engine->fcst_thresh) = *(fcst_thresh);
   *(fcst_engine->obs_thresh)  = *(fcst_thresh);

   *(fcst_engine->fcst_conv)   = *(fcst_conv);
   *(fcst_engine->obs_conv)    = *(fcst_conv);

   *(fcst_engine->fcst_mask)   = *(fcst_mask);
   *(fcst_engine->obs_mask)    = *(fcst_mask);

   *(fcst_engine->fcst_split)  = *(fcst_split);
   *(fcst_engine->obs_split)   = *(fcst_split);

   fcst_engine->n_fcst = n_fcst;
   fcst_engine->n_obs  = n_fcst;

   fcst_engine->need_fcst_filter     = false;
   fcst_engine->need_fcst_conv       = false;
   fcst_engine->need_fcst_thresh     = false;
   fcst_engine->need_fcst_split      = false;
   fcst_engine->need_fcst_merge      = true;
   fcst_engine->need_fcst_clus_split = true;

   fcst_engine->need_obs_filter      = false;
   fcst_engine->need_obs_conv        = false;
   fcst_engine->need_obs_thresh      = false;
   fcst_engine->need_obs_split       = false;
   fcst_engine->need_obs_merge       = true;
   fcst_engine->need_obs_clus_split  = true;

   fcst_engine->need_match           = true;

   //
   // Copy the top level set collection down to the fcst engine
   // to capture any merging that has already occurred, via
   // the double threshold method
   //
   for(i=0; i<collection.n_sets; i++) {
      if(collection.set[i].n_fcst > 0) {
         for(j=0; j<collection.set[i].n_fcst; j++) {

            fcst_engine->collection.set[fcst_engine->collection.n_sets].add_pair(
               -1, collection.set[i].fcst_number[j]);
         }
         fcst_engine->collection.n_sets++;
      }
   }

   //
   // Perform merging and matching for the fcst_engine using the
   // simple merging and matching approach
   //
   fcst_engine->do_match_merge();

   //
   // Generate the fcst and obs composite split fields
   //
   fcst_engine->do_fcst_clus_splitting();
   fcst_engine->do_obs_clus_splitting();

   //
   // Based on the results of the merging and matching
   // of the fcst_engine, update the set collection
   // to indicate merging
   //
   for(i=0; i<fcst_engine->collection.n_sets; i++) {

      //
      // All objects will at least match themselves.
      // only define sets when they are composed of two or
      // more simple objects.
      //
      if(fcst_engine->collection.set[i].n_fcst >= 2) {
         for(j=0; j<fcst_engine->collection.set[i].n_fcst; j++) {

            collection.set[collection.n_sets].add_pair(
               fcst_engine->collection.set[i].fcst_number[j], -1);
         }
         collection.n_sets++;
      }
   } // end for i

   //
   // Compute the cluster single and pair features
   //
   fcst_engine->do_cluster_features();

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Perform merging of the observation field using a fuzzy engine
// matching approach
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_merge_engine(const char *default_config,
                                 const char *merge_config) {
   int i, j;
   ShapeData obs_merge_split;
   ConcatString path;

   do_obs_splitting();

   if(n_obs >= max_singles) {

      mlog << Error << "\nModeFuzzyEngine::do_obs_merge_engine() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << n_obs << "\n\n";
      exit(1);
   }

   //
   // Will be deleted by destructor if allocated
   //
   obs_engine = new ModeFuzzyEngine;

   if(!obs_engine) {
      mlog << Error << "\nModeFuzzyEngine::do_obs_merge_engine() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Specify the configuration for the observation merging fuzzy engine
   //
   obs_engine->ctable = ctable;
   if(default_config && merge_config) {
      obs_engine->conf_info.read_config(default_config, merge_config);
      obs_engine->conf_info.process_config(conf_info.fcst_info->file_type(),
                                           conf_info.obs_info->file_type());
      path = replace_path(obs_engine->conf_info.object_pi.color_table);
      obs_engine->ctable.read(path);
   }

   //
   // Copy over the observation threshold values
   //
   obs_engine->conf_info.fcst_raw_thresh        = conf_info.obs_raw_thresh;
   obs_engine->conf_info.obs_raw_thresh         = conf_info.obs_raw_thresh;

   obs_engine->conf_info.fcst_conv_thresh       = conf_info.obs_conv_thresh;
   obs_engine->conf_info.obs_conv_thresh        = conf_info.obs_conv_thresh;

   obs_engine->conf_info.fcst_area_thresh       = conf_info.obs_area_thresh;
   obs_engine->conf_info.obs_area_thresh        = conf_info.obs_area_thresh;

   obs_engine->conf_info.fcst_inten_perc_thresh = conf_info.obs_inten_perc_thresh;
   obs_engine->conf_info.obs_inten_perc_thresh  = conf_info.obs_inten_perc_thresh;

   obs_engine->conf_info.fcst_merge_thresh      = conf_info.obs_merge_thresh;
   obs_engine->conf_info.obs_merge_thresh       = conf_info.obs_merge_thresh;

   //
   // Copy the previously defined fuzzy engine fields
   // setting the current observation field to both the
   // forecast and observation fields in the obs_engine.
   // this will capture any previous merging performed
   // in the current observation field.
   //
   *(obs_engine->fcst_raw)    = *(obs_raw);
   *(obs_engine->obs_raw)     = *(obs_raw);

   *(obs_engine->fcst_filter) = *(obs_filter);
   *(obs_engine->obs_filter)  = *(obs_filter);

   *(obs_engine->fcst_thresh) = *(obs_thresh);
   *(obs_engine->obs_thresh)  = *(obs_thresh);

   *(obs_engine->fcst_conv)   = *(obs_conv);
   *(obs_engine->obs_conv)    = *(obs_conv);

   *(obs_engine->fcst_mask)   = *(obs_mask);
   *(obs_engine->obs_mask)    = *(obs_mask);

   *(obs_engine->fcst_split)  = *(obs_split);
   *(obs_engine->obs_split)   = *(obs_split);

   obs_engine->n_fcst = n_obs;
   obs_engine->n_obs  = n_obs;

   obs_engine->need_fcst_filter     = false;
   obs_engine->need_fcst_conv       = false;
   obs_engine->need_fcst_thresh     = false;
   obs_engine->need_fcst_split      = false;
   obs_engine->need_fcst_merge      = true;
   obs_engine->need_fcst_clus_split = true;

   obs_engine->need_obs_filter      = false;
   obs_engine->need_obs_conv        = false;
   obs_engine->need_obs_thresh      = false;
   obs_engine->need_obs_split       = false;
   obs_engine->need_obs_merge       = true;
   obs_engine->need_obs_clus_split  = true;

   obs_engine->need_match           = true;

   //
   // Copy the top level set collection down to the obs engine
   // to capture any merging that has already occurred, via
   // the double threshold method.  But only copy down those
   // sets containing merged observation objects.
   //
   for(i=0; i<collection.n_sets; i++) {
      if(collection.set[i].n_obs > 0) {
         for(j=0; j<collection.set[i].n_obs; j++) {

            obs_engine->collection.set[obs_engine->collection.n_sets].add_pair(
               -1, collection.set[i].obs_number[j]);
         }
         obs_engine->collection.n_sets++;
      }
   }

   //
   // Perform merging and matching for the obs_engine using the
   // simple merging and matching approach
   //
   obs_engine->do_match_merge();

   //
   // Generate the fcst and obs composite split fields
   //
   obs_engine->do_fcst_clus_splitting();
   obs_engine->do_obs_clus_splitting();

   //
   // Based on the results of the merging and matching
   // of the obs_engine, update the set collection
   // to indicate merging
   //
   for(i=0; i<obs_engine->collection.n_sets; i++) {

      //
      // All objects will at least match themselves.
      // only define sets when they are composed of two or
      // more simple objects.
      //
      if(obs_engine->collection.set[i].n_obs >= 2) {
         for(j=0; j<obs_engine->collection.set[i].n_obs; j++) {

            collection.set[collection.n_sets].add_pair(
               -1, obs_engine->collection.set[i].obs_number[j]);
         }
         collection.n_sets++;
      }
   }

   //
   // Compute the cluster single and pair features
   //
   obs_engine->do_cluster_features();

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Perform matching between the already merged forecast and
// observation fields allowing no additional merging of objects in
// the observation field
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_match_fcst_merge() {
   int j, k, n;
   InterestInfo junkinfo;
   ShapeData * fcst_shape = (ShapeData *) 0;
   ShapeData * obs_shape = (ShapeData *) 0;

   do_fcst_splitting();
   do_obs_splitting();

   if((n_fcst >= max_singles) || (n_obs >= max_singles)) {
      mlog << Error << "\nModeFuzzyEngine::do_match_fcst_merge() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << max(n_fcst, n_obs) << "\n\n";
      exit(1);
   }

   clear_colors();

   fcst_shape = new ShapeData [n_fcst];
   obs_shape = new ShapeData [n_obs];

   if(!fcst_shape || !obs_shape) {
      mlog << Error << "\nModeFuzzyEngine::do_match_fcst_merge() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Do the single features
   //
   for(j=0; j<n_fcst; j++) {
      fcst_shape[j] = select(*fcst_split, j+1);
      fcst_single[j].set(*fcst_filter, *fcst_thresh, fcst_shape[j],
                         conf_info.inten_perc_value);
      fcst_single[j].object_number = j+1;
   }

   for(j=0; j<n_obs; j++) {
      obs_shape[j] = select(*obs_split, j+1);
      obs_single[j].set(*obs_filter, *obs_thresh, obs_shape[j],
                        conf_info.inten_perc_value);
      obs_single[j].object_number = j+1;
   }

   //
   // Do the pair features
   //
   for(j=0; j<n_fcst; j++) {
      for(k=0; k<n_obs; k++) {

         n = two_to_one(j, k);

         pair[n].set(fcst_single[j], obs_single[k],
                     conf_info.max_centroid_dist);
         pair[n].pair_number = n;
      }
   }

   //
   // Calculate the interest values
   //
   for(j=0; j<n_fcst; j++) {
      for(k=0; k<n_obs; k++) {

         n = two_to_one(j, k);

         info[n].fcst_number    = (j+1);
         info[n].obs_number     = (k+1);
         info[n].pair_number    = n;
         info[n].interest_value = total_interest(conf_info, 1, pair[n]);
      }
   }

   //
   // Sort the interest values in decreasing order
   //
   n = n_fcst*n_obs;

   for(j=0; j<(n-1); j++) {
      for(k=(j+1); k<n; k++) {

         if(info[j].interest_value < info[k].interest_value) {

            junkinfo = info[j];
            info[j] = info[k];
            info[k] = junkinfo;
         }
      }
   }

   //
   // Form the sets
   //
   n = n_fcst*n_obs;

   for(j=0; j<n; j++) {

      if(info[j].interest_value < conf_info.total_interest_thresh) {
         continue;
      }

      //
      // Only add this pair if the forecast object is not already matched
      //
      if(!collection.is_fcst_matched(info[j].fcst_number)) {
         collection.add_pair(info[j].fcst_number, info[j].obs_number);
      }
   }

   //
   // Clear out any empty sets
   //
   collection.clear_empty_sets();

   //
   // Assign the colors
   //
   if(collection.n_sets > ctable.n_entries()) {
      mlog << Error << "\nModeFuzzyEngine::do_match_fcst_merge() -> "
           << "not enough colors ... need at least " << (collection.n_sets)
           << "\n\n";
      exit(1);
   }

   for(j=0; j<n_fcst; j++) {
      fcst_color[j] = unmatched_color;

      for(k=0; k<(collection.n_sets); k++) {

         if(collection.set[k].has_fcst(j+1)) {
            fcst_color[j] = ctable.nearest(k+1);
            break;
         }
      }

   }

   for(j=0; j<n_obs; j++) {
      obs_color[j] = unmatched_color;

      for(k=0; k<(collection.n_sets); k++) {

         if(collection.set[k].has_obs(j+1)) {
            obs_color[j] = ctable.nearest(k+1);
            break;
         }
      }
   }

   //
   // Done
   //

   delete [] fcst_shape;  fcst_shape = (ShapeData *) 0;
   delete [] obs_shape;   obs_shape = (ShapeData *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Perform matching between the already merged forecast and observation
// fields allowing no additional merging of objects in the either field
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_match_only() {
   int j, k, n;
   InterestInfo junkinfo;
   ShapeData * fcst_shape = (ShapeData *) 0;
   ShapeData * obs_shape = (ShapeData *) 0;

   do_fcst_splitting();
   do_obs_splitting();

   if((n_fcst >= max_singles) || (n_obs >= max_singles)) {
      mlog << Error << "\nModeFuzzyEngine::do_match_only() -> "
           << "too many shapes ... increase \"max_singles\" to at least "
           << max(n_fcst, n_obs) << "\n\n";
      exit(1);
   }

   clear_colors();

   fcst_shape = new ShapeData [n_fcst];
   obs_shape = new ShapeData [n_obs];

   if(!fcst_shape || !obs_shape) {
      mlog << Error << "\nModeFuzzyEngine::do_match_only() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Do the single features
   //
   for(j=0; j<n_fcst; j++) {
      fcst_shape[j] = select(*fcst_split, j+1);
      fcst_single[j].set(*fcst_filter, *fcst_thresh, fcst_shape[j],
                         conf_info.inten_perc_value);
      fcst_single[j].object_number = j+1;
   }

   for(j=0; j<n_obs; j++) {
      obs_shape[j] = select(*obs_split, j+1);
      obs_single[j].set(*obs_filter, *obs_thresh, obs_shape[j],
                        conf_info.inten_perc_value);
      obs_single[j].object_number = j+1;
   }

   //
   // Do the pair features
   //
   for(j=0; j<n_fcst; j++) {
      for(k=0; k<n_obs; k++) {

         n = two_to_one(j, k);

         pair[n].set(fcst_single[j], obs_single[k],
                     conf_info.max_centroid_dist);
         pair[n].pair_number = n;
      }
   }

   //
   // Calculate the interest values
   //
   for(j=0; j<n_fcst; j++) {
      for(k=0; k<n_obs; k++) {

         n = two_to_one(j, k);

         info[n].fcst_number    = (j+1);
         info[n].obs_number     = (k+1);
         info[n].pair_number    = n;
         info[n].interest_value = total_interest(conf_info, 1, pair[n]);
      }
   }

   //
   // Sort the interest values in decreasing order
   //
   n = n_fcst*n_obs;
   for(j=0; j<(n-1); j++) {
      for(k=(j+1); k<n; k++) {

         if(info[j].interest_value < info[k].interest_value) {

            junkinfo = info[j];
            info[j] = info[k];
            info[k] = junkinfo;
         }
      }
   }

   //
   // Form the sets
   //
   n = n_fcst*n_obs;
   for(j=0; j<n; j++) {

      if(info[j].interest_value < conf_info.total_interest_thresh)
         continue;

      //
      // Only add this pair if both the forecast and observation objects
      // are not already matched
      //
      if(!collection.is_fcst_matched(info[j].fcst_number) &&
         !collection.is_obs_matched(info[j].obs_number)) {
         collection.add_pair(info[j].fcst_number, info[j].obs_number);
      }
   }

   //
   // Clear out any empty sets
   //
   collection.clear_empty_sets();

   //
   // Assign the colors
   //
   if(collection.n_sets > ctable.n_entries()) {
      mlog << Error << "\nModeFuzzyEngine::do_match_only() -> "
           << "not enough colors ... need at least " << (collection.n_sets)
           << "\n\n";
      exit(1);
   }

   for(j=0; j<n_fcst; j++) {
      fcst_color[j] = unmatched_color;

      for(k=0; k<(collection.n_sets); k++) {
         if(collection.set[k].has_fcst(j+1)) {
            fcst_color[j] = ctable.nearest(k+1);
            break;
         }
      }
   }

   for(j=0; j<n_obs; j++) {
      obs_color[j] = unmatched_color;

      for(k=0; k<(collection.n_sets); k++) {
         if(collection.set[k].has_obs(j+1)) {
            obs_color[j] = ctable.nearest(k+1);
            break;
         }
      }
   }

   //
   // Done
   //

   delete [] fcst_shape;   fcst_shape = (ShapeData *) 0;
   delete [] obs_shape;    obs_shape = (ShapeData *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Create a split composite field based on the merging of the single objects
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_fcst_clus_splitting() {
   int x, y, set_number;
   double v;
   ShapeData junk;

   //
   // Initialize fcst_clus_split
   //
   *fcst_clus_split = *fcst_split;

   for(x=0; x<fcst_clus_split->data.nx(); x++) {
      for(y=0; y<fcst_clus_split->data.ny(); y++) {

         v = fcst_split->data(x, y);

         if( is_eq(v, 0.0) ) continue;

         set_number = collection.fcst_set_number(nint(v));

         fcst_clus_split->data.set(set_number + 1, x, y);
      }  // end for y
   }  // end for x

   fcst_clus_split->calc_moments();

   //
   // Done
   //

   need_fcst_clus_split = false;

   return;
}

///////////////////////////////////////////////////////////////////////
//
// Create a split composite field based on the merging of the single
// objects
//
///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_obs_clus_splitting() {
   int x, y, set_number;
   double v;
   ShapeData junk;

   //
   // Initialize obs_clus_split
   //
   *obs_clus_split = *obs_split;

   for(x=0; x<obs_clus_split->data.nx(); x++) {
      for(y=0; y<obs_clus_split->data.ny(); y++) {

         v = obs_split->data(x, y);

         if(is_eq(v, 0.0)) continue;

         set_number = collection.obs_set_number(nint(v));

         obs_clus_split->data.set(set_number + 1, x, y);
      }  // end for y
   }  // end for x

   obs_clus_split->calc_moments();

   //
   // Done
   //

   need_obs_clus_split = false;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeFuzzyEngine::do_cluster_features() {
   int j;
   ShapeData * fcst_clus_shape = (ShapeData *) 0;
   ShapeData * obs_clus_shape  = (ShapeData *) 0;

   if(need_fcst_clus_split) do_fcst_clus_splitting();
   if(need_obs_clus_split)  do_obs_clus_splitting();

   //
   // Store the number of clusters
   //
   n_clus = collection.n_sets;

   fcst_clus_shape = new ShapeData [n_clus];
   obs_clus_shape  = new ShapeData [n_clus];

   if(!fcst_clus_shape || !obs_clus_shape) {

      mlog << Error << "\nModeFuzzyEngine::do_cluster_features() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   //
   // Do the single features for clusters
   //
   for(j=0; j<n_clus; j++) {
      fcst_clus_shape[j] = select(*fcst_clus_split, j+1);
      fcst_clus[j].set(*fcst_filter, *fcst_thresh, fcst_clus_shape[j],
                       conf_info.inten_perc_value);
      fcst_clus[j].object_number = j+1;

      obs_clus_shape[j] = select(*obs_clus_split, j+1);
      obs_clus[j].set(*obs_filter, *obs_thresh, obs_clus_shape[j],
                      conf_info.inten_perc_value);
      obs_clus[j].object_number = j+1;
   }

   //
   // Do the pair features
   //
   for(j=0; j<n_clus; j++) {
      pair_clus[j].set(fcst_clus[j], obs_clus[j],
                       conf_info.max_centroid_dist);
      pair_clus[j].pair_number = j+1;
   }

   //
   // Calculate the interest values
   //
   for(j=0; j<n_clus; j++) {
      info_clus[j].fcst_number    = (j+1);
      info_clus[j].obs_number     = (j+1);
      info_clus[j].pair_number    = j;
      info_clus[j].interest_value = total_interest(conf_info, 0, pair_clus[j]);
   }

   //
   // Done
   //

   delete [] fcst_clus_shape; fcst_clus_shape = (ShapeData *) 0;
   delete [] obs_clus_shape;  obs_clus_shape  = (ShapeData *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////

int ModeFuzzyEngine::get_info_index(int pair_n) const {
   int i;

   for(i=0; i<max_singles*max_singles; i++) {

      if(info[i].pair_number == pair_n) return(i);
   }

   return(-1);
}

///////////////////////////////////////////////////////////////////////

int ModeFuzzyEngine::get_matched_fcst(int area) const {
   int i, count;

   count = 0;

   for(i=0; i<n_fcst; i++) {

      if(collection.fcst_set_number(i+1) != -1) {
         if(area) count += nint(fcst_single[i].area);
         else     count += 1;
      }
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////

int ModeFuzzyEngine::get_unmatched_fcst(int area) const {
   int i, count;

   count = 0;

   for(i=0; i<n_fcst; i++) {

      if(collection.fcst_set_number(i+1) == -1) {
         if(area) count += nint(fcst_single[i].area);
         else     count += 1;
      }
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////

int ModeFuzzyEngine::get_matched_obs(int area) const {
   int i, count;

   count = 0;

   for(i=0; i<n_obs; i++) {

      if(collection.obs_set_number(i+1) != -1) {
         if(area) count += nint(obs_single[i].area);
         else     count += 1;
      }
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////

int ModeFuzzyEngine::get_unmatched_obs(int area) const {
   int i, count;

   count = 0;

   for(i=0; i<n_obs; i++) {

      if(collection.obs_set_number(i+1) == -1) {
         if(area) count += nint(obs_single[i].area);
         else     count += 1;
      }
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////

double total_interest(ModeConfInfo &mc, int dist_flag,
                      const PairFeature &p) {
   double t;
   ostream *out = (ostream *) 0;

   if(mlog.verbosity_level() >= 5) out = &cout;

   t = total_interest_print(mc, dist_flag, p, out);

   return(t);
}

///////////////////////////////////////////////////////////////////////

double total_interest_print(ModeConfInfo &mc, int dist_flag,
                            const PairFeature &p, ostream *out) {
   double attribute;
   double interest, weight;
   double confidence;
   double aspect_obs, aspect_fcst;
   double conf_obs, conf_fcst;
   double term, sum, weight_sum;
   double total;

   ////////////////////////////////////////////////////////////////////
   //
   // If the distance flag is set and the centroid distance is too
   // large, don't compute the interest for this pair.
   //
   ////////////////////////////////////////////////////////////////////

   if(dist_flag && p.centroid_dist > mc.max_centroid_dist) {
      total = bad_data_double;

      if(out) {
         (*out) << "Total Interest = " << total << "\n"
                << "Centroid Distance (" << p.centroid_dist
                << ") > Max Centroid Distance ("
                << mc.max_centroid_dist << ")\n";
      }
      return(total);
   }

   sum = 0.0;
   weight_sum = 0.0;

   ////////////////////////////////////////////////////////////////////
   //
   // Centroid distance
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.centroid_dist;
   interest    = (*mc.centroid_dist_if)(attribute);
   confidence  = area_ratio_conf(p.area_ratio);
   weight      = mc.centroid_dist_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Centroid Distance:\n"
             << "------------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Boundary distance
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.boundary_dist;
   interest    = (*mc.boundary_dist_if)(attribute);
   confidence  = 1.0;
   weight      = mc.boundary_dist_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Boundary Distance:\n"
             << "------------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Convex Hull distance
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.convex_hull_dist;
   interest    = (*mc.convex_hull_dist_if)(attribute);
   confidence  = 1.0;
   weight      = mc.convex_hull_dist_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Convex Hull Distance:\n"
             << "------------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Angle difference
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.angle_diff;
   interest    = (*mc.angle_diff_if)(attribute);
   aspect_obs  = p.Obs->aspect_ratio;
   aspect_fcst = p.Fcst->aspect_ratio;
   conf_obs    = aspect_ratio_conf(aspect_obs);
   conf_fcst   = aspect_ratio_conf(aspect_fcst);
   confidence  = sqrt(conf_obs*conf_fcst);
   weight      = mc.angle_diff_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Angle Difference:\n"
             << "-----------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Area ratio
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.area_ratio;
   interest    = (*mc.area_ratio_if)(attribute);
   confidence  = 1.0;
   weight      = mc.area_ratio_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Area Ratio:\n"
             << "-----------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Intersection/area ratio
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = (p.intersection_area)/(min(p.Obs->area, p.Fcst->area));
   interest    = (*mc.int_area_ratio_if)(attribute);
   confidence  = 1.0;
   weight      = mc.int_area_ratio_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Intersection/Area Ratio:\n"
             << "----------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Complexity ratio
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.complexity_ratio;
   interest    = (*mc.complexity_ratio_if)(attribute);
   confidence  = 1.0;
   weight      = mc.complexity_ratio_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Complexity Ratio:\n"
             << "-----------------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Nth Percentile intensity ratio
   //
   ////////////////////////////////////////////////////////////////////

   attribute   = p.percentile_intensity_ratio;
   interest    = (*mc.inten_perc_ratio_if)(attribute);
   confidence  = 1.0;
   weight      = mc.inten_perc_ratio_wt;
   term        = weight*interest*confidence;
   sum        += term;
   weight_sum += weight*confidence;

   if(out) {
      (*out) << "Percentile (" << mc.inten_perc_value
             << "th) Intensity Ratio:\n"
             << "-----------------------\n"
             << "   Value      = " << attribute  << "\n"
             << "   Interest   = " << interest   << "\n"
             << "   Confidence = " << confidence << "\n"
             << "   Weight     = " << weight     << "\n"
             << "   Term       = " << term       << "\n"
             << "\n";
   }

   ////////////////////////////////////////////////////////////////////
   //
   // Done
   //
   ////////////////////////////////////////////////////////////////////

   total = sum/weight_sum;

   if(out) {
      (*out) << "Total Interest = (sum of terms)/(sum of weights*confidence)\n\n"
             << "               = " << sum << "/" << weight_sum << "\n\n"
             << "               = " << total << "\n\n";
   }

   return(total);
}

///////////////////////////////////////////////////////////////////////

double interest_percentile(ModeFuzzyEngine &eng, const double p, const int flag) {
   int i, fcst_i, obs_i, n_values;
   double interest, ptile;
   double *v = (double *) 0;
   NumArray fcst_na, obs_na;

   if(eng.conf_info.match_flag == 0 ||
      eng.n_fcst                    == 0 ||
      eng.n_obs                     == 0) return(0.0);

   //
   // Initialize the maximum interest value for each object to zero.
   //
   for(i=0; i<eng.n_fcst; i++) fcst_na.add(0.0);
   for(i=0; i<eng.n_obs;  i++) obs_na.add(0.0);

   //
   // Loop through all the pairs and keep track of the maximum interest values.
   //
   for(i=0; i<(eng.n_fcst*eng.n_obs); i++) {

      //
      // Skip this interest info if it's not assigned
      //
      if(eng.info[i].fcst_number == 0 ||
         eng.info[i].obs_number  == 0) continue;

      interest = eng.info[i].interest_value;
      fcst_i   = eng.info[i].fcst_number - 1;
      obs_i    = eng.info[i].obs_number - 1;

      if(interest >= fcst_na[fcst_i]) fcst_na.set(fcst_i, interest);
      if(interest >= obs_na[obs_i])   obs_na.set(obs_i,   interest);
   }

   //
   // Allocate memory
   //
   v = new double [eng.n_fcst + eng.n_obs];

   //
   // Add the maximum interest values for the forecast and/or observation
   //
   n_values = 0;
   if(flag == 1 || flag == 3) {
      for(i=0; i<eng.n_fcst; i++) {
         v[n_values] = fcst_na[i];
         n_values++;
      }
   }
   if(flag == 2 || flag == 3) {
      for(i=0; i<eng.n_obs; i++) {
         v[n_values] = obs_na[i];
         n_values++;
      }
   }

   //
   // Sort the maximum interest values
   //
   sort(v, n_values);

   //
   // Get the requested percentile
   //
   ptile = percentile(v, n_values, p/100.0);

   //
   // Done
   //

   if(v) { delete [] v; v = (double *) 0; }

   return(ptile);
}

///////////////////////////////////////////////////////////////////////

void write_engine_stats(ModeFuzzyEngine &eng, const Grid &grid, AsciiTable &at) {
   int i, j, row;

   //
   // Compute the maximum number of rows possible
   //
   i = 1;                        // Header row
   i += eng.n_fcst;              // Simple forecast objects
   i += eng.n_obs;               // Simple observation objects
   i += eng.n_fcst * eng.n_obs;  // Pairs of simple objects
   i += 3*eng.collection.n_sets; // Clusters: fcst, obs, pairs

   //
   // Setup the AsciiTable to be used
   //
   at.clear();
   j = n_mode_hdr_columns + n_mode_obj_columns;
   at.set_size(i, j);                      // Set table size
   at.set_table_just(LeftJust);            // Left-justify columns
   at.set_precision(default_precision);    // Set the precision
   at.set_bad_data_value(bad_data_double); // Set the bad data value
   at.set_bad_data_str(na_str);            // Set the bad data string
   at.set_delete_trailing_blank_rows(1);   // No trailing blank rows

   //
   // Initialize row count
   //
   row = 0;

   //
   // Column headers
   //
   write_header(eng, at, row);
   row++;

   //
   // Single objects
   //
   for(i=0; i<eng.n_fcst; i++) {
      write_fcst_single(eng, i, grid, at, row);
      row++;
   }

   for(i=0; i<eng.n_obs; i++) {
      write_obs_single(eng, i, grid, at, row);
      row++;
   }

   //
   // If no matching was requested, don't write any more
   //
   if(eng.conf_info.match_flag == 0) return;

   //
   // Object pairs, increment the counter within the subroutine
   //
   for(i=0; i<eng.n_fcst; i++) {
      for(j=0; j<eng.n_obs; j++) {
         write_pair(eng, i, j, at, row);
      }
   }

   //
   // Single composites
   //
   for(i=0; i<(eng.collection.n_sets); i++) {
      write_fcst_cluster(eng, i, grid, at, row);
      row++;
   }

   for(i=0; i<(eng.collection.n_sets); i++) {
      write_obs_cluster(eng, i, grid, at, row);
      row++;
   }

   //
   // Composite Pairs
   //
   for(i=0; i<(eng.collection.n_sets); i++) {
      write_cluster_pair(eng, i, at, row);
      row++;
   }

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////

void write_header(ModeFuzzyEngine &eng, AsciiTable &at, const int row) {
   int i;
   char tmp_str[max_str_len];

   //
   // Write out the MODE header columns
   //
   for(i=0; i<n_mode_hdr_columns; i++) {
      at.set_entry(row, i, mode_hdr_columns[i]);
   }

   //
   // Write out the MODE objects columns
   //
   for(i=0; i<n_mode_obj_columns; i++) {
      at.set_entry(row, i + n_mode_hdr_columns, mode_obj_columns[i]);
   }

   //
   // Over-ride the name of the INTENSITY_USER column
   //
   sprintf(tmp_str, "INTENSITY_%d",
           nint(eng.conf_info.inten_perc_value));
   at.set_entry(row, mode_intensity_user_offset, tmp_str);

   return;
}

///////////////////////////////////////////////////////////////////////

void write_header_columns(ModeFuzzyEngine &eng, AsciiTable &at, const int row) {
   int mon, day, yr, hr, min, sec;
   char tmp_str[max_str_len];

   // Version
   at.set_entry(row, mode_version_offset, met_version);

   // Model Name
   at.set_entry(row, mode_model_offset, eng.conf_info.model);

   // Forecast lead time
   sec_to_hms(eng.fcst_raw->data.lead(), hr, min, sec);
   sprintf(tmp_str, hms_fmt, hr, min, sec);
   at.set_entry(row, mode_fcst_lead_offset, tmp_str);

   // Forecast valid time
   unix_to_mdyhms(eng.fcst_raw->data.valid(), mon, day, yr, hr, min, sec);
   sprintf(tmp_str, ymd_hms_fmt, yr, mon, day, hr, min, sec);
   at.set_entry(row, mode_fcst_valid_offset, tmp_str);


   // Forecast accumulation time
   sec_to_hms(eng.fcst_raw->data.accum(), hr, min, sec);
   sprintf(tmp_str, hms_fmt, hr, min, sec);
   at.set_entry(row, mode_fcst_accum_offset, tmp_str);

   // Observation lead time
   sec_to_hms(eng.obs_raw->data.lead(), hr, min, sec);
   sprintf(tmp_str, hms_fmt, hr, min, sec);
   at.set_entry(row, mode_obs_lead_offset, tmp_str);

   // Observation valid time
   unix_to_mdyhms(eng.obs_raw->data.valid(), mon, day, yr, hr, min, sec);
   sprintf(tmp_str, ymd_hms_fmt, yr, mon, day, hr, min, sec);
   at.set_entry(row, mode_obs_valid_offset, tmp_str);

   // Observation accumulation time
   sec_to_hms(eng.obs_raw->data.accum(), hr, min, sec);
   sprintf(tmp_str, hms_fmt, hr, min, sec);
   at.set_entry(row, mode_obs_accum_offset, tmp_str);

   // Forecast convolution radius
   at.set_entry(row, mode_fcst_rad_offset,
                eng.conf_info.fcst_conv_radius);

   // Forecast convolution threshold
   at.set_entry(row, mode_fcst_thr_offset,
                eng.conf_info.fcst_conv_thresh.get_str());

   // Observation convolution radius
   at.set_entry(row, mode_obs_rad_offset,
                eng.conf_info.obs_conv_radius);

   // Observation convolution threshold
   at.set_entry(row, mode_obs_thr_offset,
                eng.conf_info.obs_conv_thresh.get_str());

   // Forecast Variable Name
   at.set_entry(row, mode_fcst_var_offset,
                eng.conf_info.fcst_info->name());

   // Forecast Variable Level
   at.set_entry(row, mode_fcst_lev_offset,
                eng.conf_info.fcst_info->level_name());

   // Observation Variable Name
   at.set_entry(row, mode_obs_var_offset,
                eng.conf_info.obs_info->name());

   // Observation Variable Level
   at.set_entry(row, mode_obs_lev_offset,
                eng.conf_info.obs_info->level_name());

   return;
}

///////////////////////////////////////////////////////////////////////

void write_fcst_single(ModeFuzzyEngine &eng, const int n, const Grid &grid,
                       AsciiTable &at, const int row) {
   int i;
   double lat, lon;
   char tmp_str[max_str_len];

   if(n >= eng.n_fcst) {
      mlog << Error << "\nwrite_fcst_single(const ModeFuzzyEngine &, int, "
           << "AsciiTable, int) -> "
           << n << " >= number of fcst, " << eng.n_fcst << "\n\n";
      exit(1);
   }

   // Write out the common header columns
   write_header_columns(eng, at, row);

   // Object ID
   sprintf(tmp_str, "F%03d", (n+1));
   at.set_entry(row, mode_object_id_offset, tmp_str);

   // Object category
   i = eng.collection.fcst_set_number(n + 1);
   sprintf(tmp_str, "CF%03d", (i + 1));
   at.set_entry(row, mode_object_cat_offset, tmp_str);

   // Convert x,y to lat,lon
   grid.xy_to_latlon(eng.fcst_single[n].centroid_x,
                     eng.fcst_single[n].centroid_y,
                     lat, lon);

   // Object centroid, x-coordinate
   at.set_entry(row, mode_centroid_x_offset, eng.fcst_single[n].centroid_x);

   // Object centroid, y-coordinate
   at.set_entry(row, mode_centroid_y_offset, eng.fcst_single[n].centroid_y);

   // Object centroid, latitude
   at.set_entry(row, mode_centroid_lat_offset, lat);

   // Object centroid, longitude
   at.set_entry(row, mode_centroid_lon_offset, -1.0*lon);

   // Axis angle
   at.set_entry(row, mode_axis_ang_offset, eng.fcst_single[n].axis_ang);

   // Object length
   at.set_entry(row, mode_length_offset, eng.fcst_single[n].length);

   // Object width
   at.set_entry(row, mode_width_offset, eng.fcst_single[n].width);

   // Area of object
   at.set_entry(row, mode_area_offset,
                nint(eng.fcst_single[n].area));

   // Area in the raw field that is non-zero
   at.set_entry(row, mode_area_filter_offset,
                nint(eng.fcst_single[n].area_filter));

   // Area in the raw field that meets the threshold criteria
   at.set_entry(row, mode_area_thresh_offset,
                nint(eng.fcst_single[n].area_thresh));

   // Object curvature
   at.set_entry(row, mode_curvature_offset, eng.fcst_single[n].curvature);

   // Center of curvature, x-coordinate
   at.set_entry(row, mode_curvature_x_offset, eng.fcst_single[n].curvature_x);

   // Center of curvature, y-coordiante
   at.set_entry(row, mode_curvature_y_offset, eng.fcst_single[n].curvature_y);

   // Object complexity
   at.set_entry(row, mode_complexity_offset, eng.fcst_single[n].complexity);

   // 10th percentile of object intensity
   at.set_entry(row, mode_intensity_10_offset,
                eng.fcst_single[n].intensity_ptile.p10);

   // 25th percentile of object intensity
   at.set_entry(row, mode_intensity_25_offset,
                eng.fcst_single[n].intensity_ptile.p25);

   // 50th percentile of object intensity
   at.set_entry(row, mode_intensity_50_offset,
                eng.fcst_single[n].intensity_ptile.p50);

   // 75th percentile of object intensity
   at.set_entry(row, mode_intensity_75_offset,
                eng.fcst_single[n].intensity_ptile.p75);

   // 90th percentile of object intensity
   at.set_entry(row, mode_intensity_90_offset,
                eng.fcst_single[n].intensity_ptile.p90);

   // Specified percentile of object intensity
   at.set_entry(row, mode_intensity_user_offset,
                eng.fcst_single[n].intensity_ptile.pth);

   // Sum of the object intensity values
   at.set_entry(row, mode_intensity_sum_offset,
                eng.fcst_single[n].intensity_ptile.sum);

   //
   // Fill the columns that don't apply with bad data values
   //
   for(i=mode_centroid_dist_offset; i<=mode_interest_offset; i++) {
      at.set_entry(row, i, bad_data_double);
   }

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////

void write_obs_single(ModeFuzzyEngine &eng, const int n, const Grid &grid,
                      AsciiTable &at, const int row) {
   int i;
   double lat, lon;
   char tmp_str[max_str_len];

   if(n >= eng.n_obs) {
      mlog << Error << "\nwrite_obs_single(const ModeFuzzyEngine &, int, "
           << "AsciiTable, int) -> "
           << n << " >= number of obs, " << eng.n_obs << "\n\n";
      exit(1);
   }

   // Write out the common header columns
   write_header_columns(eng, at, row);

   // Object ID
   sprintf(tmp_str, "O%03d", (n+1));
   at.set_entry(row, mode_object_id_offset, tmp_str);

   // Object category
   i = eng.collection.obs_set_number(n + 1);
   sprintf(tmp_str, "CO%03d", (i + 1));
   at.set_entry(row, mode_object_cat_offset, tmp_str);

   // Convert x,y to lat,lon
   grid.xy_to_latlon(eng.obs_single[n].centroid_x,
                     eng.obs_single[n].centroid_y,
                     lat, lon);

   // Object centroid, x-coordinate
   at.set_entry(row, mode_centroid_x_offset, eng.obs_single[n].centroid_x);

   // Object centroid, y-coordinate
   at.set_entry(row, mode_centroid_y_offset, eng.obs_single[n].centroid_y);

   // Object centroid, latitude
   at.set_entry(row, mode_centroid_lat_offset, lat);

   // Object centroid, longitude
   at.set_entry(row, mode_centroid_lon_offset, -1.0*lon);

   // Axis angle
   at.set_entry(row, mode_axis_ang_offset, eng.obs_single[n].axis_ang);

   // Object length
   at.set_entry(row, mode_length_offset, eng.obs_single[n].length);

   // Object width
   at.set_entry(row, mode_width_offset, eng.obs_single[n].width);

   // Area of object
   at.set_entry(row, mode_area_offset,
                nint(eng.obs_single[n].area));

   // Area in the raw field that is non-zero
   at.set_entry(row, mode_area_filter_offset,
                nint(eng.obs_single[n].area_filter));

   // Area in the raw field that meets the threshold criteria
   at.set_entry(row, mode_area_thresh_offset,
                nint(eng.obs_single[n].area_thresh));

   // Object curvature
   at.set_entry(row, mode_curvature_offset, eng.obs_single[n].curvature);

   // Center of curvature, x-coordinate
   at.set_entry(row, mode_curvature_x_offset, eng.obs_single[n].curvature_x);

   // Center of curvature, y-coordiante
   at.set_entry(row, mode_curvature_y_offset, eng.obs_single[n].curvature_y);

   // Object complexity
   at.set_entry(row, mode_complexity_offset, eng.obs_single[n].complexity);

   // 10th percentile of object intensity
   at.set_entry(row, mode_intensity_10_offset,
                eng.obs_single[n].intensity_ptile.p10);

   // 25th percentile of object intensity
   at.set_entry(row, mode_intensity_25_offset,
                eng.obs_single[n].intensity_ptile.p25);

   // 50th percentile of object intensity
   at.set_entry(row, mode_intensity_50_offset,
                eng.obs_single[n].intensity_ptile.p50);

   // 75th percentile of object intensity
   at.set_entry(row, mode_intensity_75_offset,
                eng.obs_single[n].intensity_ptile.p75);

   // 90th percentile of object intensity
   at.set_entry(row, mode_intensity_90_offset,
                eng.obs_single[n].intensity_ptile.p90);

   // Specified percentile of object intensity
   at.set_entry(row, mode_intensity_user_offset,
                eng.obs_single[n].intensity_ptile.pth);

   // Sum of the object intensity values
   at.set_entry(row, mode_intensity_sum_offset,
                eng.obs_single[n].intensity_ptile.sum);

   //
   // Fill the columns that don't apply with bad data values
   //
   for(i=mode_centroid_dist_offset; i<=mode_interest_offset; i++) {
      at.set_entry(row, i, bad_data_double);
   }

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////

void write_pair(ModeFuzzyEngine &eng, const int n_f, const int n_o,
                AsciiTable &at, int &row) {
   int n, i, fcst_i, obs_i;
   char tmp_str[max_str_len];

   if(n_f >= eng.n_fcst || n_o >= eng.n_obs) {
      mlog << Error << "\nwrite_pair(const ModeFuzzyEngine &, int, int, "
           << "AsciiTable &, const int) -> "
           << n_f << " >= number of fcst, " << eng.n_fcst << " or "
           << n_o << " >= number of obs, " << eng.n_obs << "\n\n";
      exit(1);
   }

   n = eng.two_to_one(n_f, n_o);

   //
   // Only dump out pair features if the total interest value for the pair
   // is greater than the print interest threshold
   //
   if( eng.info[eng.get_info_index(n)].interest_value <
       eng.conf_info.print_interest_thresh)  return;

   // Write out the common header columns
   write_header_columns(eng, at, row);

   // Object ID
   sprintf(tmp_str, "F%03d_O%03d", (n_f+1), (n_o+1));
   at.set_entry(row, mode_object_id_offset, tmp_str);

   // Object category
   fcst_i = eng.collection.fcst_set_number(n_f+1);
   obs_i  = eng.collection.obs_set_number(n_o+1);
   sprintf(tmp_str, "CF%03d_CO%03d", (fcst_i+1), (obs_i+1));
   at.set_entry(row, mode_object_cat_offset, tmp_str);

   //
   // Fill the columns that don't apply with bad data values
   //
   for(i=mode_centroid_x_offset; i<=mode_intensity_sum_offset; i++) {
      at.set_entry(row, i, bad_data_double);
   }

   // Distance between centroids
   at.set_entry(row, mode_centroid_dist_offset, eng.pair[n].centroid_dist);

   // Distance between boundaries
   at.set_entry(row, mode_boundary_dist_offset, eng.pair[n].boundary_dist);

   // Distance between convex hulls
   at.set_entry(row, mode_convex_hull_dist_offset,
                eng.pair[n].convex_hull_dist);

   // Difference in angles in degrees
   at.set_entry(row, mode_angle_diff_offset, eng.pair[n].angle_diff);

   // Area ratio
   at.set_entry(row, mode_area_ratio_offset, eng.pair[n].area_ratio);

   // Intersection area
   at.set_entry(row, mode_intersection_area_offset,
                nint(eng.pair[n].intersection_area));

   // Union area
   at.set_entry(row, mode_union_area_offset,
                nint(eng.pair[n].union_area));

   // Symmetric difference area
   at.set_entry(row, mode_symmetric_diff_offset,
                nint(eng.pair[n].symmetric_diff));

   // Intersection over area
   at.set_entry(row, mode_intersection_over_area_offset,
                eng.pair[n].intersection_over_area);

   // Complexity ratio
   at.set_entry(row, mode_complexity_ratio_offset,
                eng.pair[n].complexity_ratio);

   // Percentile intensity ratio
   at.set_entry(row, mode_percentile_intensity_ratio_offset,
                eng.pair[n].percentile_intensity_ratio);

   // Total interest value
   at.set_entry(row, mode_interest_offset,
                eng.info[eng.get_info_index(n)].interest_value);

   //
   // Increment row counter
   //
   row++;

   return;
}

///////////////////////////////////////////////////////////////////////

void write_fcst_cluster(ModeFuzzyEngine &eng, const int n, const Grid &grid,
                          AsciiTable &at, const int row) {
   int i;
   double lat, lon;
   char tmp_str[max_str_len];

   // Write out the common header columns
   write_header_columns(eng, at, row);

   // Object ID
   sprintf(tmp_str, "CF%03d", (n+1));
   at.set_entry(row, mode_object_id_offset, tmp_str);

   // Object category
   at.set_entry(row, mode_object_cat_offset, tmp_str);

   // Convert x,y to lat,lon
   grid.xy_to_latlon(eng.fcst_clus[n].centroid_x,
                     eng.fcst_clus[n].centroid_y,
                     lat, lon);

   // Object centroid, x-coordinate
   at.set_entry(row, mode_centroid_x_offset, eng.fcst_clus[n].centroid_x);

   // Object centroid, y-coordinate
   at.set_entry(row, mode_centroid_y_offset, eng.fcst_clus[n].centroid_y);

   // Object centroid, latitude
   at.set_entry(row, mode_centroid_lat_offset, lat);

   // Object centroid, longitude
   at.set_entry(row, mode_centroid_lon_offset, -1.0*lon);

   // Axis angle
   at.set_entry(row, mode_axis_ang_offset, eng.fcst_clus[n].axis_ang);

   // Object length
   at.set_entry(row, mode_length_offset, eng.fcst_clus[n].length);

   // Object width
   at.set_entry(row, mode_width_offset, eng.fcst_clus[n].width);

   // Area of object
   at.set_entry(row, mode_area_offset,
                nint(eng.fcst_clus[n].area));

   // Area in the raw field that is non-zero
   at.set_entry(row, mode_area_filter_offset,
                nint(eng.fcst_clus[n].area_filter));

   // Area in the raw field that meets the threshold criteria
   at.set_entry(row, mode_area_thresh_offset,
                nint(eng.fcst_clus[n].area_thresh));

   // Object curvature
   at.set_entry(row, mode_curvature_offset, eng.fcst_clus[n].curvature);

   // Center of curvature, x-coordinate
   at.set_entry(row, mode_curvature_x_offset, eng.fcst_clus[n].curvature_x);

   // Center of curvature, y-coordiante
   at.set_entry(row, mode_curvature_y_offset, eng.fcst_clus[n].curvature_y);

   // Object complexity
   at.set_entry(row, mode_complexity_offset, eng.fcst_clus[n].complexity);

   // 10th percentile of object intensity
   at.set_entry(row, mode_intensity_10_offset,
                eng.fcst_clus[n].intensity_ptile.p10);

   // 25th percentile of object intensity
   at.set_entry(row, mode_intensity_25_offset,
                eng.fcst_clus[n].intensity_ptile.p25);

   // 50th percentile of object intensity
   at.set_entry(row, mode_intensity_50_offset,
                eng.fcst_clus[n].intensity_ptile.p50);

   // 75th percentile of object intensity
   at.set_entry(row, mode_intensity_75_offset,
                eng.fcst_clus[n].intensity_ptile.p75);

   // 90th percentile of object intensity
   at.set_entry(row, mode_intensity_90_offset,
                eng.fcst_clus[n].intensity_ptile.p90);

   // Specified percentile of object intensity
   at.set_entry(row, mode_intensity_user_offset,
                eng.fcst_clus[n].intensity_ptile.pth);

   // Sum of the object intensity values
   at.set_entry(row, mode_intensity_sum_offset,
                eng.fcst_clus[n].intensity_ptile.sum);

   //
   // Fill the columns that don't apply with bad data values
   //
   for(i=mode_centroid_dist_offset; i<=mode_interest_offset; i++) {
      at.set_entry(row, i, bad_data_double);
   }

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////

void write_obs_cluster(ModeFuzzyEngine &eng, const int n, const Grid &grid,
                         AsciiTable &at, const int row) {
   int i;
   double lat, lon;
   char tmp_str[max_str_len];

   // Write out the common header columns
   write_header_columns(eng, at, row);

   // Object ID
   sprintf(tmp_str, "CO%03d", (n+1));
   at.set_entry(row, mode_object_id_offset, tmp_str);

   // Object category
   at.set_entry(row, mode_object_cat_offset, tmp_str);

   // Convert x,y to lat,lon
   grid.xy_to_latlon(eng.obs_clus[n].centroid_x,
                     eng.obs_clus[n].centroid_y,
                     lat, lon);

   // Object centroid, x-coordinate
   at.set_entry(row, mode_centroid_x_offset, eng.obs_clus[n].centroid_x);

   // Object centroid, y-coordinate
   at.set_entry(row, mode_centroid_y_offset, eng.obs_clus[n].centroid_y);

   // Object centroid, latitude
   at.set_entry(row, mode_centroid_lat_offset, lat);

   // Object centroid, longitude
   at.set_entry(row, mode_centroid_lon_offset, -1.0*lon);

   // Axis angle
   at.set_entry(row, mode_axis_ang_offset, eng.obs_clus[n].axis_ang);

   // Object length
   at.set_entry(row, mode_length_offset, eng.obs_clus[n].length);

   // Object width
   at.set_entry(row, mode_width_offset, eng.obs_clus[n].width);

   // Area of object
   at.set_entry(row, mode_area_offset,
                nint(eng.obs_clus[n].area));

   // Area in the raw field that is non-zero
   at.set_entry(row, mode_area_filter_offset,
                nint(eng.obs_clus[n].area_filter));

   // Area in the raw field that meets the threshold criteria
   at.set_entry(row, mode_area_thresh_offset,
                nint(eng.obs_clus[n].area_thresh));

   // Object curvature
   at.set_entry(row, mode_curvature_offset, eng.obs_clus[n].curvature);

   // Center of curvature, x-coordinate
   at.set_entry(row, mode_curvature_x_offset, eng.obs_clus[n].curvature_x);

   // Center of curvature, y-coordiante
   at.set_entry(row, mode_curvature_y_offset, eng.obs_clus[n].curvature_y);

   // Object complexity
   at.set_entry(row, mode_complexity_offset, eng.obs_clus[n].complexity);

   // 10th percentile of object intensity
   at.set_entry(row, mode_intensity_10_offset,
                eng.obs_clus[n].intensity_ptile.p10);

   // 25th percentile of object intensity
   at.set_entry(row, mode_intensity_25_offset,
                eng.obs_clus[n].intensity_ptile.p25);

   // 50th percentile of object intensity
   at.set_entry(row, mode_intensity_50_offset,
                eng.obs_clus[n].intensity_ptile.p50);

   // 75th percentile of object intensity
   at.set_entry(row, mode_intensity_75_offset,
                eng.obs_clus[n].intensity_ptile.p75);

   // 90th percentile of object intensity
   at.set_entry(row, mode_intensity_90_offset,
                eng.obs_clus[n].intensity_ptile.p90);

   // Specified percentile of object intensity
   at.set_entry(row, mode_intensity_user_offset,
                eng.obs_clus[n].intensity_ptile.pth);

   // Sum of the object intensity values
   at.set_entry(row, mode_intensity_sum_offset,
                eng.obs_clus[n].intensity_ptile.sum);

   //
   // Fill the columns that don't apply with bad data values
   //
   for(i=mode_centroid_dist_offset; i<=mode_interest_offset; i++) {
      at.set_entry(row, i, bad_data_double);
   }

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////

void write_cluster_pair(ModeFuzzyEngine &eng, const int n,
                          AsciiTable &at, const int row) {
   int i;
   char tmp_str[max_str_len];

   // Write out the common header columns
   write_header_columns(eng, at, row);

   // Object ID
   sprintf(tmp_str, "CF%03d_CO%03d", (n+1), (n+1));
   at.set_entry(row, mode_object_id_offset, tmp_str);

   // Object category
   at.set_entry(row, mode_object_cat_offset, tmp_str);

   //
   // Fill the columns that don't apply with bad data values
   //
   for(i=mode_centroid_x_offset; i<=mode_intensity_sum_offset; i++) {
      at.set_entry(row, i, bad_data_double);
   }

   // Distance between centroids
   at.set_entry(row, mode_centroid_dist_offset,
                eng.pair_clus[n].centroid_dist);

   // Distance between boundaries
   at.set_entry(row, mode_boundary_dist_offset,
                eng.pair_clus[n].boundary_dist);

   // Distance between convex hulls
   at.set_entry(row, mode_convex_hull_dist_offset,
                eng.pair_clus[n].convex_hull_dist);

   // Difference in angles in degrees
   at.set_entry(row, mode_angle_diff_offset, eng.pair_clus[n].angle_diff);

   // Area ratio
   at.set_entry(row, mode_area_ratio_offset, eng.pair_clus[n].area_ratio);

   // Intersection area
   at.set_entry(row, mode_intersection_area_offset,
                nint(eng.pair_clus[n].intersection_area));

   // Union area
   at.set_entry(row, mode_union_area_offset,
                nint(eng.pair_clus[n].union_area));

   // Symmetric difference area
   at.set_entry(row, mode_symmetric_diff_offset,
                nint(eng.pair_clus[n].symmetric_diff));

   // Intersection over area
   at.set_entry(row, mode_intersection_over_area_offset,
                eng.pair_clus[n].intersection_over_area);

   // Complexity ratio
   at.set_entry(row, mode_complexity_ratio_offset,
                eng.pair_clus[n].complexity_ratio);

   // Percentile intensity ratio
   at.set_entry(row, mode_percentile_intensity_ratio_offset,
                eng.pair_clus[n].percentile_intensity_ratio);

   // Total interest value
   at.set_entry(row, mode_interest_offset,
                eng.info_clus[n].interest_value);

   //
   // Done
   //

   return;
}

///////////////////////////////////////////////////////////////////////

void calc_fcst_clus_ch_mask(const ModeFuzzyEngine &eng, ShapeData &mask) {
   int i, x, y;
   ShapeData comp;
   Polyline poly;
   Box bb;

   if(eng.need_fcst_clus_split) {
      mlog << Error << "\ncalc_fcst_clus_ch_mask -> "
           << "should not be called with need_fcst_clus_split set to true\n\n";

      exit(1);
   }

   //
   // Initialize convex hull mask wrfdata object
   //
   mask = *(eng.fcst_clus_split);
   mask.zero_field();

   //
   // Calculate fcst convex hull mask field for composite objects
   //
   for(i=0; i<eng.collection.n_sets; i++) {
      comp = select(*eng.fcst_clus_split, i+1);
      poly = comp.convex_hull();
      poly.bounding_box(bb);

      for(x = (int) floor(bb.left()-1); x <= (int) ceil(bb.right()+1); x++) {
         for(y = (int) floor(bb.bottom()-1); y <= (int) ceil(bb.top()+1); y++) {

            if(poly.is_inside(x, y)) {
               mask.data.set(1.0, x, y);
            }
         } // end for y
      } // end for x
   } // end for i

   //
   // Calculate fcst convex hull mask field for unmatched singles
   //
   for(i=0; i<eng.n_fcst; i++) {

      if(eng.collection.fcst_set_number(i+1) != -1) continue;

      comp = select(*eng.fcst_split, i+1);
      poly = comp.convex_hull();
      poly.bounding_box(bb);

      for(x = (int) floor(bb.left()-1); x <= (int) ceil(bb.right()+1); x++) {
         for(y = (int) floor(bb.bottom()-1); y <= (int) ceil(bb.top()+1); y++) {

            if(poly.is_inside(x, y)) {
               mask.data.set(1.0, x, y);
            }
         } // end for y
      } // end for x
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void calc_obs_clus_ch_mask(const ModeFuzzyEngine &eng, ShapeData &mask) {
   int i, x, y;
   ShapeData comp;
   Polyline poly;
   Box bb;

   if(eng.need_obs_clus_split) {
      mlog << Error << "\ncalc_obs_clus_ch_mask -> "
           << "should not be called with need_obs_clus_split set to true\n\n";
      exit(1);
   }

   //
   // Initialize convex hull mask wrfdata object
   //
   mask = *(eng.obs_clus_split);
   mask.zero_field();

   //
   // Calculate obs convex hull mask field for composite objects
   //
   for(i=0; i<eng.collection.n_sets; i++) {
      comp = select(*eng.obs_clus_split, i+1);
      poly = comp.convex_hull();
      poly.bounding_box(bb);

      for(x=(int) floor(bb.left()-1); x<=ceil(bb.right()+1); x++) {
         for(y=(int) floor(bb.bottom()-1); y<=ceil(bb.top()+1); y++) {

            if(poly.is_inside(x, y)) {
               mask.data.set(1.0, x, y);
            }
         } // end for y
      } // end for x
   } // end for i

   //
   // Calculate obs convex hull mask field for unmatched singles
   //
   for(i=0; i<eng.n_obs; i++) {

      if(eng.collection.obs_set_number(i+1) != -1) continue;

      comp = select(*eng.obs_split, i+1);
      poly = comp.convex_hull();
      poly.bounding_box(bb);

      for(x=(int) floor(bb.left()-1); x<=ceil(bb.right()+1); x++) {
         for(y=(int) floor(bb.bottom()-1); y<=ceil(bb.top()+1); y++) {

            if(poly.is_inside(x, y)) {
               mask.data.set(1.0, x, y);
            }
         } // end for y
      } // end for x
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void calc_fcst_cluster_mask(const ModeFuzzyEngine &eng, ShapeData &comp, const int n_set) {
   int i, x, y;
   ShapeData junk;

   comp.data.set_size(eng.fcst_raw->data.nx(), eng.fcst_raw->data.ny());

   for(i=0; i<eng.n_fcst; i++) {

      if(!(eng.collection.set[n_set].has_fcst(i + 1)))  continue;

      junk = select(*(eng.fcst_split), i + 1);

      for(x=0; x<(eng.fcst_raw->data.nx()); x++) {
         for(y=0; y<(eng.fcst_raw->data.ny()); y++) {

            if( junk.is_nonzero(x, y) ) {
               comp.data.set(1.0, x, y);
            }
         } // end for y
      } // end for x
   } // end for i

   comp.calc_moments();

   return;
}

///////////////////////////////////////////////////////////////////////

void calc_obs_cluster_mask(const ModeFuzzyEngine &eng, ShapeData &comp, const int n_set) {
   int i, x, y;
   ShapeData junk;

   comp.data.set_size(eng.obs_raw->data.nx(), eng.obs_raw->data.ny());

   for(i=0; i<eng.n_obs; i++) {

      if(!(eng.collection.set[n_set].has_obs(i + 1)))  continue;

      junk = select(*(eng.obs_split), i + 1);

      for(x=0; x<(eng.obs_raw->data.nx()); x++) {
         for(y=0; y<(eng.obs_raw->data.ny()); y++) {

            if( junk.is_nonzero(x, y) ) {
               comp.data.set(1.0, x, y);
            }
         } // end for y
      } // end for x
   } // end for i

   comp.calc_moments();

   return;
}

///////////////////////////////////////////////////////////////////////


double aspect_ratio_conf(double t)

{

const double tm1   = t - 1.0;
const double ratio = (tm1*tm1)/(t*t + 1.0);

return( pow(ratio, 0.3) );

}


///////////////////////////////////////////////////////////////////////
