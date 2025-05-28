#include "Server.hpp"


void Server::loop() {
    this->read_set = this->current; // Copy the master set to the working set for select()


    if (g_signal_recv){
        this->reporter->log("Signal handler");
        this->quit = true;
        this->reporter->log("Quitting");
    }

    struct timeval timeout;
    timeout.tv_sec = 0; 
    timeout.tv_usec = 0;
    int activity = select(this->max_fd + 1, &(this->read_set), NULL, NULL, &timeout);
    
    if (activity < 0) {
        if (errno == EINTR) { 
            return; 
        }
        this->reporter->err("select() error");
        this->fatal = true; 
        return;
    }

    if (activity == 0) {
        return;
    }

    for (int fd = 0; fd <= this->max_fd; ++fd) {
        if (FD_ISSET(fd, &(this->read_set))) {

            if (fd == this->sockfd) {                
                if (this->client_count < 3) {
                    
                    int clientfd = accept(this->sockfd, (struct sockaddr *)&(this->servaddr), &(this->len));

                    if (clientfd < 0) {
                        this->reporter->err("accept() failed");
                        continue;
                    }

                    std::ostringstream oss_accept;
                    oss_accept << "Accepted new connection from fd: " << clientfd;
                    this->reporter->info(oss_accept.str());
                    FD_SET(clientfd, &(this->current)); 
                    if (clientfd > this->max_fd) {
                        this->max_fd = clientfd; 
                    }
                    
                    int client_slot_idx = clientfd % 3; 
                    std::memset((this->clients)[client_slot_idx].msg, 0, sizeof((this->clients)[client_slot_idx].msg));
                    (this->clients)[client_slot_idx].id = (this->gid)++; 
                    this->client_count++;
                } else {
                    this->reporter->info("Max clients reached. Rejecting new connection.");
                    int rejectfd = accept(this->sockfd, (struct sockaddr *)&(this->servaddr), &(this->len));
                    if (rejectfd >= 0) {
                        close(rejectfd);
                    }
                }

            } else {
                std::memset(this->recv_buffer, 0, sizeof(this->recv_buffer)); // Clear recv_buffer
                int bytes_received = recv(fd, this->recv_buffer, sizeof(this->recv_buffer) - 1, 0); // -1 for null terminator

                if (bytes_received > 0) {
                    this->recv_buffer[bytes_received] = '\0'; 
                    int client_slot_idx = fd % 3; 
                    char* client_msg_buf = (this->clients)[client_slot_idx].msg;
                    size_t client_msg_capacity = sizeof((this->clients)[client_slot_idx].msg);

                    for (int i = 0; i < bytes_received; ++i) {
                        char current_char = this->recv_buffer[i];
                        size_t current_len = strlen(client_msg_buf);

                        if (current_len < client_msg_capacity - 1) { 
                            client_msg_buf[current_len] = current_char;
                            client_msg_buf[current_len + 1] = '\0'; 

                            if (current_char == '\n') {
                                client_msg_buf[current_len] = '\0'; 
                                std::ostringstream oss_recv;
                                oss_recv << "User input from fd " << fd << ": \"" << client_msg_buf << "\"";
                                this->reporter->info(oss_recv.str());

                                if (std::string(client_msg_buf) == "quit") {
                                    this->reporter->info("Quit command received. Server will shut down based on this->quit flag.");
                                    this->quit = true; 
                                }

                                std::memset(client_msg_buf, 0, client_msg_capacity);
                            }
                        } else {
                            this->reporter->err("Client message buffer full. Clearing and discarding data.");
                            std::memset(client_msg_buf, 0, client_msg_capacity);
                            break; 
                        }
                    }
                } else {
                    if (bytes_received == 0) {
                        std::ostringstream oss_disc;
                        oss_disc << "Client fd " << fd << " disconnected gracefully.";
                        this->reporter->info(oss_disc.str());
                    } else { 
                        std::ostringstream oss_err;
                        oss_err << "recv() error on fd " << fd; 
                        this->reporter->err(oss_err.str());
                    }

                    close(fd); 
                    FD_CLR(fd, &(this->current)); 

                    int client_slot_idx = fd % 3;
                    std::memset((this->clients)[client_slot_idx].msg, 0, sizeof((this->clients)[client_slot_idx].msg));

                    this->client_count--;
                }
            }

        } 
    } 
}



Server::Server(Tintin_reporter* reporter):fatal(false), reporter(reporter), max_fd(0), gid(0), client_count(0), len(sizeof(socklen_t)), quit(false){
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1) 
        this->reporter->err("");
    this->max_fd = this->sockfd;
    
    // Enable port reuse
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        this->reporter->err("setsockopt(SO_REUSEADDR) failed");
        this->fatal =true;
        return;
    }
    
    FD_ZERO(&(this->current));
    FD_SET(this->sockfd, &(this->current));
    std::memset(this->clients, 0, sizeof(this->clients));
    std::memset(&(this->servaddr), 0, sizeof(this->servaddr));

    this->servaddr.sin_family = AF_INET; 
    this->servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
    this->servaddr.sin_port = htons(4242);

    if (bind(this->sockfd, (const struct sockaddr *)&(this->servaddr), sizeof((this->servaddr))) != 0 || listen(sockfd, 100) != 0) {
        this->reporter->err("");
        this->fatal =true;
        return;
    }
} 


Server::~Server(){
    
}

bool Server::get_quit(){
    return this->quit;
}

 