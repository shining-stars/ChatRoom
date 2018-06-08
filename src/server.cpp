//
// Created by Sixzeroo on 2018/6/6.
//

#include <string>

#include "server.h"
#include "parse.h"

// 处理接受请求逻辑
int ServerEpollWatcher::on_accept(EpollContext &epoll_context) {
    int client_fd = epoll_context.fd;

    printf("client %s:%d connected to server\n", epoll_context.client_ip, epoll_context.client_port);
    Msg m;
    m.code = M_NORMAL;
    m.context = WELCOM_MES;
    int ret = m.send_diy(client_fd);
    // LOG
    return ret;
}

// 处理来信请求
int ServerEpollWatcher::on_readable(EpollContext &epoll_context, const std::vector<int> client_list) {
    int client_fd = epoll_context.fd;
    // 只有一个客户端，发送警告信息
    if(client_list.size() == 1)
    {
        Msg m(M_NORMAL, ONLY_ONE_CAUTION);
        m.send_diy(client_fd);
    }
    else
    {
        Msg recv_m;
        recv_m.recv_diy(client_fd);
        if(recv_m.code != M_NORMAL)
        {
            // LOG ERROR
            return -1;
        }
        Msg send_m(M_NORMAL, recv_m.context);
        for(auto it : client_list)
        {
            if(it == client_fd) continue;

            send_m.send_diy(it);
        }
    }
    return 0;
}

int ChatRoomServer::start_server(const std::string bind_ip = "", int port, int backlog, int max_events) {
    // LOG INFO
    _socket_epoll.set_bind_ip(bind_ip);
    _socket_epoll.set_port(port);
    _socket_epoll.set_backlog(backlog);
    _socket_epoll.set_max_events(max_events);
    return _socket_epoll.start_epoll();
}

int ChatRoomServer::stop_server() {
    // LOG INFO
    return _socket_epoll.stop_epoll();
}