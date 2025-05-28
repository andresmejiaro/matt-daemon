
#include "Daemonizer.hpp"
#include "TintinReporter.hpp"
#include "Server.hpp"

volatile sig_atomic_t g_signal_recv = 0;

void signal_handler(int signal){
    g_signal_recv = 1;
    (void) signal;
}



int main(void){

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    Tintin_reporter reporter = Tintin_reporter();
    Server server = Server(&reporter);
    bool logopen =  reporter.open_log();
    Daemonizer maindaemon = Daemonizer(reporter,server);
    if (!logopen)
        return 1;
    if (server.fatal)
        return 1;
    

    if (maindaemon.done)
        return 1;
    else{
        maindaemon.daemonize(); 
        maindaemon.run();
        return 0;
    }
}