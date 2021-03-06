// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: block.capnp

#ifndef CAPNP_INCLUDED_80b301d2f5eb2404_
#define CAPNP_INCLUDED_80b301d2f5eb2404_

#include "capnproto/capnp/generated-header-support.h"

#if CAPNP_VERSION != 6000
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(8366c4d3fe60a244);
CAPNP_DECLARE_SCHEMA(cbad615e85e24557);
CAPNP_DECLARE_SCHEMA(b5df659d2659c78e);

}  // namespace schemas
}  // namespace capnp


struct Block {
  Block() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(8366c4d3fe60a244, 2, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

struct BlockRequest {
  BlockRequest() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(cbad615e85e24557, 1, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

struct BlockResponse {
  BlockResponse() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(b5df659d2659c78e, 0, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class Block::Reader {
public:
  typedef Block Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand);
  }
#endif  // !CAPNP_LITE

  inline  ::int64_t getTimestamp() const;

  inline  ::int64_t getCardId() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Block::Builder {
public:
  typedef Block Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::int64_t getTimestamp();
  inline void setTimestamp( ::int64_t value);

  inline  ::int64_t getCardId();
  inline void setCardId( ::int64_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Block::Pipeline {
public:
  typedef Block Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class BlockRequest::Reader {
public:
  typedef BlockRequest Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand);
  }
#endif  // !CAPNP_LITE

  inline  ::int64_t getCardId() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class BlockRequest::Builder {
public:
  typedef BlockRequest Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::int64_t getCardId();
  inline void setCardId( ::int64_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class BlockRequest::Pipeline {
public:
  typedef BlockRequest Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class BlockResponse::Reader {
public:
  typedef BlockResponse Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand);
  }
#endif  // !CAPNP_LITE

  inline bool hasBlock() const;
  inline  ::Block::Reader getBlock() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class BlockResponse::Builder {
public:
  typedef BlockResponse Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline bool hasBlock();
  inline  ::Block::Builder getBlock();
  inline void setBlock( ::Block::Reader value);
  inline  ::Block::Builder initBlock();
  inline void adoptBlock(::capnp::Orphan< ::Block>&& value);
  inline ::capnp::Orphan< ::Block> disownBlock();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class BlockResponse::Pipeline {
public:
  typedef BlockResponse Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

  inline  ::Block::Pipeline getBlock();
private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline  ::int64_t Block::Reader::getTimestamp() const {
  return _reader.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::int64_t Block::Builder::getTimestamp() {
  return _builder.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void Block::Builder::setTimestamp( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline  ::int64_t Block::Reader::getCardId() const {
  return _reader.getDataField< ::int64_t>(
      1 * ::capnp::ELEMENTS);
}

inline  ::int64_t Block::Builder::getCardId() {
  return _builder.getDataField< ::int64_t>(
      1 * ::capnp::ELEMENTS);
}
inline void Block::Builder::setCardId( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      1 * ::capnp::ELEMENTS, value);
}

inline  ::int64_t BlockRequest::Reader::getCardId() const {
  return _reader.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::int64_t BlockRequest::Builder::getCardId() {
  return _builder.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void BlockRequest::Builder::setCardId( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline bool BlockResponse::Reader::hasBlock() const {
  return !_reader.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline bool BlockResponse::Builder::hasBlock() {
  return !_builder.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline  ::Block::Reader BlockResponse::Reader::getBlock() const {
  return ::capnp::_::PointerHelpers< ::Block>::get(
      _reader.getPointerField(0 * ::capnp::POINTERS));
}
inline  ::Block::Builder BlockResponse::Builder::getBlock() {
  return ::capnp::_::PointerHelpers< ::Block>::get(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
#if !CAPNP_LITE
inline  ::Block::Pipeline BlockResponse::Pipeline::getBlock() {
  return  ::Block::Pipeline(_typeless.getPointerField(0));
}
#endif  // !CAPNP_LITE
inline void BlockResponse::Builder::setBlock( ::Block::Reader value) {
  ::capnp::_::PointerHelpers< ::Block>::set(
      _builder.getPointerField(0 * ::capnp::POINTERS), value);
}
inline  ::Block::Builder BlockResponse::Builder::initBlock() {
  return ::capnp::_::PointerHelpers< ::Block>::init(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
inline void BlockResponse::Builder::adoptBlock(
    ::capnp::Orphan< ::Block>&& value) {
  ::capnp::_::PointerHelpers< ::Block>::adopt(
      _builder.getPointerField(0 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::Block> BlockResponse::Builder::disownBlock() {
  return ::capnp::_::PointerHelpers< ::Block>::disown(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}


#endif  // CAPNP_INCLUDED_80b301d2f5eb2404_
