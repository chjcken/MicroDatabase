/* 
 * File:   Profiler.cpp
 * Author: datbt
 * 
 * Created on August 25, 2016, 3:34 PM
 */

#include "Profiler.h"

Poco::Mutex Profiler::_mutex;
std::map<std::string, ProfileStat> Profiler::_apiMap;

Err::Code Profiler::getAPIStat(const std::string& api, ProfileStat& stat) {
	Poco::Mutex::ScopedLock lock(_mutex);
	std::map<std::string, ProfileStat>::iterator iter = _apiMap.find(api);
	if (iter == _apiMap.end()){
		return Err::NOT_EXIST;
	}
	stat = iter->second;
	return Err::SUCCESS;
}

Profiler::Profiler(const std::string& api) : _apiName(api) {
	Poco::Mutex::ScopedLock lock(_mutex);
	if (_apiMap.count(api) <= 0){
		ProfileStat stat;
		stat.totalReq = 0;
		stat.totalTimeProc = 0;
		stat.lastTimeProc = 0;
		std::pair<std::string, ProfileStat> pair(api, stat);
		_apiMap.insert(pair);
	}		
	__begin = getCurrentTimeMicroSec();
}

Profiler::~Profiler() {	
	int64_t procTime = getCurrentTimeMicroSec() - __begin;
	
	Poco::Mutex::ScopedLock lock(_mutex);
	std::map<std::string, ProfileStat>::iterator iter = _apiMap.find(_apiName);
	if (iter == _apiMap.end()){
		printf("get stat err: %s\n", _apiName.c_str());
	} else {
		iter->second.totalReq++;
		iter->second.totalTimeProc += procTime;
		iter->second.lastTimeProc = procTime;
	}
}

