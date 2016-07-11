#pragma once

#include <pipeline/processing_module_interface.h>
#include <Eigen/Dense>

#include "../datablob.h"

class Weighter : public ProcessingModuleInterface<DataBlob>
{
public:
  Weighter(){}

  virtual std::string Name();
  virtual void Initialize();
  virtual void Run(DataBlob &blob);
};
