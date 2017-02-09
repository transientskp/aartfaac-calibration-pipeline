#include <glog/logging.h>
#include <boost/asio.hpp>
#include <pipeline/pipeline.h>
#include <boost/algorithm/string.hpp>

#include "version.h"
#include "config.h"
#include "pipeline/datablob.h"
#include "server/server.h"
#include "utils/validators.h"
#include "utils/antenna_positions.h"
#include "pipeline/omodules/diskwriter.h"
#include "pipeline/omodules/tcpclient.h"
#include "pipeline/pmodules/flagger.h"
#include "pipeline/pmodules/calibrator.h"
#include "pipeline/pmodules/weighter.h"

#define USAGE "AARTFAAC Calibration Pipeline"

DEFINE_int32(nthreads, 2, "Number of pipeline threads");
DEFINE_int32(antcfg, 0, "0=LBA_OUTER, 1=LBA_INNER, 2=LBA_SPARSE_EVEN, 3=LBA_SPARSE_ODD");
DEFINE_int32(port, 4000, "Port to listen on for incoming data");
DEFINE_int32(buffer, 20, "Ringbuffer size in number of seconds");
DEFINE_string(output, "", "Output locations e.g. 'tcp:127.0.0.1:5000,file:/tmp/calibrated.vis'");
DEFINE_string(antpos, "", "Antenna positions filename");
DEFINE_int32(subband, -1, "Lofar subband that defines the frequency of incoming data");
DEFINE_string(channels, "0-62", "List of channel ranges to use e.g. '0-10,12-30,31-31' (inclusive)");
DEFINE_double(antsigma, 4.0, "Sigma used for clipping of antennas");
DEFINE_double(vissigma, 3.0, "Sigma used for clipping of visibilities across channels");

int main(int argc, char *argv[])
{
  FLAGS_alsologtostderr = 1;
  FLAGS_v = 1;

  ::google::RegisterFlagValidator(&FLAGS_antsigma, &val::ValidateSigma);
  ::google::RegisterFlagValidator(&FLAGS_vissigma, &val::ValidateSigma);
  ::google::RegisterFlagValidator(&FLAGS_subband, &val::ValidateSubband);
  ::google::RegisterFlagValidator(&FLAGS_port, &val::ValidatePort);
  ::google::RegisterFlagValidator(&FLAGS_nthreads, &val::ValidateNumThreads);
  ::google::RegisterFlagValidator(&FLAGS_channels, &val::ValidateChannels);
  ::google::RegisterFlagValidator(&FLAGS_output, &val::ValidateOutput);
  ::google::RegisterFlagValidator(&FLAGS_antpos, &val::ValidateFile);
  ::google::RegisterFlagValidator(&FLAGS_antcfg, &val::ValidateAntCfg);


  ::google::SetUsageMessage(USAGE);
  ::google::SetVersionString(VERSION);
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  AntennaPositions::CreateInstance(FLAGS_antpos);

  Pipeline<DataBlob> pipeline(FLAGS_nthreads);
  pipeline.CreateMemoryPool(
      NUM_BASELINES*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(output_header_t),
      FLAGS_buffer*2);
  pipeline.AddProcessingModule<Weighter>();
  pipeline.AddProcessingModule<Flagger>();
  pipeline.AddProcessingModule<Calibrator>();

  std::vector<std::string> list;
  boost::split(list, FLAGS_output, boost::is_any_of(","));

  for (auto &s : list)
  {
    if (s.substr(0, 4) == "file")
      pipeline.AddOutputModule<DiskWriter>();
    else if (s.substr(0, 3) == "tcp")
      pipeline.AddOutputModule<TcpClient>();
  }
  pipeline.Start();

  try
  {
    boost::asio::io_service io_service;
    Server s(io_service, pipeline, FLAGS_port);
    io_service.run();
  }
  catch (std::exception& e)
  {
    LOG(ERROR) << e.what();
  }

  pipeline.Stop();

  return 0;
}
