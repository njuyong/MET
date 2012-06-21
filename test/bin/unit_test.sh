#!/bin/bash

# Check that required unit test environment variables are set
if [[ -z "${MET_BASE}"       ||
      -z "${MET_TEST_BASE}"  ||
      -z "${MET_TEST_INPUT}" ||
      -z "${MET_TEST_OUTPUT}" ]]; then
  echo
  echo "ERROR: Required environment variable(s) unset:"
  echo "ERROR: \${MET_BASE}, \${MET_TEST_BASE}, \${MET_TEST_INPUT}, \${MET_TEST_OUTPUT}"
  echo
  exit
else
  echo "export MET_BASE=${MET_BASE}"
  echo "export MET_TEST_BASE=${MET_TEST_BASE}"
  echo "export MET_TEST_INPUT=${MET_TEST_OUTPUT}"
  echo "export MET_TEST_OUTPUT=${MET_TEST_OUTPUT}"
fi

# Unit test script
PERL_UNIT=${MET_TEST_BASE}/perl/unit.pl

# Unit test XML
UNIT_XML="unit_ascii2nc.xml \
          unit_madis2nc.xml \
          unit_trmm2nc.xml \
          unit_pb2nc.xml \
          unit_gen_poly_mask.xml \
          unit_pcp_combine.xml \
          unit_wwmca_regrid.xml \
          unit_point_stat.xml \
          unit_duplicate_flag.xml \
          unit_grid_stat.xml \
          unit_wavelet_stat.xml \
          unit_ensemble_stat.xml \
          unit_mode.xml \
          unit_plot_point_obs.xml \
          unit_plot_data_plane.xml \
          unit_wwmca_plot.xml \
          unit_met_test_scripts.xml"

# Run each unit test
for CUR_XML in ${UNIT_XML}; do

  echo
  echo "CALLING: ${PERL_UNIT} ${MET_TEST_BASE}/xml/${CUR_XML}"
  echo
  ${PERL_UNIT} ${MET_TEST_BASE}/xml/${CUR_XML}
  RET_VAL=$?

  # Fail on non-zero return status
  if [ ${RET_VAL} != 0 ]; then
    echo
    echo "ERROR: ${PERL_UNIT} ${CUR_XML} failed."
    echo
    echo "*** UNIT TESTS FAILED ***"
    echo
    exit 1
  fi
  
done

# Success if we reach here
echo
echo "*** ALL UNIT TESTS PASSED ***"
echo
