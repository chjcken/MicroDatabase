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
    Index(string& dbName, Storage* storage);
    virtual ~Index();
    
    Err::Code addRecord(string& key, uint64_t offset);
    Err::Code removeRecord(string& key);    
    Err::Code getBucket(string& key, Bucket& ret);
    uint64_t getRecordOffset(string& key);
    uint64_t getSize();
    
    bool isBucketCollision(Bucket& bucket);
    bool isBucketEmpty(Bucket& bucket);
    
private:
    Bucket* __bucketTable;
    Storage* _storage;
    RWLockPtr _bucketTableLock;
    vector<RWLockPtr> *_listRWLock;
    
    void _initialize();
    uint64_t _hash(string& key);
    uint64_t _getBucketIndex(string& key);
    Err::Code _getBucket(uint64_t bucketIndex, Bucket& ret);
    
    void _clearBucket(Bucket& bucket);
    Err::Code _addRecordToBucket(Bucket& bucket, uint64_t recOffset);
};

#endif	/* INDEX_H */

