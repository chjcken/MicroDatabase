/* 
 * File:   Storage.h
 * Author: datbt
 *
 * Created on August 22, 2016, 2:32 PM
 */

#ifndef STORAGE_H
#define	STORAGE_H

#include "DBCommon.h"
#include "IOControl.h"
#include <string.h>
#include <fstream>

typedef shared_ptr<fstream> FStreamPtr;
typedef struct {
    FStreamPtr fsPtr;
    MutexPtr mutexPtr;
} FileDescriptor;

class Storage {
public:
    Storage(const string& name, DatabaseMeta* metaInfo);
    ~Storage();
        
    Error::Code readRecord(uint64_t offset, Record &ret, bool ioCtrl = false);
    uint64_t readNxtColRecOff(uint64_t recOff);
    Error::Code writeRecord(Record& rec, uint64_t offset, bool ioCtrl = false);
    Error::Code writeNxtColRecOff(uint64_t nextColOff, uint64_t recOff);
    Error::Code writeRecord(const string& key, const string& value, uint64_t offset, bool ioCtrl = false);
    
    void checkDataSize();
private:
    string _dbname;
    DatabaseMeta* _metaInfo;
    vector<FileDescriptor> _listFileDescriptor;
    MutexPtr _listFDMutex;
    
    bool _warmUp();
    bool _createNewDataFile(uint32_t fd_num);
    bool __createNewFile(string& file_name, uint64_t file_size);    
    uint64_t _getFileDescriptor(uint64_t offset, FileDescriptor& retFD);
    
    uint64_t _writeData(FileDescriptor& fs, uint64_t offset, const void* data, uint32_t dataSize);
    uint64_t _readData(FileDescriptor& fs, uint64_t offset, const void* data, uint32_t dataSize);
    
    IOControl _ioCtl;
};

#endif	/* STORAGE_H */

