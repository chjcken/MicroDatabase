/* 
 * File:   main.cpp
 * Author: datbt
 *
 * Created on August 22, 2016, 2:31 PM
 */

#include <cstdlib>
#include "MicroDatabaseServer.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
	DBConfig* c = DBConfig::instance();
	
	MicroDatabaseServer server;
	server.start();
	
	return 0;
}

