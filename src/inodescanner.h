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

#ifndef INODESCANNER_H
#define INODESCANNER_H
#include "inodevector.h"
#include "inodefactory.h"
#include "device.h"
#include "jfs_types.h"
#include <iostream>
#include <map>
#include <boost/progress.hpp>
#include <boost/progress.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

namespace jfsrec{
	class InodeScanner{
		private:
			
			uint16_t chunk_blocks_;
			static const uint16_t inode_size_;
			InodeVector& inovec_;
		
			std::fstream progress_file_;
			void filter_inostamps();
			void post_process();
			void detect_ino_extents();
			void delete_dupes();
			uint64_t restore();
			void remove_unallocated();
		public:
			InodeScanner(InodeVector&);
			void scan();
			void do_scan();
	};
}

#endif
