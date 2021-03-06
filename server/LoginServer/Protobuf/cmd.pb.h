// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: cmd.proto

#ifndef PROTOBUF_cmd_2eproto__INCLUDED
#define PROTOBUF_cmd_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)

namespace protobuf_cmd_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
inline void InitDefaults() {
}
}  // namespace protobuf_cmd_2eproto
namespace platformprotocol {
}  // namespace platformprotocol
namespace platformprotocol {

enum CMD {
  PLATFORM_DEFAULT = 0,
  PLATFORM_VERIFY_TOKEN = 2000,
  CMD_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  CMD_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool CMD_IsValid(int value);
const CMD CMD_MIN = PLATFORM_DEFAULT;
const CMD CMD_MAX = PLATFORM_VERIFY_TOKEN;
const int CMD_ARRAYSIZE = CMD_MAX + 1;

const ::google::protobuf::EnumDescriptor* CMD_descriptor();
inline const ::std::string& CMD_Name(CMD value) {
  return ::google::protobuf::internal::NameOfEnum(
    CMD_descriptor(), value);
}
inline bool CMD_Parse(
    const ::std::string& name, CMD* value) {
  return ::google::protobuf::internal::ParseNamedEnum<CMD>(
    CMD_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace platformprotocol

namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::platformprotocol::CMD> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::platformprotocol::CMD>() {
  return ::platformprotocol::CMD_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_cmd_2eproto__INCLUDED
