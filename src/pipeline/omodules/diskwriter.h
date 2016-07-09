#pragma once

#include <pipeline/output_module_interface.h>
#include <fstream>

class DiskWriter : public OutputModuleInterface
{
public:
  virtual void Initialize();

  DiskWriter();
  ~DiskWriter();

  virtual void Write(Datum &datum);
  virtual std::string Name();

private:
  std::ofstream mFile;
};
