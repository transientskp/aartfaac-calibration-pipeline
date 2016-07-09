#pragma once

#include <cstdlib>
#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include <pipeline/pipeline.h>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <bits/stl_set.h>

#include "packet.h"
#include "stream.h"
#include "stream_handler.h"
#include "../pipeline/datablob.h"

using boost::asio::ip::tcp;

class Server
{
public:
  Server(boost::asio::io_service& io_service, Pipeline<DataBlob> &pipeline, uint16_t port);

private:
  void Listen();
  void DoAwaitStop();

  StreamHandler mStreamHandler;
  boost::asio::signal_set mSignals;
  tcp::acceptor mAcceptor;
  tcp::socket mSocket;
};
