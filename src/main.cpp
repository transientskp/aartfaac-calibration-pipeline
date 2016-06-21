#include <glog/logging.h>
#include <pipeline/pipeline.h>

#include "version.h"

#define USAGE "AARTFAAC Calibration Pipeline"

DEFINE_int32(nthreads, 2, "Number of pipeline threads");
DEFINE_int32(port, 4100, "Port to listen on for incoming data");

int main(int argc, char *argv[])
{
  FLAGS_logtostderr = 1;
  FLAGS_v = 1;

  ::google::SetUsageMessage(USAGE);
  ::google::SetVersionString(VERSION);
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  VLOG(1) << NAME << " " << VERSION << " (" << BUILD << ")";

  Pipeline pipeline(FLAGS_nthreads);
//  pipeline.AddProcessingModule<Flagger>();
//  pipeline.AddOutputModule<DiskWriter>();
//  pipeline.CreateMemoryPool<MyBlob>(20);
//  pipeline.Start();
//
//  std::vector<uint8_t> data(4, 0);
//
//  int i = 0;
//  int *p = nullptr;
//  while (running && i < 1000)
//  {
//    p = reinterpret_cast<int*>(data.data());
//    *p = i;
//    pipeline.SwapAndProcess(data);
//    i++;
//    std::this_thread::sleep_for(std::chrono::milliseconds(r(mt)/(FLAGS_nthreads+1)));
//  }
//
//  pipeline.Stop();

  return 0;
}
