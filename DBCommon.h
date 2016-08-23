/* 
 * File:   DBCommon.h
 * Author: datbt
 *
 * Created on August 22, 2016, 2:32 PM
 */

#ifndef DBCOMMON_H
#define	DBCOMMON_H

#include <vector>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string>
#include <map>
#include <unistd.h>
#include <stddef.h>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <Poco/File.h>
#include <Poco/Mutex.h>
#include <Poco/RWLock.h>
#include <Poco/Event.h>
#include <Poco/Timestamp.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "DBConfig.h"

using namespace std;
using namespace boost;


static int64_t getCurrentTimeSec(){
    return time(NULL);
}

static int64_t getCurrentTimeMicroSec(){
    struct timeval now;
    gettimeofday (&now, NULL);
    return  (now.tv_usec + now.tv_sec * 1000000);  
}

static int64_t getCurrentTimeNano(){
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return (now.tv_nsec + now.tv_sec * 1000000000);
}

typedef shared_ptr<Poco::Mutex> MutexPtr;
typedef shared_ptr<Poco::RWLock> RWLockPtr;

typedef struct {
    enum Code {
        SUCCESS,
        NOT_EXIST,
        FAIL,
        ERROR
    };
} Err;


typedef struct {
    uint64_t currentOffset;
    uint8_t numberOfDataFile;
    uint64_t numberOfColRec;
    uint64_t numberOfFreeSlot;
} DatabaseMeta;

typedef struct {
    string key;
    string value;
    uint64_t nxtColRecOffset;
} Record;

#endif	/* DBCOMMON_H */

