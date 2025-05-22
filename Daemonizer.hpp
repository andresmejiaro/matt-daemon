#pragma once

#include "TintinReporter.hpp"

class Daemonizer {
    public:
        Daemonizer();
        Daemonizer(Tintin_reporter *reporter);
        ~Daemonizer();
        void run();
        bool done;
        static void daemonize();
        
        
        private:
        bool check_root();
        bool check_lock();
        void lock();
        int lock_fd;
        Tintin_reporter* reporter;
        Daemonizer(const Daemonizer& other);
        Daemonizer & operator=(const Daemonizer& other); 
};