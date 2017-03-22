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

//Parts of the code in this file was originally taken from JFS's xpeek (debugfs_jfs)

#include "device.h"
#ifdef __linux__
 #include <linux/fs.h>
 #include <sys/ioctl.h>
#endif


using namespace jfsrec;
using namespace std;
using namespace boost::filesystem;

Device jfsrec::device;
Device :: ~Device(){
	if (file_)
		fclose(file_);
}

string jfsrec :: bytes_suffixed(uint64_t f){
	stringstream ss;
	if (f< (((int64_t)1)<<10))
		ss<<f<<"b"<<endl;
	else if (f< (((int64_t)1)<<20))
		ss<<((f*100)>>10)/100.0<<"kb";
	else if (f< (((int64_t)1)<<30))
		ss<<((f*100)>>20)/100.0<<"Mb";
	else if (f< (((int64_t)1)<<40))
		ss<<((f*100)>>30)/100.0<<"Gb";
	else	
		ss<<((f*100)>>40)/100.0<<"Tb";
	return ss.str();
}

void Device :: open(const path & name, uint16_t blocksize){


	blocksize_=blocksize;
	file_ = fopen64(name.native_file_string().c_str(), "r");
	if (file_ == NULL) {
		cerr << "Error: Cannot open device: '"<<name.native_file_string()<<"'"<<endl;
		exit(1);
	}
	struct stat64 stat_data;
   int r;
	if ((r=fstat64(fileno(file_), &stat_data))!=0){
		cerr<< "Error: cannot stat device: '"<<name.native_file_string()<<"'"<<" "<<r<<endl;		
		exit(1);
	}
	
	
	if (blocksize_==4096){
		log2blocksize_=12;
	}else if (blocksize_==2048){
		log2blocksize_=11;
	}else if (blocksize_==1024){
		log2blocksize_=10;
	}else if (blocksize_==512){
		log2blocksize_=9;
	}else{
		cerr<<"Invalid blocksize: "<<blocksize<<endl;
		exit(1);
	}
	fssize_ = get_size_bytes();

	cout << "Opened device: "<<name.native_file_string()<<endl;
// 	cout << "  Size in blocks: "<<(fssize_>>log2blocksize)<<endl;
	cout << bytes_suffixed(fssize_);
	
	
	cout << "  Blocksize: "<<device.get_blocksize()<<endl;
	
/*

	if (!is_valid(&stat_data)){
		cerr << "Error: Not a valid device: '"<<uc.c_str()<<"'"<<endl;
		exit(1);
	}*/
// /*
// 	/* Do we have a /*/*/*block special device or regular file? */
// #if defined(__DragonFly__)
// 	if (!S_ISCHR(st->st_mode) && !S_ISREG(st->st_mode))
// #else /* __linux__ etc. */
// 	if (!S_ISBLK(st->st_mode) && !S_ISREG(st->st_mode))
// #endif
// 		return false;*/*/*/
// }*/
}





void Device :: read_bytes(int64_t disk_offset,uint32_t disk_count,uint8_t *data_buffer) throw (invalid_read_exception){
// 	cout << "Device :: read_bytes(...) disk_offset: "<<disk_offset<<" disk_count: "<<disk_count<<endl;
	if (disk_offset>fssize_){
		throw invalid_read_exception("Requested read beyond file system size.");
	}
	int Seek_Result;
	size_t Bytes_Transferred;
	Seek_Result = fseeko64(file_, disk_offset, SEEK_SET);
	if (Seek_Result != 0) {
		rewind(file_);
		throw invalid_read_exception("unable to seek");
	}

	if (disk_count == 0) {
		//fprintf(stderr, "ujfs_rw_diskblocks: disk_count is 0\n");
		rewind(file_);
		throw invalid_read_exception("could not read 0 bytes");
		//return; //TODO: maybe throw exception
	}

	Bytes_Transferred = fread(data_buffer, 1, disk_count, file_);


	if (Bytes_Transferred != disk_count) {
		if (Bytes_Transferred == -1)
			cerr<<"Device::read_blocks"<<endl;
		else
			cerr<<"Device::read_blocks: read " <<Bytes_Transferred<<" of "<< disk_count<<" bytes at offset "<<disk_offset<<endl;
	}
}
// /*
// void Device :: read_blocks(int64_t address,uint32_t disk_count,uint8_t *data_buffer) throw (invalid_read_exception){
// 	int64_t block_address;
// 	uint8_t *block_buffer;
// 	int64_t length;
// 	unsigned offset;
// 	if (address>fssize_){
// 		throw invalid_read_exception("Requested read beyond file system size.");
// 	}
// 
// 	offset = address & (blocksize_ - 1);
// 	length = (offset + disk_count + blocksize_ - 1) & ~(blocksize_ - 1);
// 
// 	if ((offset == 0) & (length == disk_count)){
// 		read_bytes(address, disk_count, data_buffer);
// 		return;
// 	}
// 
// 	block_address = address - offset;
// 	block_buffer = (uint8_t *) malloc(length);
// 	if (block_buffer == 0)
// 		throw invalid_read_exception("Unable to allocate block buffer");
// 		
// 
// 	read_bytes(block_address, length, block_buffer);
// 
// 	memcpy(data_buffer, block_buffer + offset, disk_count);
// 	free(block_buffer);
// }*/

/*
int Device :: flush(){
	if (fsync(fileno(file_)) == -1)
		return errno;
#ifdef BLKFLSBUF
	return ioctl(fileno(file_), BLKFLSBUF, 0);
#else
	return (0);
#endif
}
*/
















/*
 * NAME: ujfs_get_dev_size
 *
 * FUNCTION: Uses the device driver interface to determine the raw capacity of
 *      the specified device.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      device  - device
 *      size    - filled in with size of device; not modified if failure occurs
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 if successful; anything else indicates failures
 */






int64_t Device :: get_size_blocks() const{
	return get_size_bytes()>>log2blocksize_;
}
int64_t Device :: get_size_bytes() const{
	int64_t size;
	off_t Starting_Position;	/* position within file/device upon
					 * entry to this function. */
	off_t Current_Position = 16777215;	/* position we are attempting
						 * to read from. */
	off_t Last_Valid_Position = 0;	/* Last position we could successfully
					 * read from. */
	off_t First_Invalid_Position;	/* first invalid position we attempted
					 * to read from/seek to. */
	int Seek_Result;	/* value returned by lseek. */
	size_t Read_Result = 0;	/* value returned by read. */
	int rc;
	struct stat64 stat_data;
	int devfd = fileno(file_);


	rc = fstat64(devfd, &stat_data);
#if defined(__linux__)
        if ( S_ISBLK(stat_data.st_mode) ) {
		/* this is a block device */
                int64_t size = 0;
                int ret = ioctl(devfd, BLKGETSIZE, &size);
                if( ret ) {
                        perror("BLKGETSIZE ioctl");
                        return 0;
                }
                return size << 9; /* ioctl returns size in 512byte blocks */
        } else 
		/* This is a regular file.  */
		return (int64_t) ((stat_data.st_size / 1024) * 1024);
#endif /* __linux __ */

#if defined(__DragonFly__)
	{
		struct diskslices dss;
		struct disklabel dl;
		struct diskslice *sliceinfo;
		int slice;
		dev_t dev = stat_data.st_rdev;

		rc = ioctl(devfd, DIOCGSLICEINFO, &dss);
		if (rc < 0)
			return 0; //throw exception

		slice = dkslice(dev);
		sliceinfo = &dss.dss_slices[slice];

		if (sliceinfo) {
			if (slice == WHOLE_DISK_SLICE || slice == 0) {
				size = (int64_t) sliceinfo->ds_size * dss.dss_secsize;
			} else {
				if (sliceinfo->ds_label) {
					rc = ioctl(devfd, DIOCGDINFO, &dl);
					if (!rc) {
						size = (int64_t) dl.d_secperunit * dss.dss_secsize;
					} else {
						return (-1); //throw exception
					}
				}
			}
		} else {
			return (-1); //throw exception
		}

		
		return size;
	}
#endif

	/*
	 * If the ioctl above fails or is undefined, use a binary search to
	 * find the last byte in the partition.  This works because an lseek to
	 * a position within the partition does not return an error while an
	 * lseek to a position beyond the end of the partition does.  Note that
	 * some SCSI drivers may log an 'access beyond end of device' error
	 * message.
	 */

	/* Save the starting position so that we can restore it when we are
	 * done! */
	Starting_Position = ftello64(file_);
	if (Starting_Position < 0)
		return -1;//TODO:throw exception

	/*
	 * Find a position beyond the end of the partition.  We will start by
	 * attempting to seek to and read the 16777216th byte in the partition.
	 * We start here because a JFS partition must be at least this big.  If
	 * it is not, then we can not format it as JFS.
	 */
	do {
		/* Seek to the location we wish to test. */
		Seek_Result = fseeko64(file_, Current_Position, SEEK_SET);
		if (Seek_Result == 0) {
			/* Can we read from this location? */
			Read_Result = fgetc(file_);
			if (Read_Result != EOF) {
				/* The current test position is valid.  Save it
				 * for future reference. */
				Last_Valid_Position = Current_Position;

				/* Lets calculate the next location to test. */
				Current_Position = ((Current_Position + 1) * 2)
						   - 1;

			}
		}
	} while ((Seek_Result == 0) && (Read_Result == 1));

	/*
	 * We have exited the while loop, which means that Current Position is
	 * beyond the end of the partition or is unreadable due to a hardware
	 * problem (bad block).  Since the odds of hitting a bad block are very
	 * low, we will ignore that condition for now.  If time becomes
	 * available, then this issue can be revisited.
	 */

	/* Is the drive greater than 16MB? */
	if (Last_Valid_Position == 0) {
		/*
		 * Determine if drive is readable at all.  If it is, the drive
		 * is too small.  If not, it could be a newly created partion,
		 * so we need to issue a different error message
		 */
		size = 0;	/* Indicates not readable at all */
		Seek_Result = fseeko64(file_, Last_Valid_Position, SEEK_SET);
		if (Seek_Result == 0) {
			/* Can we read from this location? */
			Read_Result = fgetc(file_);
			if (Read_Result != EOF)
				/* non-zero indicates readable, but too small */
				size = 1;
		}
		goto restore;
	}
	/*
	 * The drive is larger than 16MB.  Now we must find out exactly how
	 * large.
	 *
	 * We now have a point within the partition and one beyond it.  The end
	 * of the partition must lie between the two.  We will use a binary
	 * search to find it.
	 */

	/* Setup for the binary search. */
	First_Invalid_Position = Current_Position;
	Current_Position = Last_Valid_Position +
			   ((Current_Position - Last_Valid_Position) / 2);

	/*
	 * Iterate until the difference between the last valid position and the
	 * first invalid position is 2 or less.
	 */
	while ((First_Invalid_Position - Last_Valid_Position) > 2) {
		/* Seek to the location we wish to test. */
		Seek_Result = fseeko64(file_, Current_Position, SEEK_SET);
		if (Seek_Result == 0) {
			/* Can we read from this location? */
			Read_Result = fgetc(file_);
			if (Read_Result != EOF) {
				/* The current test position is valid.
				 * Save it for future reference. */
				Last_Valid_Position = Current_Position;

				/*
				 * Lets calculate the next location to test. It
				 * should be half way between the current test
				 * position and the first invalid position that
				 * we know of.
				 */
				Current_Position = Current_Position +
						   ((First_Invalid_Position -
						     Last_Valid_Position) / 2);

			}
		} else
			Read_Result = 0;

		if (Read_Result != 1) {
			/* Out test position is beyond the end of the partition.
			 * It becomes our first known invalid position. */
			First_Invalid_Position = Current_Position;

			/* Our new test position should be half way between our
			 * last known valid position and our current test
			 * position. */
			Current_Position =
			    Last_Valid_Position +
			    ((Current_Position - Last_Valid_Position) / 2);
		}
	}

	/*
	 * The size of the drive should be Last_Valid_Position + 1 as
	 * Last_Valid_Position is an offset from the beginning of the partition.
	 */
	size = Last_Valid_Position + 1;


restore:
	/* Restore the original position. */
	if (fseeko64(file_, Starting_Position, SEEK_SET) != 0)
		return -1;//TODO:throw exception

	return size;
}









/*
int xRead(int64_t address, unsigned count, char *buffer)
{
	int64_t block_address;
	char *block_buffer;
	int64_t length;
	unsigned offset;

	offset = address & (bsize - 1);
	length = (offset + count + bsize - 1) & ~(bsize - 1);

	if ((offset == 0) & (length == count))
		return ujfs_rw_diskblocks(fp, address, count, buffer, GET);

	block_address = address - offset;
	block_buffer = (char *) malloc(length);
	if (block_buffer == 0)
		return 1;

	if (ujfs_rw_diskblocks(fp, block_address, length, block_buffer, GET)) {
		free(block_buffer);
		return 1;
	}
	memcpy(buffer, block_buffer + offset, count);
	free(block_buffer);
	return 0;
}*/
/*
int xWrite(int64_t address, unsigned count, char *buffer)
{
	int64_t block_address;
	char *block_buffer;
	int64_t length;
	unsigned offset;

	offset = address & (bsize - 1);
	length = (offset + count + bsize - 1) & ~(bsize - 1);

	if ((offset == 0) & (length == count))
		return ujfs_rw_diskblocks(fp, address, count, buffer, PUT);

	block_address = address - offset;
	block_buffer = (char *) malloc(length);
	if (block_buffer == 0)
		return 1;

	if (ujfs_rw_diskblocks(fp, block_address, length, block_buffer, GET)) {
		free(block_buffer);
		return 1;
	}
	memcpy(block_buffer + offset, buffer, count);
	if (ujfs_rw_diskblocks(fp, block_address, length, block_buffer, PUT)) {
		free(block_buffer);
		return 1;
	}
	free(block_buffer);
	return 0;
}*/
