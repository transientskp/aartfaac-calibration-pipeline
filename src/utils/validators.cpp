#include "validators.h"
#include "utils.h"

#include <vector>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

namespace val
{

bool ValidatePort(const char *flagname, const int value)
{
  (void) flagname;
  return value > 0 && value < (1<<16);
}

bool ValidateSubband(const char *flagname, const int value)
{
  (void) flagname;
  return value >= 0 && value < 512;
}

bool ValidateNumThreads(const char *flagname, const int value)
{
  (void) flagname;
  return value > 0;
}

bool ValidateSigma(const char *flagname, const double value)
{
  (void) flagname;
  return value > 0.0;
}

bool ValidateAntCfg(const char *flagname, const int value)
{
  (void) flagname;
  return value >= 0 && value <= 3;
}

bool ValidateChannels(const char *flagname, const std::string &value)
{
  (void) flagname;
  auto ranges = utils::ParseChannels(value);

  if (ranges.empty())
    return false;

  auto &a = ranges[0];
  if (ranges.size() == 1)
    return a.first <= a.second;

  for (int i = 1, n = ranges.size(); i < n; i++)
  {
    if (a.second >= ranges[i].first)
      return false;

    if (a.first > a.second)
      return false;

    a = ranges[i];

    if (a.first > a.second)
      return false;
  }

  return true;
}

bool ValidateOutput(const char *flagname, const std::string &value)
{
  (void) flagname;
  std::vector<std::string> list;
  boost::split(list, value, boost::is_any_of(","));

  if (list.size() > 2)
    return false;

  bool valid = true;
  for (auto &s : list)
    valid = valid && s.size() > 5 && ("tcp:" == s.substr(0, 4) || s.substr(0, 5) == "file:");
  return valid;
}

bool ValidateFile(const char *flagname, const std::string &value)
{
  (void) flagname;
  std::ifstream file(value, std::ifstream::in);
  return file.good();
}

} // namespace val
