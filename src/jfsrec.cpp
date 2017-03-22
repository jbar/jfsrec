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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "options.h"
#include "inodevector.h"
#include "inodescanner.h"
#include "device.h"
#include "recoverer.h"
using namespace std;
using namespace jfsrec;
// extern Device jfsrec::device;
int main(int argc, char **argv){
	cout<< "starting..."<<endl; //TODO: it seems like the first cout:ed row dissappears in kdev! hence this line
	options.init(argc, argv);
	device.open(options.get_device(),options.get_blocksize());

	InodeVector inovec;

	InodeScanner inoscan(inovec);

	inoscan.scan();
	inovec.print_stats();
	
	Recoverer rec(inovec);
	rec.recover();
	cout << "Done!"<<endl;
}

