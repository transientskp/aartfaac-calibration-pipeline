#include "diskwriter.h"

#include <glog/logging.h>

DECLARE_string(output);

std::string DiskWriter::Name()
{
  return "DiskWriter";
}

void DiskWriter::Write(Datum &datum)
{
  mFile.write(reinterpret_cast<char*>(datum.data()), datum.size());
}

DiskWriter::DiskWriter()
{
}

DiskWriter::~DiskWriter()
{
  mFile.close();
}

void DiskWriter::Initialize()
{
  mFile.open(FLAGS_output.substr(5).c_str(), std::ios::binary|std::ios::ate);
}
