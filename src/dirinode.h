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


#ifndef DIRINODE_H
#define DIRINODE_H
#include "inode.h"
#include "device.h"
#include "jfs_unicode.h"
#include "unicode_to_utf8.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h>
#define JFS_PATH_MAX 4096
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/lexical_cast.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <algorithm>
	

namespace jfsrec{

	class BadDirData{
		private:
			std::string what_;
		public:
			BadDirData(const std::string& w):what_(w){}
			std::string what(){return what_;}
	};

	class DirInode : public Inode{
		private:
// 			virtual void create_dir();
		protected:
			std::vector<Inode*> siblings_;
		public:
			
			explicit DirInode(uint8_t*,uint64_t);
// 			explicit DirInode(uint64_t i):Inode(i){};
			virtual ~DirInode(){/*TODO*/}
			virtual void set_parent(Inode*);
			virtual void print_dir_structure(uint32_t&);
			virtual void add_sibling(Inode* i){ siblings_.push_back(i);}
			virtual void recover();
			virtual std::vector<InodeName> extract_inode_names();
			InodeName  get_direntry(struct dtslot *slot, uint8_t head_index);
			void descend(std::vector<InodeName>& vin,struct idtentry *node_entry)throw(BadDirData,invalid_read_exception);
			void next_leaf(std::vector<InodeName>& vin, dtpage_t& dtree,uint8_t* stbl)throw(BadDirData,invalid_read_exception);
 			virtual void create_dir();
			virtual void count(InodeCounter& ic ) const {++ic.num_dirs_;}	
			virtual char get_typechar() const {return 'D';}			
			void set_metadata();
	};
}
#endif
