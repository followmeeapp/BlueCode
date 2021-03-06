// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: trace.capnp

#ifndef CAPNP_INCLUDED_cad9c64cbca23db2_
#define CAPNP_INCLUDED_cad9c64cbca23db2_

#include "capnproto/capnp/generated-header-support.h"

#if CAPNP_VERSION != 6000
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(c84a3e33b072af14);
CAPNP_DECLARE_SCHEMA(c28e9c165474ea25);
CAPNP_DECLARE_SCHEMA(f5015670fa084db3);
enum class Function_f5015670fa084db3: uint16_t {
  UNKNOWN,
  HANDLE_CLIENT_MESSAGE,
  READ_REQUEST,
  PREPARE_RESPONSE,
  FINALIZE_RESPONSE,
  FINALIZE_ERROR_RESPONSE,
  SEND_RESPONSE,
  PREPARE_TRACE,
  HANDLE_INVALID_MESSAGE,
  HANDLE_REQUEST_NOT_IMPLEMENTED,
  HANDLE_HELLO_REQUEST,
  HANDLE_CARD_REQUEST,
  HANDLE_JOIN_REQUEST,
  HANDLE_CREATE_CARD_REQUEST,
  HANDLE_UPDATE_CARD_REQUEST,
  HANDLE_CREATE_BACKUP_REQUEST,
  HANDLE_BACKUP_LIST_REQUEST,
  HANDLE_BACKUP_REQUEST,
};
CAPNP_DECLARE_ENUM(Function, f5015670fa084db3);
CAPNP_DECLARE_SCHEMA(9024301303529243);
enum class Type_9024301303529243: uint16_t {
  BEGIN,
  END,
  THROW,
};
CAPNP_DECLARE_ENUM(Type, 9024301303529243);

}  // namespace schemas
}  // namespace capnp


struct Trace {
  Trace() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  struct Event;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(c84a3e33b072af14, 4, 4)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

struct Trace::Event {
  Event() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  typedef ::capnp::schemas::Function_f5015670fa084db3 Function;

  typedef ::capnp::schemas::Type_9024301303529243 Type;


  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(c28e9c165474ea25, 2, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class Trace::Reader {
public:
  typedef Trace Reads;

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

  inline  ::int64_t getRequestId() const;

  inline  ::int64_t getDeviceId() const;

  inline  ::int64_t getTimestamp() const;

  inline  ::int64_t getDuration() const;

  inline bool hasStackTrace() const;
  inline  ::capnp::Text::Reader getStackTrace() const;

  inline bool hasRequestData() const;
  inline  ::capnp::Data::Reader getRequestData() const;

  inline bool hasError() const;
  inline  ::capnp::Text::Reader getError() const;

  inline bool hasEvents() const;
  inline  ::capnp::List< ::Trace::Event>::Reader getEvents() const;

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

class Trace::Builder {
public:
  typedef Trace Builds;

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

  inline  ::int64_t getRequestId();
  inline void setRequestId( ::int64_t value);

  inline  ::int64_t getDeviceId();
  inline void setDeviceId( ::int64_t value);

  inline  ::int64_t getTimestamp();
  inline void setTimestamp( ::int64_t value);

  inline  ::int64_t getDuration();
  inline void setDuration( ::int64_t value);

  inline bool hasStackTrace();
  inline  ::capnp::Text::Builder getStackTrace();
  inline void setStackTrace( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initStackTrace(unsigned int size);
  inline void adoptStackTrace(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownStackTrace();

  inline bool hasRequestData();
  inline  ::capnp::Data::Builder getRequestData();
  inline void setRequestData( ::capnp::Data::Reader value);
  inline  ::capnp::Data::Builder initRequestData(unsigned int size);
  inline void adoptRequestData(::capnp::Orphan< ::capnp::Data>&& value);
  inline ::capnp::Orphan< ::capnp::Data> disownRequestData();

  inline bool hasError();
  inline  ::capnp::Text::Builder getError();
  inline void setError( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initError(unsigned int size);
  inline void adoptError(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownError();

  inline bool hasEvents();
  inline  ::capnp::List< ::Trace::Event>::Builder getEvents();
  inline void setEvents( ::capnp::List< ::Trace::Event>::Reader value);
  inline  ::capnp::List< ::Trace::Event>::Builder initEvents(unsigned int size);
  inline void adoptEvents(::capnp::Orphan< ::capnp::List< ::Trace::Event>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::Trace::Event>> disownEvents();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Trace::Pipeline {
public:
  typedef Trace Pipelines;

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

class Trace::Event::Reader {
public:
  typedef Event Reads;

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

  inline  ::Trace::Event::Function getFunction() const;

  inline  ::Trace::Event::Type getType() const;

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

class Trace::Event::Builder {
public:
  typedef Event Builds;

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

  inline  ::Trace::Event::Function getFunction();
  inline void setFunction( ::Trace::Event::Function value);

  inline  ::Trace::Event::Type getType();
  inline void setType( ::Trace::Event::Type value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Trace::Event::Pipeline {
public:
  typedef Event Pipelines;

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

// =======================================================================================

inline  ::int64_t Trace::Reader::getRequestId() const {
  return _reader.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::int64_t Trace::Builder::getRequestId() {
  return _builder.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void Trace::Builder::setRequestId( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline  ::int64_t Trace::Reader::getDeviceId() const {
  return _reader.getDataField< ::int64_t>(
      1 * ::capnp::ELEMENTS);
}

inline  ::int64_t Trace::Builder::getDeviceId() {
  return _builder.getDataField< ::int64_t>(
      1 * ::capnp::ELEMENTS);
}
inline void Trace::Builder::setDeviceId( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      1 * ::capnp::ELEMENTS, value);
}

inline  ::int64_t Trace::Reader::getTimestamp() const {
  return _reader.getDataField< ::int64_t>(
      2 * ::capnp::ELEMENTS);
}

inline  ::int64_t Trace::Builder::getTimestamp() {
  return _builder.getDataField< ::int64_t>(
      2 * ::capnp::ELEMENTS);
}
inline void Trace::Builder::setTimestamp( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      2 * ::capnp::ELEMENTS, value);
}

inline  ::int64_t Trace::Reader::getDuration() const {
  return _reader.getDataField< ::int64_t>(
      3 * ::capnp::ELEMENTS);
}

inline  ::int64_t Trace::Builder::getDuration() {
  return _builder.getDataField< ::int64_t>(
      3 * ::capnp::ELEMENTS);
}
inline void Trace::Builder::setDuration( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      3 * ::capnp::ELEMENTS, value);
}

inline bool Trace::Reader::hasStackTrace() const {
  return !_reader.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline bool Trace::Builder::hasStackTrace() {
  return !_builder.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Trace::Reader::getStackTrace() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _reader.getPointerField(0 * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Trace::Builder::getStackTrace() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
inline void Trace::Builder::setStackTrace( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(
      _builder.getPointerField(0 * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Trace::Builder::initStackTrace(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(
      _builder.getPointerField(0 * ::capnp::POINTERS), size);
}
inline void Trace::Builder::adoptStackTrace(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(
      _builder.getPointerField(0 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Trace::Builder::disownStackTrace() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}

inline bool Trace::Reader::hasRequestData() const {
  return !_reader.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline bool Trace::Builder::hasRequestData() {
  return !_builder.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Data::Reader Trace::Reader::getRequestData() const {
  return ::capnp::_::PointerHelpers< ::capnp::Data>::get(
      _reader.getPointerField(1 * ::capnp::POINTERS));
}
inline  ::capnp::Data::Builder Trace::Builder::getRequestData() {
  return ::capnp::_::PointerHelpers< ::capnp::Data>::get(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}
inline void Trace::Builder::setRequestData( ::capnp::Data::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Data>::set(
      _builder.getPointerField(1 * ::capnp::POINTERS), value);
}
inline  ::capnp::Data::Builder Trace::Builder::initRequestData(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Data>::init(
      _builder.getPointerField(1 * ::capnp::POINTERS), size);
}
inline void Trace::Builder::adoptRequestData(
    ::capnp::Orphan< ::capnp::Data>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Data>::adopt(
      _builder.getPointerField(1 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Data> Trace::Builder::disownRequestData() {
  return ::capnp::_::PointerHelpers< ::capnp::Data>::disown(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}

inline bool Trace::Reader::hasError() const {
  return !_reader.getPointerField(2 * ::capnp::POINTERS).isNull();
}
inline bool Trace::Builder::hasError() {
  return !_builder.getPointerField(2 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Trace::Reader::getError() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _reader.getPointerField(2 * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Trace::Builder::getError() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _builder.getPointerField(2 * ::capnp::POINTERS));
}
inline void Trace::Builder::setError( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(
      _builder.getPointerField(2 * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Trace::Builder::initError(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(
      _builder.getPointerField(2 * ::capnp::POINTERS), size);
}
inline void Trace::Builder::adoptError(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(
      _builder.getPointerField(2 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Trace::Builder::disownError() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(
      _builder.getPointerField(2 * ::capnp::POINTERS));
}

inline bool Trace::Reader::hasEvents() const {
  return !_reader.getPointerField(3 * ::capnp::POINTERS).isNull();
}
inline bool Trace::Builder::hasEvents() {
  return !_builder.getPointerField(3 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::Trace::Event>::Reader Trace::Reader::getEvents() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Trace::Event>>::get(
      _reader.getPointerField(3 * ::capnp::POINTERS));
}
inline  ::capnp::List< ::Trace::Event>::Builder Trace::Builder::getEvents() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Trace::Event>>::get(
      _builder.getPointerField(3 * ::capnp::POINTERS));
}
inline void Trace::Builder::setEvents( ::capnp::List< ::Trace::Event>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Trace::Event>>::set(
      _builder.getPointerField(3 * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::Trace::Event>::Builder Trace::Builder::initEvents(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Trace::Event>>::init(
      _builder.getPointerField(3 * ::capnp::POINTERS), size);
}
inline void Trace::Builder::adoptEvents(
    ::capnp::Orphan< ::capnp::List< ::Trace::Event>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Trace::Event>>::adopt(
      _builder.getPointerField(3 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::Trace::Event>> Trace::Builder::disownEvents() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Trace::Event>>::disown(
      _builder.getPointerField(3 * ::capnp::POINTERS));
}

inline  ::int64_t Trace::Event::Reader::getTimestamp() const {
  return _reader.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::int64_t Trace::Event::Builder::getTimestamp() {
  return _builder.getDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void Trace::Event::Builder::setTimestamp( ::int64_t value) {
  _builder.setDataField< ::int64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline  ::Trace::Event::Function Trace::Event::Reader::getFunction() const {
  return _reader.getDataField< ::Trace::Event::Function>(
      4 * ::capnp::ELEMENTS);
}

inline  ::Trace::Event::Function Trace::Event::Builder::getFunction() {
  return _builder.getDataField< ::Trace::Event::Function>(
      4 * ::capnp::ELEMENTS);
}
inline void Trace::Event::Builder::setFunction( ::Trace::Event::Function value) {
  _builder.setDataField< ::Trace::Event::Function>(
      4 * ::capnp::ELEMENTS, value);
}

inline  ::Trace::Event::Type Trace::Event::Reader::getType() const {
  return _reader.getDataField< ::Trace::Event::Type>(
      5 * ::capnp::ELEMENTS);
}

inline  ::Trace::Event::Type Trace::Event::Builder::getType() {
  return _builder.getDataField< ::Trace::Event::Type>(
      5 * ::capnp::ELEMENTS);
}
inline void Trace::Event::Builder::setType( ::Trace::Event::Type value) {
  _builder.setDataField< ::Trace::Event::Type>(
      5 * ::capnp::ELEMENTS, value);
}


#endif  // CAPNP_INCLUDED_cad9c64cbca23db2_
