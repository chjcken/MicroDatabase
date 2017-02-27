/* 
 * File:   WriteWorker.cpp
 * Author: datbt
 * 
 * Created on August 31, 2016, 3:09 PM
 */

#include "WriteWorker.h"

WriteWorker::WriteWorker(DatabaseModel& model) : _model(model) {
	_workerPool.addCapacity(DBConfig::instance()->nworkers);
}

WriteWorker::~WriteWorker() {
}

uint32_t WriteWorker::put(const string& key, const string& value) {
	Poco::Mutex::ScopedLock lock(_queueLock);
	WriteJob job(new WriteRunnable(_model, true, key, value, &_waitWorker));
	_jobQueue.push(job);
	_waitJob.set();
	_sequence++;
	return _sequence;
}

uint32_t WriteWorker::remove(const string& key) {
	Poco::Mutex::ScopedLock lock(_queueLock);
	WriteJob job(new WriteRunnable(_model, false, key, "", &_waitWorker));
	_jobQueue.push(job);
	_waitJob.set();
	_sequence++;
	return _sequence;
}

void WriteWorker::run() {
	while (true) {
		bool empty = false;
		{
			Poco::Mutex::ScopedLock lock(_queueLock);
			empty = _jobQueue.empty();
		}
		if (empty) {
			_waitJob.wait();
		}
		if (_workerPool.available() < 1) {
			_waitWorker.wait();
		}
		try {
			Poco::Mutex::ScopedLock lock(_queueLock);
			WriteJob job = _jobQueue.front();
			_workerPool.start(*job);
			_jobQueue.pop();
		} catch (Poco::NoThreadAvailableException &ex) {
			
		}
	}
}

void WriteWorker::start() {
	_jobManager.start(*this);
}


WriteRunnable::WriteRunnable(DatabaseModel& model, const bool isPutOp, const string& key, const string& value, Poco::Event* waitWorker) :
	_model(model), _isPutOp(isPutOp), _key(key), _value(value), _waitWorker(waitWorker) {
}

void WriteRunnable::run() {
	if (_isPutOp) {
		_model.put(_key, _value);
	} else {
		_model.remove(_key);
	}
}

WriteRunnable::~WriteRunnable() {
	_waitWorker->set();
}
