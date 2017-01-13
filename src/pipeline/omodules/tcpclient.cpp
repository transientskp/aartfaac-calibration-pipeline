#include "tcpclient.h"

#include <glog/logging.h>

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


void TcpClient::Initialize()
{
  mEndPointIterator = mResolver.resolve({ "localhost", "5000" });
  boost::asio::async_connect(mSocket, mEndPointIterator,
                             [this](boost::system::error_code ec, tcp::resolver::iterator)
                             {
                               if (!ec)
                               {
                                 VLOG(1) << "Connected to localhost:5000";
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
                    mSocket.close();
                    VLOG(1) << "Closed localhost:5000";
                  });
}