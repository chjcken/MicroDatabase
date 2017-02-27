/* 
 * File:   IOControl.cpp
 * Author: datbt
 * 
 * Created on August 22, 2016, 2:54 PM
 */

#include "IOControl.h"
#include "Poco/Thread.h"

IOControl::IOControl(): _bStop(false), _readLimitSize(0), _writeLimitSize(0) {
}

IOControl::~IOControl() {
}

bool IOControl::init(int64_t readLimitSize, int64_t writeLimitSize) {
	_readLimitSize = readLimitSize;
	_writeLimitSize = writeLimitSize;
	return true;
}

void IOControl::close() {
	_bStop = true;
	_evWaitRead.set();
	_evWaitWrite.set();
}

int64_t IOControl::getReadLimitSize() const {
	return _readLimitSize;
}

int64_t IOControl::getWriteLimitSize() const {
	return _writeLimitSize;
}

bool IOControl::setLimit(int64_t readLimitSize, int64_t writeLimitSize) {
	_readLimitSize = readLimitSize;
	_writeLimitSize = writeLimitSize;
}

void IOControl::onRead(int64_t size) {
	if (_readLimitSize <= 0) {
		return;
	}

	int64_t ts = getCurrentTimeMicroSec() / 1000;///
	int64_t span = ts - _trackRead.time;
	if (span > 1000) {
		_trackRead.time = ts;
		_trackRead.size = size;
		return;
	}

	_trackRead.size += size;
	if (_trackRead.size >= _readLimitSize) {
		// track size exceed limit size: wait for next second
		_evWaitRead.tryWait(span);
		if (_bStop) {
			return;
		}
	}
}

void IOControl::onWrite(int64_t size) {
	if (_writeLimitSize <= 0) {
		return;
	}

	int64_t ts = getCurrentTimeMicroSec() / 1000;///
	int64_t span = ts - _trackWrite.time;
	if (span > 1000) {
		_trackWrite.time = ts;
		_trackWrite.size = size;
		return;
	}

	_trackWrite.size += size;
	if (_trackWrite.size >= _writeLimitSize) {
		// track size exceed limit size: wait for next second
		_evWaitWrite.tryWait(span);
		if (_bStop) {
			return;
		}
	}
}