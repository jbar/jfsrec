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
#include "inodefactory.h"
#include "fileinode.h"
#include "dirinode.h"
#include "unallocatedinode.h"
#include "symlinkinode.h"
#include "specialinode.h"
using namespace jfsrec;

Inode * InodeFactory :: create_if_sane(uint8_t*di,uint64_t offset){
	struct dinode* ip = (struct dinode*)di;
	if (( (ip->di_ixpxd.len!=4) || (ip->di_fileset!=16) )){
		return NULL;
	}


	if (ip->di_nlink==0){
		return new UnallocatedInode(di, offset);
	}

	if ((ip->di_mode & IFMT) == IFREG){
		return new FileInode(di,offset);
	}
	if((ip->di_mode & IFMT) == IFDIR){
		return new DirInode(di,offset);
	}
	if ((ip->di_mode & IFMT) == IFLNK){
		return new SymlinkInode(di,offset);
	}
	if ( ((ip->di_mode & IFMT) == IFBLK) 
	  || ((ip->di_mode & IFMT) == IFCHR)
	  || ((ip->di_mode & IFMT) == IFFIFO) 
	  || ((ip->di_mode & IFMT) == IFSOCK)) {
		return new SpecialInode(di,offset);
	}
// 	exit(0);
	return NULL; //unhandled inode type

}

Inode * InodeFactory :: read_from_disk(uint64_t offset){
	struct dinode ip;
	device.read_bytes(offset,512,(uint8_t*)&ip);

	if (ip.di_nlink==0){
		return new UnallocatedInode((uint8_t*)&ip,offset);
	}
	if ((ip.di_mode & IFMT) == IFREG){
		return new FileInode((uint8_t*)&ip,offset);
	}
	if((ip.di_mode & IFMT) == IFDIR){
		return new DirInode((uint8_t*)&ip,offset);
	}
	if ((ip.di_mode & IFMT) == IFLNK){
		return new SymlinkInode((uint8_t*)&ip,offset);
	}
	if ( ((ip.di_mode & IFMT) == IFBLK) 
	  || ((ip.di_mode & IFMT) == IFCHR)
	  || ((ip.di_mode & IFMT) == IFFIFO) 
	  || ((ip.di_mode & IFMT) == IFSOCK)) {
		return new SpecialInode((uint8_t*)&ip,offset);
	}
// exit(0);
	return NULL; //unhandled inode type
	//TODO: Better throw an exception or sumething, now the client needs to check for NULL, or else a segfault may occur

}
