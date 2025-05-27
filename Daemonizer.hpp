#ifndef DAEMONIZER_HPP
    #define DAEMONIZER_HPP
    #include "TintinReporter.hpp"
    #include "Server.hpp"


    class Daemonizer {
        public:
        Daemonizer(Tintin_reporter &reporter,Server &server);
        ~Daemonizer();
        void run();
        bool done;
        void daemonize();
        
        
        private:
            Daemonizer();
            bool check_root();
            bool check_lock();
            void lock();
            void unlock();
            int lock_fd;
            Tintin_reporter& reporter;
            Daemonizer(const Daemonizer& other);
            Daemonizer & operator=(const Daemonizer& other);
            Server&  server; 
    };
#endif