// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: player.proto

#ifndef PROTOBUF_player_2eproto__INCLUDED
#define PROTOBUF_player_2eproto__INCLUDED

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
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace protobuf_player_2eproto {
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
void InitDefaultsPlayerInfoImpl();
void InitDefaultsPlayerInfo();
inline void InitDefaults() {
  InitDefaultsPlayerInfo();
}
}  // namespace protobuf_player_2eproto
namespace protocol {
class PlayerInfo;
class PlayerInfoDefaultTypeInternal;
extern PlayerInfoDefaultTypeInternal _PlayerInfo_default_instance_;
}  // namespace protocol
namespace google {
namespace protobuf {
template<> ::protocol::PlayerInfo* Arena::Create< ::protocol::PlayerInfo>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace protocol {

// ===================================================================

class PlayerInfo : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:protocol.PlayerInfo) */ {
 public:
  PlayerInfo();
  virtual ~PlayerInfo();

  PlayerInfo(const PlayerInfo& from);

  inline PlayerInfo& operator=(const PlayerInfo& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  PlayerInfo(PlayerInfo&& from) noexcept
    : PlayerInfo() {
    *this = ::std::move(from);
  }

  inline PlayerInfo& operator=(PlayerInfo&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const PlayerInfo& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const PlayerInfo* internal_default_instance() {
    return reinterpret_cast<const PlayerInfo*>(
               &_PlayerInfo_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(PlayerInfo* other);
  friend void swap(PlayerInfo& a, PlayerInfo& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline PlayerInfo* New() const PROTOBUF_FINAL {
    return ::google::protobuf::Arena::Create<PlayerInfo>(NULL);
  }

  PlayerInfo* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL {
    return ::google::protobuf::Arena::Create<PlayerInfo>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const PlayerInfo& from);
  void MergeFrom(const PlayerInfo& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(PlayerInfo* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string UserName = 2;
  void clear_username();
  static const int kUserNameFieldNumber = 2;
  const ::std::string& username() const;
  void set_username(const ::std::string& value);
  #if LANG_CXX11
  void set_username(::std::string&& value);
  #endif
  void set_username(const char* value);
  void set_username(const char* value, size_t size);
  ::std::string* mutable_username();
  ::std::string* release_username();
  void set_allocated_username(::std::string* username);

  // string NickName = 3;
  void clear_nickname();
  static const int kNickNameFieldNumber = 3;
  const ::std::string& nickname() const;
  void set_nickname(const ::std::string& value);
  #if LANG_CXX11
  void set_nickname(::std::string&& value);
  #endif
  void set_nickname(const char* value);
  void set_nickname(const char* value, size_t size);
  ::std::string* mutable_nickname();
  ::std::string* release_nickname();
  void set_allocated_nickname(::std::string* nickname);

  // int32 Uid = 1;
  void clear_uid();
  static const int kUidFieldNumber = 1;
  ::google::protobuf::int32 uid() const;
  void set_uid(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:protocol.PlayerInfo)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr username_;
  ::google::protobuf::internal::ArenaStringPtr nickname_;
  ::google::protobuf::int32 uid_;
  mutable int _cached_size_;
  friend struct ::protobuf_player_2eproto::TableStruct;
  friend void ::protobuf_player_2eproto::InitDefaultsPlayerInfoImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// PlayerInfo

// int32 Uid = 1;
inline void PlayerInfo::clear_uid() {
  uid_ = 0;
}
inline ::google::protobuf::int32 PlayerInfo::uid() const {
  // @@protoc_insertion_point(field_get:protocol.PlayerInfo.Uid)
  return uid_;
}
inline void PlayerInfo::set_uid(::google::protobuf::int32 value) {
  
  uid_ = value;
  // @@protoc_insertion_point(field_set:protocol.PlayerInfo.Uid)
}

// string UserName = 2;
inline void PlayerInfo::clear_username() {
  username_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& PlayerInfo::username() const {
  // @@protoc_insertion_point(field_get:protocol.PlayerInfo.UserName)
  return username_.GetNoArena();
}
inline void PlayerInfo::set_username(const ::std::string& value) {
  
  username_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:protocol.PlayerInfo.UserName)
}
#if LANG_CXX11
inline void PlayerInfo::set_username(::std::string&& value) {
  
  username_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:protocol.PlayerInfo.UserName)
}
#endif
inline void PlayerInfo::set_username(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  username_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:protocol.PlayerInfo.UserName)
}
inline void PlayerInfo::set_username(const char* value, size_t size) {
  
  username_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:protocol.PlayerInfo.UserName)
}
inline ::std::string* PlayerInfo::mutable_username() {
  
  // @@protoc_insertion_point(field_mutable:protocol.PlayerInfo.UserName)
  return username_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* PlayerInfo::release_username() {
  // @@protoc_insertion_point(field_release:protocol.PlayerInfo.UserName)
  
  return username_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void PlayerInfo::set_allocated_username(::std::string* username) {
  if (username != NULL) {
    
  } else {
    
  }
  username_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), username);
  // @@protoc_insertion_point(field_set_allocated:protocol.PlayerInfo.UserName)
}

// string NickName = 3;
inline void PlayerInfo::clear_nickname() {
  nickname_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& PlayerInfo::nickname() const {
  // @@protoc_insertion_point(field_get:protocol.PlayerInfo.NickName)
  return nickname_.GetNoArena();
}
inline void PlayerInfo::set_nickname(const ::std::string& value) {
  
  nickname_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:protocol.PlayerInfo.NickName)
}
#if LANG_CXX11
inline void PlayerInfo::set_nickname(::std::string&& value) {
  
  nickname_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:protocol.PlayerInfo.NickName)
}
#endif
inline void PlayerInfo::set_nickname(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  nickname_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:protocol.PlayerInfo.NickName)
}
inline void PlayerInfo::set_nickname(const char* value, size_t size) {
  
  nickname_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:protocol.PlayerInfo.NickName)
}
inline ::std::string* PlayerInfo::mutable_nickname() {
  
  // @@protoc_insertion_point(field_mutable:protocol.PlayerInfo.NickName)
  return nickname_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* PlayerInfo::release_nickname() {
  // @@protoc_insertion_point(field_release:protocol.PlayerInfo.NickName)
  
  return nickname_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void PlayerInfo::set_allocated_nickname(::std::string* nickname) {
  if (nickname != NULL) {
    
  } else {
    
  }
  nickname_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), nickname);
  // @@protoc_insertion_point(field_set_allocated:protocol.PlayerInfo.NickName)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_player_2eproto__INCLUDED