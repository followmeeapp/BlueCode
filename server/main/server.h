//
//  main.h
//  Blue
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include "external/lmdb/lmdb.h"

#include <chrono>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/request.capnp.h"
#include "schema/response.capnp.h"
#include "schema/rpc.capnp.h"
#include "schema/trace.capnp.h"
#include "schema/user.capnp.h"

// implemented by (test_)main.cpp
#define CALLBACK_BUFFER_SIZE (4096 * 64)

struct CallbackBuffers {
  CallbackBuffers() : deviceId(0), messageDeviceId(0), didSetPublicKey(false), engineMessageBufferLength(0),
    branchMessageBufferLength(0), traceMessageBufferLength(0), deviceMessageBufferLength(0)
  {
    engineMessageBuffer = (char *)malloc(CALLBACK_BUFFER_SIZE);
    branchMessageBuffer = (char *)malloc(CALLBACK_BUFFER_SIZE);
    traceMessageBuffer  = (char *)malloc(CALLBACK_BUFFER_SIZE);
    deviceMessageBuffer = (char *)malloc(CALLBACK_BUFFER_SIZE);
    devicePublicKey = (unsigned char *)malloc(32);
  }

  ~CallbackBuffers() {
    free(engineMessageBuffer);
    free(branchMessageBuffer);
    free(traceMessageBuffer);
    free(deviceMessageBuffer);
    free(devicePublicKey);
  }

  uint64_t deviceId;
  uint64_t messageDeviceId;
  unsigned char *devicePublicKey;
  bool didSetPublicKey;

  char *engineMessageBuffer;
  char *branchMessageBuffer;
  char *traceMessageBuffer;
  char *deviceMessageBuffer;

  size_t engineMessageBufferLength;
  size_t branchMessageBufferLength;
  size_t traceMessageBufferLength;
  size_t deviceMessageBufferLength;
};

class ServerCallback {
public:
  bool sendEngineMessage(uint8_t *data, size_t length);
  void sendTraceMessage(uint8_t *data, size_t length);
  void sendBranchMessage(const char *msg, size_t len);
  void setDevicePublicKey(uint64_t deviceId, unsigned char *pk);
  void sendDeviceMessage(uint64_t deviceId, const char *msg, size_t len);

  CallbackBuffers m_callbackBuffers;
};

#define MAX_TRACE_EVENTS 255

struct RequestEvent {
  uint64_t timestamp;
  Trace::Event::Function function;
  Trace::Event::Type eventType;
};

// implemented by server.cpp
class Server {
public:
  Server(std::string path);
  ~Server();

  void handleBranchMessage(const char *msg, size_t len, ServerCallback *callbacks);
  void handleClientMessage(uint64_t deviceId, const char *msg, size_t len, bool discoveryAvailable, ServerCallback *callbacks);

  // These are public so that unit tests can inspect LMDB directly.
  MDB_env *m_env;

  MDB_dbi m_metadata;          // metadataId (8 byte int) => value (byte buffer)
  MDB_dbi m_deviceForUuid;     // deviceUUID (16 byte buffer) => deviceId (8 byte int)
  MDB_dbi m_uuidForDevice;     // deviceId (8 byte int) => deviceUUID (16 byte buffer)
  MDB_dbi m_userForDevice;     // deviceId (8 byte int) => userId (8 byte int)
  MDB_dbi m_discoveredDevicesForDevice; // * deviceId (8 byte int) => set of timestamp, deviceId (pair<8 byte int, 8 byte int>)
  MDB_dbi m_recentCardsForDevice;       // * deviceId (8 byte int) => set of timestamp, cardId (pair<8 byte int, 8 byte int>)
  MDB_dbi m_cardsForUser;      // userId (8 byte int) => set of cardId (8 byte int)
  MDB_dbi m_devicesForUser;    // userId (8 byte int) => set of deviceId (8 byte int)
  MDB_dbi m_sectionsForUser;   // userId (8 byte int) => set of sectionId (8 byte int)
  MDB_dbi m_userForTelephone;  // telephone (8 byte int) => userId (8 byte int)
  MDB_dbi m_userForDigitsId;   // digitsId (string) => userId (8 byte int)
  MDB_dbi m_digitsIdForUser;   // userId (8 byte int) => digitsId (string)
  MDB_dbi m_user;              // userId (8 byte int) => Cap’n Proto User object w/ active device id
  MDB_dbi m_userBackup;        // * userId (8 byte int) => set of timestamp, backupId (pair<8 byte int, 8 byte int>)
  MDB_dbi m_backup;            // backupId (8 byte int) => byte buffer
  MDB_dbi m_section;           // sectionId (8 byte int) => Cap’n Proto Section object w/ list of (timestamp, cardId) pairs and update timestamp
  MDB_dbi m_cardForPublicCard; // publicCardId (8 byte int) => cardId (8 byte int)
  MDB_dbi m_card;              // cardId (8 byte int) => Cap’n Proto Card object w/ list of networks
  MDB_dbi m_cardAnalytics;     // cardId (8 byte int) => Cap’n Proto CardAnalytics object
  MDB_dbi m_cardAnalyticsExactUniques; // pair<cardId,TypeId> (8 byte unsigned int) => 8 byte card ids (must insert with no overwrite)
  MDB_dbi m_cardAnalyticsHyperLogLog;  // pair<cardId,TypeId> (8 byte unsigned int) => 2016 byte HyperLogLog buffer (== 504 32-bit UInts and 336 6-bit registers)
  MDB_dbi m_blocksForUser;     // * userId (8 byte int) => set of timestamp, cardId (pair<8 byte int, 8 byte int>)
  MDB_dbi m_deletionsForUser;  // * userId (8 byte int) => set of timestamp, sectionId, cardId (tuple<8 byte int, 8 byte int, 8 byte int>)
  MDB_dbi m_branchRequest;     // timestamp (8 byte int) => UTF-8 buffer containing JSON
  MDB_dbi m_discoveryResponseForDevice; // deviceId (8 byte int) => Cap'n Proto DiscoveryResponse object
  MDB_dbi m_syncInfoForDevice; // deviceId (8 byte int) => Cap’n Proto Card object

  int64_t m_databaseId;

  uint64_t m_lastUserId;
  uint64_t m_lastBackupId;
  uint64_t m_lastCardId;
  uint64_t m_lastDeviceId;
  uint64_t m_lastSectionId;
  uint64_t m_lastBranchRequest;

  uint64_t m_traceTimestampOffset;
  unsigned int m_traceIndex;
  struct RequestEvent m_eventTraces[MAX_TRACE_EVENTS];

  char *m_errorMessage;
  char *m_stacktraceString;

  Server(const Server&) = delete;            // No copy constructor
  Server& operator=(Server const&) = delete; // No assignment
  Server() = delete;                         // No default constructor

private:
  void throwWithStackTrace();
  void makeError(char *file, int line, char *msg, char *mdb_error);
  void finalizeTrace(uint64_t deviceId, int64_t requestId, const char *msg, size_t len, ServerCallback *callbacks);
  
  // Device actions
  void handleInvalidMessage(Response::Kind::Builder &responseKind);
  void handleRequestNotImplemented(Response::Kind::Builder &responseKind);
  auto handleHelloRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, HelloRequest::Reader helloRequest, ServerCallback *callbacks) -> uint64_t;
  void handleCardRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, CardRequest::Reader cardRequest, ServerCallback *callbacks);
  void handleJoinRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, JoinRequest::Reader joinRequest, ServerCallback *callbacks);
  void handleCreateCardRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, CreateCardRequest::Reader createCardRequest, ServerCallback *callbacks);
  void handleUpdateCardRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, UpdateCardRequest::Reader updateCardRequest, ServerCallback *callbacks);
  void handleCreateBackupRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, CreateBackupRequest::Reader createBackupRequest, ServerCallback *callbacks);
  void handleBackupListRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, BackupListRequest::Reader backupListRequest, ServerCallback *callbacks);
  void handleBackupRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, BackupRequest::Reader backupRequest, ServerCallback *callbacks);
};
