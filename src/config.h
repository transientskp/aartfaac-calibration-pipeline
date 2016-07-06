#pragma once

#include "utils/utils.h"

/** Number of polarizations */
constexpr int NUM_POLARIZATIONS = 2;

/** Number of antennas per station */
constexpr int NUM_ANTENNAS_PER_STATION = 96;

/** Number of stations */
constexpr int NUM_STATIONS = 6;

/** Number of channels per subband */
constexpr int NUM_CHANNELS = 63;

/** Number of active antennas */
constexpr int NUM_ANTENNAS = (NUM_STATIONS*NUM_ANTENNAS_PER_STATION/2);

/** Number of baselines including self correlation */
constexpr int NUM_BASELINES = (NUM_ANTENNAS*(NUM_ANTENNAS+1)/2);

/** Number of total antennas */
constexpr int NUM_TOTAL_ANTENNAS = (NUM_ANTENNAS_PER_STATION*NUM_STATIONS);
