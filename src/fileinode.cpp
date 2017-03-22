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
#include "fileinode.h"
using namespace jfsrec;
using namespace std;
using namespace boost::filesystem;
using namespace boost;
void print_xad(xad_t* xad);
const uint32_t FileInode :: buffsize_=(1<<16);

FileInode :: FileInode(uint8_t* ip,uint64_t bo):Inode(ip,bo){
}



void FileInode :: recover(){
	static int saved=0;
	static int skipped=0;

	try{
	path p = get_full_path();
	//TODO: Check below. Not really sure that the date-check works.
	if ( (exists(p)) && ((file_size(p)==get_dinode()->di_size) || (last_write_time(p) > get_dinode()->di_otime.tv_sec))){
		++skipped;
		cout <<endl<<endl<<"Skipping "<<di_number_<<": "<<p.string()<<endl;
		
	}else{
		++saved;
		cout <<endl<<endl<<"Saving   "<<di_number_<<": "<<p.string()<<endl;

		save_file();

		set_mode();

		set_atime(get_dinode()->di_atime);
		set_ctime(get_dinode()->di_ctime);
		set_mtime(get_dinode()->di_mtime);
		set_otime(get_dinode()->di_otime);
	}
	cout << "Saved: "<<saved<<" Skipped: "<<skipped<<endl;
	free_dinode();
	}catch(filesystem_error e){
		cout << "Caught filesystem_error in FileInode :: recover(). Reason:"<<e.what()<<endl;
		free_dinode();
		return;
	}
	catch(invalid_read_exception e){
		cout << "Caught invalid_read_exception in FileInode :: recover(). Reason:"<<e.what()<<endl;
		free_dinode();
		return;
	}
}

void FileInode :: print_dir_structure(uint32_t& level){
	for (uint16_t i=0;i<level;++i)
		cout <<" ";
	++level;
	cout << name_<<endl;
	--level;
// 	getchar();
	
}
void FileInode ::  write_extent_to_file(xad_t* x, FILE* f,progress_display* prog) throw (invalid_read_exception){
// 	cout <<endl<<endl<<"Extent size: "<< bytes_suffixed(lengthXAD(x)*device.get_blocksize())<<endl;

	
	static uint8_t buffer[buffsize_];
	uint64_t bytes_read=0;
	while (lengthXAD(x)*device.get_blocksize()>bytes_read){
		if (lengthXAD(x)*device.get_blocksize()-bytes_read>buffsize_){
			device.read_bytes(addressXAD(x)*device.get_blocksize()+bytes_read,buffsize_,buffer);
			fseeko64(f,offsetXAD(x)*device.get_blocksize()+bytes_read,SEEK_SET);
			fwrite(buffer,1,buffsize_,f);
			bytes_read+=buffsize_;

			if (prog)
				prog->operator+=(buffsize_/device.get_blocksize());
		}else{
			uint64_t bytes_to_read = lengthXAD(x)*device.get_blocksize()-bytes_read;
			device.read_bytes(addressXAD(x)*device.get_blocksize()+bytes_read,bytes_to_read,buffer);
			fseeko64(f,offsetXAD(x)*device.get_blocksize()+bytes_read,SEEK_SET);
			fwrite(buffer,1,bytes_to_read,f);
			bytes_read+=bytes_to_read;

			if (prog)
				prog->operator+=(bytes_to_read/device.get_blocksize());
		}
		
	}
}

void FileInode :: get_internal_xtpage(xad_t* xad,vector<xad_t>& v) throw (extent_error){

	xtpage_t xtree_area;
	xtpage_t *xtree = &xtree_area;
	int64_t 	xtpage_address = addressXAD(xad)* options.get_blocksize();
	
	
	device.read_bytes(xtpage_address, sizeof (xtpage_t), (uint8_t *) xtree);
		
	if (xtree->header.nextindex <= 2)
		return;
	if (xtree->header.flag & BT_LEAF){
		get_leaf_xads(xtree->xad, xtree->header.nextindex, v);
 	}else{ 
		get_internal_xads(xtree->xad,xtree->header.nextindex, v);
	}
	
}

/*
	progress_display* prog;
	try{
		if ((options.get_skip_unsane_xads() && !xads_sane(xad,nextindex))){
			throw extent_error("Unsane extents-set found. Skipping.");
		}
		int i;
		cout << "Extents: ";
		uint64_t filesize = 0;
		for (i = 2; i < nextindex; i++) {
			cout << " "<<bytes_suffixed(lengthXAD(&xad[i])*device.get_blocksize());
			filesize+=lengthXAD(&xad[i])*device.get_blocksize();
			}   
// 		for (i = 2; i < nextindex; i++) {
// 			print_xad(&xad[i]);
// 		}
		cout << "\nTotal size: "<<bytes_suffixed(filesize)<<" (rounded up to closest block boundrary)"<<endl<<endl;
		
		
		if (filesize>20*buffsize_)
			prog = new progress_display(filesize/device.get_blocksize());
		else	
			prog = NULL;
		
		for (i = 2; i < nextindex; i++) {
			write_extent_to_file(&xad[i],file,prog);
		}
		if (prog)
			delete prog;*/




void FileInode ::  get_leaf_xads(xad_t * xad, short nextindex, vector<xad_t>& v) throw (extent_error,filesystem_error,invalid_read_exception){

 	for (int i = 2; i < nextindex; i++) {
		v.push_back(xad[i]);
	}
	
}


void FileInode :: get_internal_xads(xad_t * xad, short nextindex, vector<xad_t>& v) throw (boost::filesystem::filesystem_error,extent_error,invalid_read_exception){
	int i;
	for (i = 2; i < nextindex; i++) {
		get_internal_xtpage(&xad[i],v);
	}
}


void FileInode :: save_file() throw(filesystem_error,invalid_read_exception){
	
	string fn = get_full_path().string();
	FILE* file;	
	try{

		xtpage_t *xtree;
	
		
		do {
			file = fopen64(fn.c_str(), "wb+");
			if (file==NULL){
				cerr << "Unable to open file: "<<fn<<endl;
				cerr << strerror(errno) <<endl;
				cerr << "Press enter to try again."<<endl;
				getchar();
			}
		}while (file==NULL);
	
		get_dinode();
		
		xtree = (xtpage_t *) & (dinode_->di_btroot);
		//printf("Root X-Tree Node of inode %d\n\n", ip->di_number);
		//display_xtpage(xtree);
		if (xtree->header.nextindex <= 2){
			cout << "Inode contains no extents"<<endl;	
			free_dinode();
			fclose(file);
			return;
		}

		
		vector<xad_t> xads;

		if (xtree->header.flag & BT_LEAF){
			get_leaf_xads(xtree->xad, xtree->header.nextindex, xads);
		}else{
			get_internal_xads(xtree->xad, xtree->header.nextindex, xads);
		}
	

		print_extents(xads);
		uint64_t filesize = filesize_from_xad_vector(xads);
		cout << "Total size: "<< bytes_suffixed(filesize)<< " (rounded up to closest block boundrary) ";
		cout << "Total number of extents: "<<xads.size()<<endl;

		progress_display* prog = new progress_display(filesize/device.get_blocksize());


		for (uint32_t i=0;i<xads.size();++i){
			write_extent_to_file(&xads[i],file,prog);
		}

		ftruncate(fileno(file), get_dinode()->di_size);
	
		free_dinode();
	
		fclose(file);
	
		//printf("File closed\n");
		cout << endl;	
	}catch(filesystem_error e){
		cout << "Caught filesystem_error in FileInode :: save_file(). Reason:"<<e.what()<<endl;
// 		ftruncate(fileno(file), get_dinode()->di_size);
		free_dinode();
		fclose(file);
		throw;
// 		return;
	}catch(extent_error e){
		cout << "Caught extent_error in FileInode :: save_file(). Reason:"<<e.what()<<endl;
// 		ftruncate(fileno(file), get_dinode()->di_size);
		free_dinode();
		fclose(file);
// 		return;
	}catch(invalid_read_exception e){
		cout << "Caught invalid_read_exception in FileInode :: save_file(). Reason:"<<e.what()<<endl;
// 		ftruncate(fileno(file), get_dinode()->di_size);
		free_dinode();
		fclose(file);
		throw;
// 		return;
	}
}

void print_xad(xad_t* xad){

		printf("XAD\n");
		printf("[1] xad.flag\t  %x\t\t", xad->flag);
		printf("[4] xad.len\t  0x%06x\n", xad->len);
		printf("[2] xad.off1\t  0x%02x\t\t", xad->off1);
		printf("[5] xad.addr1\t  0x%02x\n", xad->addr1);
		printf("[3] xad.off2\t  0x%08x\t", xad->off2);
		printf("[6] xad.addr2\t  0x%08x\n", xad->addr2);
		printf("    xad.off  \t  %lld\t\t", (long long) offsetXAD(((xad))));
		printf("    xad.addr\t  %lld\n", (long long) addressXAD(((xad))));

}


void FileInode :: print_extents(const vector<xad_t>& xads) const{
	cout << "Extents: ";
	for (uint64_t i=0;i<xads.size();++i)
		cout << bytes_suffixed( lengthXAD(&xads[i]) * device.get_blocksize() ) << " ";

	cout <<endl;

}

uint64_t FileInode :: filesize_from_xad_vector(const vector<xad_t>& xads) const{
	uint64_t filesize=0;

	for (uint64_t i=0;i<xads.size();++i)
		filesize += lengthXAD(&xads[i])*device.get_blocksize();
	return filesize;
}


bool FileInode :: xads_sane(const vector<xad_t>& xads) const{

	uint64_t prev_xad_addr = addressXAD(&xads[0]);
 	uint64_t filesize = lengthXAD(&xads[0])*device.get_blocksize();
	
	for (uint16_t i = 1; i < xads.size(); i++){
		if (prev_xad_addr>addressXAD(&xads[i])){
			cerr << "XADs are non-linear!"<<endl;
			return false;
		}

		prev_xad_addr = addressXAD(&xads[i]);
		
    	filesize+=lengthXAD(&xads[i])*device.get_blocksize();
	}

	if (filesize>get_dinode()->di_size+device.get_blocksize()){//blocksize as a safety margin.
		cerr << "Extents contains _more_ data than the inode describes!"<<endl;
		return false;
	}
		
	
	for (uint16_t i = 0; i < xads.size(); i++){
		for (uint16_t j = i+1; j < xads.size(); j++){
			if (offsetXAD(&xads[i])+lengthXAD(&xads[i])>offsetXAD(&xads[j])){
				cerr << "Overlapping extents found!"<<endl;
				return false;
			}
		}
	}
	return true;
}
