#ifndef SERVER_HPP
    #define SERVER_HPP
    #include "TintinReporter.hpp"


    extern volatile sig_atomic_t g_signal_recv;

    struct Client {
        int id;
        char msg[290000];
    };


    class Server{
        public:
            ~Server();
            Server(Tintin_reporter*);
            bool fatal;
            void loop();
            bool get_quit();
            void sig_handler();
            
        private:
            Server();
            Tintin_reporter* reporter;
            Client clients[3]; // Limit to 3 clients only
            int max_fd;
            int gid;
            int client_count; // Track number of connected clients
            char recv_buffer[300000];
            fd_set read_set;
            fd_set current;
            struct sockaddr_in servaddr;
            socklen_t len;
            int sockfd;
            bool quit;



        Server(const Server& other);
        Server & operator=(const Server& other); 
    };
#endif