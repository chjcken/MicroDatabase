/* 
 * File:   Index.cpp
 * Author: datbt
 * 
 * Created on August 22, 2016, 5:15 PM
 */

#include "Index.h"

Index::Index(const string& dbName, Storage* storage) : _storage(storage) {
	_initialize(dbName);
}

Index::~Index() {
}

void Index::_initialize(const string& dbName) {
	string indexPath = DATA_DIRECTORY + dbName + "/index.dat";
	__bucketTable = (Bucket*) initMMapData(indexPath, INDEX_FILE_SIZE);
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

Err::Code Index::getBucket(const string& key, Bucket** ret) {
	//Poco::ScopedReadRWLock readLock(*_bucketTableLock);
	uint64_t _hash = hashF(key);
	uint64_t bucketIndex = _hash % INDEX_SIZE;
	*ret = &__bucketTable[bucketIndex];

	return Err::SUCCESS;
}

uint64_t Index::getRecordOffset(const string& key) {
	Bucket * b;
	getBucket(key, &b);
	return b->recordOffset;
}

Err::Code Index::addRecord(const string& key, uint64_t offset) {
	Bucket * buck;
	Err::Code err = getBucket(key, &buck);
	if (err != Err::SUCCESS) {
		return err;
	}
	return _addRecordToBucket(*buck, offset);
}

Err::Code Index::_addRecordToBucket(Bucket& bucket, uint64_t recOffset) {
	if (isBucketEmpty(bucket)) { // if bucket is empty, just add record to bucket and done
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
	Err::Code err;

	err = _storage->writeNxtColRecOff(recOffset, __recOff);

	if (err == Err::SUCCESS) {
		bucket.lastColRecOffset = recOffset;
	}
	return err;
}

void Index::_clearBucket(Bucket& bucket) {
	bucket.recordOffset = 0;
	bucket.lastColRecOffset = 0;
}

Err::Code Index::removeRecord(const string& key, uint64_t recOffset) {
	Bucket * buck;
	Err::Code err = getBucket(key, &buck);
	if (err != Err::SUCCESS) {
		return err;
	}
	if (isBucketEmpty(*buck)) {
		return Err::SUCCESS;
	}
	if (!isBucketCollision(*buck)) {
		if (buck->recordOffset == recOffset) {
			_clearBucket(*buck);
			return Err::SUCCESS;
		} else {
			return Err::NOT_EXIST;
		}
	}

	Record rec;
	int64_t recOff = buck->recordOffset;

	if (recOff == recOffset) { //record to be removed is in bucket
		uint64_t nxtColRecOff = _storage->readNxtColRecOff(recOff);
		if (nxtColRecOff < 1) {
			return Err::FAIL;
		}
		buck->recordOffset = nxtColRecOff; //push next record up to bucket
		return Err::SUCCESS;
	}

	while (recOff >= 0) {		
		err = _storage->readRecord(recOff, rec);
		uint64_t nxtColRecOff = _storage->readNxtColRecOff(recOff);
		if (nxtColRecOff < 1) {
			return Err::FAIL;
		}
		if (nxtColRecOff == recOffset) { //next record is our target
			uint64_t nxtColRecOffOfRmv = _storage->readNxtColRecOff(recOffset); //read next record of removed record
			if (nxtColRecOffOfRmv < 1) {
				return Err::FAIL;
			}
			err = _storage->writeNxtColRecOff(nxtColRecOffOfRmv, recOff); //write next record of removed record to current record
			return err;
		}
		recOff = nxtColRecOff;
	}

	return Err::NOT_EXIST;
}

