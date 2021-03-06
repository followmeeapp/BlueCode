// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: discovery.capnp

#ifndef CAPNP_INCLUDED_8bce6c366afb87bd_
#define CAPNP_INCLUDED_8bce6c366afb87bd_

#include "capnproto/capnp/generated-header-support.h"

#if CAPNP_VERSION != 6000
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif

#include "card_info.capnp.h"
#include "device_info.capnp.h"
#include "section.capnp.h"

namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(9e801d0f2b203ef8);
CAPNP_DECLARE_SCHEMA(c9077587d76c5afa);

}  // namespace schemas
}  // namespace capnp


struct DiscoveryRequest {
  DiscoveryRequest() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(9e801d0f2b203ef8, 1, 2)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

struct DiscoveryResponse {
  DiscoveryResponse() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(c9077587d76c5afa, 1, 2)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class DiscoveryRequest::Reader {
public:
  typedef DiscoveryRequest Reads;

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

  inline  ::int64_t getLastDiscoveryTimestamp() const;

  inline bool hasCards() const;
  inline  ::capnp::List< ::CardInfo>::Reader getCards() const;

  inline bool hasDevices() const;
  inline  ::capnp::List< ::DeviceInfo>::Reader getDevices() const;

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

class DiscoveryRequest::Builder {
public:
  typedef DiscoveryRequest Builds;

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

  inline  ::int64_t getLastDiscoveryTimestamp();
  inline void setLastDiscoveryTimestamp( ::int64_t value);

  inline bool hasCards();
  inline  ::capnp::List< ::CardInfo>::Builder getCards();
  inline void setCards( ::capnp::List< ::CardInfo>::Reader value);
  inline  ::capnp::List< ::CardInfo>::Builder initCards(unsigned int size);
  inline void adoptCards(::capnp::Orphan< ::capnp::List< ::CardInfo>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::CardInfo>> disownCards();

  inline bool hasDevices();
  inline  ::capnp::List< ::DeviceInfo>::Builder getDevices();
  inline void setDevices( ::capnp::List< ::DeviceInfo>::Reader value);
  inline  ::capnp::List< ::DeviceInfo>::Builder initDevices(unsigned int size);
  inline void adoptDevices(::capnp::Orphan< ::capnp::List< ::DeviceInfo>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::DeviceInfo>> disownDevices();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class DiscoveryRequest::Pipeline {
public:
  typedef DiscoveryRequest Pipelines;

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

class DiscoveryResponse::Reader {
public:
  typedef DiscoveryResponse Reads;

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

  inline bool hasSection() const;
  inline  ::Section::Reader getSection() const;

  inline bool hasCards() const;
  inline  ::capnp::List< ::CardInfo>::Reader getCards() const;

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

class DiscoveryResponse::Builder {
public:
  typedef DiscoveryResponse Builds;

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

  inline bool hasSection();
  inline  ::Section::Builder getSection();
  inline void setSection( ::Section::Reader value);
  inline  ::Section::Builder initSection();
  inline void adoptSection(::capnp::Orphan< ::Section>&& value);
  inline ::capnp::Orphan< ::Section> disownSection();

  inline bool hasCards();
  inline  ::capnp::List< ::CardInfo>::Builder getCards();
  inline void setCards( ::capnp::List< ::CardInfo>::Reader value);
  inline  ::capnp::List< ::CardInfo>::Builder initCards(unsigned int size);
  inline void adoptCards(::capnp::Orphan< ::capnp::List< ::CardInfo>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::CardInfo>> disownCards();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class DiscoveryResponse::Pipeline {
public:
  typedef DiscoveryResponse Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

  inline  ::Section::Pipeline getSection();
private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline  ::int64_t DiscoveryRequest::Reader::getLastDiscoveryTimestamp() const {
  return _reader.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::int64_t DiscoveryRequest::Builder::getLastDiscoveryTimestamp() {
  return _builder.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void DiscoveryRequest::Builder::setLastDiscoveryTimestamp( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline bool DiscoveryRequest::Reader::hasCards() const {
  return !_reader.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline bool DiscoveryRequest::Builder::hasCards() {
  return !_builder.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::CardInfo>::Reader DiscoveryRequest::Reader::getCards() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::get(
      _reader.getPointerField(0 * ::capnp::POINTERS));
}
inline  ::capnp::List< ::CardInfo>::Builder DiscoveryRequest::Builder::getCards() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::get(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
inline void DiscoveryRequest::Builder::setCards( ::capnp::List< ::CardInfo>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::set(
      _builder.getPointerField(0 * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::CardInfo>::Builder DiscoveryRequest::Builder::initCards(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::init(
      _builder.getPointerField(0 * ::capnp::POINTERS), size);
}
inline void DiscoveryRequest::Builder::adoptCards(
    ::capnp::Orphan< ::capnp::List< ::CardInfo>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::adopt(
      _builder.getPointerField(0 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::CardInfo>> DiscoveryRequest::Builder::disownCards() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::disown(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}

inline bool DiscoveryRequest::Reader::hasDevices() const {
  return !_reader.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline bool DiscoveryRequest::Builder::hasDevices() {
  return !_builder.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::DeviceInfo>::Reader DiscoveryRequest::Reader::getDevices() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::DeviceInfo>>::get(
      _reader.getPointerField(1 * ::capnp::POINTERS));
}
inline  ::capnp::List< ::DeviceInfo>::Builder DiscoveryRequest::Builder::getDevices() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::DeviceInfo>>::get(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}
inline void DiscoveryRequest::Builder::setDevices( ::capnp::List< ::DeviceInfo>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::DeviceInfo>>::set(
      _builder.getPointerField(1 * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::DeviceInfo>::Builder DiscoveryRequest::Builder::initDevices(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::DeviceInfo>>::init(
      _builder.getPointerField(1 * ::capnp::POINTERS), size);
}
inline void DiscoveryRequest::Builder::adoptDevices(
    ::capnp::Orphan< ::capnp::List< ::DeviceInfo>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::DeviceInfo>>::adopt(
      _builder.getPointerField(1 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::DeviceInfo>> DiscoveryRequest::Builder::disownDevices() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::DeviceInfo>>::disown(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}

inline  ::int64_t DiscoveryResponse::Reader::getTimestamp() const {
  return _reader.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::int64_t DiscoveryResponse::Builder::getTimestamp() {
  return _builder.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void DiscoveryResponse::Builder::setTimestamp( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline bool DiscoveryResponse::Reader::hasSection() const {
  return !_reader.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline bool DiscoveryResponse::Builder::hasSection() {
  return !_builder.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline  ::Section::Reader DiscoveryResponse::Reader::getSection() const {
  return ::capnp::_::PointerHelpers< ::Section>::get(
      _reader.getPointerField(0 * ::capnp::POINTERS));
}
inline  ::Section::Builder DiscoveryResponse::Builder::getSection() {
  return ::capnp::_::PointerHelpers< ::Section>::get(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
#if !CAPNP_LITE
inline  ::Section::Pipeline DiscoveryResponse::Pipeline::getSection() {
  return  ::Section::Pipeline(_typeless.getPointerField(0));
}
#endif  // !CAPNP_LITE
inline void DiscoveryResponse::Builder::setSection( ::Section::Reader value) {
  ::capnp::_::PointerHelpers< ::Section>::set(
      _builder.getPointerField(0 * ::capnp::POINTERS), value);
}
inline  ::Section::Builder DiscoveryResponse::Builder::initSection() {
  return ::capnp::_::PointerHelpers< ::Section>::init(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
inline void DiscoveryResponse::Builder::adoptSection(
    ::capnp::Orphan< ::Section>&& value) {
  ::capnp::_::PointerHelpers< ::Section>::adopt(
      _builder.getPointerField(0 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::Section> DiscoveryResponse::Builder::disownSection() {
  return ::capnp::_::PointerHelpers< ::Section>::disown(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}

inline bool DiscoveryResponse::Reader::hasCards() const {
  return !_reader.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline bool DiscoveryResponse::Builder::hasCards() {
  return !_builder.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::CardInfo>::Reader DiscoveryResponse::Reader::getCards() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::get(
      _reader.getPointerField(1 * ::capnp::POINTERS));
}
inline  ::capnp::List< ::CardInfo>::Builder DiscoveryResponse::Builder::getCards() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::get(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}
inline void DiscoveryResponse::Builder::setCards( ::capnp::List< ::CardInfo>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::set(
      _builder.getPointerField(1 * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::CardInfo>::Builder DiscoveryResponse::Builder::initCards(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::init(
      _builder.getPointerField(1 * ::capnp::POINTERS), size);
}
inline void DiscoveryResponse::Builder::adoptCards(
    ::capnp::Orphan< ::capnp::List< ::CardInfo>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::adopt(
      _builder.getPointerField(1 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::CardInfo>> DiscoveryResponse::Builder::disownCards() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::CardInfo>>::disown(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}


#endif  // CAPNP_INCLUDED_8bce6c366afb87bd_
