/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TintinReporter.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amejia <amejia@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 21:23:41 by samusanc          #+#    #+#             */
/*   Updated: 2025/05/26 22:53:27 by amejia           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TintinReporter.hpp"
#include <system_error>

Tintin_reporter::Tintin_reporter() {
	// lock = false;
	// this->open_log();
	// this->open_lock();
	// this->info("Started.");
}



Tintin_reporter::~Tintin_reporter() {
	// if (lock)
	// 	this->delete_lock();
	// this->_log_file.close();
	// this->info("Quitting.");
}


//Abre el log file y en caso de fallo cierra el programa

bool	Tintin_reporter::open_log() {
    
	const std::string filename = "/var/log/matt_daemon/matt_daemon.log";
	//const std::string filename = "/home/andres/matt_daemon.log"; //remove
	std::filesystem::path p{filename};

	std::error_code ec;
	std::filesystem::create_directories(p.parent_path(), ec);
	if (ec)
	{
		std::cerr << "Failed to create log directory "
				  << p.parent_path() << ": " << ec.message() << "\n";
		return false;
	}

	_log_file.open(filename, std::ios::app);
	if (!_log_file.is_open())
	{
		std::cerr << "Can't open log file " << filename << std::endl;
		return false;
	}
    return true;
}

void	Tintin_reporter::info(std::string str) {
	this->log_def(str, INFO);
}

void	Tintin_reporter::err(std::string str) {
	this->log_def(str, ERR);
}

void	Tintin_reporter::log(std::string str) {
	this->log_def(str, LOG);
}


std::string	Tintin_reporter::date() {
    std::time_t now = std::time(0);
    std::tm* t = std::localtime(&now);
    
    std::ostringstream oss;
    oss << "[" 
        << std::setfill('0') << std::setw(2) << t->tm_mday << "/"
        << std::setfill('0') << std::setw(2) << (t->tm_mon + 1) << "/"
        << (t->tm_year + 1900) << "-"
        << std::setfill('0') << std::setw(2) << t->tm_hour << ":"
        << std::setfill('0') << std::setw(2) << t->tm_min << ":"
        << std::setfill('0') << std::setw(2) << t->tm_sec << "]";
    
    return oss.str();
}

void	Tintin_reporter::log_def(std::string &str, t_log flag) {
	std::stringstream	result;

	result << this->date() << " [ ";
	switch (flag) {
		case INFO:
			result << "INFO";
			break;
		case ERR:
			result << "ERROR";
			break;
		case LOG:
			result << "LOG";
			break;
		default:
			result << "?";
			break;
	}
	result << " ] - Matt_daemon: " << str << std::endl;
	_log_file << result.str();
    _log_file.flush();
}