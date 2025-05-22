#include "Daemonizer.hpp"
#include "TintinReporter.hpp"
#include <filesystem>
#include <unistd.h>
#include <iostream>
#include  <sys/file.h>
#include  <sys/stat.h>
#include <fcntl.h>
#define LOCK_PATH "/var/run/matt_daemon.lock"

Daemonizer::Daemonizer(): done(false), lock_fd(-1), reporter(NULL){
    if (!check_lock()){
        this->done = true;
        std::cerr << "Another instance of matt-daemon is running. Stoping" << std::endl;
    }
    if (!check_root()){
        this->done = true;
        std::cerr <<"This program needs root permissions to run" << std::endl;
    }
    return;
}


Daemonizer::Daemonizer(Tintin_reporter *reporter): done(false), lock_fd(-1), reporter(reporter){
    if (!check_lock()){
        this->done = true;
        this->reporter->err("Another instance of matt-daemon is running. Stoping");
        std::cerr << "Another instance of matt-daemon is running. Stoping" << std::endl;
    }
    if (!check_root()){
        this->done = true;
        this->reporter->err("This program needs root permissions to run");
        std::cerr <<"This program needs root permissions to run" << std::endl;
    }
    return;
}




bool Daemonizer::check_lock(){
    return !std::filesystem::exists(LOCK_PATH);
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
    while this->server;
}

void Daemonizer::lock(){
    this->lock_fd = open(LOCK_PATH, O_RDWR | O_CREAT, 0640);
    if (lock_fd < 0 || flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
      this->done = true;
      std::cerr << "An error occured while building the lock" << std::endl;
      exit(1);
    }
}

void Daemonizer::daemonize(){
    
    pid_t pid = fork();
    if (pid > 0)
        exit( 0);
    setsid();
    pid = fork();
    if (pid > 0)
        exit( 0);
    chdir("/");
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
}