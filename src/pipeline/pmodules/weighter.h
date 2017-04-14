#pragma once

#include <pipeline/processing_module_interface.h>
#include <Eigen/Dense>

#include "../datablob.h"

class Weighter : public ProcessingModuleInterface<DataBlob>
{
public:
  Weighter(){}

  virtual std::string Name();
  virtual void Initialize(DataBlob &blob);
  virtual void Run(DataBlob &blob);

private:
  int32_t Index(int i, int j);

  uint32_t mMaxNum;
  DataBlob *mBlob;
};
