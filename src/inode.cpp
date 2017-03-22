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
#include "inode.h"

using namespace jfsrec;
using namespace std;
using namespace boost::filesystem;

Inode :: ~Inode(){
	if (dinode_available_)
		delete dinode_;

}
/*
Inode :: Inode(uint64_t byte_offset):byte_offset_(byte_offset),is_valid_(true),dinode_available_(false){
	struct dinode* i = get_dinode();
	
	dtroot_t* root = (dtroot_t *) & (i->di_btroot);
	parent_di_number_ = root->header.idotdot;

	di_number_=i->di_number;
	parent_=NULL;
	free_dinode();

}*/

Inode :: Inode(uint8_t* ip, uint64_t byte_offset):byte_offset_(byte_offset),is_marked_(),dinode_available_(false){
	struct dinode* i = (struct dinode*)ip;
	
	dtroot_t* root = (dtroot_t *) & (i->di_btroot);
	parent_di_number_ = root->header.idotdot;

	di_number_=i->di_number;
	parent_=NULL;
}

void Inode :: set_parent(Inode* i){
	parent_=i;
}

void Inode :: recover(){
}

Inode * Inode :: get_root(){
	if ((parent_ == NULL) || (parent_==this))
		return this;
	else
		return parent_->get_root();
}

path Inode :: get_full_path(){
	try {
		if ((parent_ == NULL) || (parent_==this))
			return (options.get_output_dir()/path(name_));
		else
			return (parent_->get_full_path()/path(name_));
	}catch (filesystem_error e){
		cerr<<e.what()<<endl;
		stringstream ss;
		ss<<get_di_number();
		set_name(ss.str());
		cout <<"illegal filename"<<endl;
		if ((parent_ == NULL) || (parent_==this))
			return (options.get_output_dir()/name_);
		else
			return (parent_->get_full_path()/name_);
	}
}

void Inode :: print_parent(uint16_t level){
	cout << di_number_<<"->";
	cout << flush;
	level++;
	if (parent_ == NULL){
		cout << "/"<<endl;	
// 		abort();
	}else{
		if (parent_!=this)
			parent_->print_parent(level);
	}
	level--;
	
}



vector<InodeName> Inode::extract_inode_names(){
// 	cout<<"vector<InodeName> Inode::extract_inode_names()"<<endl;
	vector<InodeName> in;
	return in;
}

dinode* Inode :: get_dinode() const{
	if (!dinode_available_)
		read_dinode();
	return dinode_;
}

void Inode :: free_dinode() const{
	if (dinode_available_){
		delete dinode_;	
		dinode_available_=false;
		dinode_=NULL;
	}
}

void Inode :: read_dinode() const{
	dinode_ = new dinode;
	
	device.read_bytes(byte_offset_,512,(uint8_t*)dinode_);
	dinode_available_=true;
}

void Inode :: print() const{
	
	struct dinode* inode = get_dinode();

	printf("[1] di_inostamp:\t0x%08x\t", inode->di_inostamp);
	printf("[19] di_mtime.tv_nsec:\t0x%08x\n", inode->di_mtime.tv_nsec);
	printf("[2] di_fileset:\t\t%d\t\t", inode->di_fileset);
	printf("[20] di_otime.tv_sec:\t0x%08x\n", inode->di_otime.tv_sec);
	printf("[3] di_number:\t\t%d\t\t", inode->di_number);
	printf("[21] di_otime.tv_nsec:\t0x%08x\n", inode->di_otime.tv_nsec);
	printf("[4] di_gen:\t\t%d\t\t", inode->di_gen);
	printf("[22] di_acl.flag:\t0x%02x\n", inode->di_acl.flag);
	printf("[5] di_ixpxd.len:\t%d\t\t", inode->di_ixpxd.len);
	printf("[23] di_acl.rsrvd:\tNot Displayed\n");
	printf("[6] di_ixpxd.addr1:\t0x%02x\t\t", inode->di_ixpxd.addr1);
	printf("[24] di_acl.size:\t0x%08x\n", inode->di_acl.size);
	printf("[7] di_ixpxd.addr2:\t0x%08x\t", inode->di_ixpxd.addr2);
	printf("[25] di_acl.len:\t%d\n", inode->di_acl.len);
	printf("     di_ixpxd.address:\t%lld\t\t", (long long) addressPXD(&(inode->di_ixpxd)));
	printf("[26] di_acl.addr1:\t0x%02x\n", inode->di_acl.addr1);
	printf("[8] di_size:\t0x%016llx\t", (long long) inode->di_size);
	printf("[27] di_acl.addr2:\t0x%08x\n", inode->di_acl.addr2);
	printf("[9] di_nblocks:\t0x%016llx\t", (long long) inode->di_nblocks);
	printf("     di_acl.address:\t%lld\n", (long long) addressDXD(&inode->di_acl));
	printf("[10] di_nlink:\t\t%d\t\t", inode->di_nlink);
	printf("[28] di_ea.flag:\t0x%02x\n", inode->di_ea.flag);
	printf("[11] di_uid:\t\t%d\t\t", inode->di_uid);
	printf("[29] di_ea.rsrvd:\tNot Displayed\n");
	printf("[12] di_gid:\t\t%d\t\t", inode->di_gid);
	printf("[30] di_ea.size:\t0x%08x\n", inode->di_ea.size);
	printf("[13] di_mode:\t\t0x%08x\t", inode->di_mode);
	printf("[31] di_ea.len:\t\t%d\n", inode->di_ea.len);
//	printf("\t\t%07o\t%10s\t", inode->di_mode & 0177777, mode_string(inode->di_mode));
	printf("[32] di_ea.addr1:\t0x%02x\n", inode->di_ea.addr1);
	printf("[14] di_atime.tv_sec:\t0x%08x\t", inode->di_atime.tv_sec);
	printf("[33] di_ea.addr2:\t0x%08x\n", inode->di_ea.addr2);
	printf("[15] di_atime.tv_nsec:\t0x%08x\t", inode->di_atime.tv_nsec);
	printf("     di_ea.address:\t%lld\n", (long long) addressDXD(&inode->di_ea));
	printf("[16] di_ctime.tv_sec:\t0x%08x\t", inode->di_ctime.tv_sec);
	printf("[34] di_next_index:\t%d\n", inode->di_next_index);
	printf("[17] di_ctime.tv_nsec:\t0x%08x\t", inode->di_ctime.tv_nsec);
	printf("[35] di_acltype:\t0x%08x\n", inode->di_acltype);
	printf("[18] di_mtime.tv_sec:\t0x%08x\n", inode->di_mtime.tv_sec);
// 	getchar();
	free_dinode();
}



//TODO: Unfortionally boost::filesystem only provides a way to set the mtime, not the others...
void Inode :: set_atime(const timestruc_t & t){}

void Inode :: set_ctime(const timestruc_t & t){}

void Inode :: set_otime(const timestruc_t & t){}


void Inode :: set_mtime(const timestruc_t & t){
	last_write_time(get_full_path(),t.tv_sec);
}

bool jfsrec :: operator<(const timestruc_t& t1,const timestruc_t& t2){
	if (t1.tv_sec == t2.tv_sec) return t1.tv_nsec < t2.tv_nsec;
	return t1.tv_sec < t2.tv_sec;
}
