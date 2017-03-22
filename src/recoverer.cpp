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
#include "recoverer.h"

using namespace std;
using namespace jfsrec;
using namespace boost;

Recoverer::Recoverer(InodeVector& iv)
	:inovec_(iv){
}

void Recoverer::recover(){
	//recover_names();
	find_directory_parents();
	find_file_parents_and_names();
	find_roots();
	if (!(options.get_last_speced() ||options.get_first_speced()||options.get_exclude_speced()) ){
		recover_dirtree();
		recover_iterative();
	}else{
		deque<Inode*> prio = generate_prio_queue();
		recover_dirtree(prio);
		recover_iterative(prio);
	}
	//recover_recursive();
}

deque<Inode*> Recoverer :: generate_prio_queue(){
	deque<Inode*> first;
 	deque<Inode*> last;
	deque<Inode*> normal;
	cout << "Generating prio queue"<<endl<<endl;
	
	if (options.get_exclude_speced()){
		progress_display pd(inovec_.size());
		for (uint32_t i=0;i<inovec_.size();++i){
			++pd;	
			if (regex_search(inovec_[i].get_full_path().native_file_string(),options.get_exclude())) {
// 				cout << "Matched exclude regex!!!"<<endl;
			}else{
				if ((options.get_first_speced() && regex_search(inovec_[i].get_full_path().native_file_string(),options.get_first()))){
					first.push_back(&inovec_[i]);
					continue;
				}
				if (options.get_last_speced() && (regex_search(inovec_[i].get_full_path().native_file_string(),options.get_last()))){
					last.push_back(&inovec_[i]);
					continue;
				}
				normal.push_back(&inovec_[i]);
			
			}
		}
	} else{
		progress_display pd(inovec_.size());
		for (uint32_t i=0;i<inovec_.size();++i){
			++pd;
// 			cout <<"matching:"<< inovec_[i].get_full_path().native_file_string()<<endl;
			if ((options.get_first_speced() && regex_search(inovec_[i].get_full_path().native_file_string(),options.get_first()))){
				first.push_back(&inovec_[i]);
				continue;
			}
			if (options.get_last_speced() && (regex_search(inovec_[i].get_full_path().native_file_string(),options.get_last()))){
				last.push_back(&inovec_[i]);
				continue;
			}
			normal.push_back(&inovec_[i]);
		}
	}

	cout << "Queue: first:"<<first.size()<<" normal:"<<normal.size()<<" last:"<<last.size()<<endl;
  //TODO: There's probably a more STL-ish way of doing this...
	while (normal.size()){
		first.push_back(*normal.begin());
		normal.pop_front();
	}
	while (last.size()){
		first.push_back(*last.begin());
		last.pop_front();
	}
	cout << "Generating prio queue [DONE]"<<endl;

		

	return first;
}

void Recoverer::recover_dirtree(){
	cout << "\n\nCreating directory structure"<<endl;
	boost::filesystem::path p(options.get_output_dir());
	boost::filesystem::create_directory(p);
	progress_display pd(inovec_.size());
	for (uint32_t i=0;i<inovec_.size();++i){
		inovec_[i].create_dir();
		++pd;
	}
}

void Recoverer::recover_dirtree(const deque<Inode*>& prio){
	cout << "\n\nCreating directory structure"<<endl;
	boost::filesystem::path p(options.get_output_dir());
	boost::filesystem::create_directory(p);
	progress_display pd(prio.size());
	for (uint32_t i=0;i<prio.size();++i){
		prio[i]->create_dir();
		++pd;
	}
}

void Recoverer :: recover_iterative(){

// 	cout << "Number of roots: " <<roots_.size()<<endl;
	for (uint32_t i=0;i<inovec_.size();++i){
		cout << "Progress :"<<i<<" of "<<inovec_.size() << "("<<i/inovec_.size()<<"%)"<<endl;
		inovec_[i].recover();
	}

}

void Recoverer :: recover_iterative(const deque<Inode*>& prio){

// 	cout << "Number of roots: " <<roots_.size()<<endl;
	for (uint32_t i=0;i<prio.size();++i){
		cout << "Progress :"<<i<<" of "<<prio.size() << " ("<<100.0*i/prio.size()<<"%)"<<endl;
		prio[i]->recover();
	}

}


void Recoverer :: find_directory_parents(){

	cout <<endl<<endl<<"Searching for directory parents"<<endl;
	progress_display pd(inovec_.size());

// 	cout << "Searching for directory parents"<<endl;
	for (uint32_t i=0;i<inovec_.size();++i){
		++pd;
		Inode* p;
		Inode & c=inovec_[i];
// 		cout << "\r"<<((double)i/(double)inovec_.size())*100.0<<"%"<<flush;
		if ((dynamic_cast<DirInode*>(&c))!=NULL){
			p=inovec_.get_by_di_number(c.get_parent_di_number());
//	Kolla så att indexet funkar. hittas rätt inode? Kanske göra en sanity-checkmetod i inovector för att kolla integreteten på indexet


			if (p!=NULL){
				if (dynamic_cast<FileInode*>(p)!=NULL){
// 					cerr << "Parent is a regular file:" << c.get_di_number()<<" parent: "<< p->get_di_number()<<endl;
// 				getchar();
				}else{
// 					cerr << "Found a single parent to: "<< c.get_di_number()<<" parent: "<< p->get_di_number()<<endl;
					c.set_parent(p);
				}

			}else{ //

/*				Sane-by-association:
					An inode is considered sane, even if it was invalidated during the postprocessing after the inode scan, 
					if an other inode wants it as parent. This code takes care of missing non-leaf directory inodes. 
					That is, inodes that are referenced from "below".
*/
				vector<Inode*> v = inovec_.get_invalidated_by_di_number(c.get_parent_di_number());
				if (v.size()>0){
					timestruc_t newest = v[0]->get_dinode()->di_otime;
					uint32_t newest_j=0;
					for (uint32_t j=1;j<v.size();++j){
						if (v[j]->get_dinode()->di_otime<newest){
							newest=v[j]->get_dinode()->di_otime;
							newest_j = j;
						}
					}
					inovec_.add_inode(v[newest_j]);
					c.set_parent(v[newest_j]);
// 					cout << "Validated "<<v[newest_j]->get_di_number()<<" because it is parent to "<<c.get_di_number()<<endl;
				}	else {
// 					cerr << c.get_di_number() << " requested "<< c.get_parent_di_number() << " but it was not found."<<endl;
				}
			}
		}
	}
	cout <<endl;
}

void Recoverer :: find_file_parents_and_names(){

 	cout <<endl<< "Searching for file parents and names"<<endl;
	
	progress_display pd(inovec_.size());
	for (uint32_t i=0;i<inovec_.size();++i){
// 		cout << "\r"<<((double)i/(double)inovec_.size())*100.0<<"%"<<flush;
		vector<InodeName> in = inovec_[i].extract_inode_names();
		for (uint32_t j=0;j<in.size();++j){
			Inode* ino = inovec_.get_by_di_number(in[j].di_number_);
			if (ino!=NULL){
				ino->set_name(in[j].name_);
				ino->set_parent(&inovec_[i]);
			}else{


/*				Sane-by-association:
					An inode is considered sane, even if it was invalidated during the postprocessing after the inode scan, 
					if an other inode wants it as its child.
*/
				vector<Inode*> v = inovec_.get_invalidated_by_di_number(in[j].di_number_);
				if (v.size()>0){
					timestruc_t newest = v[0]->get_dinode()->di_otime;
					uint32_t newest_k=0;
					for (uint32_t k=0;k<v.size();++k){
						if (v[k]->get_dinode()->di_otime<newest){
							newest=v[k]->get_dinode()->di_otime;
							newest_k = k;
						}
					}
					inovec_.add_inode(v[newest_k]);
					v[newest_k]->set_name(in[j].name_);
					v[newest_k]->set_parent(&inovec_[i]);
// 					cout << "Validated "<<v[newest_k]->get_di_number()<<" because it is a sibling to "<<inovec_[i].get_di_number()<<endl;
				}	else {
// 					cerr << in[j].di_number_ << " requested "<< inovec_[i].get_di_number() << " but it was not found."<<endl;
				}



			}
// 			cout << in[j].di_number_<<"\t"<<in[j].name_<<endl;
		}
		++pd;
	}
	for (uint32_t i=0;i<inovec_.size();++i){
		if ( (inovec_[i].get_name().empty()) || 
			  (inovec_[i].get_name().find("\n")!=string::npos) ||
			  (inovec_[i].get_name().find("\r")!=string::npos)) { //or otherwise is an illegal name
// 			cout << "using di_number for name "<<inovec_[i].get_di_number()<<endl;
			stringstream ss;
			ss<<inovec_[i].get_di_number();
			inovec_[i].set_name(ss.str());
		}
	}
	if (inovec_.get_by_di_number(2)!=NULL)
		inovec_.get_by_di_number(2)->set_name("original_root");
	cout <<endl;
}




void Recoverer :: find_roots(){
	cout << "Searching for roots"<<endl;
	vector<Inode*> r;
	for (uint32_t i=0;i<inovec_.size();++i){
		if (inovec_[i].has_parent()){
			Inode * p = inovec_[i].get_root() ;
			r.push_back(p);
// 			cout << "root of "<<inovec_[i].get_di_number()<< "is "<<p->get_di_number()<<endl;
		}else{
			r.push_back(&inovec_[i]);
		}
	}
	//TODO: clean up vector removal
	cout << "Getting unique roots"<<endl;
	sort(r.begin(), r.end());
	vector<Inode*>::iterator e = unique(r.begin(), r.end());
	for (vector<Inode*>::iterator b = r.begin();b != e;b++) {
// 		cout << (*b)->get_di_number() << endl;
		roots_.push_back(*b);
	}
	
}


void Recoverer :: print_tree(){
	find_roots();
	cout << "Found "<<roots_.size() << " roots."<<endl;
	uint32_t level=0;
	for (uint16_t i=0;i<roots_.size();++i){
		DirInode* t = dynamic_cast<DirInode*>(roots_[i]);
		if (t!=NULL)
			t->print_dir_structure(level);
	}

}
void Recoverer :: print_tree_reverse(){
	find_roots();
	cout << "Found "<<roots_.size() << " roots."<<endl;
		
	for (uint16_t i=0;i<inovec_.size();++i){
		inovec_[i].print_parent(0);
		cout << endl;
	}

}

