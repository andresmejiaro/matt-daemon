#ifndef TINTINREPORTER_HPP
	#define TINTINREPORTER_HPP
	# include <string>
	# include <cstring>
	# include <unistd.h>
	# include <sys/socket.h>
	# include <netinet/in.h>
	# include <iostream>
	# include <cstdlib>
	# include <sstream>
	# include <iostream>
	# include <fstream>
	# include <filesystem>
	# include <ctime>
	# include <iomanip>
	# include <csignal>

	typedef enum	e_log{
		INFO,
		ERR,
		LOG
	}	t_log;

	class	Tintin_reporter{
		public:
			Tintin_reporter();
			~Tintin_reporter();
			void	info(std::string str);
			void	err(std::string str);
			void	log(std::string str);
			bool	open_log();
			
			
		private:
			void	log_def(std::string &str, t_log flag);
			std::string		date();
			std::ofstream	_log_file;
			bool lock;
	};
#endif
