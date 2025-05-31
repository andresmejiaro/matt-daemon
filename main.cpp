
#include "Daemonizer.hpp"
#include "TintinReporter.hpp"
#include "Server.hpp"

volatile sig_atomic_t g_signal_recv = 0;

void signal_handler(int signal){
    g_signal_recv = 1;
    (void) signal;
}



int main(void){

    for (int i = 1; i< 64; i++)
        std::signal(i, signal_handler);
    

    Tintin_reporter reporter;
    Server server(&reporter);
    bool logopen =  reporter.open_log();
    Daemonizer maindaemon(reporter,server);
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