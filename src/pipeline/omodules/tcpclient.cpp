#include "tcpclient.h"

#include <glog/logging.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

DECLARE_string(output);

std::string TcpClient::Name()
{
  return "TcpClient";
}

void TcpClient::Write(Datum &datum)
{
  mIoService.post(
    [this, datum]()
    {
      bool ready = mDeque.empty();
      mDeque.push_back(datum);
      if (ready)
      {
        DoWrite();
      }
    }
  );
}


void TcpClient::DoWrite()
{
  boost::asio::async_write(mSocket,
                           boost::asio::buffer(mDeque.front().data(),
                                               mDeque.front().size()),
                           [this](boost::system::error_code ec, std::size_t /*length*/)
                           {
                             if (!ec)
                             {
                               mDeque.pop_front();
                               if (!mDeque.empty())
                               {
                                 DoWrite();
                               }
                             }
                             else
                             {
                               LOG(ERROR) << "Closing socket";
                               mSocket.close();
                             }
                           });
}


TcpClient::TcpClient():
  mSocket(mIoService),
  mResolver(mIoService),
  mBuffer(1024)
{
}


TcpClient::~TcpClient()
{
  Close();
  mThread.join();
}


void TcpClient::Read()
{
  boost::asio::async_read(mSocket,
                          boost::asio::buffer(mBuffer.data(), mBuffer.size()),
                          [this](boost::system::error_code ec, std::size_t /*length*/)
                          {
                            if (!ec)
                            {
                              Read();
                            }
                            else
                            {
                              mSocket.close();
                            }
                          });
}


void TcpClient::Initialize() {
  std::vector<std::string> list, address;
  boost::split(list, FLAGS_output, boost::is_any_of(","));
  for (auto &s : list)
  {
    if (s.substr(0, 3) == "tcp")
    {
      boost::split(address, s, boost::is_any_of(":"));
      break;
    }
  }
  mEndPointIterator = mResolver.resolve({ address[1], address[2] });
  boost::asio::async_connect(mSocket, mEndPointIterator,
                             [this](boost::system::error_code ec, tcp::resolver::iterator)
                             {
                               if (!ec)
                               {
                                 VLOG(1) << "Connected to " << mSocket.remote_endpoint().address() << ":" << mSocket.remote_endpoint().port();
                                 Read();
                               }
                               else
                               {
                                 LOG(ERROR) << ec.message();
                               }
                             });

  mThread = std::thread([this]() {mIoService.run();} );
}


void TcpClient::Close()
{
  mIoService.post([this]()
                  {
                    VLOG(1) << "Closing " << mSocket.remote_endpoint().address() << ":" << mSocket.remote_endpoint().port();
                    mSocket.close();
                  });
}