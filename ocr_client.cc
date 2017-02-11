#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <iterator>

#include <grpc++/grpc++.h>

#include "ocr.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ocr::OcrRequest;
using ocr::OcrReply;
using ocr::OcrService;

class OcrServiceClient {
 public:
  OcrServiceClient(std::shared_ptr<Channel> channel)
      : stub_(OcrService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Parse(const std::string& user) {
    // Data we are sending to the server.
    OcrRequest request;
    request.set_content(user);

    // Container for the data we expect from the server.
    OcrReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Parse(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.content();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<OcrService::Stub> stub_;
};


int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  OcrServiceClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  std::ifstream ifs(argv[1]);
  std::string str(std::istreambuf_iterator<char>{ifs}, {});
  std::string reply = greeter.Parse(str);
  std::cout << "OcrService received: " << reply << std::endl;

  return 0;
}
