#include "stream.h"
#include "stream_handler.h"
#include "../config.h"
#include "../utils/utils.h"
#include "../utils/timer.h"

#include <glog/logging.h>
#include <iomanip>

DECLARE_int32(subband);

Stream::Stream(tcp::socket socket, StreamHandler &handler):
  mSocket(std::move(socket)),
  mHandler(handler)
{
  mData.resize(NUM_BASELINES*NUM_POLARIZATIONS*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(input_header_t), 0);
  mXX.resize(NUM_BASELINES*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(input_header_t), 0);
  mYY.resize(NUM_BASELINES*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(input_header_t), 0);

  mHeader = reinterpret_cast<input_header_t*>(mData.data());
}


void Stream::Start()
{
  mBytesRead = mTotalBytesRead = 0;
  VLOG(1) << mSocket.remote_endpoint().address() << ":" << mSocket.remote_endpoint().port() << " connected";
  mTime = timer::GetRealTime();
  Read();
}


void Stream::Read()
{
  auto self(shared_from_this());

  // 256 is the biggest number divisible to mData.size()
  mSocket.async_read_some(boost::asio::buffer(mData.data()+mBytesRead, 256),
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
  mBytesRead += length;
  mTotalBytesRead += length;

  CHECK(mBytesRead <= mData.size());

  if (mBytesRead == mData.size())
  {
    CHECK(mHeader->magic == INPUT_MAGIC) << "Invalid magic!";

    // copy header to pols
    output_header_t hdr;
    hdr.subband = FLAGS_subband;
    hdr.start_time = mHeader->startTime;
    hdr.end_time = mHeader->endTime;
    hdr.polarization = 0;
    hdr.magic = OUTPUT_MAGIC;
    hdr.num_dipoles = 288;

    memcpy(mXX.data(), &hdr, sizeof(hdr));
    hdr.polarization = 1;
    memcpy(mYY.data(), &hdr, sizeof(hdr));

    std::complex<float> *xx = reinterpret_cast<std::complex<float>*>(mXX.data() + sizeof(input_header_t));
    std::complex<float> *yy = reinterpret_cast<std::complex<float>*>(mYY.data() + sizeof(input_header_t));
    std::complex<float> *src = reinterpret_cast<std::complex<float>*>(mData.data() + sizeof(input_header_t));

    // XX pol
    for (int b = 0, i = 0; b < NUM_BASELINES; b++)
      for (int c = 0; c < NUM_CHANNELS; c++, i++)
        memcpy(&xx[i],
               &src[0 + c*NUM_POLARIZATIONS + b*NUM_POLARIZATIONS*NUM_CHANNELS],
               sizeof(std::complex<float>));

    mHandler.mPipeline.SwapAndProcess(mXX);

    // YY pol
    for (int b = 0, i = 0; b < NUM_BASELINES; b++)
      for (int c = 0; c < NUM_CHANNELS; c++, i++)
        memcpy(&yy[i],
               &src[1 + c*NUM_POLARIZATIONS + b*NUM_POLARIZATIONS*NUM_CHANNELS],
               sizeof(std::complex<float>));

    mHandler.mPipeline.SwapAndProcess(mYY);
    mBytesRead = 0;
  }

  Read();
}


void Stream::Stop()
{
  mTime = timer::GetRealTime() - mTime;
  VLOG(1) << "Throughput " << (mTotalBytesRead*8/(mTime*1e9)) << " Gb/s";
  VLOG(1) << mSocket.remote_endpoint().address() << ":" << mSocket.remote_endpoint().port() << " disconnected";
  mSocket.close();
}
