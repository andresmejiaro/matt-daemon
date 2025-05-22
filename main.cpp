
#include "Daemonizer.hpp"
#include "TintinReporter.hpp"
#include "Server.hpp"

int main(void){
    Tintin_reporter reporter = Tintin_reporter();
    bool logopen =  reporter.open_log();
    if (!logopen)
        return 1;
    Server server = Server(&reporter);
    if (server.fatal)
        return 1;
    Daemonizer maindaemon = Daemonizer(&reporter);
    if (maindaemon.done)
        return 1;
    else{
        maindaemon.daemonize();
        maindaemon.run();
        return 0;
    }
}