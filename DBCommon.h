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

static int64_t getCurrentTimeSec() {
    return time(NULL);
}

static int64_t getCurrentTimeMicroSec() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_usec + now.tv_sec * 1000000);
}

static int64_t getCurrentTimeNano() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return (now.tv_nsec + now.tv_sec * 1000000000);
}

static uint64_t hashF(const string& key) {
    uint64_t h = 0;
    size_t keySize = key.size();
    if (keySize > 0) {
        const char* val = key.c_str();

        for (int i = 0; i < keySize; i++) {
            h = 31 * h + val[i];
        }
    }
    return h;
}

static void* initMMapData(const string& path, size_t size) {
    int fd;
    int result;
    void* data;
    Poco::File file(path);
    if (!file.exists()) {
        /* Open a file for writing.
         *  - Creating the file if it doesn't exist.
         *  - Truncating it to 0 size
         *
         * Note: "O_WRONLY" mode is not sufficient when mmaping.
         */
        fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
        if (fd == -1) {
            printf("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        /* Stretch the file size to the size of the (mmapped) array
         */
        result = lseek(fd, size - 1, SEEK_SET);
        if (result == -1) {
            close(fd);
            printf("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }

        /* Something needs to be written at the end of the file to
         * have the file actually have the new size.
         * Just writing an empty string at the current file position will do.
         *
         * Note:
         *  - The current position in the file is at the end of the stretched 
         *    file due to the call to lseek().
         *  - An empty string is actually a single '\0' character, so a zero-byte
         *    will be written at the last byte of the file.
         */
        result = write(fd, "", 1);
        if (result != 1) {
            close(fd);
            printf("Error writing last byte of the file: %s", path.c_str());
            exit(EXIT_FAILURE);
        }
    } else {
        fd = open(path.c_str(), O_RDWR);
        if (fd == -1) {
            printf("Error opening file for writing: %s", path.c_str());
            exit(EXIT_FAILURE);
        }
    }
    /* Now the file is ready to be mmapped.
     */
    data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        printf("Error mmapping file: %s", path.c_str());
        exit(EXIT_FAILURE);
    }
    return data;
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
    uint64_t numberOfRec;
    uint64_t numberOfFreeSlot;
} DatabaseMeta;

typedef struct {
    string key;
    string value;
    uint64_t nxtColRecOffset;
} Record;

#endif	/* DBCOMMON_H */

