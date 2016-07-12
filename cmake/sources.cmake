# === All *.{cpp,h} files

# === Pipeline sources
set (PIPELINE_SOURCES
  src/main.cpp
  src/utils/utils.cpp
  src/utils/validators.cpp
  src/utils/antenna_positions.cpp
  src/pipeline/datablob.cpp
  src/pipeline/pmodules/flagger.cpp
  src/pipeline/pmodules/calibrator.cpp
  src/pipeline/pmodules/weighter.cpp
  src/pipeline/omodules/diskwriter.cpp
  src/server/server.cpp
  src/server/stream_handler.cpp
  src/server/stream.cpp
)

# === Test sources

