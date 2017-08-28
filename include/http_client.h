#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iostream>


using boost::asio::ip::tcp;

using namespace std;
using namespace boost;

class boostHttpclient_c
{
public:
    boostHttpclient_c(boost::asio::io_service& io_service,
      const std::string& server, const std::string& path, const std::string &post_content);

private:
  void handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator);

  void handle_connect(const boost::system::error_code& err);

  void handle_write_request(const boost::system::error_code& err);

  void handle_read_status_line(const boost::system::error_code& err);

  void handle_read_headers(const boost::system::error_code& err);

  void handle_read_content(const boost::system::error_code& err);

  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
};

#endif // HTTP_CLIENT_H
