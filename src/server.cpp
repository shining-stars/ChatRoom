//
// Created by Sixzeroo on 2018/6/6.
//

#include <string>

#include "server.h"
#include "parse.h"
#include "config.h"
#include "log.h"

ChatRoomServer::ChatRoomServer() {
    _socket_epoll.set_watcher(&_server_handler);
}

// 处理接受请求逻辑
int ServerEpollWatcher::on_accept(EpollContext &epoll_context) {
    LOG(DEBUG)<<"Handle: on_accept start"<<std::endl;
    int client_fd = epoll_context.fd;

    printf("client %s:%d connected to server\n", epoll_context.client_ip.c_str(), epoll_context.client_port);
    Msg m;
    m.code = M_NORMAL;
    m.context = WELCOM_MES;
    int ret = m.send_diy(client_fd);

    return ret;
}

// 处理来信请求, 出错返回-1， 退出连接返回-2
int ServerEpollWatcher::on_readable(EpollContext &epoll_context, const std::vector<int> client_list) {
    int client_fd = epoll_context.fd;
    Msg recv_m;
    recv_m.recv_diy(client_fd);
    // 处理客户端退出的情况
    if(recv_m.code == M_EXIT)
    {
        return -2;
    }
    // 只有一个客户端，发送警告信息
    if(client_list.size() == 1)
    {
        Msg m(M_NORMAL, ONLY_ONE_CAUTION);
        m.send_diy(client_fd);
    }
    else
    {
        // 处理消息格式
        std::string meg_str = recv_m.context;
        meg_str = std::to_string(client_fd) + ">" + meg_str;
        Msg send_m(M_NORMAL, meg_str);
        // 进行广播
        for(int it : client_list)
        {
            if(it == client_fd) continue;
            send_m.send_diy(it);
        }
    }
    return 0;
}

int ChatRoomServer::start_server(const std::string bind_ip, int port, int backlog, int max_events) {
    LOG(INFO)<<"Server start: start"<<std::endl;
    std::cout<<"ChatRoom start!"<<std::endl;
    _socket_epoll.set_bind_ip(bind_ip);
    _socket_epoll.set_port(port);
    _socket_epoll.set_backlog(backlog);
    _socket_epoll.set_max_events(max_events);
    return _socket_epoll.start_epoll();
}

int ChatRoomServer::stop_server() {
    LOG(INFO)<<"Server: stop server"<<std::endl;
    return _socket_epoll.stop_epoll();
}

int main()
{
    // 设置配置
    std::map<std::string, std::string> config;
    get_config_map("server.config", config);
    init_logger("server_log", "debug.log", "info.log", "warn.log", "error.log", "all.log");
    set_logger_mode(1);
    ChatRoomServer server;
    server.start_server(config["ip"], std::stoi(config["port"]), 20, 200);
    return 0;
}