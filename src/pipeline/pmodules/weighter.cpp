#include "weighter.h"

std::string Weighter::Name()
{
  return "Weighter";
}

void Weighter::Initialize()
{
}

void Weighter::Run(DataBlob &b)
{
  uint32_t sum = 0;
  for (int i = 0; i < 87; i++)
    sum += b.mHdr->weights[i];

  // Header doesn't support weights yet
  if (sum == 0)
    return;
}