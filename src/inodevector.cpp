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
#include "inodevector.h"

using namespace jfsrec;
using namespace std;
using namespace boost;

InodeVector :: ~InodeVector(){
	for (uint32_t i=0;i<inodes_.size();++i)
		delete inodes_[i];
//
//		TODO: This generates an illegal instruction, strange...
// 	for (uint32_t j=0;j<invalidated_inodes_.size();++j)
// 		delete invalidated_inodes_[j];
//			
}

Inode* InodeVector :: get_by_di_number(uint32_t di){
	if (!di_number_index_ok_){
		generate_di_number_index();
	} 
	map<uint32_t,Inode*>:: iterator i = di_number_index_.find(di);		
	
		if ( i != di_number_index_.end())
			return i->second;
		else
			return NULL;
}

Inode* InodeVector :: get_by_number(uint32_t i){
	return inodes_[i];
}


vector<Inode*> InodeVector :: get_invalidated_by_di_number(uint32_t di){
	vector<Inode*> iv;
	for (uint32_t i=0;i<invalidated_inodes_.size();++i)
		if (invalidated_inodes_[i]->get_di_number()==di)
			iv.push_back(invalidated_inodes_[i]);
	return iv;
}

void InodeVector::add_inode(Inode* i){

// 	cout << "Added inode. Number of inos:"<<inodes_.size()<<endl;
	if (!di_number_index_ok_){
		inodes_.push_back(i);
	}else{
		if ((di_number_index_.find(i->get_di_number()) == di_number_index_.end())){
			di_number_index_[i->get_di_number()] = i;
			inodes_.push_back(i);
			di_number_index_ok_=true;
		}else {
			inodes_.push_back(i); //dupe inodes.
			di_number_index_ok_=false;
		}
	}
}

void InodeVector :: print_stats(){
		cout << "Total number of inodes stored:"<<inodes_.size()<<endl;
		InodeCounter ic;
		ic.num_files_ = 0;
		ic.num_dirs_  = 0;
		ic.num_unall_ = 0;
		ic.num_symlinks_ = 0;
		ic.num_specials_ = 0;
		for (uint32_t i=0;i<inodes_.size();++i){	
			inodes_[i]->count(ic);
		}
		cout << "Number of\n  Directories: "<<ic.num_dirs_<<"\n  files: "<<ic.num_files_<<"\n  symlinks: "<<ic.num_symlinks_<<"\n  special: "<<ic.num_specials_<<"\n  unallocated: "<<ic.num_unall_<<endl;

}

void InodeVector :: print_stats_compact(){
		InodeCounter ic;
		ic.num_files_ = 0;
		ic.num_dirs_  = 0;
		ic.num_unall_ = 0;
		ic.num_symlinks_ = 0;
		ic.num_specials_ = 0;
		for (uint32_t i=0;i<inodes_.size();++i){
			inodes_[i]->count(ic);
		}
		cout << "Number of\n  Directories: "<<ic.num_dirs_<<"\n  files: "<<ic.num_files_<<"\n  symlinks: "<<ic.num_symlinks_<<"\n  special: "<<ic.num_specials_<<"\n  unallocated: "<<ic.num_unall_<<endl;
//		getchar();
}

void InodeVector :: print_di_numbers(){
		cout << "Inodes stored:"<<inodes_.size()<<endl;

		for (uint32_t i=0;i<inodes_.size();++i){
			cout << inodes_[i]->get_di_number()<<inodes_[i]->get_typechar()<<"\t";
		}
		cout << endl;
// 		getchar();
}

void InodeVector :: free_dinodes(){
	cout << "Freeing dinodes"<<flush;
	for (uint32_t i=0;i<inodes_.size();++i){
		inodes_[i]->free_dinode();
	}
	cout << "\rFreeing dinodes [DONE]"<<endl;

}

void InodeVector :: remove_marked(){
	// 	cout << "numinos: "<<inodes_.size()<<endl;
	for (uint32_t i=0;i<inodes_.size();++i){
		if (inodes_[i]->is_marked()){
			invalidated_inodes_.push_back(inodes_[i]);
			inodes_[i]=NULL;
		}
	}
 	inodes_.erase(remove(inodes_.begin(), inodes_.end(), (Inode*)NULL), inodes_.end());
	di_number_index_ok_=false;
}

void InodeVector :: mark_all(){
	// 	cout << "numinos: "<<inodes_.size()<<endl;
	for (uint32_t i=0;i<inodes_.size();++i){
		inodes_[i]->mark();
	}
}

void InodeVector :: unmark_all(){
	// 	cout << "numinos: "<<inodes_.size()<<endl;
	for (uint32_t i=0;i<inodes_.size();++i){
		inodes_[i]->unmark();
	}
}

void InodeVector :: remove_unmarked(){
	// 	cout << "numinos: "<<inodes_.size()<<endl;
	for (uint32_t i=0;i<inodes_.size();++i){
		if (!inodes_[i]->is_marked()){
			invalidated_inodes_.push_back(inodes_[i]);
			inodes_[i]=NULL;
		}
	}
 	inodes_.erase(remove(inodes_.begin(), inodes_.end(), (Inode*)NULL), inodes_.end());
	di_number_index_ok_=false;
}

void InodeVector :: remove_dupes(){
	cout << "Removing duplicates"<<flush;
	for (uint32_t i=0;i<inodes_.size();++i)
		inodes_[i]->unmark();
	generate_di_number_index();
	inomap_type::iterator it;
	for(it=di_number_index_.begin(); it!=di_number_index_.end(); ++it){
		it->second->mark();
	}
	remove_unmarked();
	cout << "Done"<<endl;
}

void InodeVector :: save_positions(const boost::filesystem::path& fn){
	cout << endl <<"Saving inodes to file..."<<endl;
	progress_display pd(inodes_.size());

// 	cout << "Saving inodes..."<<flush;
	boost::filesystem::ofstream f;
	f.open(fn,ios::out);
	for (uint32_t i=0;i<inodes_.size();++i){
		f << inodes_[i]->get_byte_offset()<<"\n";
		++pd;
	}
	f<<flush;
	f.close();
	if (invalidated_inodes_.size()>0){
		cout << endl <<"Saving invalidated inodes to file..."<<endl;
		progress_display pd1(invalidated_inodes_.size());

// 	cout << "Saving inodes..."<<flush;
		
		f.open(options.get_progress_dir()/"invalidated.txt",ios::out);
		for (uint32_t i=0;i<invalidated_inodes_.size();++i){
			f << invalidated_inodes_[i]->get_byte_offset()<<"\n";
			++pd1;
		}
		f<<flush;
		f.close();
	}
// 	cout << " [Done]"<<endl;
}

void InodeVector :: load_from_positions(const boost::filesystem::path& fn){
	
	 
	
	cout << "Loading inodes..."<<flush;
	boost::filesystem::ifstream f;
	f.open(fn,ios::in);
	
	uint64_t bo;
	Inode* i;
	while (!f.eof()){
		f>>bo;
		i = InodeFactory :: read_from_disk(bo);
		inodes_.push_back(i);
	}
	f.close();
	cout << " [Done] "<<inodes_.size()<<" inodes in total."<<endl;
	
	cout << "Loading invalidated inodes..."<<flush;
	f.open(options.get_progress_dir()/"invalidated.txt",ios::in);
	
	while (!f.eof()){
		f>>bo;
		i = InodeFactory :: read_from_disk(bo);
		invalidated_inodes_.push_back(i);
	}
	f.close();
	cout << "Number of invalidated inodes: "<<invalidated_inodes_.size()<<endl;

	di_number_index_ok_=false;
}



void InodeVector :: generate_di_number_index() const{
// 	cout << "Generating di_number index..."<<flush;

	di_number_index_.clear();
	for (uint32_t i=0;i<inodes_.size();++i){
		if (di_number_index_.find(inodes_[i]->get_di_number()) == di_number_index_.end()){
			di_number_index_[inodes_[i]->get_di_number()] = inodes_[i];
		}else{
			if ((
					(*di_number_index_[inodes_[i]->get_di_number()]).get_dinode()->di_otime) <
				   (inodes_[i]->get_dinode()->di_otime)
					){
				di_number_index_[inodes_[i]->get_di_number()] = inodes_[i];  
				//if two identical di_numbers are found, use the lates allocated
			}
// 			cout << "Non-unique di_number found! "<<inodes_[i]->get_di_number() <<endl;
// 			di_number_index_.find(inodes_[i]->get_di_number())->second->print();
// 			cout << endl<<endl;
// 			inodes_[i]->print();
			
// 			getchar();
// 			cout <<endl<<endl<<endl;
		}
	}
	di_number_index_ok_=true;
// 	cout << " [Done]"<<endl;
}
