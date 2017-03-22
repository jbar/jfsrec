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
#ifndef RECOVERER_H
#define RECOVERER_H

#include "inodevector.h"
#include "inode.h"
#include <vector>
#include <string>
#include <deque>
// #include <unistd.h>
#include "boost/filesystem/operations.hpp"
#include <boost/regex.hpp>
#include "options.h"
#include <boost/progress.hpp>
namespace jfsrec{
	class Recoverer{
		private:
			InodeVector& inovec_;
			std::vector<Inode*> roots_;
		public:
			Recoverer(InodeVector&);

			void find_directory_parents();
			void find_file_parents_and_names();
			void find_roots();

			void print_tree();
			void print_tree_reverse();

			void recover();
			void recover_dirtree();
			void recover_iterative();

			std::deque<Inode*> generate_prio_queue();
			void recover_iterative(const std::deque<Inode*>&);
			void recover_dirtree(const std::deque<Inode*>&);
	};
}



#endif
