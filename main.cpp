/* 
 * File:   main.cpp
 * Author: datbt
 *
 * Created on August 22, 2016, 2:31 PM
 */

#include <cstdlib>
#include "Database.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
	
	Database db("test");
	Err::Code err = db.put("key", "value");
	printf("put err: %d\n", err);
	
	string v;
	err = db.get("key", v);
	
	printf("get val: %d - %s", err, v.c_str());
	
	return 0;
}

