/* 
 * File:   WriteWorker.h
 * Author: datbt
 *
 * Created on August 31, 2016, 3:09 PM
 */

#ifndef WRITEWORKER_H
#define	WRITEWORKER_H

#include "DBCommon.h"
#include "DatabaseModel.h"

class WriteRunnable;
typedef shared_ptr<WriteRunnable> WriteJob;


class WriteWorker: public Poco::Runnable {
public:
    WriteWorker(DatabaseModel& model);
    virtual ~WriteWorker();
    
    uint32_t put(const string& key, const string& value);
    uint32_t remove(const string& key);
    
    virtual void run();
    
    void start();
private:
    DatabaseModel _model;
    Poco::Event _waitJob;
    Poco::Event _waitWorker;
    queue<WriteJob> _jobQueue;
    Poco::ThreadPool _workerPool;
    Poco::Mutex _queueLock;
    uint32_t _sequence;
    Poco::Thread _jobManager;
    
};

class WriteRunnable: public Poco::Runnable {
public:
    WriteRunnable (DatabaseModel& model, const bool isPutOp, const string& key, const string& value, Poco::Event* waitWorker);
    ~WriteRunnable();
    virtual void run();
    
private:
    DatabaseModel _model;
    const bool _isPutOp;
    const string _key;
    const string _value;
    Poco::Event* _waitWorker;
};

#endif	/* WRITEWORKER_H */

