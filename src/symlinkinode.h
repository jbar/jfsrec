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
#ifndef JFSRECSYMLINKINODE_H
#define JFSRECSYMLINKINODE_H

#include <inode.h>

namespace jfsrec {
	class SymlinkInode : public Inode{
		public:
			explicit SymlinkInode(uint8_t*,uint64_t);
    		SymlinkInode();
	    	virtual ~SymlinkInode();
			virtual void recover(){/*TODO: actually create the symlink*/}
			virtual void print_dir_structure(uint32_t&){}
			virtual void count(InodeCounter& ic ) const {++ic.num_symlinks_;}	
			virtual char get_typechar() const {return 'l';}			
		};
}


#endif
