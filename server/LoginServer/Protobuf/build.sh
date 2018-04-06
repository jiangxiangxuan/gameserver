#!/bin/bash
/usr/local/protobuf/bin/protoc --cpp_out=./ cmd.proto
protoc  --proto_path=../../ProtocolProto --proto_path=./ --cpp_out=./ ./login.proto