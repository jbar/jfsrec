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

#ifndef OPTIONS_H
#define OPTIONS_H
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>
#include "jfs_types.h"

namespace jfsrec{
	class Options{
		private:
			uint16_t blocksize_;

			boost::filesystem::path device_;
			boost::filesystem::path output_dir_;
			boost::filesystem::path progress_dir_;
			
			boost::regex first_;
			boost::regex exclude_;
			boost::regex last_;
			
			bool exclude_speced_;
			bool last_speced_;
			bool first_speced_;

			bool skip_unsane_xads_;

		public:
			~Options();
			Options(){};
			void init(int, char**);//Call with argv and argc


			inline uint16_t get_blocksize() const {return blocksize_;}

			boost::filesystem::path get_device() const {return device_;}
			boost::filesystem::path get_output_dir() const{return output_dir_;}
			boost::filesystem::path get_progress_dir() const{return progress_dir_;}

			inline boost::regex get_first(){return first_;}
			inline boost::regex get_last(){return last_;}
			inline boost::regex get_exclude(){return exclude_;}

			inline bool get_last_speced() {return last_speced_;}
			inline bool get_first_speced() {return first_speced_;}
			inline bool get_exclude_speced() {return exclude_speced_;}

			inline bool get_skip_unsane_xads(){return skip_unsane_xads_;}

	};
  	extern Options options;
}
#endif
