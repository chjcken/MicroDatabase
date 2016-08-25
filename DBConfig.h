/* 
 * File:   DBConfig.h
 * Author: datbt
 *
 * Created on August 22, 2016, 2:46 PM
 */

#ifndef DBCONFIG_H
#define	DBCONFIG_H

#define _1GB                    1073741824
#define _8GB                    8589934592

#define INDEX_SIZE              10000 //
#define RECORD_MAX_SIZE         10240 // 10kb

#define STORAGE_FILE_SIZE       _8GB

#define SLOT_LOCK_NUM           1000

#define DATA_DIRECTORY          "/data/mdb/"

#define INDEX_FILE_SIZE         INDEX_SIZE * RECORD_MAX_SIZE

#define REUSE_SLOT_SIZE         1000

#endif	/* DBCONFIG_H */

