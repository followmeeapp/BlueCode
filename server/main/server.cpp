//
//  server.cpp
//  Blue
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

#include "server.h"
#include "stacktrace.h"

using namespace std;

 // 1GB memory-mapped database
#define DEFAULT_DB_SIZE 1024

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

// From https://unix4lyfe.org/benchmarking/
uint64_t rdtscp(void) {
  uint32_t lo, hi;
  __asm__ volatile ("rdtscp"
      : /* outputs */ "=a" (lo), "=d" (hi)
      : /* no inputs */
      : /* clobbers */ "%rcx");
  return (uint64_t)lo | (((uint64_t)hi) << 32);
}

#define TRACE(fun, evt)                               \
  do {                                                \
    struct RequestEvent *reqEvt = m_eventTraces;      \
    reqEvt = reqEvt + m_traceIndex;                   \
    m_traceIndex++;                                   \
    if (unlikely(m_traceIndex == MAX_TRACE_EVENTS)) { \
      m_traceIndex--;                                 \
    }                                                 \
    reqEvt->timestamp = rdtscp();                     \
    reqEvt->function = Trace::Event::Function::fun;   \
    reqEvt->eventType = Trace::Event::Type::evt;      \
  } while (false)

#define TRACE_BEGIN(fun) TRACE(fun, BEGIN)
#define TRACE_END(fun)   TRACE(fun, END)
#define TRACE_THROW(fun) TRACE(fun, THROW)

#define CREATE_TRACE(fun)                           \
  do {                                              \
    m_traceIndex = 1;                               \
    m_traceTimestampOffset = rdtscp();              \
    struct RequestEvent *reqEvt = m_eventTraces;    \
    reqEvt->timestamp = m_traceTimestampOffset;     \
    reqEvt->function = Trace::Event::Function::fun; \
    reqEvt->eventType = Trace::Event::Type::BEGIN;  \
  } while (false)

#define END_TRACE(fun) TRACE(fun, END)

void Server::throwWithStackTrace() {
  m_stacktraceString = xy::stacktrace();
  cout << m_stacktraceString << endl;
  throw nullptr;
}

void Server::makeError(char *file, int line, char *msg, char *mdb_error) {
  std::stringstream ss;
  ss << file << ":" << line << ": " << msg << ": " << mdb_error;
  m_errorMessage = strdup(ss.str().c_str());
}

#define E(expr) CHECK((rc = (expr)) == MDB_SUCCESS, #expr)
#define CHECK(test, msg) ((test) ? (void)0 : ((void)makeError((char*)__FILE__, __LINE__, (char*)msg, mdb_strerror(rc)), throwWithStackTrace()))

struct CompositeDupKey {
  uint64_t first;
  uint64_t second;
  uint64_t third;
};

auto makeReader(MDB_val data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 && "expected data.mv_size to be a multiple of 8");

  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data, data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}

int lmdbCmpFunction(const MDB_val *a, const MDB_val *b) {
  uint64_t *a1 = static_cast<uint64_t *>(a->mv_data);
  uint64_t *b1 = static_cast<uint64_t *>(b->mv_data);

  if (*a1 < *b1) return -1;
  else if (*a1 > *b1) return 1;
  else return 0;
}

Server::Server(std::string path) : m_lastUserId(0), m_lastBackupId(0), m_lastCardId(0), m_lastDeviceId(0), m_lastSectionId(0), m_lastBranchRequest(0),
  m_traceTimestampOffset(0), m_traceIndex(0), m_errorMessage(nullptr), m_stacktraceString(nullptr)
{
  int rc;
  MDB_txn *txn = nullptr;
  MDB_cursor *cursor = nullptr;
  MDB_val key, data;

  // Create lmdb environment.
  E(mdb_env_create(&m_env));
  E(mdb_env_set_maxreaders(m_env, 126));  // same as default
  E(mdb_env_set_maxdbs(m_env, 26));
  E(mdb_env_set_mapsize(m_env, DEFAULT_DB_SIZE * 1024 * 1024 * 1));
  E(mdb_env_open(m_env, path.c_str(), MDB_NOTLS, 0664));

  // Open and configure databases.
  E(mdb_txn_begin(m_env, NULL, 0, &txn));

  E(mdb_dbi_open(txn, "metadata",                   MDB_CREATE | MDB_INTEGERKEY,                                &m_metadata));
  E(mdb_dbi_open(txn, "deviceForUuid",              MDB_CREATE,                                                 &m_deviceForUuid));
  E(mdb_dbi_open(txn, "uuidForDevice",              MDB_CREATE | MDB_INTEGERKEY,                                &m_uuidForDevice));
  E(mdb_dbi_open(txn, "userForDevice",              MDB_CREATE | MDB_INTEGERKEY,                                &m_userForDevice));
  E(mdb_dbi_open(txn, "discoveredDevicesForDevice", MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &m_discoveredDevicesForDevice));
  E(mdb_dbi_open(txn, "recentCardsForDevice",       MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &m_recentCardsForDevice));
  E(mdb_dbi_open(txn, "cardsForUser",               MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &m_cardsForUser));
  E(mdb_dbi_open(txn, "devicesForUser",             MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &m_devicesForUser));
  E(mdb_dbi_open(txn, "sectionsForUser",            MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &m_sectionsForUser));
  E(mdb_dbi_open(txn, "userForTelephone",           MDB_CREATE | MDB_INTEGERKEY,                                &m_userForTelephone));
  E(mdb_dbi_open(txn, "userForDigitsId",            MDB_CREATE,                                                 &m_userForDigitsId));
  E(mdb_dbi_open(txn, "digitsIdForUser",            MDB_CREATE | MDB_INTEGERKEY,                                &m_digitsIdForUser));
  E(mdb_dbi_open(txn, "user",                       MDB_CREATE | MDB_INTEGERKEY,                                &m_user));
  E(mdb_dbi_open(txn, "userBackup",                 MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &m_userBackup));
  E(mdb_dbi_open(txn, "backup",                     MDB_CREATE | MDB_INTEGERKEY,                                &m_backup));
  E(mdb_dbi_open(txn, "section",                    MDB_CREATE | MDB_INTEGERKEY,                                &m_section));
  E(mdb_dbi_open(txn, "cardForPublicCard",          MDB_CREATE | MDB_INTEGERKEY,                                &m_cardForPublicCard));
  E(mdb_dbi_open(txn, "card",                       MDB_CREATE | MDB_INTEGERKEY,                                &m_card));
  E(mdb_dbi_open(txn, "cardAnalytics",              MDB_CREATE | MDB_INTEGERKEY,                                &m_cardAnalytics));
  E(mdb_dbi_open(txn, "cardAnalyticsExactUniques",  MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &m_cardAnalyticsExactUniques));
  E(mdb_dbi_open(txn, "cardAnalyticsHyperLogLog",   MDB_CREATE | MDB_INTEGERKEY,                                &m_cardAnalyticsHyperLogLog));
  E(mdb_dbi_open(txn, "blocksForUser",              MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &m_blocksForUser));
  E(mdb_dbi_open(txn, "deletionsForUser",           MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &m_deletionsForUser));
  E(mdb_dbi_open(txn, "branchRequest",              MDB_CREATE | MDB_INTEGERKEY,                                &m_branchRequest));
  E(mdb_dbi_open(txn, "discoveryResponseForDevice", MDB_CREATE | MDB_INTEGERKEY,                                &m_discoveryResponseForDevice));
  E(mdb_dbi_open(txn, "syncInfoForDevice",          MDB_CREATE | MDB_INTEGERKEY,                                &m_syncInfoForDevice));

  // Set custom comparison functions.
  E(mdb_set_dupsort(txn, m_discoveredDevicesForDevice, lmdbCmpFunction));
  E(mdb_set_dupsort(txn, m_recentCardsForDevice,       lmdbCmpFunction));
  E(mdb_set_dupsort(txn, m_userBackup,                 lmdbCmpFunction));
  E(mdb_set_dupsort(txn, m_blocksForUser,              lmdbCmpFunction));
  E(mdb_set_dupsort(txn, m_deletionsForUser,           lmdbCmpFunction));

  // Gather metadata.
  uint64_t metadataKey = 0; // databaseId is stored under key 0
  key.mv_size = 8;
  key.mv_data = &metadataKey;
  rc = mdb_get(txn, m_metadata, &key, &data);
  if (rc != 0) {
    if (rc == MDB_NOTFOUND) {
      int64_t tmp;
      arc4random_buf(&tmp, sizeof(tmp));

      key.mv_size = 8;
      key.mv_data = &metadataKey;
      data.mv_size = 8;
      data.mv_data = &tmp;

      rc = mdb_put(txn, m_metadata, &key, &data, 0);
      if (rc != 0) {
        cout << "Failed to insert the generated m_databaseId into metadata";
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
        exit(1);

      } else {
        m_databaseId = tmp;
      }

    } else {
      cout << "Failed to get the m_databaseId from metadata";
      cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
      exit(1);
    }

  } else {
    m_databaseId = *(int64_t *)(data.mv_data);
  }

  assert(m_databaseId != 0 && "m_databaseId must not be zero");

  E(mdb_cursor_open(txn, m_user, &cursor));
  if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
      m_lastUserId = *(uint64_t *)(key.mv_data);
  }
  mdb_cursor_close(cursor);
  cursor = nullptr;

  E(mdb_cursor_open(txn, m_backup, &cursor));
  if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
    m_lastBackupId = *(uint64_t *)(key.mv_data);
  }
  mdb_cursor_close(cursor);
  cursor = nullptr;

  E(mdb_cursor_open(txn, m_card, &cursor));
  if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
    m_lastCardId = *(uint64_t *)(key.mv_data);
  }
  mdb_cursor_close(cursor);
  cursor = nullptr;

  E(mdb_cursor_open(txn, m_uuidForDevice, &cursor));
  if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
    m_lastDeviceId = *(uint64_t *)(key.mv_data);
  }
  mdb_cursor_close(cursor);
  cursor = nullptr;

  E(mdb_cursor_open(txn, m_section, &cursor));
  if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
    m_lastSectionId = *(uint64_t *)(key.mv_data);
  }
  mdb_cursor_close(cursor);
  cursor = nullptr;

  E(mdb_cursor_open(txn, m_branchRequest, &cursor));
  if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
    m_lastBranchRequest = *(uint64_t *)(key.mv_data);
  }
  mdb_cursor_close(cursor);
  cursor = nullptr;

  E(mdb_txn_commit(txn));
  txn = nullptr;

  cout << "m_databaseId = "        << m_databaseId        << endl;
//  cout << "m_lastCardId = "        << m_lastCardId        << endl;
//  cout << "m_lastDeviceId = "      << m_lastDeviceId      << endl;
//  cout << "m_lastSectionId = "     << m_lastSectionId     << endl;
//  cout << "m_lastBranchRequest = " << m_lastBranchRequest << endl;
}

Server::~Server() {
  mdb_env_close(m_env);
}

void Server::handleInvalidMessage(Response::Kind::Builder &responseKind) {
  TRACE_BEGIN(HANDLE_INVALID_MESSAGE);
  auto error = responseKind.initErrorResponse();
  error.setMessage("Need to say Hello first.");
  TRACE_END(HANDLE_INVALID_MESSAGE);
}

void Server::handleRequestNotImplemented(Response::Kind::Builder &responseKind) {
  TRACE_BEGIN(HANDLE_REQUEST_NOT_IMPLEMENTED);
  auto error = responseKind.initErrorResponse();
  error.setMessage("Not implemented.");
  TRACE_END(HANDLE_REQUEST_NOT_IMPLEMENTED);
}

auto Server::handleHelloRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, HelloRequest::Reader helloRequest, ServerCallback *callbacks) -> uint64_t {
  TRACE_BEGIN(HANDLE_HELLO_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  try {
    if (!helloRequest.hasUuid()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("Hello request has no UUID assigned.");
      throwWithStackTrace();
    }

    auto uuidBytes = helloRequest.getUuid().asBytes();

    if (uuidBytes.size() != 16) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("Hello request UUID has the wrong length (should be 16).");
      throwWithStackTrace();
    }

    if (helloRequest.getVersion() == 0) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("Hello request has no version.");
      throwWithStackTrace();
    }

    bool joinRequired = false;
    bool isClientCompatible = false;

    if (helloRequest.getVersion() >= 4) {
      isClientCompatible = true;
    }

    unsigned char publicKeyBytes[32];

    // We now require a public key to be provided.
    if (!helloRequest.hasPublicKey()) {
      isClientCompatible = false;

    } else {
      auto publicKey = helloRequest.getPublicKey();

      for (int idx=0, len=32; idx<len; ++idx) {
        publicKeyBytes[idx] = publicKey[idx];
      }
    }

    E(mdb_txn_begin(m_env, NULL, 0, &txn));

    MDB_val key, data;

    key.mv_size = 16;
    key.mv_data = (void *)uuidBytes.begin();

    rc = mdb_get(txn, m_deviceForUuid, &key, &data);
    if (rc != 0) {
      if (rc == MDB_NOTFOUND) {
        // This is the first time this device has said Hello.
        joinRequired = true;

        // We need to map the UUID to a new device id, and the new device id to the UUID.
        deviceId = ++m_lastDeviceId;

        data.mv_size = 8;
        data.mv_data = &deviceId;

        rc = mdb_put(txn, m_deviceForUuid, &key, &data, 0);
        if (rc != 0) {
          --m_lastDeviceId;

          // An error occurred trying to associate the UUID with the device id.
          cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

          auto error = responseKind.initErrorResponse();
          error.setMessage(mdb_strerror(rc));

          mdb_txn_abort(txn);
          txn = nullptr;

        } else {
          rc = mdb_put(txn, m_uuidForDevice, &data, &key, 0);
          if (rc != 0) {
            --m_lastDeviceId;

            // An error occurred trying to associate the device id with the UUID.
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;

          } else {
            E(mdb_txn_commit(txn));
            txn = nullptr;

            callbacks->setDevicePublicKey(deviceId, publicKeyBytes);

            auto helloResponse = responseKind.initHelloResponse();
            helloResponse.setDeviceId(deviceId);
            helloResponse.setDatabaseID(m_databaseId);
            helloResponse.setIsClientCompatible(isClientCompatible);
            helloResponse.setJoinRequired(joinRequired);
          }
        }

      } else {
        // An error occurred trying to retrieve the device id.
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));
      }

    } else {
      // We do have a device. We need to see if we have a DiscoveryResponse to send back,
      // and determine if a join is required.
      deviceId = *(uint64_t *)data.mv_data;
      callbacks->setDevicePublicKey(deviceId, publicKeyBytes);

      key.mv_size = 8;
      key.mv_data = &deviceId;

      rc = mdb_get(txn, m_userForDevice, &key, &data);
      if (rc == MDB_NOTFOUND) {
        joinRequired = true;

      } else if (rc == 0) {
        uint64_t userId = *(uint64_t *)data.mv_data;

        // Are we the device currently active for this user?
        key.mv_size = 8;
        key.mv_data = &userId;

        rc = mdb_get(txn, m_user, &key, &data);
        if (rc == 0) {
          auto reader = makeReader(data);
          auto existingUser = reader.getRoot<User>();

          if (existingUser.getActiveDevice() != deviceId) {
            joinRequired = true;
          }
        }
      }

      key.mv_size = 8;
      key.mv_data = &deviceId;

      rc = mdb_get(txn, m_discoveryResponseForDevice, &key, &data);
      if (rc != 0 && rc != MDB_NOTFOUND) {
        // An error occurred trying to retrieve an existing discoveryResponse.
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

        // I'm choosing not to send an error for this situation.
        // auto error = responseKind.initErrorResponse();
        // error.setMessage(mdb_strerror(rc));

      } else if (rc == MDB_NOTFOUND) {
        auto helloResponse = responseKind.initHelloResponse();
        helloResponse.setDeviceId(deviceId);
        helloResponse.setDatabaseID(m_databaseId);
        helloResponse.setIsClientCompatible(isClientCompatible);
        helloResponse.setJoinRequired(joinRequired);

      } else {
        auto helloResponse = responseKind.initHelloResponse();
        helloResponse.setDeviceId(deviceId);
        helloResponse.setDatabaseID(m_databaseId);
        helloResponse.setIsClientCompatible(isClientCompatible);
        helloResponse.setJoinRequired(joinRequired);

        auto reader = makeReader(data);
        auto discoveryResponse = reader.getRoot<DiscoveryResponse>();
        helloResponse.setDiscoveryResponse(discoveryResponse);
      }
    }

  } catch (...) {
    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (1)");
    }
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_HELLO_REQUEST);

  } else {
    TRACE_END(HANDLE_HELLO_REQUEST);
  }

  return deviceId;
}

void Server::handleCardRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, CardRequest::Reader cardRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_CARD_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  try {
    uint64_t cardId = cardRequest.getId();
    uint32_t version = cardRequest.getVersion();

    cout << "Card: " << cardId << " version: " << version << endl;

    E(mdb_txn_begin(m_env, NULL, MDB_RDONLY, &txn));

    MDB_val key, data;

    key.mv_size = sizeof(cardId);
    key.mv_data = &cardId;

    rc = mdb_get(txn, m_card, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();

      if (rc == MDB_NOTFOUND) {
        error.setMessage("Card not found.");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

    } else {
      auto reader = makeReader(data);
      auto card = reader.getRoot<Card>();

      auto cardResponse = responseKind.initCardResponse();

      if (card.getVersion() == version) {
        cardResponse.setStatus(CardResponse::Status::CURRENT);

      } else {
        cardResponse.setStatus(CardResponse::Status::UPDATED);
        cardResponse.setCard(card);
      }
    }

  } catch (...) {
    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (2)");
    }
  }

  if (txn != nullptr) {
    E(mdb_txn_commit(txn));
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_CARD_REQUEST);

  } else {
    TRACE_END(HANDLE_CARD_REQUEST);
  }
}

void Server::handleJoinRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, JoinRequest::Reader joinRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_JOIN_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  bool createdUser = false;

  try {
    E(mdb_txn_begin(m_env, NULL, 0, &txn));

    MDB_val key, data;

    uint64_t userId = 0;

    // Look up userId by digitsId.
    const char *digitsId = joinRequest.getDigitsId().cStr();

    key.mv_size = strlen(digitsId);
    key.mv_data = (void *)digitsId;

    rc = mdb_get(txn, m_userForDigitsId, &key, &data);
    if (rc != 0 && rc != MDB_NOTFOUND) {
      cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else if (rc == MDB_NOTFOUND) {
      // We don't have a userId yet. Create one and remember it.
      userId = ++m_lastUserId;
      createdUser = true;

      key.mv_size = strlen(digitsId);
      key.mv_data = (void *)digitsId;

      data.mv_size = sizeof(userId);
      data.mv_data = &userId;

      rc = mdb_put(txn, m_userForDigitsId, &key, &data, MDB_NOOVERWRITE);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      data.mv_size = strlen(digitsId);
      data.mv_data = (void *)digitsId;

      rc = mdb_put(txn, m_digitsIdForUser, &key, &data, MDB_NOOVERWRITE);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

      // Also enable userId lookup by telephone (in case we need it in the future).
      unsigned long long telephone = std::stoull(joinRequest.getTelephone().cStr());

      key.mv_size = sizeof(telephone);
      key.mv_data = &telephone;

      data.mv_size = sizeof(userId);
      data.mv_data = &userId;

      rc = mdb_put(txn, m_userForTelephone, &key, &data, MDB_NOOVERWRITE);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

    } else {
      // We have a pre-existing userId.
      userId = *(uint64_t *)data.mv_data;
    }

    // Associate the device with the user.
    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    data.mv_size = sizeof(deviceId);
    data.mv_data = &deviceId;

    rc = mdb_put(txn, m_devicesForUser, &key, &data, 0);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // Associate the user with the device.
    // TODO: Should we be verifying that userForDevice, if already set, matches?
    key.mv_size = sizeof(deviceId);
    key.mv_data = &deviceId;

    data.mv_size = sizeof(userId);
    data.mv_data = &userId;

    rc = mdb_put(txn, m_userForDevice, &key, &data, 0);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // Make sure we have a User object, and that it is up-to-date.
    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    data.mv_size = sizeof(deviceId);
    data.mv_data = &deviceId;

    rc = mdb_get(txn, m_user, &key, &data);
    if (rc != 0 && rc != MDB_NOTFOUND) {
      cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else if (rc == MDB_NOTFOUND) {
//      cout << "This is a new user." << endl;

      capnp::MallocMessageBuilder userBuilder;
      User::Builder newUser = userBuilder.getRoot<User>();

      newUser.setId(userId);
      newUser.setActiveDevice(deviceId);

      newUser.setTelephone(joinRequest.getTelephone());
      newUser.setEmail(joinRequest.getEmail());
      newUser.setEmailVerified(joinRequest.getEmailVerified());

      key.mv_size = sizeof(deviceId);
      key.mv_data = &deviceId;

      rc = mdb_get(txn, m_uuidForDevice, &key, &data);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

      char *uuidData = (char *)data.mv_data;
      auto uuid = newUser.initActiveDeviceUUID(16);
      for (int idx=0, len=16; idx<len; ++idx) {
        uuid[idx] = uuidData[idx];
      }

      kj::Array<capnp::word> words = capnp::messageToFlatArray(userBuilder);
      kj::ArrayPtr<kj::byte> bytes = words.asBytes();
      size_t size = bytes.size();
      char *from = (char *)(bytes.begin());

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      data.mv_size = size;
      data.mv_data = from;

      rc = mdb_put(txn, m_user, &key, &data, MDB_NOOVERWRITE);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));
        throwWithStackTrace();
      }

      auto joinResponse = responseKind.initJoinResponse();
      joinResponse.setStatus(JoinResponse::Status::NEW);

      joinResponse.setUser(newUser);

      E(mdb_txn_commit(txn));
      txn = nullptr;

    } else {
//      cout << "This is an existing user." << endl;

      auto reader = makeReader(data);
      auto existingUser = reader.getRoot<User>();

      auto joinResponse = responseKind.initJoinResponse();
      joinResponse.setStatus(JoinResponse::Status::EXISTING);
        
      if (existingUser.getActiveDevice() != deviceId) {
//        cout << "Device is not the active device" << endl;

        capnp::MallocMessageBuilder userBuilder;
        userBuilder.setRoot(existingUser);
        User::Builder updatedUser = userBuilder.getRoot<User>();

        updatedUser.setActiveDevice(deviceId);

        // Need to also update the activeDeviceUUID
        key.mv_size = sizeof(deviceId);
        key.mv_data = &deviceId;
        rc = mdb_get(txn, m_uuidForDevice, &key, &data);
        if (rc != 0) {
          // This is an error: we should have the UUID bytes for this device at this point.
          cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

          auto error = responseKind.initErrorResponse();
          error.setMessage(mdb_strerror(rc));

          mdb_txn_abort(txn);
          txn = nullptr;
          throwWithStackTrace();

        } else {
          char *uuidData = (char *)data.mv_data;
          auto uuid = updatedUser.initActiveDeviceUUID(16);
          for (int idx=0, len=16; idx<len; ++idx) {
            uuid[idx] = uuidData[idx];
          }
        }

        kj::Array<capnp::word> words = capnp::messageToFlatArray(userBuilder);
        kj::ArrayPtr<kj::byte> bytes = words.asBytes();
        size_t size = bytes.size();
        char *from = (char *)(bytes.begin());

        key.mv_size = sizeof(userId);
        key.mv_data = &userId;

        data.mv_size = size;
        data.mv_data = from;

        rc = mdb_put(txn, m_user, &key, &data, 0);
        if (rc != 0) {
          cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;

          auto error = responseKind.initErrorResponse();
          error.setMessage(mdb_strerror(rc));
          throwWithStackTrace();
        }

        joinResponse.setUser(updatedUser);

      } else {
        joinResponse.setUser(existingUser);
      }

      // We assume users only have one card.
      E(mdb_cursor_open(txn, m_cardsForUser, &cursor));

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
      if (rc == 0) {
        rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST_DUP);
        if (rc == 0) {
          uint64_t foundUserId = *(uint64_t *)key.mv_data;
          if (foundUserId == userId) {
            uint64_t cardId = *(uint64_t *)data.mv_data;
            key.mv_data = &cardId;

            rc = mdb_get(txn, m_card, &key, &data);
            if (rc != 0) {
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

              auto error = responseKind.initErrorResponse();
              error.setMessage(mdb_strerror(rc));
              throwWithStackTrace();
            }

            auto reader = makeReader(data);
            auto userCard = reader.getRoot<Card>();
            joinResponse.setCard(userCard);
          }
        }
      }
      mdb_cursor_close(cursor);
      cursor = nullptr;

//      // Count sections
//      size_t count = 0;
//      E(mdb_cursor_open(txn, m_sectionsForUser, &cursor));
//      if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
//        do { count++; } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0);
//      }
//
//      if (count > 0) {
//        // Copy the sections into the JoinResponse.
//        auto sectionList = joinResponse.initSections(count);
//        count = 0;
//
//        MDB_val key2, data2;
//        key2.mv_size = 8;
//
//        if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
//          do {
//            key2.mv_data = data.mv_data;
//
//            rc = mdb_get(txn, m_section, &key2, &data2);
//            if (rc == 0) {
//              auto reader = makeReader(data2);
//              auto section = reader.getRoot<Section>();
//
//              sectionList[count].setInfo(section.getInfo());
//              sectionList[count].setCards(section.getCards());
//            }
//            count++;
//          } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0);
//        }
//      }
//
//      mdb_cursor_close(cursor);
//      cursor = nullptr;

      E(mdb_txn_commit(txn));
      txn = nullptr;
    }

  } catch (...) {
    if (createdUser) --m_lastUserId; // Undo increment.

    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (4)");
    }
  }

  if (cursor != nullptr) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_JOIN_REQUEST);

  } else {
    TRACE_END(HANDLE_JOIN_REQUEST);
  }
}

void Server::handleCreateCardRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, CreateCardRequest::Reader createCardRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_CREATE_CARD_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  bool createdCard = false;
  bool createdBranchRequest = false;

  try {
//    cout << "Create Card: " << deviceId << endl;

    E(mdb_txn_begin(m_env, NULL, 0, &txn));

    uint64_t userId = 0;

    MDB_val key, data;

    key.mv_size = sizeof(deviceId);
    key.mv_data = &deviceId;

    rc = mdb_get(txn, m_userForDevice, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();
      if (rc == MDB_NOTFOUND) {
        cout << "User has not joined Blue yet 1" << endl;
        error.setMessage("User has not joined Blue yet! (1)");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      userId = *(uint64_t *)data.mv_data;
    }

    // Validation:

    // 1. User cannot have already created a card.
    bool hasCardAlready = false;

    // We assume users only have one card.
    E(mdb_cursor_open(txn, m_cardsForUser, &cursor));

    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
    if (rc == 0) {
      rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST_DUP);
      if (rc == 0) {
        uint64_t foundUserId = *(uint64_t *)key.mv_data;
        if (foundUserId == userId) hasCardAlready = true;
      }
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    if (hasCardAlready) {
      cout << "User already has a card!" << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage("User already has a card!");

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // 2. Card must have id = 0 and version = 0.
    auto desiredCard = createCardRequest.getCard();
    if (desiredCard.getId() != 0 || desiredCard.getVersion() != 0) {
      cout << "Card has an id or version not equal to zero" << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage("Card has an id or version not equal to zero");

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // TODO: Additional card validation here?

    // Okay, ready to insert. First generate a new card id.
//    cout << "lastCardId prior to incrementing: " << m_lastCardId << endl;
    uint64_t cardId = ++m_lastCardId;
    createdCard = true;
    //    cout << "lastCardId after incrementing: " << m_lastCardId << endl;
//    cout << "cardId: " << cardId << endl;

    // Insert the userId->cardId mapping in the database.
    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    data.mv_size = sizeof(cardId);
    data.mv_data = &cardId;

    // We currently only allow one card per user, hence the MDB_NODUPDATA.
    rc = mdb_put(txn, m_cardsForUser, &key, &data, MDB_NOOVERWRITE | MDB_NODUPDATA);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // Update the card with the correct cardId and version.
    capnp::MallocMessageBuilder cardBuilder;
    cardBuilder.setRoot(desiredCard);
    // Copies card--it'd be nice to do the updates in place, but I'm not
    // sure how to make that happen.

    Card::Builder newCard = cardBuilder.getRoot<Card>();
    newCard.setId(cardId);
    newCard.setVersion(1);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(cardBuilder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    key.mv_size = sizeof(cardId);
    key.mv_data = &cardId;

    data.mv_size = size;
    data.mv_data = from;

    rc = mdb_put(txn, m_card, &key, &data, 0);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // Update the user with the correct card.
    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    rc = mdb_get(txn, m_user, &key, &data);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      auto reader = makeReader(data);
      auto existingUser = reader.getRoot<User>();

      // Update the user with the correct cardId.
      capnp::MallocMessageBuilder builder;
      builder.setRoot(existingUser);
      // Copies user--it'd be nice to do the updates in place, but I'm not
      // sure how to make that happen.

      User::Builder newUser = builder.getRoot<User>();
      newUser.setCard(cardId);

      kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
      kj::ArrayPtr<kj::byte> bytes = words.asBytes();
      size_t size = bytes.size();
      char *from = (char *)(bytes.begin());

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      data.mv_size = size;
      data.mv_data = from;

      rc = mdb_put(txn, m_user, &key, &data, 0);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }
    }

    // Insert Branch.io request into Branch.io worker queue.
    stringstream ss;
    if (desiredCard.hasLocation()) {
      ss << "{ \"cardId\": " << cardId << ", \"fullName\": \"" << desiredCard.getFullName().cStr() << "\", \"location\": \"" << desiredCard.getLocation().cStr() << "\" }";

    } else {
      ss << "{ \"cardId\": " << cardId << ", \"fullName\": \"" << desiredCard.getFullName().cStr() << "\" }";
    }
    string request = ss.str();
    const char *cRequest = request.c_str();

    uint64_t branchRequest = ++m_lastBranchRequest;
    createdBranchRequest = true;

    key.mv_size = sizeof(branchRequest);
    key.mv_data = &branchRequest;

    data.mv_size = strlen(cRequest);
    data.mv_data = (void *)cRequest;

    rc = mdb_put(txn, m_branchRequest, &key, &data, 0);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    auto cardResponse = responseKind.initCardResponse();
    cardResponse.setCard(newCard);
    cardResponse.setStatus(CardResponse::Status::CREATED);

    E(mdb_txn_commit(txn));
    txn = nullptr;

  } catch (...) {
    if (createdCard) m_lastCardId--; // Undo increment.
    if (createdBranchRequest) m_lastBranchRequest--; // Undo increment.

    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (5)");
    }
  }

  if (cursor != nullptr) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_CREATE_CARD_REQUEST);

  } else {
    TRACE_END(HANDLE_CREATE_CARD_REQUEST);
  }
}

void Server::handleUpdateCardRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, UpdateCardRequest::Reader updateCardRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_UPDATE_CARD_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  bool createdBranchRequest = false;

  try {
//    cout << "Update Card: " << deviceId << endl;

    E(mdb_txn_begin(m_env, NULL, 0, &txn));

    uint64_t userId = 0;

    MDB_val key, data;

    key.mv_size = sizeof(deviceId);
    key.mv_data = &deviceId;

    rc = mdb_get(txn, m_userForDevice, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();
      if (rc == MDB_NOTFOUND) {
        cout << "User has not joined Blue yet 2" << endl;
        error.setMessage("User has not joined Blue yet! (2)");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      userId = *(uint64_t *)data.mv_data;
    }

    // Validation:

    // 1. User must have already created a card.
    E(mdb_cursor_open(txn, m_cardsForUser, &cursor));

    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    uint64_t cardId = updateCardRequest.getCard().getId();

    data.mv_size = sizeof(cardId);
    data.mv_data = &cardId;

    bool hasCardAlready = false;

    // We assume users only have one card.
    rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
    if (rc == 0) {
      rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST_DUP);
      if (rc == 0) {
        uint64_t foundUserId = *(uint64_t *)key.mv_data;
        uint64_t foundCardId = *(uint64_t *)data.mv_data;
        if (foundUserId == userId && foundCardId == cardId) hasCardAlready = true;
      }
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    if (!hasCardAlready) {
      cout << "User has not created a card!" << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage("User has not created a card!");

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    key.mv_data = &cardId;

    rc = mdb_get(txn, m_card, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();
      if (rc == MDB_NOTFOUND) {
        cout << "Card not found?" << endl;
        error.setMessage("Card not found?");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    auto reader = makeReader(data);
    auto existingCard = reader.getRoot<Card>();

    // 2. Updated card must have the same id and version as the existing card.
    auto desiredCard = updateCardRequest.getCard();
    if (desiredCard.getId() != cardId || desiredCard.getVersion() != existingCard.getVersion()) {
      cout << "Existing card has a different id or version." << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage("Existing card has a different id or version.");

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // TODO: Additional card validation here?

    // Update the card with the correct cardId and version.
    capnp::MallocMessageBuilder builder;
    builder.setRoot(desiredCard);
    // Copies card--it'd be nice to do the updates in place, but I'm not
    // sure how to make that happen.

    Card::Builder newCard = builder.getRoot<Card>();
    newCard.setVersion(existingCard.getVersion() + 1);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    key.mv_data = &cardId;
    data.mv_size = size;
    data.mv_data = from;

    rc = mdb_put(txn, m_card, &key, &data, 0);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

    // Insert Branch.io request into Branch.io worker queue.
    stringstream ss;
    if (desiredCard.hasLocation()) {
      ss << "{ \"cardId\": " << cardId << ", \"fullName\": \"" << desiredCard.getFullName().cStr() << "\", \"location\": \"" << desiredCard.getLocation().cStr() << "\", \"isUpdate\": true }";

    } else {
      ss << "{ \"cardId\": " << cardId << ", \"fullName\": \"" << desiredCard.getFullName().cStr() << "\", \"isUpdate\": true }";
    }
    string request = ss.str();
    const char *cRequest = request.c_str();

    uint64_t branchRequest = ++m_lastBranchRequest;
    createdBranchRequest = true;

    key.mv_size = sizeof(branchRequest);
    key.mv_data = &branchRequest;

    data.mv_size = strlen(cRequest);
    data.mv_data = (void *)cRequest;

    rc = mdb_put(txn, m_branchRequest, &key, &data, 0);
    if (rc != 0) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();
    }

//    cout << "Updated card: " << cardId << endl;
    auto cardResponse = responseKind.initCardResponse();
    cardResponse.setCard(newCard);
    cardResponse.setStatus(CardResponse::Status::UPDATED);

    E(mdb_txn_commit(txn));
    txn = nullptr;

  } catch (...) {
    if (createdBranchRequest) m_lastBranchRequest--;

    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (6)");
    }
  }

  if (cursor != nullptr) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_UPDATE_CARD_REQUEST);

  } else {
    TRACE_END(HANDLE_UPDATE_CARD_REQUEST);
  }
}

void Server::handleCreateBackupRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, CreateBackupRequest::Reader createBackupRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_CREATE_BACKUP_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  bool createdBackup = false;

  try {
    E(mdb_txn_begin(m_env, NULL, 0, &txn));

    uint64_t userId = 0;

    MDB_val key, data;

    key.mv_size = sizeof(deviceId);
    key.mv_data = &deviceId;

    rc = mdb_get(txn, m_userForDevice, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();
      if (rc == MDB_NOTFOUND) {
        cout << "User has not joined Blue yet 3" << endl;
        error.setMessage("User has not joined Blue yet! (3)");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      userId = *(uint64_t *)data.mv_data;
    }

    int64_t mostRecentBackup = 0;
    E(mdb_cursor_open(txn, m_userBackup, &cursor));

    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
    if (rc == 0) {
      rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST_DUP);
      if (rc == 0) {
        uint64_t foundUserId = *(uint64_t *)key.mv_data;
        if (foundUserId == userId) {
          CompositeDupKey dupKey = *(CompositeDupKey *)data.mv_data;
          mostRecentBackup = dupKey.first;
        }
      }

      mdb_cursor_close(cursor);
      cursor = nullptr;

    } else if (rc != MDB_NOTFOUND) {
      cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
      auto error = responseKind.initErrorResponse();
      error.setMessage(mdb_strerror(rc));

      mdb_cursor_close(cursor);
      cursor = nullptr;

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      // We couldn't find the key, so mostRecentBackup == 0 is correct.
      mdb_cursor_close(cursor);
      cursor = nullptr;
    }

    if (mostRecentBackup != createBackupRequest.getPreviousBackup()) {
      auto error = responseKind.initErrorResponse();
      error.setCode(ErrorResponse::Code::PREVIOUS_BACKUP_NOT_APPLIED);
      error.setMessage("You need to apply the previous backup first.");

      mdb_txn_abort(txn);
      txn = nullptr;

    } else {
      auto desiredBackup = createBackupRequest.getBackup();

      auto backupId = ++m_lastBackupId;
      createdBackup = true;

      CompositeDupKey dupKey;
      dupKey.first = desiredBackup.getTimestamp();
      dupKey.second = backupId;

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      data.mv_size = 16;
      data.mv_data = &dupKey;

      rc = mdb_put(txn, m_userBackup, &key, &data, MDB_NODUPDATA);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

      key.mv_size = sizeof(backupId);
      key.mv_data = &backupId;

      auto bytes = desiredBackup.getData().asBytes();
      size_t size = bytes.size();
      char *from = (char *)(bytes.begin());

      data.mv_size = size;
      data.mv_data = from;

      rc = mdb_put(txn, m_backup, &key, &data, MDB_NOOVERWRITE);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << endl;
        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

      auto backupResponse = responseKind.initBackupResponse();
      auto backup = backupResponse.initBackup();
      backup.setTimestamp(createBackupRequest.getBackup().getTimestamp());
      
      E(mdb_txn_commit(txn));
      txn = nullptr;
    }

  } catch (...) {
    if (createdBackup) m_lastBackupId--;

    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (7)");
    }
  }

  if (cursor != nullptr) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_CREATE_BACKUP_REQUEST);

  } else {
    TRACE_END(HANDLE_CREATE_BACKUP_REQUEST);
  }
}

void Server::handleBackupListRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, BackupListRequest::Reader backupListRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_BACKUP_LIST_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  try {
    E(mdb_txn_begin(m_env, NULL, MDB_RDONLY, &txn));

    uint64_t userId = 0;

    MDB_val key, data;

    key.mv_size = sizeof(deviceId);
    key.mv_data = &deviceId;

    rc = mdb_get(txn, m_userForDevice, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();
      if (rc == MDB_NOTFOUND) {
        cout << "User has not joined Blue yet 4" << endl;
        error.setMessage("User has not joined Blue yet! (4)");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      userId = *(uint64_t *)data.mv_data;
    }

    size_t count = 0;

    E(mdb_cursor_open(txn, m_userBackup, &cursor));

    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
    if (rc == 0) {
      rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST_DUP);
      while (rc == 0) {
        uint64_t foundUserId = *(uint64_t *)key.mv_data;
        if (foundUserId != userId) break;
        count++;
        rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP);
      }
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    auto backupListResponse = responseKind.initBackupListResponse();
    auto backups = backupListResponse.initBackups(count);

    E(mdb_cursor_open(txn, m_userBackup, &cursor));

    key.mv_size = sizeof(userId);
    key.mv_data = &userId;

    size_t idx = 0;
    rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
    if (rc == 0) {
      rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST_DUP);
      while (rc == 0) {
        uint64_t foundUserId = *(uint64_t *)key.mv_data;
        if (foundUserId != userId) break;

        CompositeDupKey dupKey = *(CompositeDupKey *)data.mv_data;
        auto backup = backups[idx++];
        backup.setTimestamp(dupKey.first);

        rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP);
      }
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    mdb_txn_abort(txn);
    txn = nullptr;

  } catch (...) {
    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (8)");
    }
  }

  if (cursor != nullptr) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_BACKUP_LIST_REQUEST);

  } else {
    TRACE_END(HANDLE_BACKUP_LIST_REQUEST);
  }
}

void Server::handleBackupRequest(Response::Kind::Builder &responseKind, uint64_t deviceId, BackupRequest::Reader backupRequest, ServerCallback *callbacks) {
  TRACE_BEGIN(HANDLE_BACKUP_REQUEST);

  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  try {
    E(mdb_txn_begin(m_env, NULL, MDB_RDONLY, &txn));

    uint64_t userId = 0;

    MDB_val key, data;

    key.mv_size = sizeof(deviceId);
    key.mv_data = &deviceId;

    rc = mdb_get(txn, m_userForDevice, &key, &data);
    if (rc != 0) {
      auto error = responseKind.initErrorResponse();
      if (rc == MDB_NOTFOUND) {
        cout << "User has not joined Blue yet 5" << endl;
        error.setMessage("User has not joined Blue yet! (5)");

      } else {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        error.setMessage(mdb_strerror(rc));
      }

      mdb_txn_abort(txn);
      txn = nullptr;
      throwWithStackTrace();

    } else {
      userId = *(uint64_t *)data.mv_data;
    }

    int64_t timestamp = backupRequest.getTimestamp();

    uint64_t backupId = 0;

    if (timestamp == 0) {
      E(mdb_cursor_open(txn, m_userBackup, &cursor));

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      rc = mdb_cursor_get(cursor, &key, &data, MDB_SET);
      if (rc == 0) {
        rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST_DUP);
        if (rc == 0) {
          uint64_t foundUserId = *(uint64_t *)key.mv_data;
          if (foundUserId == userId) {
            CompositeDupKey dupKey = *(CompositeDupKey *)data.mv_data;
            timestamp = dupKey.first;
            backupId = dupKey.second;
          }
        }
      }
      mdb_cursor_close(cursor);
      cursor = nullptr;

    } else {
      E(mdb_cursor_open(txn, m_userBackup, &cursor));

      key.mv_size = sizeof(userId);
      key.mv_data = &userId;

      CompositeDupKey dupKey;
      dupKey.first = timestamp;

      data.mv_size = 16;
      data.mv_data = &dupKey;

      if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_GET_BOTH)) == 0) {
        uint64_t foundUserId = *(uint64_t *)key.mv_data;
        if (foundUserId == userId) {
          CompositeDupKey dupKey2 = *(CompositeDupKey *)data.mv_data;
          if (dupKey2.first == timestamp) {
            backupId = dupKey2.second;
          }
        }
      }
      mdb_cursor_close(cursor);
      cursor = nullptr;
    }

    if (backupId == 0 && timestamp == 0) {
      auto backupResponse = responseKind.initBackupResponse();
      auto backup = backupResponse.initBackup();
      backup.setTimestamp(timestamp);

      mdb_txn_abort(txn);
      txn = nullptr;

    } else if (backupId == 0) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("Backup not found.");

    } else {
      key.mv_size = sizeof(backupId);
      key.mv_data = &backupId;

      rc = mdb_get(txn, m_backup, &key, &data);
      if (rc != 0) {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;
        auto error = responseKind.initErrorResponse();
        error.setMessage(mdb_strerror(rc));

        mdb_txn_abort(txn);
        txn = nullptr;
        throwWithStackTrace();
      }

      auto backupResponse = responseKind.initBackupResponse();
      auto backup = backupResponse.initBackup();
      backup.setTimestamp(timestamp);

      // TODO(Erich): Surely there's a faster way to do this?
      unsigned char *backupDataBuffer = (unsigned char *)data.mv_data;
      auto backupData = backup.initData(data.mv_size);
      for (size_t idx=0, len=data.mv_size; idx<len; ++idx) {
        backupData[idx] = backupDataBuffer[idx];
      }

      mdb_txn_abort(txn);
      txn = nullptr;
    }

  } catch (...) {
    if (!responseKind.hasErrorResponse()) {
      auto error = responseKind.initErrorResponse();
      error.setMessage("An unknown error occurred. (9)");
    }
  }

  if (cursor != nullptr) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }

  if (m_stacktraceString != nullptr) {
    TRACE_THROW(HANDLE_BACKUP_REQUEST);

  } else {
    TRACE_END(HANDLE_BACKUP_REQUEST);
  }
}

void Server::handleBranchMessage(const char *msg, size_t len, ServerCallback *callbacks) {
  int rc;
  MDB_txn *txn = nullptr;

  MDB_cursor *cursor = nullptr;

  try {
    std::string msgStr(msg, len);
    unsigned long long branchRequestId = std::stoull(msgStr.c_str());
//    cout << "branchRequestId as 64-bit: " << branchRequestId << endl;

    if (branchRequestId != 0) cout << "Got Branch request: " << branchRequestId << endl;

    E(mdb_txn_begin(m_env, NULL, 0, &txn));

    MDB_val key, data;

    if (branchRequestId == 0) {
      // This is a request to get the next branchRequest.
      E(mdb_cursor_open(txn, m_branchRequest, &cursor));

      rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST);
      if (rc == 0) {
        branchRequestId = *(unsigned long long *)key.mv_data;

        char *branchRequest = strndup((char *)data.mv_data, data.mv_size);

        stringstream ss;
        ss << "{ \"id\": " << branchRequestId << ", \"request\": " << branchRequest << " }";
        string request = ss.str();
        const char *cRequest = request.c_str();
        callbacks->sendBranchMessage(cRequest, strlen(cRequest));

      } else {
        char *okMessage = (char *)"{}";
        callbacks->sendBranchMessage(okMessage, 2);
      }

      mdb_cursor_close(cursor);
      cursor = nullptr;

      mdb_txn_abort(txn);
      txn = nullptr;

    } else {
      key.mv_size = sizeof(branchRequestId);
      key.mv_data = &branchRequestId;

      rc = mdb_del(txn, m_branchRequest, &key, NULL);
      if (rc != 0) {
        if (rc == MDB_NOTFOUND) {
          cout << "Unknown branchRequestId? " << branchRequestId << endl;

          mdb_txn_abort(txn);
          txn = nullptr;

        } else {
          // An error occurred trying to delete branchRequestId.
          cout << "An error occurred trying to delete branchRequestId = " << branchRequestId << endl;
          cout << __LINE__ << " - mdb_del() error: " << mdb_strerror(rc) << endl;

          mdb_txn_abort(txn);
          txn = nullptr;
          throwWithStackTrace();
        }
      }

      E(mdb_txn_commit(txn));
      txn = nullptr;

      char *okMessage = (char *)"OK";
      callbacks->sendBranchMessage(okMessage, 2);
    }

  } catch (...) {
    mdb_txn_abort(txn);
    txn = nullptr;

    char *errorMessage = (char *)"ERROR";
    callbacks->sendBranchMessage(errorMessage, 5);
  }

  if (cursor) {
    mdb_cursor_close(cursor);
    cursor = nullptr;
  }

  if (txn != nullptr) {
    mdb_txn_abort(txn);
    txn = nullptr;
  }
}

void Server::handleClientMessage(uint64_t deviceId, const char *msg, size_t len, bool discoveryAvailable, ServerCallback *callbacks) {
  CREATE_TRACE(HANDLE_CLIENT_MESSAGE);

  TRACE_BEGIN(READ_REQUEST);
  kj::ArrayPtr<const capnp::word> view((const capnp::word *)msg, len / 8);
  auto requestReader = capnp::FlatArrayMessageReader(view);
  Request::Reader request = requestReader.getRoot<Request>();
  int64_t requestId = request.getId();
  auto requestKind = request.getKind();
  auto which = requestKind.which();
  TRACE_END(READ_REQUEST);

  // We *always* send a response.
  TRACE_BEGIN(PREPARE_RESPONSE);
  capnp::MallocMessageBuilder responseBuilder;
  Response::Builder response = responseBuilder.initRoot<Response>();
  response.setId(requestId);
  Response::Kind::Builder responseKind = response.getKind();
  TRACE_END(PREPARE_RESPONSE);

  // The first request MUST be a Hello request.
  if (deviceId == 0 && which != Request::Kind::HELLO_REQUEST) {
    cout << "Got invalid message: " << requestId << endl;
    handleInvalidMessage(responseKind);

  } else {
    switch (which) {
      case Request::Kind::HELLO_REQUEST:
        cout << "Got Hello request: " << requestId << endl;
        deviceId = handleHelloRequest(responseKind, deviceId, requestKind.getHelloRequest(), callbacks);
        break;

      case Request::Kind::CARD_REQUEST:
        cout << "Got Card request: " << requestId << endl;
        handleCardRequest(responseKind, deviceId, requestKind.getCardRequest(), callbacks);
        break;

        case Request::Kind::JOIN_REQUEST:
        cout << "Got Join request: " << requestId << endl;
        handleJoinRequest(responseKind, deviceId, requestKind.getJoinRequest(), callbacks);
        break;

        case Request::Kind::CREATE_CARD_REQUEST:
        cout << "Got Create Card request: " << requestId << endl;
        handleCreateCardRequest(responseKind, deviceId, requestKind.getCreateCardRequest(), callbacks);
        break;

      case Request::Kind::UPDATE_CARD_REQUEST:
        cout << "Got Update Card request: " << requestId << endl;
        handleUpdateCardRequest(responseKind, deviceId, requestKind.getUpdateCardRequest(), callbacks);
        break;

      case Request::Kind::CREATE_BACKUP_REQUEST:
        cout << "Got Create Backup request: " << requestId << endl;
        handleCreateBackupRequest(responseKind, deviceId, requestKind.getCreateBackupRequest(), callbacks);
        break;

      case Request::Kind::BACKUP_LIST_REQUEST:
        cout << "Got Backup List request: " << requestId << endl;
        handleBackupListRequest(responseKind, deviceId, requestKind.getBackupListRequest(), callbacks);
        break;

      case Request::Kind::BACKUP_REQUEST:
        cout << "Got Backup request: " << requestId << endl;
        handleBackupRequest(responseKind, deviceId, requestKind.getBackupRequest(), callbacks);
        break;

      default:
        cout << "Unknown request kind" << (int)which << endl;
        handleInvalidMessage(responseKind);
    }
  }

  // We *always* send a response.
  TRACE_BEGIN(FINALIZE_RESPONSE);
  kj::Array<capnp::word> responseWords = capnp::messageToFlatArray(responseBuilder);
  kj::ArrayPtr<kj::byte> responseBytes = responseWords.asBytes();
  TRACE_END(FINALIZE_RESPONSE);

  TRACE_BEGIN(SEND_RESPONSE);
  callbacks->sendDeviceMessage(deviceId, (const char *)responseBytes.begin(), responseBytes.size());
  TRACE_END(SEND_RESPONSE);

  END_TRACE(HANDLE_CLIENT_MESSAGE);

  finalizeTrace(deviceId, requestId, msg, len, callbacks);
}

void Server::finalizeTrace(uint64_t deviceId, int64_t requestId, const char *msg, size_t len, ServerCallback *callbacks) {
  TRACE_BEGIN(PREPARE_TRACE);
  capnp::MallocMessageBuilder traceBuilder;
  Trace::Builder trace = traceBuilder.initRoot<Trace>();

  trace.setDeviceId(deviceId);
  trace.setRequestId(requestId);

  long long millisecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  trace.setTimestamp(millisecondsSinceEpoch);

  if (m_stacktraceString) {
    trace.setStackTrace(m_stacktraceString);

    auto requestData = trace.initRequestData(static_cast<uint32_t>(len));
    for (int idx=0; idx<len; ++idx) {
      requestData[idx] = msg[idx];
    }
  }

  if (m_errorMessage != nullptr) {
    trace.setError(m_errorMessage);
  }

  uint64_t offset = m_traceTimestampOffset;
  auto events = trace.initEvents(m_traceIndex + 1);
  for (unsigned int idx=0, len=m_traceIndex; idx<len; ++idx) {
    RequestEvent *evt = m_eventTraces + idx;
    events[idx].setTimestamp(evt->timestamp - offset);
    events[idx].setFunction(evt->function);
    events[idx].setType(evt->eventType);
  }

  // Simulate TRACE_END(PREPARE_TRACE);
  events[m_traceIndex].setFunction(Trace::Event::Function::PREPARE_TRACE);
  events[m_traceIndex].setType(Trace::Event::Type::END);
  events[m_traceIndex].setTimestamp(rdtscp() - offset);

  trace.setDuration(events[m_traceIndex].getTimestamp());

  // Publish trace.
  kj::Array<capnp::word> traceWords = capnp::messageToFlatArray(traceBuilder);
  kj::ArrayPtr<kj::byte> traceBytes = traceWords.asBytes();

  callbacks->sendTraceMessage(traceBytes.begin(), traceBytes.size());

  // Clean up error messages, if any.
  if (m_errorMessage != nullptr) {
    free(m_errorMessage);
    m_errorMessage = nullptr;
  }
  
  if (m_stacktraceString != nullptr) {
    free(m_stacktraceString);
    m_stacktraceString = nullptr;
  }
}
