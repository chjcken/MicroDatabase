/* 
 * File:   Storage.cpp
 * Author: datbt
 * 
 * Created on August 22, 2016, 2:32 PM
 */

#include "Storage.h"
#include <stdlib.h>

Storage::Storage(const string& name, DatabaseMeta* metaInfo) : _dbname(name), _listFileDescriptor(16), _listFDMutex(new Poco::Mutex()), _metaInfo(metaInfo) {
	bool isWarmUpSuccess = _warmUp();
	if (!isWarmUpSuccess) {
		printf("warm up fail\n");
		exit(1);
	}
	uint64_t limit = 1024 * 1024 * 40;
	_ioCtl.init(limit, limit);
}

Storage::~Storage() {
}

Error::Code Storage::writeRecord(Record& rec, uint64_t offset, bool ioCtrl) {
	FileDescriptor fd;
	uint64_t _offset = _getFileDescriptor(offset, fd);
	
	size_t keySize = rec.key.size();
	size_t valueSize = rec.value.size();
	
	_offset = _writeData(fd, _offset, &keySize, sizeof (keySize));
	_offset = _writeData(fd, _offset, &valueSize, sizeof (valueSize));
	_offset = _writeData(fd, _offset, &rec.nxtColRecOffset, sizeof (rec.nxtColRecOffset));
	_offset = _writeData(fd, _offset, rec.key.c_str(), keySize);
	_offset = _writeData(fd, _offset, rec.value.c_str(), valueSize);
	if (ioCtrl) {
		size_t recSize = 0;
		recSize += sizeof (keySize);
		recSize += keySize;
		recSize += sizeof (valueSize);
		recSize += valueSize;
		recSize += sizeof (rec.nxtColRecOffset);
		_ioCtl.onWrite(recSize);
	}

	return Error::SUCCESS;
}

Error::Code Storage::writeNxtColRecOff(uint64_t nextColOff, uint64_t recOff) {
	FileDescriptor fd;
	uint64_t _offset = _getFileDescriptor(recOff, fd);
	
	_offset += sizeof (size_t);
	_offset += sizeof (size_t);
	
	_writeData(fd, _offset, &nextColOff, sizeof (nextColOff));
	
	return Error::SUCCESS;
}

Error::Code Storage::writeRecord(const string& key, const string& value, uint64_t offset, bool ioCtrl) {
	FileDescriptor fd;
	uint64_t _offset = _getFileDescriptor(offset, fd);
	
	size_t keySize = key.size();
	size_t valueSize = value.size();
	
	_offset += sizeof (keySize);
	_offset = _writeData(fd, _offset, &valueSize, sizeof (valueSize));
	_offset += sizeof (uint64_t);
	_offset += keySize;
	_offset = _writeData(fd, _offset, value.c_str(), valueSize);
	if (ioCtrl) {
		size_t recSize = 0;
		recSize += sizeof (valueSize);
		recSize += valueSize;
		_ioCtl.onWrite(recSize);
	}

	return Error::SUCCESS;
}


Error::Code Storage::readRecord(uint64_t offset, Record& ret, bool ioCtrl) {
	FileDescriptor fd;
	uint64_t _offset = _getFileDescriptor(offset, fd);
	
	size_t keySize = 0;
	size_t valueSize = 0;

	_offset = _readData(fd, _offset, &keySize, sizeof (keySize));
	_offset = _readData(fd, _offset, &valueSize, sizeof (valueSize));
	_offset = _readData(fd, _offset, &ret.nxtColRecOffset, sizeof (ret.nxtColRecOffset));
	ret.key.resize(keySize);
	ret.value.resize(valueSize);
	_offset = _readData(fd, _offset, (char*) ret.key.data(), keySize);
	_offset = _readData(fd, _offset, (char*) ret.value.data(), valueSize);

	if (ioCtrl) {
		size_t recSize = 0;
		recSize += sizeof (keySize);
		recSize += sizeof (ret.nxtColRecOffset);
		recSize += sizeof (valueSize);
		recSize += valueSize;
		recSize += keySize;
		_ioCtl.onRead(recSize);
	}

	return Error::SUCCESS;
}

uint64_t Storage::readNxtColRecOff(uint64_t recOff) {
	FileDescriptor fd;
	uint64_t _offset = _getFileDescriptor(recOff, fd);
	
	_offset += sizeof (size_t); //jump to key size
	_offset += sizeof (size_t);// jump to value size
	
	uint64_t ret;
	_readData(fd, _offset, &ret, sizeof (ret));
	return ret;
}

bool Storage::_warmUp() {
	_listFileDescriptor.clear();
	if (_metaInfo->numberOfDataFile > 0) {
		for (int i = 0; i < _metaInfo->numberOfDataFile; ++i) {
			string data_file_name = DATA_DIRECTORY + _dbname + "/data_" + lexical_cast<string>(i);
			Poco::File data_file(data_file_name);
			if (!data_file.exists()) {
				printf("data file not found: %s", data_file_name.c_str());
				return false;
			}
			Poco::Mutex::ScopedLock lock(*_listFDMutex);
			FStreamPtr fs(new fstream(data_file_name.c_str(), ios::in | ios::out | ios::binary));
			MutexPtr mtx(new Poco::Mutex());
			FileDescriptor fd;
			fd.fsPtr = fs;
			fd.mutexPtr = mtx;
			_listFileDescriptor.push_back(fd);
		}
		return true;
	} else {
		bool isSuccess = _createNewDataFile(0);
		if (isSuccess) {
			_metaInfo->numberOfDataFile = 1;
		}
		return isSuccess;
	}
}

bool Storage::__createNewFile(string& file_name, uint64_t file_size) {
	ofstream outfile(file_name.c_str(), ios::out | ios::binary);
	int32_t errorCode = truncate(file_name.c_str(), file_size);
	outfile.close();

	return errorCode == 0;
}

bool Storage::_createNewDataFile(uint32_t fd_num) {
	string dir_path = "/data/mdb/" + _dbname;
	Poco::File dataDir(dir_path);
	if (!dataDir.exists()) {
		dataDir.createDirectories();
	}
	string data_file_name = dir_path + "/data_" + lexical_cast<string>(fd_num);
	Poco::File data_file(data_file_name);
	if (data_file.exists()) {
		return false;
	}

	bool success = __createNewFile(data_file_name, STORAGE_FILE_SIZE);
	if (success) {
		Poco::Mutex::ScopedLock lock(*_listFDMutex);
		FStreamPtr fs(new fstream(data_file_name.c_str(), ios::in | ios::out | ios::binary));
		MutexPtr mtx(new Poco::Mutex());
		FileDescriptor fd;
		fd.fsPtr = fs;
		fd.mutexPtr = mtx;
		_listFileDescriptor.push_back(fd);
		_metaInfo->numberOfDataFile = fd_num + 1;
	}
	return success;
}

uint64_t Storage::_getFileDescriptor(uint64_t offset, FileDescriptor& retFD) {
	Poco::Mutex::ScopedLock lock(*_listFDMutex);
	uint32_t i = offset / STORAGE_FILE_SIZE;
	uint64_t j = offset - (i * STORAGE_FILE_SIZE);
	retFD = _listFileDescriptor.at(i);
	return j;
}

uint64_t Storage::_writeData(FileDescriptor& fd, uint64_t offset, const void* data, uint32_t dataSize) {
	Poco::Mutex::ScopedLock lock(*(fd.mutexPtr));
	fd.fsPtr->seekp(offset);
	fd.fsPtr->write((char*) data, dataSize);
	fd.fsPtr->flush();
	return (offset + dataSize);
}

uint64_t Storage::_readData(FileDescriptor& fd, uint64_t offset, const void* data, uint32_t dataSize) {
	Poco::Mutex::ScopedLock lock(*(fd.mutexPtr));
	fd.fsPtr->seekp(offset);
	fd.fsPtr->read((char*) data, dataSize);
	return (offset + dataSize);
}

void Storage::checkDataSize() {
	uint32_t fd_num = _metaInfo->numberOfDataFile;
	//	{
	//		Poco::Mutex::ScopedLock lock(*_listFDMutex);
	//		fd_num = _listFileDescriptor.size();
	//	}

	uint64_t bound = fd_num * STORAGE_FILE_SIZE - DBConfig::instance()->record_max_size;
	if (_metaInfo->currentOffset >= bound) {
		bool stt = _createNewDataFile(fd_num);
		if (!stt) {
			printf("new file err\n");
			exit(1);
		}
	}
}

