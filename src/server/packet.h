#pragma once

#include <stdint.h>

#define OUTPUT_MAGIC 0x4141525446414143
#define INPUT_MAGIC 0x3B98F002

struct input_header_t
{
  uint32_t magic;               ///< magic to determine header
  uint32_t pad0;                ///< first padding
  double   startTime;           ///< start time (unix)
  double   endTime;             ///< end time (unix)
  uint32_t weights[87];         ///< station weights n*(n+1) / 2, where n in {6, 12}
  uint8_t  pad1[140];           ///< 512 byte block
};

struct output_header_t
{
  uint64_t magic;               ///< magic to determine header
  double start_time;            ///< start time (unix)
  double end_time;              ///< end time (unix)
  int32_t subband;              ///< lofar subband
  int32_t num_dipoles;          ///< number of dipoles (288 or 576)
  int32_t polarization;         ///< XX=0, YY=1
  uint64_t flagged_channels;    ///< bitset of flagged channels
  uint64_t flagged_dipoles[9];  ///< bitset of flagged dipoles
  uint8_t pad[512-120];         ///< 512 byte block
};

static_assert(sizeof(input_header_t) == 512, "header must be 512 bytes");
static_assert(sizeof(output_header_t) == 512, "header must be 512 bytes");