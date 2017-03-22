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
#include "options.h"
using namespace std;
using namespace jfsrec;

Options jfsrec::options;
using namespace boost::program_options;
using namespace boost::filesystem;
/**
 * 
 * @param argc 
 * @param argv 
 */
void Options :: init(int argc, char **argv){
	

	options_description optdes("Allowed options");

	optdes.add_options()	("help", "Displays a help message")
								("output",value<string>(),"A directory where the extracted files are saved")
								("logdir",value<string>(),"A directory where logfiles are saved")
								("device",value<string>(),"The device from which files are to be extracted")
								("blocksize",value<uint16_t>(&blocksize_)->default_value(4096),"The blocksize of the device")
								("first",value<string>(),"A perl compliant regex that specifies which files are to be recovered first")
								("last",value<string>(),"A perl compliant regex that specifies which files are to be recovered last")
								("exclude",value<string>(),"Exclude files that matches this perl compliant regex")
								("skip-unsane-extents","Skip files that does not have a sane extent-set.")
								;
	variables_map vm;
	store(parse_command_line(argc, argv, optdes), vm);
	notify(vm);    

	if (vm.count("help")) {
		cout << optdes << "\n";
		exit(1);
	}
	if (vm.count("skip-unsane-extents")){
		skip_unsane_xads_=true;
	}else{
		skip_unsane_xads_=false;
	}
	if (vm.count("output")) {
    	output_dir_ = vm["output"].as<string>();
	} else {
		cout << optdes << "\n";
    	cout << "No output directory selected!"<<endl;
		exit(1);
	}
	
	if (vm.count("logdir")) {
    	progress_dir_ = vm["logdir"].as<string>();
	} else {
		cout << optdes << "\n";
    	cout << "No log directory selected!"<<endl;
		exit(1);
	}
	
	if (vm.count("device")) {
    	device_ = vm["device"].as<string>();
	} else {
		cout << optdes << "\n";
    	cout << "No device selected!"<<endl;
		exit(1);
	}

	exclude_speced_=false;
	first_speced_=false;
	last_speced_=false;

	if (vm.count("first")){
   	first_ = boost::regex(vm["first"].as<string>(),boost::regex::perl);
		first_speced_=true;
		cout << "'first' regex: "<<vm["first"].as<string>()<<endl;
	}

	if (vm.count("last")){
   	last_ = boost::regex(vm["last"].as<string>(),boost::regex::perl);
		last_speced_=true;
		cout << "'last' regex: "<<vm["last"].as<string>()<<endl;
	}

	if (vm.count("exclude")){
		exclude_ = boost::regex(vm["exclude"].as<string>(),boost::regex::perl);
		exclude_speced_=true;
		cout << "'exclude' regex: "<<vm["exclude"].as<string>()<<endl;
	}

}

Options::~Options(){

}

