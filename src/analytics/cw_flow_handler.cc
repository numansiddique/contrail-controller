#include "cw_flow_handler.h"
#include <sandesh/sandesh_types.h>
#include <sandesh/sandesh.h>
#include <sandesh/sandesh_message_builder.h>
#include <sandesh/protocol/TXMLProtocol.h>
#include <boost/bind.hpp>

#include "io/event_manager.h"

#define CW_FLOW_SERVER_PORT 9933


boost::lockfree::queue<std::string *> flow_queue_(1000);
boost::atomic<bool> thread_done_(false);


CWFlowHandler::CWFlowHandler(EventManager *evm)
 : http_client_(NULL), thread_id_(pthread_self()), evm_(evm) {
    boost::system::error_code ec;
    fs_endpoint_.address(boost::asio::ip::address::from_string("127.0.0.1", ec));
    fs_endpoint_.port(CW_FLOW_SERVER_PORT);
    fs_path_ = "flows";
}

CWFlowHandler::~CWFlowHandler() {

}

void CWFlowHandler::Init() {
    if (!http_client_) {
        http_client_ = new HttpClient(evm_);
        http_client_->Init();
        pthread_create(&thread_id_, NULL, &CWFlowHandler::ThreadRun, this);
    }
}


void CWFlowHandler::Shutdown() {
    thread_done_ = true;
    delete http_client_;
    http_client_ = NULL;
}

void CWFlowHandler::ProcessFlowMessage(std::string flow_message) {
  if (http_client_) {
    HttpConnection *conn = http_client_->CreateConnection(fs_endpoint_);
    conn->HttpPut(flow_message, fs_path_,
                  boost::bind(&CWFlowHandler::HandleFlowMessageResponse, this,
                  _1, _2, conn));
  }

}

void CWFlowHandler::HandleFlowMessage(VizMsg *vmsgp) {
  std::string *flow_message = new std::string((vmsgp->msg->ExtractMessage()));
  flow_queue_.push(flow_message);
}

void CWFlowHandler::HandleFlowMessageResponse(
    const std::string &msg,
    const boost::system::error_code &ec, HttpConnection *conn) {
    if (http_client_)
        http_client_->RemoveConnection(conn);
}


void *CWFlowHandler::ThreadRun(void *objp) {
    CWFlowHandler *flow_handler = static_cast<CWFlowHandler *>(objp);
    std::string *flow_message;
    while(!thread_done_) {
         while (flow_queue_.pop(flow_message)) {
            flow_handler->ProcessFlowMessage(*flow_message);
            delete flow_message;
            sleep(1);
        }
        sleep(1);
    }
    return NULL;
}
