#include "server.h"

#include <glog/logging.h>

Server::Server(boost::asio::io_service &io_service, Pipeline<DataBlob> &pipeline, uint16_t port):
  mStreamHandler(pipeline),
  mSignals(io_service),
  mAcceptor(io_service, tcp::endpoint(tcp::v4(), port)),
  mSocket(io_service)
{
  mSignals.add(SIGINT);
  mSignals.add(SIGTERM);
  mSignals.add(SIGQUIT);
  DoAwaitStop();

  VLOG(1) << "Listening on port " << port;
  Listen();
}

void Server::Listen()
{
  mAcceptor.async_accept(mSocket,
                         [this](boost::system::error_code ec)
                         {
                           if (!mAcceptor.is_open())
                             return;

                           if (!ec)
                           {
                             size_t size = 64*1024*1024;
                             boost::asio::socket_base::receive_buffer_size option(size);
                             mSocket.set_option(option);
                             mStreamHandler.Start(std::make_shared<Stream>(std::move(mSocket), mStreamHandler));
                           }
                           else
                             LOG(ERROR) << ec.message();

                           Listen();
                         });
}

void Server::DoAwaitStop()
{
  mSignals.async_wait(
    [this](boost::system::error_code /*ec*/, int s)
    {
      LOG(INFO) << "Received signal `" << strsignal(s) << "'";
      mAcceptor.close();
      mStreamHandler.StopAll();
    }
  );
}




