/* 
 * File:   Database.cpp
 * Author: datbt
 * 
 * Created on August 24, 2016, 10:37 AM
 */

#include "Database.h"

Database::Database(const string& name) : _name(name), _listRWLock(SLOT_LOCK_NUM) {
	_initialize(name);
	_storage = new Storage(name, _meta);
	_index = new Index(name, _storage);
}

Database::~Database() {
}

void Database::_initialize(const string& name) {
	//init lock
	_listRWLock.clear();
	for (int16_t i=0; i < SLOT_LOCK_NUM; i++) {		
		RWLockPtr rwLock(new Poco::RWLock());
		_listRWLock.push_back(rwLock);
	}
	
	//init data directory
	string dir_path = "/data/mdb/" + name;
	Poco::File dataDir(dir_path);
	if (!dataDir.exists()) {
		dataDir.createDirectories();
	}
	
	//init meta
	_initMeta(name);

	//init list empty record offset
	_initListEmptyRecord(name);
}

void Database::_initMeta(const string& dbName) {
	string metaPath = DATA_DIRECTORY + dbName + "/meta.dat";
	size_t metaSize = sizeof (uint64_t) * 3 + sizeof (uint8_t);
	
	_meta = (DatabaseMeta*) initMMapData(metaPath, metaSize);	
	
	if (_meta->currentOffset < 1) {
		_meta->currentOffset = 1;
	}
	if (_meta->numberOfFreeSlot < 0) {
		_meta->numberOfFreeSlot = 0;
	}
	if (_meta->numberOfRec < 0) {
		_meta->numberOfRec = 0;
	}
	if (_meta->numberOfDataFile < 0) {
		_meta->numberOfDataFile = 0;
	}
}

void Database::_initListEmptyRecord(const string& name) {
	string listEmptyRecPath = DATA_DIRECTORY + name + "/emptyrecords.dat";
	_listEmptyRecordOffset = (uint64_t*) initMMapData(listEmptyRecPath, REUSE_SLOT_SIZE);
}

Err::Code Database::_getRecord(const string& key, Record& ret) {
	uint64_t recordOffset = _index->getRecordOffset(key);

	if (recordOffset < 1) {
		return Err::NOT_EXIST;
	}
	while (recordOffset > 0) {
		Err::Code err = _storage->readRecord(recordOffset, ret);
		if (err != Err::SUCCESS) {
			return err;
		}
		if (ret.key == key) {
			return Err::SUCCESS;
		}
		recordOffset = ret.nxtColRecOffset;
	}
	return Err::NOT_EXIST;
}

uint64_t Database::_getRecordOffset(const string& key) {
	uint64_t recordOffset = _index->getRecordOffset(key);
	Record ret;
	if (recordOffset < 1) {
		return 0;
	}
	while (recordOffset > 0) {
		Err::Code err = _storage->readRecord(recordOffset, ret);
		if (err != Err::SUCCESS) {
			return 0;
		}
		if (ret.key == key) {
			return recordOffset;
		}
		recordOffset = ret.nxtColRecOffset;
	}
	return 0;
}

Err::Code Database::get(const string& key, string& value) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedReadRWLock readLock(*rwlock); //read lock
	
	Record rec;
	Err::Code err = _getRecord(key, rec);
	if (err == Err::SUCCESS) {
		value = rec.value;
	}
	return err;
}

Err::Code Database::exist(const string& key) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedReadRWLock readLock(*rwlock); //read lock

	Record rec;
	Err::Code err = _getRecord(key, rec);
	return err;
}

Err::Code Database::put(const string& key, const string& value) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedWriteRWLock writeLock(*rwlock); //write lock	
	
	uint64_t recOff = _getRecordOffset(key);
	Err::Code err;
	if (recOff > 0) { //offset > 0 mean we are overwrite an exist record
		err = _storage->writeRecord(key, value, recOff); //only overwrite value
		return err;
	}
	
	//put new record
	Record rec;
	rec.key = key;
	rec.value = value;
	rec.nxtColRecOffset = 0;
	
	uint64_t offsetToWrite;
	bool reuseSlot = false;
	if (_meta->numberOfFreeSlot > 0) {//reuse empty slot if possible
		offsetToWrite = _listEmptyRecordOffset[_meta->numberOfFreeSlot - 1];
		reuseSlot = true;
	} else {
		offsetToWrite = _meta->currentOffset;
	}
	
	err = _storage->writeRecord(rec, offsetToWrite); //write record to disk
	if (err != Err::SUCCESS){
		return err;
	}
	
	err = _index->addRecord(key, offsetToWrite); //add record to index
	
	if (err != Err::SUCCESS){
		return err;
	}
	
	//update meta
	if (reuseSlot) {
		_meta->numberOfFreeSlot--;
	} else {
		_meta->currentOffset += RECORD_MAX_SIZE;
	}
	_meta->numberOfRec++;
	_storage->checkDataSize();
	
	return err;
}

Err::Code Database::remove(const string& key) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedWriteRWLock writeLock(*rwlock); //write lock
	
	uint64_t recOff = _getRecordOffset(key);

	if (recOff < 1) {
		return Err::NOT_EXIST;
	}

	Err::Code err = _index->removeRecord(key, recOff);
	if (err == Err::SUCCESS) {
		_listEmptyRecordOffset[_meta->numberOfFreeSlot] = recOff; // add removed record to list free slot for reusing
		_meta->numberOfFreeSlot++; //increase free slot
		_meta->numberOfRec--; //decrease num of record
	}
	return err;
}
