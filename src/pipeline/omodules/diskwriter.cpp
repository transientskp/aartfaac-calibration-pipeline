#include "diskwriter.h"

#include <glog/logging.h>
#include <boost/algorithm/string.hpp>

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
  std::string filename;
  auto start = FLAGS_output.find("file:");
  auto end = FLAGS_output.find(",", start);

  if (end != std::string::npos)
    filename = FLAGS_output.substr(start+5, end-5);
  else
    filename = FLAGS_output.substr(start+5);
  VLOG(1) << "Writing data to `" << filename << "'";
  mFile.open(filename.c_str(), std::ios::binary|std::ios::ate);
}
