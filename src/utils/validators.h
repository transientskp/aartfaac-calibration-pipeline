#pragma once

#include <string>

namespace val 
{
bool ValidateOutput(const char *flagname, const std::string &value);
bool ValidateSubband(const char *flagname, const int value);
bool ValidatePort(const char *flagname, const int value);
bool ValidateNumThreads(const char *flagname, const int value);
bool ValidateChannels(const char *flagname, const std::string &value);
bool ValidateSigma(const char *flagname, const double value);
}
