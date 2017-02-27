/* 
 * File:   DBConfig.h
 * Author: datbt
 *
 * Created on August 22, 2016, 2:46 PM
 */

#ifndef DBCONFIG_H
#define	DBCONFIG_H
#include <stdint.h>

#include "Poco/AutoPtr.h"
#include "DBCommon.h"
#include <Poco/Util/PropertyFileConfiguration.h>

#define _1GB                    1073741824
#define _8GB                    8589934592

#define CONFIG_FILE_NAME        "conf/app.conf"

#define INDEX_SIZE              10000 //
#define RECORD_MAX_SIZE         10240 // 10kb

#define STORAGE_FILE_SIZE       _8GB

#define SLOT_LOCK_NUM           1000

#define DATA_DIRECTORY          "/data/mdb/"

#define INDEX_FILE_SIZE         INDEX_SIZE * RECORD_MAX_SIZE

#define REUSE_SLOT_SIZE         1000

static Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _pConf = new Poco::Util::PropertyFileConfiguration(CONFIG_FILE_NAME);

class DBConfig {
public:
    static DBConfig* instance() {
        static DBConfig inst;
        return &inst;
    }
    
    const std::string db_name;
    const uint64_t index_size;
    const uint32_t record_max_size;
    const uint32_t reuse_slot_size;    
    const uint32_t slot_lock_num;
    const uint64_t index_file_size;
    const uint32_t io_threads;
    const uint32_t nworkers;
    const uint32_t port;
    
private:
    //Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _pConf;
    
    DBConfig(): 
    db_name(_pConf->getString("db_name", "testdb")),
    index_size(_pConf->getInt("index_size", INDEX_SIZE)),
    record_max_size(_pConf->getInt("record_max_size", RECORD_MAX_SIZE)),
    reuse_slot_size(_pConf->getInt("reuse_slot_size", REUSE_SLOT_SIZE)),
    slot_lock_num(_pConf->getInt("slot_lock_num", SLOT_LOCK_NUM)),
    index_file_size(index_size * record_max_size),
    io_threads(_pConf->getInt("io_threads", 0)),
    nworkers(_pConf->getInt("nworkers", 0)),
    port(_pConf->getInt("port", 9876))
    {      
    }
};

#endif	/* DBCONFIG_H */

