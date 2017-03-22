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

#ifndef INODE_H
#define INODE_H

#include "jfs_types.h"
#include "jfs_dinode.h"
#include "device.h"
#include "options.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <boost/utility.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>


namespace jfsrec{

	class extent_error{
		private:
			std::string error_;
		public:
			extent_error(const std::string& e):error_(e){}
			std::string what(){return error_;}
	};

	struct InodeName{
		std::string name_;
		uint32_t di_number_;
	};

	struct InodeCounter{
		int num_unall_;
		int num_files_;
		int num_dirs_;
		int num_symlinks_;
		int num_specials_;
	};

	bool operator<(const timestruc_t&,const timestruc_t&);

	class Inode : boost::noncopyable{
		protected:
			uint64_t byte_offset_;
			Inode* parent_;
			std::string name_;
			uint32_t parent_di_number_;
			uint32_t di_number_;
			bool is_marked_;
			
			mutable bool dinode_available_;
			mutable dinode* dinode_;
			virtual void read_dinode() const;

		public:
			Inode();
			virtual ~Inode();
			explicit Inode(uint8_t*,uint64_t);
// 			explicit Inode(uint64_t);

			virtual void unmark(){is_marked_=false;}
			virtual void mark(){is_marked_=true;}
			virtual bool is_marked(){return is_marked_;}

			virtual void recover()=0; //pure virtual

			virtual void set_parent(Inode*);
			
			virtual bool has_parent(){return ((parent_!=NULL)||(parent_==this));}
	
			virtual void set_name(const std::string& n){name_=n;}
			virtual std::string get_name() const{return name_;}
			virtual uint32_t get_di_number() const {return di_number_;}
			virtual uint32_t get_parent_di_number() const {return parent_di_number_;}
			virtual uint64_t get_byte_offset() const {return byte_offset_;}
			virtual void print_parent(uint16_t);
			virtual void print_dir_structure(uint32_t&)=0; 
			virtual Inode* get_root();
			virtual std::vector<InodeName> extract_inode_names();

			virtual dinode* get_dinode() const;
			virtual void free_dinode() const;

			virtual boost::filesystem::path get_full_path();
			virtual void create_dir(){};
			virtual void print() const;
			virtual void count(InodeCounter&) const = 0;		
			virtual char get_typechar() const = 0;
			virtual void set_atime(const timestruc_t&);
			virtual void set_ctime(const timestruc_t&);
			virtual void set_mtime(const timestruc_t&);
			virtual void set_otime(const timestruc_t&);
			virtual void set_mode(){};
			
	};


}
#endif
