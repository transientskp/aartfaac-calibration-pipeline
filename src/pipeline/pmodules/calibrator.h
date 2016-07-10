#pragma once

#include <pipeline/processing_module_interface.h>
#include <Eigen/Dense>

#include "../datablob.h"

class Calibrator : public ProcessingModuleInterface<DataBlob>
{
public:
  Calibrator() {}

  virtual std::string Name();
  virtual void Initialize();
  virtual void Run(DataBlob &blob);
};



