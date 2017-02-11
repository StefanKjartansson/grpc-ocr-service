#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <grpc++/grpc++.h>

#include "ocr.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using ocr::OcrRequest;
using ocr::OcrReply;
using ocr::OcrService;

std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

// Logic and data behind the server's behavior.
class OcrServiceServiceImpl final : public OcrService::Service {
  Status Parse(ServerContext* context, const OcrRequest* request,
                  OcrReply* reply) override {

    std::string fileName = random_string(16) + ".png";
    std::ofstream file(fileName);
    file << request->content();
    file.close();

    char *outText;

    // TODO: reuse. 
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

    // TODO: language in flag or envvar.
    if (api->Init(NULL, "deu")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = pixRead(fileName.c_str());
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    printf("OCR output:\n%s", outText);
    reply->set_content(outText);

    // Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);
    
    // TODO: delete file.

    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  OcrServiceServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
