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
#include "dirinode.h"
using namespace std;

using namespace jfsrec;
 using namespace boost;

DirInode :: DirInode(uint8_t* ip,uint64_t bo):Inode(ip,bo){
	

}

void DirInode :: set_parent(Inode* i){
	/*if (i==this)
		return;*/
	parent_=i;
	
	DirInode* t = dynamic_cast<DirInode*>(i);
	if (t!=NULL)
		t->add_sibling(this);
}



void DirInode :: recover(){

	set_mode();

	set_atime(get_dinode()->di_atime);
	set_ctime(get_dinode()->di_ctime);
	set_mtime(get_dinode()->di_mtime);
	set_otime(get_dinode()->di_otime);
	free_dinode();
}


void DirInode :: print_dir_structure(uint32_t& level){
	for (uint16_t i=0;i<level;++i)
		cout <<" ";
	cout << name_<<endl;
// 	getchar();
	level++;
	for (uint32_t j=0;j<siblings_.size();++j){
		if (siblings_[j]!=this)
			siblings_[j]->print_dir_structure(level);

	}
	level--;
}



void strToUcs(UniChar *, char *, int);
char UTF8_Buffer[8 * JFS_PATH_MAX];


void print_inode_names(const vector<InodeName>& in){
	for (unsigned int i=0;i<in.size();++i)
		cout << in[i].di_number_<<" "<<in[i].name_<<endl;



}

void DirInode::descend(vector<InodeName>& vin,struct idtentry *node_entry) throw(BadDirData,invalid_read_exception){
// 	cout << "Descend"<<endl;
// 	print_inode_names(vin);
	int64_t node_address;
	dtpage_t dtree;
	try{
// 		printf("pxd_t.len: %u, addr1: %u, addr2: %ul\n",node_entry->xd.len,node_entry->xd.addr1,node_entry->xd.addr2);
// 		printf("pxd_t.len: %u, addr: %lli\n",lengthPXD((&(node_entry->xd))),addressPXD((&(node_entry->xd))));
		
		if ((addressPXD(&(node_entry->xd))==0) || ( addressPXD(&(node_entry->xd)) > device.get_size_blocks()) || ( node_entry->xd.len> device.get_size_blocks()))
			throw BadDirData("unsane PXD in inode:"+lexical_cast<string>(di_number_)); 

		node_address = addressPXD(&(node_entry->xd)) << device.get_log2blocksize();
// 		cout << "Descend at:"<<node_address<<endl;
	/*
		if (node_address==0) //TODO better sanity check of dtrees
			return;
   */
		device.read_bytes(node_address, sizeof (dtpage_t), (uint8_t *) &dtree);
		//TODO:throw exception
	

	/* swap if on big endian machine */
// 	ujfs_swap_dtpage_t(&dtree, type_jfs);
		uint8_t *stbl;
		stbl = (uint8_t *) & (dtree.slot[dtree.header.stblindex]);
		if (!(dtree.header.flag & BT_LEAF)) {
			node_entry = (struct idtentry *) & (dtree.slot[stbl[0]]);
			descend(vin,node_entry);
			return;
		}
		next_leaf(vin,dtree,stbl);
	}
	catch (invalid_read_exception e){
// 		cerr<<"caught exception DirInode::descend:"<<e.what()<<endl;
		throw;
// 		exit(1);
	}
}

void DirInode::next_leaf(vector<InodeName>& vin, dtpage_t& dtree,uint8_t* stbl) throw (BadDirData,invalid_read_exception){
    try{  	
// 		cout << "next leaf: nextindex "<<(int)dtree.header.nextindex<<endl;
		if ((int)dtree.header.nextindex==-1){
			throw BadDirData("Next leaf == -1 ");
			
		}
		for (int32_t i = 0; i < dtree.header.nextindex; i++) {
			//print_direntry(dtree.slot, stbl[i]);
			vin.push_back(get_direntry(dtree.slot, stbl[i]));
		}
		if (dtree.header.next) {
			device.read_bytes(dtree.header.next << device.get_log2blocksize(), sizeof (dtpage_t), (uint8_t *) &dtree);

		/* swap if on big endian machine */
// 		ujfs_swap_dtpage_t(&dtree, type_jfs);

			stbl = (uint8_t *) & (dtree.slot[dtree.header.stblindex]);
			next_leaf(vin,dtree,stbl);
		}
	}
	catch (invalid_read_exception e){
// 		cerr<<"caught exception DirInode::next_leaf: "<<e.what()<<endl;
		throw;
// 		exit(1);
	}
}
/*
 *	display_hex: display region in hex/ascii
 */
static void display_hex(char *addr, unsigned length, unsigned offset){
	uint8_t hextext[37];
	uint8_t asciitxt[17];
	uint8_t *x = (uint8_t *) addr, x1, x2;
	int i, j, k, l;

	hextext[36] = '\0';
	asciitxt[16] = '\0';	/* null end of string */

	l = 0;

	for (i = 1; i <= ((length + 15) / 16); i++) {
	//	if (i > 1 && ((i - 1) % 16) == 0)
// 			if (more())
// 				break;

		/* print address/offset */
		printf("%08x: ", offset + l);

		/* print 16 bytes per line */
		for (j = 0, k = 0; j < 16; j++, x++, l++) {
			if ((j % 4) == 0)
				hextext[k++] = ' ';
			if (l < length) {
				hextext[k++] = ((x1 = ((*x & 0xf0) >> 4)) < 10)
				    ? ('0' + x1) : ('A' + x1 - 10);
				hextext[k++] = ((x2 = (*x & 0x0f)) < 10)
				    ? ('0' + x2) : ('A' + x2 - 10);
				asciitxt[j] = ((*x < 0x20) || (*x >= 0x7f)) ? '.' : *x;
			} else {	/* byte not in range */
				hextext[k++] = ' ';
				hextext[k++] = ' ';
				asciitxt[j] = '.';
			}
		}
		printf("%s   |%s|\n", hextext, asciitxt);
	}
}


/**
 * 
 * @return 
 */
vector<InodeName> DirInode::extract_inode_names(){
// // 	cout <<"vector<InodeName> DirInode::extract_inode_names()"<<endl;
// 	cout <<endl<<"Extracting names for: "<<di_number_<<endl;
//  	if (di_number_==32) 
// 		cout << "hIt"<<endl;

	vector<InodeName> vin;
	try{
		
		int i;
		struct dinode inode;
// 		int64_t inode_address;
// 		unsigned inum;
// 		int64_t node_address;
		struct idtentry *node_entry;
		dtroot_t *root;
// 		uint8_t *stbl;

		device.read_bytes(byte_offset_, sizeof (struct dinode), (uint8_t *) &inode);
		
// 		display_hex((char*)(&inode),512,0);
	
// 		printf("raw data read '%s'\n\n",inode);
	/* swap if on big endian machine */
// 	ujfs_swap_dinode(&inode, GET, type_jfs);

		if ((inode.di_mode & IFMT) != IFDIR) {
// 			cerr << "directory: Not a directory! (You should not see this message)"<<endl;
			return vin;
		}

		root = (dtroot_t *) & (inode.di_btroot);
// 		printf("idotdot = %d\n\n", root->header.idotdot);

		if (root->header.flag & BT_LEAF) {
			if (root->header.nextindex == 0) {
// 				cout<< "Empty directory."<<endl;
				return vin;
			}
// 			cout <<"Extracting names from within inode"<<endl;
			for (i = 0; i < root->header.nextindex; i++) {
// 				cout << "i: "<<i<<endl;
				vin.push_back(get_direntry(root->slot, root->header.stbl[i]));
			}
			return vin;
		}

	/* Root is not a leaf node, we must descend to the leftmost leaf */
	//	cout << "root->header.stbl[0]: "<<(int8_t)(root->header.stbl[0])<<endl;
// 	    printf("root->header.stbl[0]: %i\n",root->header.stbl[0]);

		node_entry = (struct idtentry *) & (root->slot[root->header.stbl[0]]);
		descend(vin,node_entry);
	/* dtree (contained in node) is the left-most leaf node */

// 		cout <<"exit vector<InodeName> DirInode::extract_inode_names()"<<endl;
		return vin;
	}
	catch (invalid_read_exception e){
// 		cerr<<"caught exception in DirInode::extract_inode_names(): "<<e.what()<<endl;
// 		getchar();
		return vin;
// 		exit(1);
	}
	catch (BadDirData e){
// 		cerr<<"caught exception in DirInode::extract_inode_names(): "<<e.what()<<endl;
// 		getchar();
		return vin;
// 		exit(1);
	}
}

InodeName DirInode :: get_direntry(struct dtslot *slot, uint8_t head_index)
{
	InodeName in;
	struct ldtentry *entry;
	int len;
	UniChar *n;
	UniChar *name;
	int namlen;
	int next;
	struct dtslot *s;

	entry = (struct ldtentry *) & (slot[head_index]);
	namlen = entry->namlen;
// 	name = (UniChar *) malloc(sizeof (UniChar) * (namlen + 1));
	name = new UniChar[namlen+1];
	//TODO: throw-catch bad alloc
// 	if (name == 0) {
// 		fputs("dirname: malloc error!\n", stderr);
// 		return in;
// 	}
	name[namlen] = 0;
	len = MIN(namlen, DTLHDRDATALEN);
	UniStrncpy(name, entry->name, len);
	next = entry->next;
	n = name + len;
	vector<int8_t> rec;
	while (next >= 0) {
		rec.push_back(next);
		s = &(slot[next]);
		namlen -= len;
		len = MIN(namlen, DTSLOTDATALEN);
		UniStrncpy(n, s->name, len);
		next = s->next;
		if (find(rec.begin(), rec.end(), next)!=rec.end()){
// 			cout << "recurrent slot parsing detected, aborting"<<endl;
			break;	
		}
		n += len;
	}

        /* Clear the UTF8 conversion buffer. */
//         memset(UTF8_Buffer, 0, sizeof (UTF8_Buffer));

        /* Convert the name into UTF8 */
//         Unicode_String_to_UTF8_String((unsigned char*)UTF8_Buffer, name, entry->namlen);
//         printf("%d\t'%s'\n", entry->inumber, UTF8_Buffer);


	in.name_=Unicode_String_to_UTF8_std_string(name, entry->namlen);
// 	cout << "'"<<in.name_<<"'"<<endl;
	in.di_number_=entry->inumber;
// 	printf("%d\t%s\n", entry->inumber, UTF8_Buffer);
// 	cout << "about to free..."<<endl;
	delete [] name;
// 	cout << "freed!"<<endl;
	return in;

// /*
// 
// 	/* Clear the UTF8 conversion buffer. */
// 	memset(UTF8_Buffer, 0, sizeof (UTF8_Buffer));
// 
// 	/* Convert the name into UTF8 */
// 	Unicode_String_to_UTF8_String((unsigned char*)UTF8_Buffer, name, entry->namlen);
// // 	printf("%d\t%s\n", entry->inumber, UTF8_Buffer);
// // 	cout << "about to free..."<<endl;
// 	delete [] name;
// // 	cout << "freed!"<<endl;
// 	in.di_number_=entry->inumber;
// 	in.name_ = string((char*)UTF8_Buffer);
// 	return in;*/
}

void DirInode :: create_dir(){
// 	cout << "Creating dir: "<<get_full_path().native_file_string()<<endl;
	boost::filesystem::create_directories(get_full_path());
}
