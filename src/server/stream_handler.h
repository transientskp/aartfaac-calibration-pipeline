#pragma once

#include <set>
#include <pipeline/pipeline.h>
#include "stream.h"

class StreamHandler
{
public:
  StreamHandler(const StreamHandler&) = delete;
  StreamHandler& operator=(const StreamHandler&) = delete;

  StreamHandler(Pipeline &pipeline);

  void Start(StreamPtr stream);
  void Stop(StreamPtr stream);
  void StopAll();

  Pipeline &mPipeline;

private:
  std::set<StreamPtr> mStreams;
};
