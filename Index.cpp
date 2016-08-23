/* 
 * File:   Index.cpp
 * Author: datbt
 * 
 * Created on August 22, 2016, 5:15 PM
 */

#include "Index.h"

Index::Index(string& dbName, Storage* storage): _storage(storage) {	
	_initialize(dbName);
}

Index::~Index() {
}

void Index::_initialize(string& dbName) {
	int fd;
	int result;

	/* Open a file for writing.
	 *  - Creating the file if it doesn't exist.
	 *  - Truncating it to 0 size if it already exists. (not really needed)
	 *
	 * Note: "O_WRONLY" mode is not sufficient when mmaping.
	 */
	string indexPath = DATA_DIRECTORY + dbName + "/index.dat";
	fd = open(indexPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
	if (fd == -1) {
		printf("Error opening file for writing");
		exit(EXIT_FAILURE);
	}

	/* Stretch the file size to the size of the (mmapped) array of ints
	 */
	result = lseek(fd, INDEX_FILE_SIZE - 1, SEEK_SET);
	if (result == -1) {
		close(fd);
		printf("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
	}

	/* Something needs to be written at the end of the file to
	 * have the file actually have the new size.
	 * Just writing an empty string at the current file position will do.
	 *
	 * Note:
	 *  - The current position in the file is at the end of the stretched 
	 *    file due to the call to lseek().
	 *  - An empty string is actually a single '\0' character, so a zero-byte
	 *    will be written at the last byte of the file.
	 */
	result = write(fd, "", 1);
	if (result != 1) {
		close(fd);
		printf("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}

	/* Now the file is ready to be mmapped.
	 */
	__bucketTable = (Bucket*) mmap(0, INDEX_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (__bucketTable == MAP_FAILED) {
		close(fd);
		printf("Error mmapping the file");
		exit(EXIT_FAILURE);
	}
}

uint64_t Index::getSize() {
	return INDEX_SIZE;
}

bool Index::isBucketEmpty(Bucket& bucket) {
	return bucket.recordOffset < 1;
}

bool Index::isBucketCollision(Bucket& bucket) {
	return bucket.lastColRecOffset < 1;
}

uint64_t Index::_hash(string& key) {	
    uint64_t h = 0;
	size_t keySize = key.size();
    if (keySize > 0) {
        const char* val = key.c_str();

        for (int i = 0; i < keySize; i++) {
            h = 31 * h + val[i];
        }
    }
    return h;
}

Err::Code Index::getBucket(string& key, Bucket& ret) {
	uint64_t hash = _hash(key);
	uint64_t bucketIndex = hash % INDEX_SIZE;
	ret = __bucketTable[bucketIndex];
	
	return Err::SUCCESS;
}


Err::Code Index::addRecord(string& key, uint64_t offset) {
	Bucket buck;
	Err::Code err = getBucket(key, buck);
	if (err != Err::SUCCESS) {
		return err;
	}
	return _addRecordToBucket(buck, offset);
}

Err::Code Index::_addRecordToBucket(Bucket& bucket, uint64_t recOffset) {
	if (isBucketEmpty(bucket)) {
		bucket.recordOffset = recOffset;
		return Err::SUCCESS;
	}
	//add record to collision chain
	uint64_t __recOff;
	if (!isBucketCollision(bucket)) {
		__recOff = bucket.recordOffset;
	} else {
		__recOff = bucket.lastColRecOffset;
	}
	Record rec;
	Err::Code err = _storage->readRecord(__recOff, rec);
	if (err != Err::SUCCESS || rec.nxtColRecOffset < 1) {
		return err;
	}
	rec.nxtColRecOffset = recOffset;
	err = _storage->writeRecord(rec, __recOff);
	if (err == Err::SUCCESS) {
		bucket.lastColRecOffset = recOffset;
	}
	return err;
}


