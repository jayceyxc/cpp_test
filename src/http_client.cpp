#include "http_client.h"


boostHttpclient_c::boostHttpclient_c(boost::asio::io_service& io_service,
      const std::string& server, const std::string& path, const std::string &post_content)
    : resolver_(io_service),
      socket_(io_service)
{
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream(&request_);

    // postString是要发送给DSP产商的json格式的竞价请求内容
    // string postString = "{\"id\":\"ec02aa0c-015d-1000-d176-00837f3700b1}\"";

    request_stream << "POST " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";

    request_stream << "Content-Type: " << "Content-type: application/json" << "\r\n";
    request_stream << "Content-Length: " << post_content.length() << "\r\n";
    request_stream << "Connection: keep-alive\r\n\r\n";
    request_stream << post_content ;


    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    tcp::resolver::query query("112.124.33.66", "9988");
    resolver_.async_resolve(query,
        boost::bind(&boostHttpclient_c::handle_resolve, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::iterator));
}

void boostHttpclient_c::handle_resolve(const boost::system::error_code& err,
  tcp::resolver::iterator endpoint_iterator)
{
    if (!err)
    {
      // Attempt a connection to each endpoint in the list until we
      // successfully establish a connection.
      boost::asio::async_connect(socket_, endpoint_iterator,
          boost::bind(&boostHttpclient_c::handle_connect, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
}

void boostHttpclient_c::handle_connect(const boost::system::error_code& err)
{
    if (!err)
    {
      // The connection was successful. Send the request.
      boost::asio::async_write(socket_, request_,
          boost::bind(&boostHttpclient_c::handle_write_request, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
}

void boostHttpclient_c::handle_write_request(const boost::system::error_code& err)
{

    if (!err)
    {
      // Read the response status line. The response_ streambuf will
      // automatically grow to accommodate the entire line. The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      printf("http write OK start listen \n ");
      boost::asio::async_read_until(socket_, response_, "\r\n",
          boost::bind(&boostHttpclient_c::handle_read_status_line, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
}

void boostHttpclient_c::handle_read_status_line(const boost::system::error_code& err)
{
    if (!err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }

      printf("response headers get OK \n ");
      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
          boost::bind(&boostHttpclient_c::handle_read_headers, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
}

void boostHttpclient_c::handle_read_headers(const boost::system::error_code& err)
{
    if (!err)
    {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r")
        std::cout << header << "\n";
      std::cout << "\n";

      // Start reading remaining data until EOF.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&boostHttpclient_c::handle_read_content, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
}

void boostHttpclient_c::handle_read_content(const boost::system::error_code& err)
{
    if (!err)
    {
      boost::asio::streambuf::const_buffers_type bufs = response_.data();
      std::string line(
          boost::asio::buffers_begin(bufs),
          boost::asio::buffers_begin(bufs) + response_.size());

      if(line.find("\r\n\r\n")!=string::npos){
        std::cout << line;
        // 匹配这样的字符串, 用花括号括起来的内容: {hello}
        regex expression("^[{](.*)[}]$");
        cmatch what;
        if(regex_search(line.c_str(),what,expression)){
            //regex_match ： 是对整个输入块的匹配，整个块如不匹配则不能成功
            for(unsigned int i=0;i<what.size();i++)
                cout<<"str match is :"<<what[i].str()<<endl;
            printf("\n----\n");
        }
        else{
            cout<<"Error Match"<<endl;
        }
      }

      // Continue reading remaining data until EOF.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&boostHttpclient_c::handle_read_content, this,
            boost::asio::placeholders::error));
    }
    else if (err != boost::asio::error::eof)
    {
      std::cout << "Error: " << err << "\n";
    }
}

/*
int main(int argc, char* argv[])
{
  try
  {
    boost::asio::io_service io_service;
    boostHttpclient_c c(io_service, "112.124.33.66:9988", "/inmobibid");
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
*/
