namespace cpp microdatabase
namespace java com.kissdee.microdatabase.thrift

struct TResult {
    1: required i32 error,
    2: optional binary value,
}


service MicroDatabase {
    i32 ping();
    
    TResult get(1:required binary key);
    
    i32 exist(1:required binary key);
    
    i32 put(1:required binary key, 2:required binary value);
    
    i32 remove(1:required binary key);
}