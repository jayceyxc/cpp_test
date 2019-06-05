#include <boost/asio.hpp>

using boost::asio
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

//创建一个io_service实例
io_service service;
// 监听端口2001
ip::tcp::endpoint ep(ip::tcp::v4(), 2001);
//创建接收器acc——一个接口客户端连接，创建虚拟的socket，异步等待客户端连接的对象
ip::tcp::acceptor acc(service, ep);

socket_ptr sock(new ip::tcp::socket(service));
start_accept(sock);
service.run();

void start_accept(socket_ptr sock) {
    acc.async_accept(*sock, boost::bind(handle_accept, sock, -1));
}

void handle_accept(socket_ptr sock, const boost::system::error_code &
err) {
    if ( err) return;
    // 从这里开始, 你可以从socket读取或者写入
    socket_ptr sock(new ip::tcp::socket(service));
    start_accept(sock);
}
