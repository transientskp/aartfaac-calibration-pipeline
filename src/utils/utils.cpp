#include "utils.h"

namespace utils
{

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

}
