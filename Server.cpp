#include "Server.hpp"


void Server::loop(){
    this->read_set = this->current;
    if (select(this->max_fd +1, &(this->read_set),nullptr,nullptr,nullptr)==-1)
        return;
    for (int fd =0; fd <= this->max_fd; fd++){
        if (FD_ISSET(fd,&(this->read_set))){
            if (this->client_count < 3){
                int clientfd = accept(this->sockfd, (struct sockaddr *)& servaddr,&(this->len));
                if (clientfd == -1)
                    return;
                if (clientfd == this->max_fd)
                    max_fd = clientfd;
                (this->clients)[clientfd % 3].id =(this->gid)++;
                FD_SET(clientfd,&(this->current));
                this->client_count++;
            } else {
                int clientfd = accept(sockfd, (struct sockaddr *)& (this->servaddr), &(this->len));
                if (clientfd != -1)
                    close(clientfd);
            }
        } else {
            int ret = recv(fd, this->recv_buffer, sizeof(this->recv_buffer),0);
            if (ret == 0){
                FD_CLR(fd,&(this->current));
                close(fd);
                std::memset((this->clients)[fd % 3].msg, 0, sizeof((this->clients)[fd % 3].msg));
                this->client_count--;
            } else {
                this->recv_buffer[ret] = '\0';
                for (int i = 0, j = strlen((this->clients)[fd % 3].msg); i<= ret; i++, j++){
                    (this->clients[fd % 3]).msg[j] = (this->recv_buffer)[i];
                    if ((this->clients)[fd % 3].msg[j] == '\n'){
                        (this->clients)[fd % 3].msg[j] = '\0';
                        if (std::string((this->clients)[fd % 3].msg) == "quit")
                            this->quit = true;
                        this->reporter->info("User input: " + std::string(this->clients[fd % 3].msg));
                        std::memset(this->clients[fd % 3].msg,0,sizeof(this->clients[fd % 3].msg));
                        j = -1;
                    }
                }
            }
        }
        break;
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