#pragma once

#include <set>
#include <pipeline/pipeline.h>
#include "stream.h"
#include "../pipeline/datablob.h"

class StreamHandler
{
public:
  StreamHandler(const StreamHandler&) = delete;
  StreamHandler& operator=(const StreamHandler&) = delete;

  StreamHandler(Pipeline<DataBlob> &pipeline);

  void Start(StreamPtr stream);
  void Stop(StreamPtr stream);
  void StopAll();

  Pipeline<DataBlob> &mPipeline;

private:
  std::set<StreamPtr> mStreams;
};
