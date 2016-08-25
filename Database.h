/* 
 * File:   Database.h
 * Author: datbt
 *
 * Created on August 24, 2016, 10:37 AM
 */

#ifndef DATABASE_H
#define	DATABASE_H

#include "DBCommon.h"
#include "Index.h"

class Database {
public:
    Database(const string& name);
    virtual ~Database();
    
    Err::Code get(const string& key, string& value);
    Err::Code put(const string& key, const string& value);
    Err::Code exist(const string& key);
    Err::Code remove(const string& key);
private:
    string _name;
    Index* _index;
    Storage* _storage;
    DatabaseMeta* _meta;
    uint64_t* _listEmptyRecordOffset;
    vector<RWLockPtr> _listRWLock;
    
    void _initialize(const string& name);
    void _initMeta(const string& name);
    void _initListEmptyRecord(const string& name);
    
    Err::Code _getRecord(const string& key, Record& ret);
    uint64_t _getRecordOffset(const string& key);
};

#endif	/* DATABASE_H */

