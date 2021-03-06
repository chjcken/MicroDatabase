/**
 * Autogenerated by Thrift Compiler (0.9.2)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "microdatabase_types.h"

#include <algorithm>
#include <ostream>

#include <thrift/TToString.h>

namespace microdatabase {


TResult::~TResult() throw() {
}


void TResult::__set_error(const int32_t val) {
  this->error = val;
}

void TResult::__set_value(const std::string& val) {
  this->value = val;
__isset.value = true;
}

const char* TResult::ascii_fingerprint = "96705E9A3FD7B072319C71653E0DBB90";
const uint8_t TResult::binary_fingerprint[16] = {0x96,0x70,0x5E,0x9A,0x3F,0xD7,0xB0,0x72,0x31,0x9C,0x71,0x65,0x3E,0x0D,0xBB,0x90};

uint32_t TResult::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_error = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->error);
          isset_error = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->value);
          this->__isset.value = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_error)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TResult::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  oprot->incrementRecursionDepth();
  xfer += oprot->writeStructBegin("TResult");

  xfer += oprot->writeFieldBegin("error", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32(this->error);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.value) {
    xfer += oprot->writeFieldBegin("value", ::apache::thrift::protocol::T_STRING, 2);
    xfer += oprot->writeBinary(this->value);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  oprot->decrementRecursionDepth();
  return xfer;
}

void swap(TResult &a, TResult &b) {
  using ::std::swap;
  swap(a.error, b.error);
  swap(a.value, b.value);
  swap(a.__isset, b.__isset);
}

TResult::TResult(const TResult& other0) {
  error = other0.error;
  value = other0.value;
  __isset = other0.__isset;
}
TResult& TResult::operator=(const TResult& other1) {
  error = other1.error;
  value = other1.value;
  __isset = other1.__isset;
  return *this;
}
std::ostream& operator<<(std::ostream& out, const TResult& obj) {
  using apache::thrift::to_string;
  out << "TResult(";
  out << "error=" << to_string(obj.error);
  out << ", " << "value="; (obj.__isset.value ? (out << to_string(obj.value)) : (out << "<null>"));
  out << ")";
  return out;
}

} // namespace
