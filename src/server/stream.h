#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <complex>
#include "packet.h"
#include <pipeline/blob_interface.h>

using boost::asio::ip::tcp;

class StreamHandler;

class Stream: public std::enable_shared_from_this<Stream>
{
public:
  Stream(const Stream&) = delete;
  Stream& operator=(const Stream&) = delete;

  explicit Stream(tcp::socket socket, StreamHandler &handler);

  void Start();
  void Stop();

private:
  void Read();
  void Parse(std::size_t length);

  tcp::socket mSocket;
  input_header_t *mHeader;
  DataVector mData;
  DataVector mXX;
  DataVector mYY;
  StreamHandler &mHandler;
  uint32_t mBytesRead;
  uint64_t mTotalBytesRead;
  double mTime;
};

typedef std::shared_ptr<Stream> StreamPtr;
