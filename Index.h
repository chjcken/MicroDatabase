/* 
 * File:   Index.h
 * Author: datbt
 *
 * Created on August 22, 2016, 5:15 PM
 */

#ifndef INDEX_H
#define	INDEX_H

#include "DBCommon.h"
#include "Storage.h"

typedef struct Bucket {
    uint64_t recordOffset;
    uint32_t lastColRecOffset;
} Bucket;

class Index {
public:
    Index(const string& dbName, Storage* storage);
    virtual ~Index();
    
    Err::Code addRecord(const string& key, uint64_t offset);
    Err::Code removeRecord(const string& key, uint64_t recOffset);    
    Err::Code getBucket(const string& key, Bucket** ret);
    uint64_t getRecordOffset(const string& key);
    uint64_t getSize();
    
    bool isBucketCollision(Bucket& bucket);
    bool isBucketEmpty(Bucket& bucket);
    
private:
    Bucket* __bucketTable;
    Storage* _storage;
    RWLockPtr _bucketTableLock;
    vector<RWLockPtr> *_listRWLock;
    
    void _initialize(const string& dbName);
    uint64_t _getBucketIndex(const string& key);
    Err::Code _getBucket(uint64_t bucketIndex, Bucket& ret);
    
    void _clearBucket(Bucket& bucket);
    Err::Code _addRecordToBucket(Bucket& bucket, uint64_t recOffset);
};

#endif	/* INDEX_H */

