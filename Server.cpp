#include "Server.hpp"




void Server::loop() {
    this->read_set = this->current; // Copy the master set to the working set for select()


    if (g_signal_recv){
        this->reporter->log("Signal hangler");
        this->quit = true;
        this->reporter->log("Quitting");
    }

    // Consider adding a timeout to select() if non-blocking behavior is desired periodically
    // struct timeval timeout;
    // timeout.tv_sec = 1; // 1 second timeout
    // timeout.tv_usec = 0;
    // int activity = select(this->max_fd + 1, &(this->read_set), NULL, NULL, &timeout);
    int activity = select(this->max_fd + 1, &(this->read_set), NULL, NULL, NULL); // Block indefinitely

    if (activity < 0) {
        // select() error
        // In C++98, you might not have easy access to errno in a portable way without <cerrno>
        // but reporter->err("select() failed") or perror("select") from <cstdio> is common.
        if (errno == EINTR) { // Interrupted by a signal, can be ignored or handled
            return; // Or continue
        }
        this->reporter->err("select() error");
        this->fatal = true; // Or some other error handling to stop the server gracefully
        return;
    }

    if (activity == 0) {
        // select() timed out (only if a timeout was provided to select)
        // this->reporter->info("select() timed out");
        return;
    }

    // Iterate through file descriptors to see which one is ready
    for (int fd = 0; fd <= this->max_fd; ++fd) {
        if (FD_ISSET(fd, &(this->read_set))) {
            // An FD is ready. Check if it's the listening socket or a client socket.

            if (fd == this->sockfd) {
                // === New incoming connection on the listening socket ===
                if (this->client_count < 3) { // Assuming max 3 clients based on your original code
                    // Prepare for accept: len should hold the size of servaddr
                    // Ensure this->len is correctly initialized (e.g., in constructor as sizeof(this->servaddr))
                    // If this->len is a 'socklen_t' member:
                    // this->len = sizeof(this->servaddr); // Re-set before each accept if it can change, or ensure it's correctly set once.
                                                        // It's a value-result argument.
                    
                    int clientfd = accept(this->sockfd, (struct sockaddr *)&(this->servaddr), &(this->len));

                    if (clientfd < 0) {
                        this->reporter->err("accept() failed");
                        // Continue to check other FDs, don't let one failed accept stop the loop
                        continue;
                    }

                    // Log new connection (C++98 style)
                    std::ostringstream oss_accept;
                    oss_accept << "Accepted new connection from fd: " << clientfd;
                    this->reporter->info(oss_accept.str());

                    FD_SET(clientfd, &(this->current)); // Add new client fd to the master set
                    if (clientfd > this->max_fd) {
                        this->max_fd = clientfd; // Update the highest file descriptor
                    }

                    // Initialize client data for 'clientfd'
                    // Your original code uses `(this->clients)[clientfd % 3]`.
                    // This modulo arithmetic is fragile for mapping FD to a client slot.
                    // A robust way is to find an empty slot in `this->clients` array
                    // or if `this->clients` maps fd to client data.
                    // For now, using the modulo logic as in your original example:
                    int client_slot_idx = clientfd % 3; // Be cautious with this mapping
                    std::memset((this->clients)[client_slot_idx].msg, 0, sizeof((this->clients)[client_slot_idx].msg));
                    (this->clients)[client_slot_idx].id = (this->gid)++; // Assign new GID
                    // You might also want to store 'clientfd' in your Client struct:
                    // (this->clients)[client_slot_idx].fd = clientfd;
    this->reporter->info("Max clients reached. Rejecting new connection.");
                    int rejectfd = accept(this->sockfd, (struct sockaddr *)&(this->servaddr), &(this->len));
                    if (rejectfd >= 0) {
                        close(rejectfd);
                    }
                }

            } else {
                // === Data incoming on an existing client socket (fd) ===
                std::memset(this->recv_buffer, 0, sizeof(this->recv_buffer)); // Clear recv_buffer
                int bytes_received = recv(fd, this->recv_buffer, sizeof(this->recv_buffer) - 1, 0); // -1 for null terminator

                if (bytes_received > 0) {
                    this->recv_buffer[bytes_received] = '\0'; // Null-terminate the received data

                    // Find the client slot associated with this fd.
                    // Again, (this->clients)[fd % 3] is fragile.
                    // A better way: iterate this->clients array to find where client.fd == fd
                    int client_slot_idx = fd % 3; // Using original fragile mapping
                    char* client_msg_buf = (this->clients)[client_slot_idx].msg;
                    size_t client_msg_capacity = sizeof((this->clients)[client_slot_idx].msg);

                    // Append received data to client's message buffer and process line by line
                    for (int i = 0; i < bytes_received; ++i) {
                        char current_char = this->recv_buffer[i];
                        size_t current_len = strlen(client_msg_buf);

                        if (current_len < client_msg_capacity - 1) { // Space for char + null terminator
                            client_msg_buf[current_len] = current_char;
                            client_msg_buf[current_len + 1] = '\0'; // Keep it null-terminated

                            if (current_char == '\n') {
                                // Newline received, process the complete message
                                client_msg_buf[current_len] = '\0'; // Remove newline for processing

                                // Log and check for "quit"
                                std::ostringstream oss_recv;
                                oss_recv << "User input from fd " << fd << ": \"" << client_msg_buf << "\"";
                                this->reporter->info(oss_recv.str());

                                if (std::string(client_msg_buf) == "quit") {
                                    this->reporter->info("Quit command received. Server will shut down based on this->quit flag.");
                                    this->quit = true; // Signal server shutdown
                                    // The main server logic (outside this loop) should handle the actual shutdown.
                                }

                                // Reset client message buffer for the next message
                                std::memset(client_msg_buf, 0, client_msg_capacity);
                            }
                        } else {
                            // Client message buffer full, log error and discard.
                            this->reporter->err("Client message buffer full. Clearing and discarding data.");
                            std::memset(client_msg_buf, 0, client_msg_capacity);
                            break; // Stop processing this recv_buffer
                        }
                    }
                } else {
                    // recv() returned 0 or -1: client disconnected or error
                    if (bytes_received == 0) {
                        std::ostringstream oss_disc;
                        oss_disc << "Client fd " << fd << " disconnected gracefully.";
                        this->reporter->info(oss_disc.str());
                    } else { // bytes_received < 0
                        std::ostringstream oss_err;
                        oss_err << "recv() error on fd " << fd; // perror("recv") or strerror(errno) for details
                        this->reporter->err(oss_err.str());
                    }

                    close(fd); // Close the socket
                    FD_CLR(fd, &(this->current)); // Remove from master set

                    // Clear client data. Using the fragile modulo mapping:
                    int client_slot_idx = fd % 3;
                    std::memset((this->clients)[client_slot_idx].msg, 0, sizeof((this->clients)[client_slot_idx].msg));
                    // (this->clients)[client_slot_idx].id = 0; // Or some other reset indication
                    // (this->clients)[client_slot_idx].fd = -1; // If you store fd in client struct

                    this->client_count--;
                }
            }

            // If you want to process only one ready FD per call to select() (like your original C code's break):
            // break; 
            // If this `break` is uncommented, the loop will exit after handling the first ready FD.
            // If it's commented out (as it is now), all FDs found ready by select() in this iteration
            // will be processed before select() is called again. This is generally more efficient.
            // Since your C code had it, I'm noting it. Choose the behavior you prefer.
            // If you re-add the break, it should be here, after successfully handling an FD_ISSET condition.

        } // end if(FD_ISSET(fd, ...))
    } // end for(int fd = 0; ... )
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

    void Server::sig_handler(){
        this->reporter->info("Signal Recieved");
        this->quit = true;
    }