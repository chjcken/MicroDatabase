/* 
 * File:   IOControl.h
 * Author: datbt
 *
 * Created on August 22, 2016, 2:54 PM
 */

#ifndef IOCONTROL_H
#define	IOCONTROL_H

#include "DBCommon.h"

class IOControl {
public:
    IOControl();
    virtual ~IOControl();
    bool init(int64_t readLimitSize, int64_t writeLimitSize);
    void close();

    int64_t getReadLimitSize() const;
    int64_t getWriteLimitSize() const;

    bool setLimit(int64_t readLimitSize, int64_t writeLimitSize);

public:
    void onRead(int64_t size);
    void onWrite(int64_t size);

private:
    mutable int64_t _readLimitSize;
    mutable int64_t _writeLimitSize;
    Poco::Event _evWaitRead;
    Poco::Event _evWaitWrite;
    bool _bStop;

    struct Track {
        int64_t time;
        int64_t size;

        Track() : time(0), size(0) {
        }
    };


    Track _trackRead;
    Track _trackWrite;
};

#endif	/* IOCONTROL_H */

