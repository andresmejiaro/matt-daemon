#include "Daemonizer.hpp"
#include "TintinReporter.hpp"
#include <filesystem>
#include <unistd.h>
#include <iostream>
#include  <sys/file.h>
#include  <sys/stat.h>
#include <fcntl.h>
#define LOCK_PATH "/var/lock/matt_daemon.lock"
#include <sstream>


Daemonizer::Daemonizer(Tintin_reporter &reporter, Server &server): done(false), lock_fd(-1), reporter(reporter), server(server){
    if (!check_root()){
        this->done = true;
        reporter.err("This program needs root permissions to run");
        std::cerr <<"This program needs root permissions to run" << std::endl;
        std::exit(1);
    }
    else if (!check_lock()){
        this->done = true;
        reporter.err("Another instance of matt-daemon is running. Stoping");
        std::cerr << "Another instance of matt-daemon is running. Stoping" << std::endl;
        std::exit(1);
    }
    return;
}


bool Daemonizer::check_lock(){
    struct stat sb;
    return stat(LOCK_PATH,&sb);
}

bool Daemonizer::check_root(){
    return getuid() == 0;
}

Daemonizer::~Daemonizer(){
    if (this->lock_fd != -1) {
        flock(this->lock_fd,LOCK_UN);
        close(this->lock_fd);
        unlink(LOCK_PATH);
    }

}

void Daemonizer::run(){
    this->lock();
    while (!server.fatal && !server.get_quit()){
        server.loop();
    };
    this->unlock();
}

void Daemonizer::lock(){
    this->lock_fd = open(LOCK_PATH, O_RDWR | O_CREAT, 0640);
    if (lock_fd < 0 || flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
      this->done = true;
      std::cerr << "An error occured while building the lock" << std::endl;
      exit(1);
    }
}

void Daemonizer::unlock(){
    flock(lock_fd,LOCK_UN);
    close(this->lock_fd);
    unlink(LOCK_PATH);
}

void Daemonizer::daemonize(){
    
    reporter.info("Entering Daemon mode");
    pid_t pid = fork();
    if (pid > 0)
        exit( 0);
    setsid();
    pid = fork();
    if (pid > 0)
        exit( 0);
    chdir("/");
    umask(0);
    std::ostringstream message_stream;
    message_stream << "Started. PID: " << getpid() <<".";
    reporter.info(message_stream.str());
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
}