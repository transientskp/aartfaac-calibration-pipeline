#include "utils.h"

namespace utils
{
static const double seconds_between_julian_and_unix_epoch = 3506716800.0;

double MJDs2JD(const double inMJDs)
{
  return (inMJDs / 86400.0) + 2400000.5;
}

double MJD2UnixTime(const double inMJD)
{
  return inMJD - seconds_between_julian_and_unix_epoch;
}

double UnixTime2MJD(const double inUnixTime)
{
  return inUnixTime + seconds_between_julian_and_unix_epoch;
}

float Subband2Frequency(const int subband, const float clock)
{
  static const float lofar_subband_width = clock / 1024.0;
  return lofar_subband_width * subband;
}

float Channel2Frequency(const int subband, const int channel, const float clock)
{
  static const int   channels_per_subband = 64;
  static const float lofar_subband_width = clock / 1024.0f;
  static const float half = lofar_subband_width / 2.0f;
  static const float channel_width = lofar_subband_width / channels_per_subband;
  return (Subband2Frequency(subband, clock) - half) + channel * channel_width;
}

float Range2Frequency(const int subband, const int start, const int end, const float clock)
{
  return (Channel2Frequency(subband, start, clock) + Channel2Frequency(subband, end, clock)) * 0.5f;
}

int RoundTo(const float x, const int n)
{
  return static_cast<int>(n * roundf(x/n));
}

int PopCount(uint64_t x)
{
  int pop;
  for (pop = 0; x != 0ul; pop++)
    x &= x - 1ul;
  return pop;
}

std::vector<int> ParseAffinity(const std::string &value)
{
  std::vector<int> affinity;
  std::string num;
  int v;

  for (auto &c : value)
  {
    switch(c)
    {
      case ',':
        v = std::atoi(num.c_str());
        num = "";
        affinity.push_back(v);
        break;
      default:
        if (std::isdigit(c))
          num += c;
        break;
    }
  }

  affinity.push_back(std::atoi(num.c_str()));
  return affinity;
}

std::vector<std::pair<int,int>> ParseChannels(const std::string &value)
{
  std::vector<std::pair<int,int>> channels;
  std::string channel = "";
  std::pair<int,int> range;

  for (auto &c : value)
  {
    switch (c)
    {
      case '-':
        range.first = std::atoi(channel.c_str());
        channel = "";
        break;
      case ',':
        range.second = std::atoi(channel.c_str());
        channel = "";
        channels.push_back(range);
        break;
      default:
        if (std::isdigit(c))
          channel += c;
        break;
    }
  }

  range.second = std::atoi(channel.c_str());
  channels.push_back(range);
  std::sort(channels.begin(), channels.end());

  return channels;
}

void sunRaDec(const double inJD, double &outRa, double &outDec)
{
  double n = inJD - 2451545.0;
  double L = std::fmod(280.460 + 0.9856474 * n, 360.0);
  double g = std::fmod(357.528 + 0.9856003 * n, 360.0);
  double lambda = L + 1.915 * sin(DEG2RAD(g)) + 0.020 * sin(DEG2RAD(2*g));
  double epsilon = 23.439 - 0.0000004 * n;
  outRa = atan2(cos(DEG2RAD(epsilon)) * sin(DEG2RAD(lambda)), cos(DEG2RAD(lambda)));
  outDec = asin(sin(DEG2RAD(epsilon)) * sin(DEG2RAD(lambda)));
}

}
