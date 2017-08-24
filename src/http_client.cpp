#include<http_client.h>


boostHttpclient_c::boostHttpclient_c(boost::asio::io_service& io_service,
      const std::string& server, const std::string& path)
    : resolver_(io_service),
      socket_(io_service)
{
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream(&request_);

    string postString = "{\"id\":\"ec02aa0c-015d-1000-d176-00837f3700b1}\"";

    request_stream << "POST " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";

    request_stream << "Content-Type: " << "application/x-www-form-urlencoded" << "\r\n";
    request_stream << "Content-Length: " << postString.length() << "\r\n";
    request_stream << "Connection: keep-alive\r\n\r\n";
    request_stream << postString ;


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

      // Write whatever content we already have to output.
    //      if (response_.size() > 0){
    ////    	  std::cout << &response_;
    //    	  boost::asio::streambuf::const_buffers_type bufs = response_.data();
    //    	  std::string line(
    //    	      boost::asio::buffers_begin(bufs),
    //    	      boost::asio::buffers_begin(bufs) + response_.size());
    //    	  std::cout << line;
    //      }

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
      // Write all of the data that has been read so far.
    //      std::cout << &response_;
      boost::asio::streambuf::const_buffers_type bufs = response_.data();
      std::string line(
          boost::asio::buffers_begin(bufs),
          boost::asio::buffers_begin(bufs) + response_.size());

      if(line.find("\r\n\r\n")!=string::npos){
        std::cout << line;
        regex expression("^[\{](.*)[\}]$");
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

    //  		zoomInfo_t zoomInfo ; PointInfo_t tmpPointInfo ; int sceneWidth ,sceneHeight,WatchZoneID ;
    //  		list<PointInfo_t> *tmpZoom = &zoomInfo.zoomPonitArray ;
    //  		cJSON *root = cJSON_Parse(what[0].str().c_str());
    //  		if(root==NULL){
    //  			return ;
    //  		}
    //
    //  		cJSON *tmp = cJSON_GetObjectItem(root,"sceneWidth") ;
    //  		if(tmp!=NULL){
    //  			sceneWidth 	= 	tmp->valueint ;
    //  			printf("width:%d \n",sceneWidth);
    //  		}else{
    //  			return  ;
    //  		}
    //  		tmp = cJSON_GetObjectItem(root,"sceneHeight");
    //  		if(tmp!=NULL){
    //  			sceneHeight = 	tmp->valueint ;
    //  			printf("height:%d \n",sceneHeight);
    //  		}else{
    //
    //  		}

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
    boostHttpclient_c c(io_service, "192.168.1.46:8187", "/handle_post_request");
//    client c(io_service, "192.168.1.46:8187", "/");
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
*/