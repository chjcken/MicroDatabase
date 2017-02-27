/* 
 * File:   Database.cpp
 * Author: datbt
 * 
 * Created on August 24, 2016, 10:37 AM
 */

#include <Poco/Util/Application.h>

#include "DatabaseModel.h"

DatabaseModel::DatabaseModel(const string& name) : _name(name), _listRWLock(DBConfig::instance()->slot_lock_num) {
	_initialize(name);
	_storage = new Storage(name, _meta);
	_index = new Index(name, _storage);
}

DatabaseModel::~DatabaseModel() {
}

void DatabaseModel::_initialize(const string& name) {
	//init lock
	_listRWLock.clear();
	for (int16_t i=0; i < DBConfig::instance()->slot_lock_num; i++) {		
		RWLockPtr rwLock(new Poco::RWLock());
		_listRWLock.push_back(rwLock);
	}
	
	//init data directory
	string dir_path = DATA_DIRECTORY + name;
	Poco::File dataDir(dir_path);
	if (!dataDir.exists()) {
		dataDir.createDirectories();
	}
	
	//init meta
	_initMeta(name);

	//init list empty record offset
	_initListEmptyRecord(name);
}

void DatabaseModel::_initMeta(const string& dbName) {
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

void DatabaseModel::_initListEmptyRecord(const string& name) {
	string listEmptyRecPath = DATA_DIRECTORY + name + "/emptyrecords.dat";
	_listEmptyRecordOffset = (uint64_t*) initMMapData(listEmptyRecPath, DBConfig::instance()->reuse_slot_size);
}

bool DatabaseModel::_isPutDataValid(const string& key, const string& value) {
	size_t recSize = key.size() + value.size() + sizeof(size_t) * 2 + sizeof (uint64_t);	
	return recSize <= DBConfig::instance()->record_max_size;
}


Error::Code DatabaseModel::_getRecord(const string& key, Record& ret) {
	uint64_t recordOffset = _index->getRecordOffset(key);

	if (recordOffset < 1) {
		return Error::NOT_EXIST;
	}
	while (recordOffset > 0) {
		Error::Code err = _storage->readRecord(recordOffset, ret);
		if (err != Error::SUCCESS) {
			return err;
		}
		if (ret.key == key) {
			return Error::SUCCESS;
		}
		recordOffset = ret.nxtColRecOffset;
	}
	return Error::NOT_EXIST;
}

uint64_t DatabaseModel::_getRecordOffset(const string& key) {
	uint64_t recordOffset = _index->getRecordOffset(key);
	Record ret;
	if (recordOffset < 1) {
		return 0;
	}
	while (recordOffset > 0) {
		Error::Code err = _storage->readRecord(recordOffset, ret);
		if (err != Error::SUCCESS) {
			return 0;
		}
		if (ret.key == key) {
			return recordOffset;
		}
		recordOffset = ret.nxtColRecOffset;
	}
	return 0;
}

Error::Code DatabaseModel::get(const string& key, string& value) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedReadRWLock readLock(*rwlock); //read lock
	
	Record rec;
	Error::Code err = _getRecord(key, rec);
	if (err == Error::SUCCESS) {
		value = rec.value;
	}
	return err;
}

Error::Code DatabaseModel::exist(const string& key) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedReadRWLock readLock(*rwlock); //read lock

	Record rec;
	Error::Code err = _getRecord(key, rec);
	return err;
}

Error::Code DatabaseModel::put(const string& key, const string& value) {
	if (_isPutDataValid(key, value)) {
		return Error::PARAM_OVERSIZE;
	}
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedWriteRWLock writeLock(*rwlock); //write lock	
	
	uint64_t recOff = _getRecordOffset(key);
	Error::Code err;
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
	if (err != Error::SUCCESS){
		return err;
	}
	
	err = _index->addRecord(key, offsetToWrite); //add record to index
	
	if (err != Error::SUCCESS){
		return err;
	}
	
	//update meta
	if (reuseSlot) {
		_meta->numberOfFreeSlot--;
	} else {
		_meta->currentOffset += DBConfig::instance()->record_max_size;
	}
	_meta->numberOfRec++;
	_storage->checkDataSize();
	
	return err;
}

Error::Code DatabaseModel::remove(const string& key) {
	RWLockPtr rwlock = _listRWLock.at((hashF(key) % _listRWLock.size()));
	Poco::ScopedWriteRWLock writeLock(*rwlock); //write lock
	
	uint64_t recOff = _getRecordOffset(key);

	if (recOff < 1) {
		return Error::NOT_EXIST;
	}

	Error::Code err = _index->removeRecord(key, recOff);
	if (err == Error::SUCCESS) {
		_listEmptyRecordOffset[_meta->numberOfFreeSlot] = recOff; // add removed record to list free slot for reusing
		_meta->numberOfFreeSlot++; //increase free slot
		_meta->numberOfRec--; //decrease num of record
	}
	return err;
}
