#include <iostream>

#include "person.h"
#include "http_client.h"

using namespace std;

int main(int argc, char* argv[]) {
    Person* person = new Person(18, "hello");
    cout << person->display();

    regex expression("^[{](.*)[}]$");
    std::string line = "{hello}";
    cmatch what;
    if(regex_search(line.c_str(),what,expression)){
        //regex_match ： 是对整个输入块的匹配，整个块如不匹配则不能成功
        cout << what.size() << endl;
        for(unsigned int i=0;i<what.size();i++)
            cout<<"str match is :"<<what[i].str()<<endl;
        printf("\n----\n");
    }
    else{
        cout<<"Error Match"<<endl;
    }

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
