#!/bin/bash
/usr/local/protobuf/bin/protoc --cpp_out=./ cmd.proto
protoc --cpp_out=./ login.proto
