#pragma once

#include <pipeline/output_module_interface.h>
#include <fstream>
#include <thread>
#include <deque>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class TcpClient : public OutputModuleInterface
{
public:
  virtual void Initialize();

  TcpClient();
  ~TcpClient();

  virtual void Write(Datum &datum);
  virtual std::string Name();

private:
  boost::asio::io_service mIoService;
  tcp::socket mSocket;
  std::thread mThread;
  tcp::resolver mResolver;
  tcp::resolver::iterator mEndPointIterator;
  std::deque<Datum> mDeque;
  Datum mBuffer;
  void Close();
  void DoWrite();
  void Read();
};
