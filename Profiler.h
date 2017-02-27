/* 
 * File:   Profiler.h
 * Author: datbt
 *
 * Created on August 25, 2016, 3:34 PM
 */

#ifndef PROFILER_H
#define	PROFILER_H
#include "DBCommon.h"

typedef struct {
    uint64_t totalReq;
    uint64_t totalTimeProc;
    uint64_t lastTimeProc;
} ProfileStat;

class Profiler {
public:
    Profiler(const std::string& api);
    ~Profiler();
    static Error::Code getAPIStat(const std::string& api, ProfileStat& stat);
    
private:    
    static Poco::Mutex _mutex;
    static std::map<std::string, ProfileStat> _apiMap;
    
    std::string _apiName;
    int64_t __begin;
};

#endif	/* PROFILER_H */

