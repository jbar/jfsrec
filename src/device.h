/***************************************************************************
 *   Copyright (C) 2006 by Simon Lundell   *
 *   simon.lundell@his.se   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as published by  *
 *   the Free Software Foundation.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* Some code of this module were taken from xpeek of jfs-utils-1.1.11
 * which is under the copyright of  International Business Machines Corp., 2000-2002
 * and licensed under GNU General Public License (GPL)                      */

#ifndef DEVICE_H
#define DEVICE_H



#include "jfs_types.h"
//#include <fcntl.h>
//#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
// #define BLKGETSIZE
// #include <sys/ioctl.h>

struct stat;
//ugly, should check in ./configure


#include <sys/stat.h>
#include <boost/filesystem/path.hpp>

namespace jfsrec{

	
	class invalid_read_exception{
		private:
			std::string error_;
		public:
			invalid_read_exception(const std::string& e):error_(e){}
			std::string what(){return error_;}
	};

	class Device{
		private:
			int16_t blocksize_;
			int8_t log2blocksize_;
			FILE* file_;
			bool is_valid(struct stat*);
			int64_t fssize_;
			
		public:
			~Device();
			void open(const boost::filesystem::path&, uint16_t);
			void close();
			int64_t get_size_blocks() const;
			int64_t get_size_bytes() const;
			
			int16_t get_blocksize() const {return blocksize_;}
			int8_t  get_log2blocksize() const {return log2blocksize_;}
			
// 			void read_blocks(int64_t, uint32_t, uint8_t*) throw (invalid_read_exception);
			void read_bytes( int64_t, uint32_t, uint8_t*) throw (invalid_read_exception);
	
	};



  	extern Device device;
	std::string bytes_suffixed(uint64_t);
}


#endif
