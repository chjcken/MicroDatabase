/* 
 * File:   MicroDatabaseServer.cpp
 * Author: datbt
 * 
 * Created on August 30, 2016, 5:07 PM
 */

#include "MicroDatabaseServer.h"
#include "WriteWorker.h"

class MicroDatabaseHandler : virtual public MicroDatabaseIf {
public:

	MicroDatabaseHandler() : _model(new DatabaseModel(DBConfig::instance()->db_name)), _writeJobManager(new WriteWorker(*_model)) {
		Poco::Thread t;
		t.start(*_writeJobManager);
	}

	int32_t ping() {
		return 0;
	}

	void get(TResult& _return, const std::string& key) {
		_return.error = -_model->get(key, _return.value);

	}

	int32_t exist(const std::string& key) {
		return -_model->exist(key);
	}

	int32_t put(const std::string& key, const std::string& value) {
		return _writeJobManager->put(key, value);
	}

	int32_t remove(const std::string& key) {
		return _writeJobManager->remove(key);
	}

private:
	DatabaseModel* _model;
	WriteWorker* _writeJobManager;

};

MicroDatabaseServer::MicroDatabaseServer(): _protocolFactory(new TBinaryProtocolFactory()), _handler(new MicroDatabaseHandler()), _processor(new MicroDatabaseProcessor(_handler)) {
}

MicroDatabaseServer::~MicroDatabaseServer() {
}

void MicroDatabaseServer::start() {
	if (DBConfig::instance()->nworkers > 0) {
		_threadManager = ThreadManager::newSimpleThreadManager(DBConfig::instance()->nworkers);
		boost::shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory());
		_threadManager->threadFactory(threadFactory);
		_threadManager->start();
		_server.reset(new TNonblockingServer(_processor, _protocolFactory, DBConfig::instance()->port, _threadManager));
	} else {
		_server.reset(new TNonblockingServer(_processor, _protocolFactory, DBConfig::instance()->port));
	}
	if (DBConfig::instance()->io_threads > 0) {
		_server->setNumIOThreads(DBConfig::instance()->io_threads);
	}
	printf("Server start at port %d...", DBConfig::instance()->port);
	_server->serve();
}
