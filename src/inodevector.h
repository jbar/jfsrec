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

#ifndef INODEVECTOR_H
#define INODEVECTOR_H
#include <vector>

#include <map>
#include "inode.h"
#include "dirinode.h"
#include "fileinode.h"
#include "unallocatedinode.h"
#include "inodefactory.h"
#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <boost/progress.hpp>
namespace jfsrec{
	class InodeVector{
		typedef std::map <uint32_t, Inode*> inomap_type;
		private:
			std::vector<Inode*> inodes_;
			std::vector<Inode*> invalidated_inodes_;
			mutable inomap_type di_number_index_;
			mutable bool di_number_index_ok_;
		public:
			~InodeVector();
			void add_inode(Inode* i);
			Inode* get_by_di_number(uint32_t);
			Inode* get_by_number(uint32_t) ;
			Inode* get_invalidated_by_number(uint32_t i){return invalidated_inodes_[i];}
			std::vector<Inode*> get_invalidated_by_di_number(uint32_t i);
			inline const Inode& operator[](uint32_t i) const {return *inodes_[i];}
			inline Inode& operator[](uint32_t i) { return *inodes_[i];}
			uint32_t size(){return inodes_.size();}
			void print_stats();
			void print_stats_compact();
			void print_di_numbers();
			void remove_marked();
			void remove_unmarked();
			void remove_dupes();
			void free_dinodes();
			void load_from_positions(const boost::filesystem::path&);
			void save_positions(const boost::filesystem::path&);
			void generate_di_number_index() const;
			void unmark_all();
			void mark_all();
	};
}

#endif

