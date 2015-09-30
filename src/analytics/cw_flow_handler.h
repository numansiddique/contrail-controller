#ifndef CW_FLOW_HANDLER_H_
#define CW_FLOW_HANDLER_H_

#include "viz_message.h"
#include "base/task.h"
#include "base/queue_task.h"
#include "base/timer.h"

#include "io/tcp_server.h"
#include "io/tcp_session.h"

#include "http/http_request.h"
#include "http/http_session.h"
#include "http/http_server.h"
#include "http/client/http_client.h"
#include "http/client/http_curl.h"

#include <pthread.h>
#include <tbb/atomic.h>

#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <iostream>

#include <boost/atomic.hpp>


namespace ip = boost::asio::ip;

class CWFlowSession : public TcpSession {
public:
  CWFlowSession(TcpServer *server, Socket *sock);
  virtual void OnRead(Buffer buffer);

};

class CWFlowHandler{
public:
  CWFlowHandler(EventManager *evm);
  ~CWFlowHandler();
  void Init();
  void HandleFlowMessage(VizMsg *vmsgp);
  void HandleFlowMessageResponse(
    const std::string &msg,
    const boost::system::error_code &ec, HttpConnection *conn);
  static void *ThreadRun(void *objp);
  void ProcessFlowMessage(std::string flow_message);
  void Shutdown();
private:
  bool DequeueEvent(EnqueuedCb);
  HttpClient *http_client_;
  boost::asio::ip::tcp::endpoint fs_endpoint_;
  std::string fs_path_;
  pthread_t thread_id_;
  EventManager *evm_;

};

#endif /* CW_FLOW_HANDLER_H_ */
