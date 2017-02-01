#include "stream.h"
#include "stream_handler.h"
#include "../config.h"
#include "../utils/utils.h"
#include "../utils/timer.h"

#include <glog/logging.h>
#include <immintrin.h>
#include <iomanip>

DECLARE_int32(subband);

Stream::Stream(tcp::socket socket, StreamHandler &handler):
  mSocket(std::move(socket)),
  mHandler(handler)
{
  // create a buffer size b such that:
  //   n = 0 (mod b) and b = 0 (mod m), where
  //   n = 41616*63*2*8 for 288 antennas and n = 166176*63*2*8 for 576 antennas
  //   b = 64
  // mBuffer.resize(332352);
  mBuffer.resize(166464);
  mXX.resize(NUM_BASELINES*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(output_header_t), 0);
  mYY.resize(NUM_BASELINES*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(output_header_t), 0);
}


void Stream::Start()
{
  mBytesRead = mTotalBytesRead = 0;
  VLOG(1) << mSocket.remote_endpoint().address() << ":" << mSocket.remote_endpoint().port() << " connected";
  mTime = timer::GetRealTime();
  Read(sizeof(input_header_t));
}

void Stream::Read(int n)
{
  auto self(shared_from_this());
  boost::asio::async_read(mSocket, boost::asio::buffer(mBuffer.data(), n),
                          [this, self](boost::system::error_code ec, std::size_t length)
                          {
                            if (!ec)
                            {
                              Parse(length);
                            }
                            else if (ec != boost::asio::error::operation_aborted)
                            {
                              mHandler.Stop(shared_from_this());
                            }
                          });
}


void Stream::Parse(std::size_t length)
{
  mTotalBytesRead += length;

  if (length == sizeof(input_header_t))
  {
    memcpy(&mHeader, mBuffer.data(), sizeof(input_header_t));
    mBytesRead = 0;
    Read(mBuffer.size());
    return;
  }

  // here we de-interleave the XX and YY polarization using AVX instructions
  // src: [XX0 YY1 XX2 YY3 XX4 YY5 XX6 YY7 ...]
  // xx:  [XX0 XX2 XX4 XX6 ...]
  // yy:  [YY1 YY3 YY5 YY7 ...]
  const __m256i *src = reinterpret_cast<const __m256i*>(mBuffer.data());
  __m256i *xx = reinterpret_cast<__m256i*>(mXX.data() + mBytesRead/2 + sizeof(output_header_t));
  __m256i *yy = reinterpret_cast<__m256i*>(mYY.data() + mBytesRead/2 + sizeof(output_header_t));

  __m256i a, b, c;
  for (int i = 0, j = 0, n = mBuffer.size()/32; i < n; i += 2, j++)
  {
    a = _mm256_stream_load_si256(src + i);
    b = _mm256_stream_load_si256(src + i + 1);
    c = _mm256_unpacklo_epi64(a, b);
    c = _mm256_permute4x64_epi64(c, 0xd8);
    _mm256_stream_si256(xx + j, c);
    c = _mm256_unpackhi_epi64(a, b);
    c = _mm256_permute4x64_epi64(c, 0xd8);
    _mm256_stream_si256(yy + j, c);
  }
  mBytesRead += length;

  if (mBytesRead >= NUM_BASELINES*NUM_POLARIZATIONS*NUM_CHANNELS*8)
  {
    CHECK(mHeader.magic == INPUT_MAGIC) << "Invalid magic!";
    // copy header to pols
    output_header_t hdr;
    hdr.subband = FLAGS_subband;
    hdr.start_time = mHeader.startTime;
    hdr.end_time = mHeader.endTime;
    hdr.flagged_dipoles.reset();
    hdr.flagged_channels.reset();
    hdr.flagged_channels[0] = true;
    hdr.num_channels = NUM_CHANNELS + 1;
    hdr.magic = OUTPUT_MAGIC;
    hdr.num_dipoles = NUM_ANTENNAS;
    memcpy(hdr.weights, mHeader.weights, 78*sizeof(uint32_t));

    hdr.polarization = 0;
    memcpy(mXX.data(), &hdr, sizeof(hdr));
    hdr.polarization = 1;
    memcpy(mYY.data(), &hdr, sizeof(hdr));

    mHandler.mPipeline.SwapAndProcess(mXX);
    mHandler.mPipeline.SwapAndProcess(mYY);

    Read(sizeof(input_header_t));
  }
  else
    Read(mBuffer.size());
}


void Stream::Stop()
{
  mTime = timer::GetRealTime() - mTime;
  VLOG(1) << "Throughput " << (mTotalBytesRead*8/(mTime*1e9)) << " Gb/s";
  VLOG(1) << mSocket.remote_endpoint().address() << ":" << mSocket.remote_endpoint().port() << " disconnected";
  mSocket.close();
}
