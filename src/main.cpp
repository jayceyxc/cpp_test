#include <iostream>

#include "person.h"
#include "http_client.h"

using namespace std;

int main(int argc, char* argv[]) {
    Person* person = new Person(18, "hello");
    cout << person->display();

    try
    {
      boost::asio::io_service io_service;
//    boostHttpclient_c c(io_service, "192.168.1.46:8187", "/handle_post_request");
      boostHttpclient_c c(io_service, "112.124.33.66:9988", "/inmobibid");
//    client c(io_service, "192.168.1.46:8187", "/");
      io_service.run();
    }
    catch (std::exception& e)
    {
      std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
