#include "c_netuser.hpp"

c_netuser::c_netuser(std::string& username) : c_user(username),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service) {
    threads_maker(2);
    create_server();
}

c_netuser::c_netuser(std::string&& username) : c_user(username),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service) {
    threads_maker(2);
    create_server();
}



void c_netuser::send_token_bynet(const std::string &ip_address, const std::string &reciever_pubkey) {
    
    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip_address, ec );
    if(ec) { ///< boost error - not needed
        throw std::runtime_error("bad ip");
    }
    if(!addr.is_v4()) {
        std::string msg = addr.to_string();
        msg += " is not valid IPv4 address";
        throw std::exception(std::invalid_argument(msg));
    }
    ip::tcp::endpoint server_endpoint(addr, server_port);

    client_socket.connect(server_endpoint,ec);
    if(ec) {
        throw std::runtime_error("send_token_bynet -- fail to connect");
    }

    std::string packet = get_token_packet(reciever_pubkey);
    uint32_t packet_size = packet.size();
    client_socket.write_some(boost::asio::buffer(&packet_size, 4),ec);

    client_socket.write_some(boost::asio::buffer(packet.c_str(), packet_size),ec);

}

void c_netuser::create_server() {

    ip::tcp::acceptor my_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v4(),server_port));
    my_acceptor.async_accept(server_socket,
                            [this](boost::system::error_code ec) {
                                if(!ec) {
                                    this->do_read(std::move(server_socket));
                                }
                                this->create_server();
                            });


}

void c_netuser::do_read(ip::tcp::socket socket_) {

    boost::system::error_code ec;
    char tok_s[4];						// first 4 bytes always is lenght of token
    memset(tok_s,0,4);

    ip::tcp::socket local_socket(std::move(socket_));
    size_t recieved_bytes;
    recieved_bytes = local_socket.read_some(buffer(tok_s,4),ec);
    assert(recieved_bytes == 4);
    uint32_t* tok_size = reinterpret_cast<uint32_t*>(tok_s);

    std::string packet;
    recieved_bytes = 0;
    while(recieved_bytes < *tok_size) {
        size_t loop_data_size = local_socket.read_some(buffer(data_,max_length),ec);
        packet.assign(data_,loop_data_size);

        recieved_bytes += loop_data_size;
    }
    recieve_from_packet(packet);
}


c_netuser::~c_netuser() {

    m_io_service.stop();
    for(auto &t : m_threads){
        t.join();
    }
}

void c_netuser::threads_maker(unsigned num) {

    m_threads.reserve(num);
    for(int i = 0; i < num; ++i) {
        m_threads.emplace_back([this](){this->m_io_service.run();});
    }
}
