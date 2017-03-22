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


#ifndef FILEINODE_H
#define FILEINODE_H	
#include "inode.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"
#include <boost/progress.hpp>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
namespace jfsrec{
	class FileInode : public Inode {
		private:
		
			static const uint32_t buffsize_;

			void save_file() throw(boost::filesystem::filesystem_error,invalid_read_exception);

			void write_extent_to_file(xad_t*, FILE*,boost::progress_display*)throw (invalid_read_exception);
	
   		void get_internal_xads(xad_t *, short nextindex, std::vector<xad_t>&) throw (boost::filesystem::filesystem_error,extent_error,invalid_read_exception);
			void get_leaf_xads(xad_t *, short nextindex, std::vector<xad_t>&)  throw (boost::filesystem::filesystem_error,extent_error,invalid_read_exception);
			void get_internal_xtpage(xad_t* xad,std::vector<xad_t>&) throw (extent_error);
     		


			bool xads_sane(const std::vector<xad_t>&) const;
			uint64_t filesize_from_xad_vector(const std::vector<xad_t>&) const;
			void print_extents(const std::vector<xad_t>&) const;
// 			bool write_extent_trunc_block(xad_t*, FILE*);
		public:
			explicit FileInode(uint8_t*,uint64_t);
// 			explicit FileInode(uint64_t i):Inode(i){};
			virtual void print_dir_structure(uint32_t&);
			void recover();
			virtual void count(InodeCounter& ic ) const {++ic.num_files_;}		
			virtual char get_typechar() const {return 'F';}			
	};
}

#endif
