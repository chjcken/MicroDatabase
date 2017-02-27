//
//#include "MicroDatabase.h"
//#include "DatabaseModel.h"
//
//#include <thrift/transport/TBufferTransports.h>
//
//#include <thrift/protocol/TBinaryProtocol.h>
//#include <thrift/server/TSimpleServer.h>
//#include <thrift/concurrency/PosixThreadFactory.h>
//#include <thrift/server/TThreadPoolServer.h>
//#include <thrift/server/TThreadedServer.h>
//#include <thrift/server/TNonblockingServer.h>
//#include <thrift/transport/TSocket.h>
//#include <thrift/transport/TServerSocket.h>
//#include <thrift/transport/TTransportUtils.h>
//
//using namespace ::apache::thrift;
//using namespace ::apache::thrift::protocol;
//using namespace ::apache::thrift::transport;
//using namespace ::apache::thrift::server;
//
//using boost::shared_ptr;
//
//using namespace ::microdatabase;
//
//class MicroDatabaseHandler : virtual public MicroDatabaseIf {
//public:
//
//	MicroDatabaseHandler() : _model(new DatabaseModel(DBConfig::instance()->db_name)) {
//
//	}
//
//	int32_t ping() {
//		return 0;
//	}
//
//	void get(TResult& _return, const std::string& key) {
//		_return.error = -_model->get(key, _return.value);
//
//	}
//
//	int32_t exist(const std::string& key) {
//		return -_model->exist(key);
//	}
//
//	int32_t put(const std::string& key, const std::string& value) {
//		return -_model->put(key, value);
//	}
//
//	int32_t remove(const std::string& key) {
//		return -_model->remove(key);
//	}
//
//private:
//	DatabaseModel* _model;
//
//};
//
//class MicroDatabaseServer {
//public:
//
//	MicroDatabaseServer() {
//		shared_ptr<MicroDatabaseHandler> handler(new MicroDatabaseHandler());
//		_processor(new ::microdatabase::MicroDatabaseProcessor(handler));
//	}
//
//	virtual ~MicroDatabaseServer() {
//
//	}
//
//	void start() {
//		if (DBConfig::instance()->nworkers > 0) {
//			_threadManager = ThreadManager::newSimpleThreadManager(DBConfig::instance()->nworkers);
//			boost::shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory());
//			_threadManager->threadFactory(threadFactory);
//			_threadManager->start();
//			_server.reset(new TNonblockingServer(_processor, _protocolFactory, DBConfig::instance()->port, _threadManager));
//		} else {
//			_server.reset(new TNonblockingServer(_processor, _protocolFactory, DBConfig::instance()->port));
//		}
//		if (DBConfig::instance()->io_threads > 0) {
//			_server->setNumIOThreads(DBConfig::instance()->io_threads);
//		}
//		printf("Server start at port %d...", DBConfig::instance()->port);
//		_server->serve();
//	}
//private:
//	shared_ptr<TNonblockingServer> _server;
//	shared_ptr<ThreadManager> _threadManager;
//	shared_ptr<TProcessor> _processor;
//	shared_ptr<TBinaryProtocolFactoryT<TTransport> > _protocolFactory;
//};
//
////int main(int argc, char **argv) {
////	int port = 9090;
////	shared_ptr<MicroDatabaseHandler> handler(new MicroDatabaseHandler());
////	shared_ptr<TProcessor> processor(new MicroDatabaseProcessor(handler));
////	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
////	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
////	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
////
////	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
////	server.serve();
////	return 0;
////}
//
