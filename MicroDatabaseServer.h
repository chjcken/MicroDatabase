/* 
 * File:   MicroDatabaseServer.h
 * Author: datbt
 *
 * Created on August 30, 2016, 5:07 PM
 */

#ifndef MICRODATABASESERVER_H
#define	MICRODATABASESERVER_H

#include "MicroDatabase.h"
#include "DatabaseModel.h"


#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;
using boost::shared_ptr;

using namespace ::microdatabase;

class MicroDatabaseServer {
public:
    MicroDatabaseServer();
    virtual ~MicroDatabaseServer();
    
    void start();
private:
    shared_ptr<TNonblockingServer> _server;
    shared_ptr<ThreadManager> _threadManager;
    shared_ptr<MicroDatabaseIf> _handler;
    shared_ptr<TProcessor> _processor;
    shared_ptr<TBinaryProtocolFactoryT<TTransport> > _protocolFactory;
};

#endif	/* MICRODATABASESERVER_H */

