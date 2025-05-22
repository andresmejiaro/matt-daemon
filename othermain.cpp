#include "MattDaemon.hpp"

struct Client {
    int id;
    char msg[290000];
};

Client clients[3]; // Limit to 3 clients only
int max_fd = 0, gid = 0;
int client_count = 0; // Track number of connected clients
char recv_buffer[300000];
fd_set read_set, current;
Tintin_reporter	*glob = NULL;

void err(const std::string& msg) {
    if (!msg.empty()) {
        std::cerr << msg << std::endl;
    } else {
        std::cerr << "Fatal error" << std::endl;
    }
    exit(1);
}
void my_cleanup() {
	glob->delete_lock();
	glob->info("Quiting.");
}

void signal_handler(int signum) {
	//my_cleanup();
    std::_Exit(signum);
}

int main(int argc, char **argv) {
	Tintin_reporter	matt;
	glob = &matt;

	if (std::atexit(my_cleanup) != 0)
	{
        std::cerr << "atexit registration failed\n";
        return 1;
    }
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT,  signal_handler);
    if (argc != 2) {
        err("Wrong number of arguments");
    }
    
    struct sockaddr_in servaddr;
    socklen_t len = sizeof(servaddr);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) err("");
    max_fd = sockfd;
    
    // Enable port reuse
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        err("setsockopt(SO_REUSEADDR) failed");
    }
    
    FD_ZERO(&current);
    FD_SET(sockfd, &current);
    std::memset(clients, 0, sizeof(clients));
    std::memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
    servaddr.sin_port = htons(std::stoi(argv[1]));

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 || listen(sockfd, 100) != 0) {
        err("");
    }
    
    while(true) {
        read_set = current;
        if (select(max_fd + 1, &read_set, nullptr, nullptr, nullptr) == -1) continue;

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_set)) {
                if (fd == sockfd) {
                    if (client_count < 3) {
                        int clientfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
                        if (clientfd == -1) continue;
                        if (clientfd > max_fd) max_fd = clientfd;
                        clients[clientfd % 3].id = gid++;
                        FD_SET(clientfd, &current);
                        client_count++;
                    } else {
                        int clientfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
                        if (clientfd != -1) {
                            close(clientfd);
                        }
                    }
                } else {
                    int ret = recv(fd, recv_buffer, sizeof(recv_buffer), 0);
                    if (ret <= 0) {
                        FD_CLR(fd, &current);
                        close(fd);
                        std::memset(clients[fd % 3].msg, 0, sizeof(clients[fd % 3].msg));
                        client_count--;
                    } else {
                        recv_buffer[ret] = '\0';
                        
                        for (int i = 0, j = strlen(clients[fd % 3].msg); i <= ret; i++, j++) {
                            clients[fd % 3].msg[j] = recv_buffer[i];
                            if (clients[fd % 3].msg[j] == '\n') {
                                clients[fd % 3].msg[j] = '\0';
								if (std::string(clients[fd % 3].msg) == "quit")
									matt.quit();
								matt.info("User input: " + std::string(clients[fd % 3].msg));
                                std::memset(clients[fd % 3].msg, 0, sizeof(clients[fd % 3].msg));
                                j = -1;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    return 0;
}