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
#include "inodescanner.h"
#include "inodefactory.h"
using namespace jfsrec;
using namespace std;
using namespace boost::filesystem;
using namespace boost;


const uint16_t InodeScanner :: inode_size_ = 512;


InodeScanner :: InodeScanner(InodeVector& inovec):chunk_blocks_(1<<15),inovec_(inovec){

}


uint64_t InodeScanner :: restore(){
	create_directories(options.get_progress_dir());
	path pfn = options.get_progress_dir()/"inoscan.txt";
	progress_file_.open(pfn.string().c_str(),ios::in);
	if (progress_file_ == NULL){
		cout<<"No progress file found, starting from byte 0"<<endl;
		progress_file_.close();
		progress_file_.open(pfn.string().c_str(),ios::out);
		return 0;
	}
	cout << "Found progress file..."<<flush;
	uint64_t bo;
	Inode* i;
	while (!progress_file_.eof()){
		progress_file_>>bo;
		i = InodeFactory :: read_from_disk(bo);
		inovec_.add_inode(i);
	}
	progress_file_.close();
	progress_file_.open(pfn.string().c_str(),ios::out|ios::app);
	cout << " restarting at "<<((bo+inode_size_)>>20)<<"Mb"<<endl;
	return bo+inode_size_;
}


		
void InodeScanner :: do_scan(){
	progress_timer pt;
	uint64_t chunk_bytes = chunk_blocks_*(uint64_t)device.get_blocksize();
	uint8_t* chunk = new uint8_t[chunk_bytes];
	uint64_t curr_ino;
	uint64_t chunk_inodes=chunk_bytes/inode_size_;

	try{		
		uint64_t devsize = device.get_size_bytes();
	
		uint64_t start = restore();
		
		cout << endl<< endl<<"Scanning for inodes"<<endl<<"This may take a while, really..."<<endl;
		boost::progress_display pd(devsize/chunk_bytes);
		pd+=(start/chunk_bytes);
		uint64_t i;
		for (i=start;i<(devsize-chunk_bytes);i+=chunk_bytes){
			device.read_bytes(i,chunk_bytes,chunk);
	
			for (curr_ino=0;curr_ino<chunk_inodes;curr_ino++){
				Inode* inode = InodeFactory::create_if_sane(&chunk[curr_ino*inode_size_],i+curr_ino*inode_size_);
	
	
	
				//TODO:/* swap if on big endian machine */
				//	ujfs_swap_dinode(&inode, GET, type_jfs);
	
	
				if (inode!=NULL){
					inovec_.add_inode(inode);
	// 				cout << inode->get_di_number()<<" ";
					progress_file_<<i+curr_ino*inode_size_<<"\n";
	// 				if (inode->get_di_number()==4268){
	// 					cout << "f"<<endl;
	// // 					getchar();
	// 				}
				}	
		
			}
	// 		inovec_.print_stats_compact(); //TODO: don't count each time, increment some variable at each find instead
	//  		cout<<"Scanned: "<<(i>>20)<<"Mb of "<<(devsize>>20)<<"Mb"<<endl; //TODO: Percent
			progress_file_<<flush;
			++pd;
		}
	
		uint64_t bytes_left=devsize-i;
		device.read_bytes(i,bytes_left,chunk);
	
		for (curr_ino=0;curr_ino<(bytes_left>>9);curr_ino++){
			Inode* inode = InodeFactory::create_if_sane(&chunk[curr_ino*inode_size_],i+curr_ino*inode_size_);
			//TODO:/* swap if on big endian machine */
			//	ujfs_swap_dinode(&inode, GET, type_jfs);
	
	
			if (inode!=NULL){
				inovec_.add_inode(inode);
	// 				cout << inode->get_di_number()<<" ";
				progress_file_<<i+curr_ino*inode_size_<<endl;
	// 				if (inode->get_di_number()==4268){
	// 					cout << "f"<<endl;
	// // 					getchar();
	// 				}
				}	
		
			}
	// 		inovec_.print_stats_compact(); //TODO: don't count each time, increment some variable at each find instead
	//  		cout<<"Scanned: "<<(i>>20)<<"Mb of "<<(devsize>>20)<<"Mb"<<endl; //TODO: Percent
				++pd;
	
		delete [] chunk;
	}	catch (invalid_read_exception e){
		cerr << "Terminating after catching an invalid_read_exception\nReason:"<<e.what()<<endl;
		delete [] chunk;
		exit(1);
	}
	cout << "Time elapsed:"; //for boost::progress_timer
}

void InodeScanner :: scan(){

	boost::filesystem::create_directory(options.get_progress_dir());

	path fn;
	boost::filesystem::ifstream f;
	fn = options.get_progress_dir()/"post-remove_dupes.txt";
	f.open(fn,ios::in);
	if (f != NULL){
		f.close();
		cout << "Skipping scan."<<endl;
		inovec_.load_from_positions(fn);
		goto scanning_done;
	}
	f.close();

	fn = options.get_progress_dir()/"post-remove_unallocated.txt";
	f.open(fn,ios::in);
	if (f != NULL){
		f.close();
		cout << "Skipping to remove duplicates."<<endl;
		inovec_.load_from_positions(fn);
		goto remove_dupes;
	}
	f.close();

	fn = options.get_progress_dir()/"post-extentscan.txt";
	f.open(fn,ios::in);
	if (f != NULL){
		f.close();
		cout << "Skipping to remove unallocated."<<endl;
		inovec_.load_from_positions(fn);
		goto remove_unallocated;
	}
	f.close();

	fn = options.get_progress_dir()/"post-scan.txt";
	f.open(fn,ios::in);
	if (f != NULL){
		f.close();
// 		cout << "Skipping to extent scan."<<endl;
		cout << "Skipping to inostamp scan."<<endl;
		inovec_.load_from_positions(fn);
// 		goto extent_scan;
		goto filter_inostamps;
	}
	f.close();

	cout << "Preparing inode scan..." << endl;	
	do_scan();
	inovec_.print_stats();
	cout <<endl<<endl;
	inovec_.save_positions(options.get_progress_dir()/"post-scan.txt");
 

	cout << "\n\n\nPostprocessing..."<<endl;

	filter_inostamps:
	filter_inostamps();
	

	extent_scan:
	detect_ino_extents();
	inovec_.print_stats();
	cout <<endl<<endl;
	inovec_.save_positions(options.get_progress_dir()/"post-extentscan.txt");


	remove_unallocated:
	remove_unallocated();
	inovec_.print_stats();
	cout <<endl<<endl;
	inovec_.save_positions(options.get_progress_dir()/"post-remove_unallocated.txt");


	remove_dupes:
  	inovec_.remove_dupes();
	inovec_.print_stats();
	cout <<endl<<endl;
  	inovec_.save_positions(options.get_progress_dir()/"post-remove_dupes.txt");
  	


	inovec_.free_dinodes();

	scanning_done:
// 	inovec_.unmark_all();
// 	inovec_.get_by_di_number(64)->mark();
// 	inovec_.remove_marked();
//   	inovec_.save_positions(options.get_progress_dir()/"dummy.txt");
	cout <<"Scan done..."<<endl<<endl;
	inovec_.print_stats();
	
}


void InodeScanner :: detect_ino_extents(){

	cout << "Searching for inode extents..."<<endl;
	
	boost::progress_display pd(inovec_.size());
	uint16_t num_consec=1;
	int64_t last_din=-2;
	int64_t last_bo =device.get_size_bytes(); //TODO: int or uint?

	for (uint32_t i=0;i<inovec_.size();++i){
		inovec_[i].unmark();
	}
	for (int64_t i=0;i<inovec_.size();++i){
		if (i>33)
			inovec_[i-33].free_dinode();
		
		int64_t j = i;
		num_consec=0;
		do{
			last_din=inovec_[j].get_dinode()->di_number;
			last_bo =inovec_[j].get_byte_offset();
			++num_consec;
			++j;
		} while ( (j<inovec_.size()) && 
					 (inovec_[j].get_dinode()->di_number == last_din+1) &&
					 (inovec_[j].get_byte_offset() == last_bo+512) &&  
					 (num_consec<32) );

		j=i;

		do {
			last_din=inovec_[j].get_dinode()->di_number;
			last_bo =inovec_[j].get_byte_offset();
			++num_consec;
			--j;
		} while ( (j>0) && 
					 (inovec_[j].get_dinode()->di_number == last_din-1) &&
					 (inovec_[j].get_byte_offset() == last_bo-512) &&  
					 (num_consec<32) );
 

// while ( (j>0) && (inovec_[j].get_dinode()->di_number == last_din-1) && (num_consec<32) );

		if (num_consec>=32){
// 			cout << "consec:"<<i<<endl;
			inovec_[i].mark();
		}
// 		cout << "Inode: "<<inovec_[i].get_di_number()<<" at byte "<<inovec_[i].get_byte_offset()<<"  num_consec: "<<num_consec<<" nlink: "<<inovec_[i].get_dinode()->di_nlink<<endl;
// 		getchar();
// 		last_din=inovec_[i].get_dinode()->di_number;
		inovec_[i].free_dinode();
		++pd;
	}	

	uint32_t num_inval=0;
	for (uint32_t i=0;i<inovec_.size();++i){
		inovec_[i].free_dinode();
		if (!inovec_[i].is_marked()){
			++num_inval;
// 			cout << inovec_[i].get_di_number() << " invalidated."<<endl;
		}
	}

	cout << "Done searching for inode extents. Invalidated "<<num_inval<<" of "<<inovec_.size()<<" inodes"<<endl;
	inovec_.remove_unmarked();
	cout << inovec_.size()<<" inodes left."<<endl;
// 	getchar();

}


void InodeScanner :: filter_inostamps(){
	cout << "\nScanning for inostamps.\nmultiple inostamps indicates inodes from different\ngenerations of the file system"<<endl;
	progress_display pd(inovec_.size());
	map<uint32_t,uint32_t> inostamps;
	
	
	for (uint32_t i=0;i<inovec_.size();++i){
		++inostamps[inovec_[i].get_dinode()->di_inostamp];
		inovec_[i].free_dinode();
		++pd;	
	}
	cout << "Found "<<inostamps.size()<<" inostamps:"<<endl;
	uint32_t ismn=0;
	uint32_t issmn=0;
	
	map<uint32_t,uint32_t>::iterator ismi=inostamps.begin();
	
	
	for(map<uint32_t,uint32_t>::iterator iter = inostamps.begin(); iter != inostamps.end(); iter++){
		cout << "  " << (*iter).first << " has " << (*iter).second << " inodes."<<endl;
		if (	(*iter).second>ismn){
			issmn=ismn;
			ismn=(*iter).second;
			ismi=iter;
		}
	}
	if ((ismn/100)>issmn){ 
		//if the most common inostamp is more than 100 times more common than the
		//second most common inostamp, save only the most common.
		cout << "Removing all inodes that does not have inostamp="<<(*ismi).first<<endl;
		inovec_.unmark_all();
		cout << "Marking..."<<endl;
		for (uint32_t i=0;i<inovec_.size();++i){
			if (inovec_[i].get_dinode()->di_inostamp != (*ismi).first)
				inovec_[i].mark();
			inovec_[i].free_dinode();
		}
		cout << "Removing"<<endl;
		inovec_.remove_marked();
		
	}
// 	/*getchar*/();
// 	cout << "\nScanning for inostamps.\nmultiple inostamps indicates inodes from different generations of the file system"<<endl;
// 	progress_display pd(inovec_.size());
// 	map<uint32_t,uint32_t> inostamps;
// 	vector<uint32_t> is;
// 	uint32_t s;
// 	for (uint32_t i=0;i<inovec_.size();++i){
// 		s=inovec_[i].get_dinode()->di_inostamp;
// 		if (find(is.begin(), is.end(), s)== is.end()){
// 			is.push_back(s);
// 		}
// 		inovec_[i].free_dinode();
// 		++pd;	
// 	}
// 	cout << "Found "<<is.size()<<" inostamps:"<<endl;
// 	for (uint32_t i=0;i<is.size();++i){
// 		cout << "  " << is[i]<<endl;
// 	}
// // 	/*getchar*/();
// 	
}
void InodeScanner :: remove_unallocated(){
	cout << endl<<endl<< "Removing unallocated inodes..."<<flush;
	for (uint32_t i=0;i<inovec_.size();++i){
		if (inovec_[i].get_dinode()->di_nlink == 0){
			inovec_[i].mark();
// 			cout << "removing unallocated ino: "<<inovec_[i].get_di_number()<<endl;
		}else{
			inovec_[i].unmark();
		}
		inovec_[i].free_dinode();
	}
	inovec_.remove_marked();
	cout << " [Done]" <<endl;
}

