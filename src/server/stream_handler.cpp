#include "stream_handler.h"

void StreamHandler::Start(StreamPtr stream)
{
  mStreams.insert(stream);
  stream->Start();
}

void StreamHandler::Stop(StreamPtr stream)
{
  mStreams.erase(stream);
  stream->Stop();
}

void StreamHandler::StopAll()
{
  for (auto s: mStreams)
    s->Stop();
  mStreams.clear();
}

StreamHandler::StreamHandler(Pipeline<DataBlob> &pipeline):
  mPipeline(pipeline)
{

}