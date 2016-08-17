//
//  main.cpp
//  Follow
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
#include <string>
#include <thread>

#include "external/lmdb/lmdb.h"

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/util/CommandOptionParser.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "libuv/uv.h"

#include "schema/request.capnp.h"
#include "schema/response.capnp.h"
#include "schema/rpc.capnp.h"
#include "schema/user.capnp.h"

#include "uWebSockets/uWS.h"

using namespace std;
using namespace aeron::util;
using namespace aeron;

const static int32_t DEFAULT_DB_SIZE              = 1024; // 1GB memory-mapped database

const static string  DEFAULT_SERVER_CHANNEL       = "aeron:udp?endpoint=localhost:40123";
const static string  DEFAULT_ENGINE_CHANNEL       = "aeron:udp?endpoint=localhost:40124";
const static int32_t DEFAULT_SERVER_STREAM_ID     = 10;
const static int32_t DEFAULT_ENGINE_STREAM_ID     = 10;
const static int     DEFAULT_FRAGMENT_COUNT_LIMIT = 10;

static atomic<bool> shouldPoll (false);

static shared_ptr<Publication>  serverPublication  = nullptr;
static shared_ptr<Subscription> engineSubscription = nullptr;

static const char optHelp = 'h';

// Aeron options
static const char optPrefix         = 'p';
static const char optServerChannel  = 'c';
static const char optEngineChannel  = 'C';
static const char optServerStreamId = 's';
static const char optEngineStreamId = 'S';
static const char optFrags          = 'f';

struct Settings {
  string  dirPrefix          = "/Users/erich/Desktop/aeron";
  string  serverChannel      = DEFAULT_SERVER_CHANNEL;
  string  engineChannel      = DEFAULT_ENGINE_CHANNEL;
  int32_t serverStreamId     = DEFAULT_SERVER_STREAM_ID;
  int32_t engineStreamId     = DEFAULT_ENGINE_STREAM_ID;
  int32_t fragmentCountLimit = DEFAULT_FRAGMENT_COUNT_LIMIT;
};

static Settings settings;

auto parseCmdLine(CommandOptionParser& cp, int argc, char** argv) -> Settings
{
  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.dirPrefix          = cp.getOption(optPrefix)        .getParam(0, s.dirPrefix);
  s.serverChannel      = cp.getOption(optServerChannel) .getParam(0, s.serverChannel);
  s.engineChannel      = cp.getOption(optEngineChannel) .getParam(0, s.engineChannel);
  s.serverStreamId     = cp.getOption(optServerStreamId).getParamAsInt(0, 1, INT32_MAX, s.serverStreamId);
  s.engineStreamId     = cp.getOption(optEngineStreamId).getParamAsInt(0, 1, INT32_MAX, s.engineStreamId);
  s.fragmentCountLimit = cp.getOption(optFrags)         .getParamAsInt(0, 1, INT32_MAX, s.fragmentCountLimit);

  return s;
}

void throwAbort() { throw nullptr; }

#define E(expr) CHECK((rc = (expr)) == MDB_SUCCESS, #expr)
#define RES(err, expr) ((rc = expr) == (err) || (CHECK(!rc, #expr), 0))
#define CHECK(test, msg)                                                  \
  ((test) ? (void)0 : ((void)fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, \
                                     __LINE__, msg, mdb_strerror(rc)),    \
                       throwAbort()))

struct CompositeDupKey {
  uint64_t first;
  uint64_t second;
  uint64_t third;
};

int connections = 0;

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

static MDB_env *env;

static MDB_dbi deviceForUuid;     // deviceUUID (16 byte buffer) => deviceId (8 byte int)
static MDB_dbi uuidForDevice;     // deviceId (8 byte int) => deviceUUID (16 byte buffer)
static MDB_dbi userForDevice;     // deviceId (8 byte int) => userId (8 byte int)
static MDB_dbi discoveredDevicesForDevice; // * deviceId (8 byte int) => set of timestamp, deviceId (pair<8 byte int, 8 byte int>)
static MDB_dbi recentCardsForDevice;       // * deviceId (8 byte int) => set of timestamp, cardId (pair<8 byte int, 8 byte int>)
static MDB_dbi cardsForUser;      // userId (8 byte int) => set of cardId (8 byte int)
static MDB_dbi devicesForUser;    // userId (8 byte int) => set of deviceId (8 byte int)
static MDB_dbi sectionsForUser;   // userId (8 byte int) => set of sectionId (8 byte int)
static MDB_dbi userForTelephone;  // telephone (8 byte int) => userId (8 byte int)
static MDB_dbi user;              // userId (8 byte int) => Cap’n Proto User object w/ active device id
static MDB_dbi section;           // sectionId (8 byte int) => Cap’n Proto Section object w/ list of (timestamp, cardId) pairs and update timestamp
static MDB_dbi cardForPublicCard; // publicCardId (8 byte int) => cardId (8 byte int)
static MDB_dbi card;              // cardId (8 byte int) => Cap’n Proto Card object w/ list of networks
static MDB_dbi blocksForUser;     // * userId (8 byte int) => set of timestamp, cardId (pair<8 byte int, 8 byte int>)
static MDB_dbi deletionsForUser;  // * userId (8 byte int) => set of timestamp, sectionId, cardId (tuple<8 byte int, 8 byte int, 8 byte int>)
static MDB_dbi branchRequest;     // timestamp (8 byte int) => UTF-8 buffer containing JSON
static MDB_dbi discoveryResponseForDevice; // deviceId (8 byte int) => Cap'n Proto DiscoveryResponse object
static MDB_dbi syncInfoForDevice; // deviceId (8 byte int) => Cap’n Proto Card object

static uint64_t lastCardId = 0;
static uint64_t lastDeviceId = 0;
static uint64_t lastSectionId = 0;
static uint64_t lastBranchRequest = 0;

static std::map<uint64_t, uv_poll_t *> socketMap; // maps a deviceId to a pointer that can be used to create a uWS::Socket object

void engineMessageHandler(AtomicBuffer& buffer, index_t offset, index_t length)
{
  uint8_t *data = buffer.buffer() + offset;
  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data, length / 8);
  auto reader = capnp::FlatArrayMessageReader(view);

  RPC::Reader rpc = reader.getRoot<RPC>();
  int64_t deviceId = rpc.getDevice();
  auto rpcKind = rpc.getKind();
  auto which = rpcKind.which();

  if (which == RPC::Kind::SYNC_REQUEST) {
    int rc;
    MDB_txn *txn = nullptr;
    MDB_cursor *cursor = nullptr;

    try {
      cout << "Got sync request: " << deviceId << endl;

      auto syncRequest = rpcKind.getSyncRequest();

      capnp::MallocMessageBuilder builder;
      auto rpcResponse = builder.initRoot<RPC>();
      rpcResponse.setDevice(deviceId);

      auto rpcResponseKind = rpcResponse.getKind();
      auto syncResponse = rpcResponseKind.initSyncResponse();

      syncResponse.setDiscoverInfo(syncRequest.getDiscoverInfo());

      E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));

      MDB_val key, data;

      key.mv_size = sizeof(deviceId);
      key.mv_data = &deviceId;

      rc = mdb_get(txn, syncInfoForDevice, &key, &data);
      if (rc != 0 && rc != MDB_NOTFOUND) {
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

        E(mdb_txn_commit(txn));
        txn = nullptr;
        return;
      }

      auto syncInfoReader = makeReader(data);
      auto syncInfo = syncInfoReader.getRoot<SyncInfo>();
      syncResponse.setSyncInfo(syncInfo);

      // If the discovery engine needs the cards advertised by the device, provide them.
      if (syncRequest.getWantsCardsAdvertisedByDevice()) {
        rc = mdb_get(txn, userForDevice, &key, &data);
        if (rc != 0 && rc != MDB_NOTFOUND) {
          cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

          E(mdb_txn_commit(txn));
          txn = nullptr;
          return;
        }

        if (rc != MDB_NOTFOUND) {
          // We have a user, who may have a card.
          uint64_t userId = *(uint64_t *)data.mv_data;

          E(mdb_cursor_open(txn, cardsForUser, &cursor));

          key.mv_data = &userId;

          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
            uint64_t cardId = *(uint64_t *)data.mv_data;

            auto cards = syncResponse.initCardsAdvertisedByDevice(1);
            cards.set(0, cardId);

          } else {
            // User does not have a card.
            syncResponse.initCardsAdvertisedByDevice(0);
          }

          mdb_cursor_close(cursor);
          cursor = nullptr;
        }
      }

      // If the discovery engine is missing some recent cards, provide them.
      if (syncRequest.getLatestRecentCard() < syncInfo.getLastCard()) {
        size_t count = 0;

        E(mdb_cursor_open(txn, recentCardsForDevice, &cursor));

        key.mv_data = &deviceId;

        CompositeDupKey dupKey;

        dupKey.first = syncRequest.getLatestRecentCard();
        dupKey.second = 0;

        data.mv_size = 16;
        data.mv_data = &dupKey;

        if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_GET_BOTH)) == 0) {
          // We need to skip over the first key since the discovery engine already has it.
          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0) {
            do { count++; } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0);
          }
        }

        if (count > 0) {
          // Copy the more recente cards into the Sync Response.
          auto recentCardsList = syncResponse.initRecentCards(count);
          count = 0;

          data.mv_data = &dupKey;

          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
            // We need to skip over the first key since the discovery engine already has it.
            if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0) {
              do {
                CompositeDupKey dupKey2 = *(CompositeDupKey *)data.mv_data;

                recentCardsList[count].setTimestamp(dupKey2.first);
                recentCardsList[count].setId(dupKey2.second);

                count++;
              } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0);
            }
          }

        } else {
          // There are no more recent cards?
          syncResponse.initRecentCards(0);
        }

        mdb_cursor_close(cursor);
        cursor = nullptr;
      }

      // If the discovery engine is missing some device discoveries, provide them.
      if (syncRequest.getLatestDiscoveredDevice() < syncInfo.getLastDeviceUpdate()) {
        size_t count = 0;

        E(mdb_cursor_open(txn, discoveredDevicesForDevice, &cursor));

        key.mv_data = &deviceId;

        CompositeDupKey dupKey;

        dupKey.first = syncRequest.getLatestDiscoveredDevice();
        dupKey.second = 0;

        data.mv_size = 16;
        data.mv_data = &dupKey;

        if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_GET_BOTH)) == 0) {
          // We need to skip over the first key since the discovery engine already has it.
          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0) {
            do { count++; } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0);
          }
        }

        if (count > 0) {
          // Copy the more recente cards into the Sync Response.
          auto discoveredDevicesList = syncResponse.initDiscoveredDevices(count);
          count = 0;

          data.mv_data = &dupKey;

          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_GET_BOTH)) == 0) {
            // We need to skip over the first key since the discovery engine already has it.
            if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0) {
              do {
                CompositeDupKey dupKey2 = *(CompositeDupKey *)data.mv_data;

                discoveredDevicesList[count].setTimestamp(dupKey2.first);
                discoveredDevicesList[count].setId(dupKey2.second);

                count++;
              } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT_DUP)) == 0);
            }
          }

        } else {
          // There are no more recent cards?
          syncResponse.initDiscoveredDevices(0);
        }

        mdb_cursor_close(cursor);
        cursor = nullptr;
      }

      kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
      kj::ArrayPtr<kj::byte> bytes = words.asBytes();

      concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
      do {
        // FIXME(Erich): Do we really want to busy-wait here?
      } while (serverPublication->offer(srcBuffer, 0, bytes.size()) < 0L);

    } catch (...) {
      cout << "Got an unknown exception" << endl;
    }

    if (cursor) {
      mdb_cursor_close(cursor);
      cursor = nullptr;
    }

    if (txn != nullptr) {
      E(mdb_txn_commit(txn));
      txn = nullptr;
    }

  } else if (which == RPC::Kind::DISCOVER_RESPONSE) {
    int rc;
    MDB_txn *txn = nullptr;
    MDB_cursor *cursor = nullptr;

    uint64_t originalSectionId = lastSectionId;

    try {
      cout << "Got discover response: " << deviceId << endl;

      auto discoverResponse = rpcKind.getDiscoverResponse();

      // RPC response
      capnp::MallocMessageBuilder rpcBuilder;
      auto rpcResponse = rpcBuilder.initRoot<RPC>();
      rpcResponse.setDevice(deviceId);

      auto rpcResponseKind = rpcResponse.getKind();
      auto ackResponse = rpcResponseKind.initDiscoverAckResponse();

      ackResponse.setTimestamp(discoverResponse.getTimestamp());

      // Client response
      capnp::MallocMessageBuilder clientBuilder;
      auto clientResponse = clientBuilder.initRoot<Response>();
      clientResponse.setId(discoverResponse.getDiscoverInfo().getRequestId());

      auto clientResponseKind = clientResponse.getKind();
      auto discoveryResponse = clientResponseKind.initDiscoveryResponse();

      discoveryResponse.setTimestamp(discoverResponse.getTimestamp());
      auto cardsList = discoveryResponse.initCards(discoverResponse.getCards().size());

      size_t count = 0;
      for (auto aCard : discoverResponse.getCards()) {
        cardsList[count].setTimestamp(aCard.getTimestamp());
        cardsList[count].setId(aCard.getId());
        count++;
      }

      E(mdb_txn_begin(env, NULL, 0, &txn));

      MDB_val key, data;

      key.mv_size = sizeof(deviceId);
      key.mv_data = &deviceId;

      rc = mdb_get(txn, syncInfoForDevice, &key, &data);
      if (rc != 0) { // MDB_NOTFOUND is an error, we should always have SyncInfo if we're doing discovery.
        cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << endl;

        mdb_txn_abort(txn);
        txn = nullptr;
        return;
      }

      auto syncInfoReader = makeReader(data);
      auto syncInfo = syncInfoReader.getRoot<SyncInfo>();

      auto providedSyncInfo = discoverResponse.getSyncInfo();

      if (providedSyncInfo.getLastDeviceUpdate() != syncInfo.getLastDeviceUpdate() ||
          providedSyncInfo.getLastCard() != syncInfo.getLastCard() ||
          providedSyncInfo.getCardsVersion() != syncInfo.getCardsVersion()) {
        cout << "provided sync info does not match current sync info; ignoring" << endl;

        mdb_txn_abort(txn);
        txn = nullptr;
        return;
      }

      // If the discover response has a new section, we need to create it.
      if (discoverResponse.hasSection()) {
        auto sectionInfo = discoverResponse.getSection();
        auto sectionInfoKind = sectionInfo.getKind();

        if (sectionInfoKind.which() != SectionInfo::Kind::TIMESTAMP_SECTION) {
          cout << "Unknown section info kind" << endl;

          mdb_txn_abort(txn);
          txn = nullptr;
          return;
        }

        uint64_t sectionId = ++lastSectionId; // From hereon, we need to decrement this if we abort the transaction.

        capnp::MallocMessageBuilder sectionBuilder;
        auto newSection = sectionBuilder.initRoot<Section>();
        newSection.setId(sectionId);
        newSection.setInfo(sectionInfo);

        // We've been sent the most recent card to include in the new section.
        // We need to use a cursor to start at that card and discover the other
        // cards.
        {
          size_t count = 0;

          E(mdb_cursor_open(txn, recentCardsForDevice, &cursor));

          key.mv_data = &deviceId;

          CompositeDupKey dupKey;

          dupKey.first = sectionInfo.getKind().getTimestampSection().getLastCard();
          dupKey.second = 0;

          data.mv_size = 16;
          data.mv_data = &dupKey;

          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_GET_BOTH)) == 0) {
            do { count++; } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_PREV_DUP)) == 0);
          }

          if (count > 0) {
            // Copy the more recent cards into the Sync Response.
            auto cardsList = newSection.initCards(count);
            count = 0;

            data.mv_data = &dupKey;

            if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_GET_BOTH)) == 0) {
              do {
                CompositeDupKey dupKey2 = *(CompositeDupKey *)data.mv_data;

                cardsList[count].setTimestamp(dupKey2.first);
                cardsList[count].setId(dupKey2.second);

                count++;
              } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_PREV_DUP)) == 0);
            }

            mdb_cursor_close(cursor);
            cursor = nullptr;

          } else {
            lastSectionId--;
            cout << "There are no cards in the new section? That's an error." << endl;

            mdb_cursor_close(cursor);
            cursor = nullptr;

            mdb_txn_abort(txn);
            txn = nullptr;
            return;
          }
        }

        // Okay, now that we've got the card info, we can delete the recent cards that
        // were moved into the new section.
        {
          E(mdb_cursor_open(txn, recentCardsForDevice, &cursor));

          key.mv_data = &deviceId;

          CompositeDupKey dupKey;

          data.mv_size = 16;
          data.mv_data = &dupKey;

          for (auto cardInfo : newSection.getCards()) {
            dupKey.first  = cardInfo.getTimestamp();
            dupKey.second = cardInfo.getId();

            rc = mdb_del(txn, recentCardsForDevice, &key, &data);
            if (rc != 0) {
              lastSectionId--;
              cout << __LINE__ << " - mdb_del() error: " << mdb_strerror(rc) << endl;

              mdb_cursor_close(cursor);
              cursor = nullptr;

              mdb_txn_abort(txn);
              txn = nullptr;
              return;
            }
          }

          mdb_cursor_close(cursor);
          cursor = nullptr;
        }

        // Save new section object in database.
        {
          kj::Array<capnp::word> words = capnp::messageToFlatArray(sectionBuilder);
          kj::ArrayPtr<kj::byte> bytes = words.asBytes();
          size_t size = bytes.size();
          char *from = (char *)(bytes.begin());

          key.mv_data = &sectionId;
          data.mv_size = size;
          data.mv_data = from;

          rc = mdb_put(txn, section, &key, &data, 0);
          if (rc != 0) {
            lastSectionId--;
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

            mdb_txn_abort(txn);
            txn = nullptr;
            return;
          }
        }

        // We send the new section back to the client.
        discoveryResponse.setSection(newSection);
      }

      // If the discover response has discovered cards, we need to add them
      // to recents and include them in the discovery response.
      if (discoverResponse.hasCards()) {
        E(mdb_cursor_open(txn, recentCardsForDevice, &cursor));

        key.mv_data = &deviceId;

        CompositeDupKey dupKey;

        data.mv_size = 16;
        data.mv_data = &dupKey;

        size_t idx = 0;
        auto cardList = discoveryResponse.initCards(discoverResponse.getCards().size());
        for (auto aCard : discoverResponse.getCards()) {
          cardList[idx].setId(aCard.getId());
          cardList[idx].setTimestamp(aCard.getTimestamp());
          idx++;

          dupKey.first = aCard.getTimestamp();
          dupKey.second = aCard.getId();

          rc = mdb_put(txn, recentCardsForDevice, &key, &data, MDB_NOOVERWRITE);
          if (rc != 0) {
            lastSectionId--;
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

            mdb_cursor_close(cursor);
            cursor = nullptr;

            mdb_txn_abort(txn);
            txn = nullptr;
            return;
          }
        }

        mdb_cursor_close(cursor);
        cursor = nullptr;
      }

      // We need to save the discovery response in the database regardless.
      {
        kj::Array<capnp::word> words = capnp::messageToFlatArray(clientBuilder);
        kj::ArrayPtr<kj::byte> bytes = words.asBytes();
        size_t size = bytes.size();
        char *from = (char *)(bytes.begin());

        key.mv_data = &deviceId;
        data.mv_size = size;
        data.mv_data = from;

        rc = mdb_put(txn, discoveryResponseForDevice, &key, &data, 0);
        if (rc != 0) {
          lastSectionId--;
          cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

          mdb_txn_abort(txn);
          txn = nullptr;
          return;
        }
      }

      E(mdb_txn_commit(txn));
      txn = nullptr;

      // Send the DiscoveryResponse to the client, if it is connected.
      if (socketMap.find(deviceId) != socketMap.end()) {
        uv_poll_t *socketPtr = socketMap[deviceId];
        kj::Array<capnp::word> words = capnp::messageToFlatArray(clientBuilder);
        kj::ArrayPtr<kj::byte> bytes = words.asBytes();

        uWS::ServerSocket(socketPtr).send((char *)(bytes.begin()), bytes.size(), uWS::OpCode::BINARY);
      }

      // Send the ack RPC back to the discovery engine.
      kj::Array<capnp::word> words = capnp::messageToFlatArray(rpcBuilder);
      kj::ArrayPtr<kj::byte> bytes = words.asBytes();

      concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
      do {
        // FIXME(Erich): Do we really want to busy-wait here?
      } while (serverPublication->offer(srcBuffer, 0, bytes.size()) < 0L);

    } catch (...) {
      cout << "Got an unknown exception" << endl;
      lastSectionId = originalSectionId;
    }

    if (cursor) {
      mdb_cursor_close(cursor);
      cursor = nullptr;
    }

    if (txn != nullptr) {
      mdb_txn_abort(txn);
      txn = nullptr;
    }

  } else {
    cout << "Unknown request from discovery engine: " << (int)which << endl;
  }
}

static uv_timer_t timerHandle;

void timerCallback(uv_timer_t* handle)
{
  if (!shouldPoll) return;

  // Poll for messages from discovery engine.
  FragmentAssembler fragmentAssembler([&](AtomicBuffer& buffer, index_t offset, index_t length, Header& header) {
    engineMessageHandler(buffer, offset, length);
  });

  engineSubscription->poll(fragmentAssembler.handler(), settings.fragmentCountLimit);
}

int main(int argc, char** argv) {
  assert(sizeof(unsigned long long) == sizeof(void *) && "Expect sizeof(unsigned long long) == sizeof(void *)");

  { // Configure LMDB.
    int rc;
    MDB_txn *txn = nullptr;
    MDB_cursor *cursor = nullptr;
    MDB_val key, data;

    // Create lmdb environment.
    E(mdb_env_create(&env));
    E(mdb_env_set_maxreaders(env, 126));  // same as default
    E(mdb_env_set_maxdbs(env, 18));
    E(mdb_env_set_mapsize(env, DEFAULT_DB_SIZE * 1024 * 1024 * 1));
    E(mdb_env_open(env, "/Users/erich/Desktop/testdb", MDB_NOTLS, 0664));

    // Open and configure databases.
    E(mdb_txn_begin(env, NULL, 0, &txn));

    E(mdb_dbi_open(txn, "deviceForUuid",              MDB_CREATE,                                                 &deviceForUuid));
    E(mdb_dbi_open(txn, "uuidForDevice",              MDB_CREATE | MDB_INTEGERKEY,                                &uuidForDevice));
    E(mdb_dbi_open(txn, "userForDevice",              MDB_CREATE | MDB_INTEGERKEY,                                &userForDevice));
    E(mdb_dbi_open(txn, "discoveredDevicesForDevice", MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &discoveredDevicesForDevice));
    E(mdb_dbi_open(txn, "recentCardsForDevice",       MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &recentCardsForDevice));
    E(mdb_dbi_open(txn, "cardsForUser",               MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &cardsForUser));
    E(mdb_dbi_open(txn, "devicesForUser",             MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &devicesForUser));
    E(mdb_dbi_open(txn, "sectionsForUser",            MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT | MDB_INTEGERDUP, &sectionsForUser));
    E(mdb_dbi_open(txn, "userForTelephone",           MDB_CREATE | MDB_INTEGERKEY,                                &userForTelephone));
    E(mdb_dbi_open(txn, "user",                       MDB_CREATE | MDB_INTEGERKEY,                                &user));
    E(mdb_dbi_open(txn, "section",                    MDB_CREATE | MDB_INTEGERKEY,                                &section));
    E(mdb_dbi_open(txn, "cardForPublicCard",          MDB_CREATE | MDB_INTEGERKEY,                                &cardForPublicCard));
    E(mdb_dbi_open(txn, "card",                       MDB_CREATE | MDB_INTEGERKEY,                                &card));
    E(mdb_dbi_open(txn, "blocksForUser",              MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &blocksForUser));
    E(mdb_dbi_open(txn, "deletionsForUser",           MDB_CREATE | MDB_INTEGERKEY | MDB_DUPSORT,                  &deletionsForUser));
    E(mdb_dbi_open(txn, "branchRequest",              MDB_CREATE | MDB_INTEGERKEY,                                &branchRequest));
    E(mdb_dbi_open(txn, "discoveryResponseForDevice", MDB_CREATE | MDB_INTEGERKEY,                                &discoveryResponseForDevice));
    E(mdb_dbi_open(txn, "syncInfoForDevice",          MDB_CREATE | MDB_INTEGERKEY,                                &syncInfoForDevice));

    // Set custom comparison functions.
    E(mdb_set_dupsort(txn, discoveredDevicesForDevice, lmdbCmpFunction));
    E(mdb_set_dupsort(txn, recentCardsForDevice, lmdbCmpFunction));
    E(mdb_set_dupsort(txn, blocksForUser, lmdbCmpFunction));
    E(mdb_set_dupsort(txn, deletionsForUser, lmdbCmpFunction));

    // Gather metadata.
    E(mdb_cursor_open(txn, card, &cursor));
    if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
      lastCardId = *(uint64_t *)(key.mv_data);
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    E(mdb_cursor_open(txn, uuidForDevice, &cursor));
    if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
      lastDeviceId = *(uint64_t *)(key.mv_data);
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    E(mdb_cursor_open(txn, section, &cursor));
    if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
      lastSectionId = *(uint64_t *)(key.mv_data);
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    E(mdb_cursor_open(txn, branchRequest, &cursor));
    if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_LAST)) == 0) {
      lastBranchRequest = *(uint64_t *)(key.mv_data);
    }
    mdb_cursor_close(cursor);
    cursor = nullptr;

    E(mdb_txn_commit(txn));
    txn = nullptr;

    std::cout << "lastCardId = "        << lastCardId        << std::endl;
    std::cout << "lastDeviceId = "      << lastDeviceId      << std::endl;
    std::cout << "lastSectionId = "     << lastSectionId     << std::endl;
    std::cout << "lastBranchRequest = " << lastBranchRequest << std::endl;
  }

  { // Configure Aeron
    CommandOptionParser cp;
    cp.addOption(CommandOption (optHelp,           0, 0, "                Displays help information."));

    cp.addOption(CommandOption (optPrefix,         1, 1, "dir             Prefix directory for aeron driver."));
    cp.addOption(CommandOption (optServerChannel,  1, 1, "channel         Server Channel."));
    cp.addOption(CommandOption (optEngineChannel,  1, 1, "channel         Engine Channel."));
    cp.addOption(CommandOption (optServerStreamId, 1, 1, "streamId        Server Stream ID."));
    cp.addOption(CommandOption (optEngineStreamId, 1, 1, "streamId        Engine Stream ID."));
    cp.addOption(CommandOption (optFrags,          1, 1, "limit           Fragment Count Limit."));

    settings = parseCmdLine(cp, argc, argv);

    cout << "Publishing Server at " << settings.serverChannel << " on Stream ID " << settings.serverStreamId << endl;
    cout << "Subscribing Engine at " << settings.engineChannel << " on Stream ID " << settings.engineStreamId << endl;

    aeron::Context context;
    int64_t publicationId;
    int64_t subscriptionId;

    if (settings.dirPrefix != "") {
      context.aeronDir(settings.dirPrefix);
    }

    context.newPublicationHandler([](const string& channel, int32_t streamId, int32_t sessionId, int64_t correlationId) {
      cout << "Got Publication: " << channel << " " << correlationId << ":" << streamId << ":" << sessionId << endl;
    });

    context.newSubscriptionHandler([](const string& channel, int32_t streamId, int64_t correlationId) {
      cout << "Got Subscription: " << channel << " " << correlationId << ":" << streamId << endl;
    });

    context.availableImageHandler([&subscriptionId](Image &image) {
      cout << "Available image correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
      cout << " at position=" << image.position() << " from " << image.sourceIdentity() << endl;

      if (image.subscriptionRegistrationId() != subscriptionId) return;

      shouldPoll = true;
    });

    context.unavailableImageHandler([&subscriptionId](Image &image) {
      cout << "Unavailable image on correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
      cout << " at position=" << image.position() << " from " << image.sourceIdentity() << endl;

      if (image.subscriptionRegistrationId() != subscriptionId) return;

      shouldPoll = false;
    });

    Aeron aeron(context);

    publicationId = aeron.addPublication(settings.serverChannel, settings.serverStreamId);
    subscriptionId = aeron.addSubscription(settings.engineChannel, settings.engineStreamId);

    serverPublication = aeron.findPublication(publicationId);
    while (!serverPublication) {
      this_thread::yield();
      serverPublication = aeron.findPublication(publicationId);
    }

    engineSubscription = aeron.findSubscription(subscriptionId);
    while (!engineSubscription) {
      this_thread::yield();
      engineSubscription = aeron.findSubscription(subscriptionId);
    }

    cout << "Aeron is up and running." << endl;

    // Set up polling.
    uv_timer_init(uv_default_loop(), &timerHandle);
    uv_timer_start(&timerHandle, timerCallback, 0, 1);
  }

  try { // Configure µWebSockets
    uWS::Server server(3000);
    uWS::Server branch(3001);

    // Branch.io WebSocket
    branch.onConnection([](uWS::ServerSocket socket) {
      cout << "BRANCH: [Connection]" << endl;
    });

    branch.onMessage([](uWS::ServerSocket socket, const char *msg, size_t len, uWS::OpCode opCode) {
      int rc;
      MDB_txn *txn = nullptr;

      try {
        cout << "BRANCH: [Message]" << endl;

        std::string msgStr(msg, len);
        unsigned long long branchRequestId = std::stoull(msgStr.c_str());
        cout << "branchRequestId as 64-bit: " << branchRequestId << endl;

        E(mdb_txn_begin(env, NULL, 0, &txn));

        MDB_val key;

        key.mv_size = sizeof(branchRequestId);
        key.mv_data = &branchRequestId;

        rc = mdb_del(txn, branchRequest, &key, NULL);
        if (rc != 0) {
          if (rc == MDB_NOTFOUND) {
            cout << "Unknown branchRequestId? " << branchRequestId << endl;

          } else {
            // An error occurred trying to delete branchRequestId.
            cout << "An error occurred trying to delete branchRequestId = " << branchRequestId << endl;
            cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;
            throw nullptr;
          }
        }

        char *okMessage = (char *)"OK";
        socket.send(okMessage, 2, uWS::OpCode::TEXT);

      } catch (...) {
        char *errorMessage = (char *)"ERROR";
        socket.send(errorMessage, 5, uWS::OpCode::TEXT);
      }

      if (txn != nullptr) {
        E(mdb_txn_commit(txn));
        txn = nullptr;
      }
    });

    branch.onDisconnection([](uWS::ServerSocket socket, int code, char *message, size_t length) {
      cout << "BRANCH: [Disconnection]" << endl;
    });

    // Client WebSocket
    server.onConnection([](uWS::ServerSocket socket) {
      cout << "[Connection] clients: " << ++connections << endl;
    });

    server.onMessage([](uWS::ServerSocket socket, const char *msg, size_t len, uWS::OpCode opCode)
    {
      void *data = socket.getData();
      size_t deviceId = data == nullptr ? 0 : (size_t)data;

      cout << "Got request from " << deviceId << endl;

      kj::ArrayPtr<const capnp::word> view((const capnp::word *)msg, len / 8);
      auto reader = capnp::FlatArrayMessageReader(view);
      Request::Reader request = reader.getRoot<Request>();
      int64_t requestId = request.getId();
      auto requestKind = request.getKind();
      auto which = requestKind.which();

      // We *always* send a response.
      capnp::MallocMessageBuilder builder;
      Response::Builder response = builder.initRoot<Response>();
      response.setId(requestId);
      auto responseKind = response.getKind();

      // The first request MUST be a Hello request.
      if (deviceId == 0 && which != Request::Kind::HELLO_REQUEST) {
        std::cout << "Made request without saying Hello first" << std::endl;

        auto error = responseKind.initErrorResponse();
        error.setMessage("Need to say Hello first.");

        kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
        kj::ArrayPtr<kj::byte> bytes = words.asBytes();

        socket.send((char *)(bytes.begin()), bytes.size(), uWS::OpCode::BINARY);
        return;
      }

      if (which == Request::Kind::HELLO_REQUEST) {
        int rc;
        MDB_txn *txn = nullptr;

        try {
          cout << "Got Hello message: " << requestId << endl;

          HelloRequest::Reader helloRequest = requestKind.getHelloRequest();

          if (!helloRequest.hasUuid()) {
            auto error = responseKind.initErrorResponse();
            error.setMessage("Hello request has no UUID assigned.");
            throw nullptr;
          }

          auto uuidBytes = helloRequest.getUuid().asBytes();

          if (uuidBytes.size() != 16) {
            auto error = responseKind.initErrorResponse();
            error.setMessage("Hello request UUID has the wrong length (should be 16).");
            throw nullptr;
          }

          E(mdb_txn_begin(env, NULL, 0, &txn));

          MDB_val key, data;

          key.mv_size = 16;
          key.mv_data = (void *)uuidBytes.begin();

          rc = mdb_get(txn, deviceForUuid, &key, &data);
          if (rc != 0) {
            if (rc == MDB_NOTFOUND) {
              // This is the first time this device has said Hello.
              // We need to map the UUID to a new device id, and the new device id to the UUID.
              uint64_t deviceId = ++lastDeviceId;

              data.mv_size = 8;
              data.mv_data = &deviceId;

              rc = mdb_put(txn, deviceForUuid, &key, &data, 0);
              if (rc != 0) {
                --lastDeviceId;

                // An error occurred trying to associate the UUID with the device id.
                cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                auto error = responseKind.initErrorResponse();
                error.setMessage(mdb_strerror(rc));

                mdb_txn_abort(txn);
                txn = nullptr;

              } else {
                rc = mdb_put(txn, uuidForDevice, &data, &key, 0);
                if (rc != 0) {
                  --lastDeviceId;

                  // An error occurred trying to associate the device id with the UUID.
                  cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                  auto error = responseKind.initErrorResponse();
                  error.setMessage(mdb_strerror(rc));

                  mdb_txn_abort(txn);
                  txn = nullptr;

                } else {
                  E(mdb_txn_commit(txn));
                  txn = nullptr;

                  socket.setData((void *)deviceId);

                  socketMap[deviceId] = socket.getSocket();

                  auto helloResponse = responseKind.initHelloResponse();
                  helloResponse.setDeviceId(deviceId);
                }
              }

            } else {
              // An error occurred trying to retrieve the device id.
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

              auto error = responseKind.initErrorResponse();
              error.setMessage(mdb_strerror(rc));
            }

          } else {
            // We do have a device. We need to see if we have a DiscoveryResponse to send back.
            uint64_t deviceId = *(uint64_t *)data.mv_data;

            socket.setData((void *)deviceId);

            socketMap[deviceId] = socket.getSocket();

            key.mv_size = 8;
            key.mv_data = &deviceId;

            rc = mdb_get(txn, discoveryResponseForDevice, &key, &data);
            if (rc!= 0 && rc != MDB_NOTFOUND) {
              // An error occurred trying to retrieve an existing discoveryResponse.
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

              // I'm choosing not to send an error for this situation.
              // auto error = responseKind.initErrorResponse();
              // error.setMessage(mdb_strerror(rc));

            } else if (rc == MDB_NOTFOUND) {
                auto helloResponse = responseKind.initHelloResponse();
                helloResponse.setDeviceId(deviceId);

            } else {
              auto helloResponse = responseKind.initHelloResponse();
              helloResponse.setDeviceId(deviceId);

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

      } else if (which == Request::Kind::CARD_REQUEST) {
        int rc;
        MDB_txn *txn = nullptr;

        try {
          auto cardRequest = requestKind.getCardRequest();
          uint64_t cardId = cardRequest.getId();
          uint32_t version = cardRequest.getVersion();

          cout << "Card: " << cardId << " version: " << version << endl;

          E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));

          MDB_val key, data;

          key.mv_size = sizeof(cardId);
          key.mv_data = &cardId;

          rc = mdb_get(txn, card, &key, &data);
          if (rc != 0) {
            auto error = responseKind.initErrorResponse();

            if (rc == MDB_NOTFOUND) {
              error.setMessage("Card not found.");

            } else {
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;
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

      } else if (which == Request::Kind::DISCOVERY_REQUEST) {
        auto discoveryRequest = requestKind.getDiscoveryRequest();
        uint64_t lastDiscovery = discoveryRequest.getLastDiscovery();

        cout << "Discovery Request: " << lastDiscovery << endl;

        if (!shouldPoll) {
          cout << "shouldPoll == false" << endl;

          auto error = responseKind.initErrorResponse();
          error.setCode(ErrorResponse::Code::DISCOVERY_NOT_AVAILABLE);
          error.setMessage("Discovery is not available at this time.");

        } else {
          int rc;
          MDB_txn *txn = nullptr;

          try {
            E(mdb_txn_begin(env, NULL, 0, &txn));

            MDB_val key, data;

            key.mv_size = 8;
            key.mv_data = &deviceId;

            rc = mdb_get(txn, discoveryResponseForDevice, &key, &data);
            if (rc != 0 && rc != MDB_NOTFOUND) {
              // An error occurred trying to retrieve an existing discoveryResponse.
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

              auto error = responseKind.initErrorResponse();
              error.setMessage(mdb_strerror(rc));

              mdb_txn_abort(txn);
              txn = nullptr;

            } else if (rc == MDB_NOTFOUND) {
              // There have been no (successful) discover requests processed so far.
              if (lastDiscovery != 0) {
                cout << "lastDiscovery != 0, but there are no processed discovery requests?" << std::endl;

                auto error = responseKind.initErrorResponse();
                error.setMessage("lastDiscovery != 0, but there are no processed discovery requests?");

                mdb_txn_abort(txn);
                txn = nullptr;
              }

            } else {
              auto reader = makeReader(data);
              auto storedDiscoveryResponse = reader.getRoot<DiscoveryResponse>();

              if (storedDiscoveryResponse.getTimestamp() != lastDiscovery) {
                // The client has not applied the most recent discovery--but we have!
                // So we cannot accept any discovery updates from the client until it does.
                // It is an error to send new cards or devices until the most recent discovery
                // request has been applied.
                if (discoveryRequest.hasCards() || discoveryRequest.hasDevices()) {
                  auto error = responseKind.initErrorResponse();
                  error.setMessage("You cannot make a discovery request with NEW cards or devices when you have a previous request that has not been applied.");

                } else {
                  // Just send back the previous request.
                  responseKind.setDiscoveryResponse(storedDiscoveryResponse);
                }

                mdb_txn_abort(txn);
                txn = nullptr;
              }
            }

            // If we've still got a transaction, then we need to continue processing the request.
            if (txn) {
              uint64_t lastCard = 0;
              uint64_t lastDeviceUpdate = 0;

              // Update recent cards for the device (if needed).
              if (discoveryRequest.hasCards()) {
                auto newCards = discoveryRequest.getCards();

                CompositeDupKey dupKey;

                data.mv_size = 16;
                data.mv_data = &dupKey;

                for (auto card : newCards) {
                  dupKey.first = card.getTimestamp();
                  dupKey.second = card.getId();

                  if (dupKey.first > lastCard) lastCard = dupKey.first;

                  rc = mdb_put(txn, recentCardsForDevice, &key, &data, 0);
                  if (rc != 0) {
                    // An error occurred trying to associate the device id with the UUID.
                    cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                    auto error = responseKind.initErrorResponse();
                    error.setMessage(mdb_strerror(rc));

                    mdb_txn_abort(txn);
                    txn = nullptr;
                    throw nullptr;
                  }
                }
              }

              // Update recent device for the device (if needed).
              if (discoveryRequest.hasDevices()) {
                auto newDevices = discoveryRequest.getDevices();

                CompositeDupKey dupKey;

                data.mv_size = 16;
                data.mv_data = &dupKey;

                for (auto device : newDevices) {
                  dupKey.first = device.getTimestamp();
                  dupKey.second = device.getId();

                  if (dupKey.first > lastDeviceUpdate) lastDeviceUpdate = dupKey.first;

                  rc = mdb_put(txn, discoveredDevicesForDevice, &key, &data, 0);
                  if (rc != 0) {
                    // An error occurred trying to associate the device id with the UUID.
                    cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                    auto error = responseKind.initErrorResponse();
                    error.setMessage(mdb_strerror(rc));

                    mdb_txn_abort(txn);
                    txn = nullptr;
                    throw nullptr;
                  }
                }
              }

              rc = mdb_get(txn, syncInfoForDevice, &key, &data);
              if (rc != 0 && rc != MDB_NOTFOUND) {
                auto error = responseKind.initErrorResponse();

                cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;
                error.setMessage(mdb_strerror(rc));

                mdb_txn_abort(txn);
                txn = nullptr;
                throw nullptr;

              } else if (rc == MDB_NOTFOUND) {
                capnp::MallocMessageBuilder builder;
                SyncInfo::Builder syncInfo = builder.initRoot<SyncInfo>();

                syncInfo.setLastCard(lastCard);
                syncInfo.setLastDeviceUpdate(lastDeviceUpdate);

                kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
                kj::ArrayPtr<kj::byte> bytes = words.asBytes();
                size_t size = bytes.size();
                char *from = (char *)(bytes.begin());

                data.mv_size = size;
                data.mv_data = from;

                rc = mdb_put(txn, syncInfoForDevice, &key, &data, 0);
                if (rc != 0) {
                  cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                  auto error = responseKind.initErrorResponse();
                  error.setMessage(mdb_strerror(rc));
                  throw nullptr;
                }

                capnp::MallocMessageBuilder rpcBuilder;
                RPC::Builder rpc = rpcBuilder.initRoot<RPC>();
                auto rpcKind = rpc.initKind();

                auto discoverRequest = rpcKind.initDiscoverRequest();

                discoverRequest.setTimestamp(lastDiscovery); // will be 0

                discoverRequest.setSyncInfo(syncInfo);

                auto discoverInfo = discoverRequest.initDiscoverInfo();
                discoverInfo.setRequestId(requestId);
                discoverInfo.setType(DiscoverInfo::Type::DISCOVER);

                words = capnp::messageToFlatArray(rpcBuilder);
                bytes = words.asBytes();
                size = bytes.size();
                from = (char *)(bytes.begin());

                E(mdb_txn_commit(txn));
                txn = nullptr;

                concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
                if (serverPublication->offer(srcBuffer, 0, bytes.size()) < 0L) {
                  auto error = responseKind.initErrorResponse();
                  error.setCode(ErrorResponse::Code::DISCOVERY_NOT_AVAILABLE);
                  error.setMessage("Discovery is not available at this time.");

                } else {
                  responseKind.setProcessing();
                }

              } else {
                auto reader = makeReader(data);
                auto cachedSyncInfo = reader.getRoot<SyncInfo>();

                if (lastCard > 0 || lastDeviceUpdate > 0) {
                  // We DO need to update the SyncInfo associated with the device.
                  capnp::MallocMessageBuilder builder;
                  SyncInfo::Builder syncInfo = builder.initRoot<SyncInfo>();

                  syncInfo.setCardsVersion(cachedSyncInfo.getCardsVersion());
                  syncInfo.setLastCard(lastCard);
                  syncInfo.setLastDeviceUpdate(lastDeviceUpdate);

                  kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
                  kj::ArrayPtr<kj::byte> bytes = words.asBytes();
                  size_t size = bytes.size();
                  char *from = (char *)(bytes.begin());

                  data.mv_size = size;
                  data.mv_data = from;

                  rc = mdb_put(txn, syncInfoForDevice, &key, &data, 0);
                  if (rc != 0) {
                    cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                    auto error = responseKind.initErrorResponse();
                    error.setMessage(mdb_strerror(rc));
                    throw nullptr;
                  }

                  capnp::MallocMessageBuilder rpcBuilder;
                  RPC::Builder rpc = rpcBuilder.initRoot<RPC>();
                  auto rpcKind = rpc.initKind();

                  auto discoverRequest = rpcKind.initDiscoverRequest();

                  discoverRequest.setTimestamp(lastDiscovery);

                  discoverRequest.setSyncInfo(syncInfo);

                  auto discoverInfo = discoverRequest.initDiscoverInfo();
                  discoverInfo.setRequestId(requestId);
                  discoverInfo.setType(DiscoverInfo::Type::DISCOVER);

                  words = capnp::messageToFlatArray(rpcBuilder);
                  bytes = words.asBytes();

                  E(mdb_txn_commit(txn));
                  txn = nullptr;

                  concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
                  if (serverPublication->offer(srcBuffer, 0, bytes.size()) < 0L) {
                    auto error = responseKind.initErrorResponse();
                    error.setCode(ErrorResponse::Code::DISCOVERY_NOT_AVAILABLE);
                    error.setMessage("Discovery is not available at this time.");

                  } else {
                    responseKind.setProcessing();
                  }

                } else {
                  // SyncInfo does NOT need to be updated. (This also implies that no
                  // cards or devices were inserted.)
                  capnp::MallocMessageBuilder rpcBuilder;
                  RPC::Builder rpc = rpcBuilder.initRoot<RPC>();
                  auto rpcKind = rpc.initKind();

                  auto discoverRequest = rpcKind.initDiscoverRequest();

                  discoverRequest.setTimestamp(lastDiscovery);

                  discoverRequest.setSyncInfo(cachedSyncInfo);

                  auto discoverInfo = discoverRequest.initDiscoverInfo();
                  discoverInfo.setRequestId(requestId);
                  discoverInfo.setType(DiscoverInfo::Type::DISCOVER);

                  kj::Array<capnp::word> words = capnp::messageToFlatArray(rpcBuilder);
                  kj::ArrayPtr<kj::byte> bytes = words.asBytes();

                  mdb_txn_abort(txn);
                  txn = nullptr;

                  concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
                  if (serverPublication->offer(srcBuffer, 0, bytes.size()) < 0L) {
                    auto error = responseKind.initErrorResponse();
                    error.setCode(ErrorResponse::Code::DISCOVERY_NOT_AVAILABLE);
                    error.setMessage("Discovery is not available at this time.");

                  } else {
                    responseKind.setProcessing();
                  }
                }
              }
            }

          } catch (...) {
            if (!responseKind.hasErrorResponse()) {
              auto error = responseKind.initErrorResponse();
              error.setMessage("An unknown error occurred. (3)");
            }
          }

          if (txn != nullptr) {
            mdb_txn_abort(txn);
            txn = nullptr;
          }
        }

      } else if (which == Request::Kind::JOIN_REQUEST) {
        int rc;
        MDB_txn *txn = nullptr;

        try {
          auto joinRequest = requestKind.getJoinRequest();
          cout << "Join: " << requestId << endl;

          unsigned long long userId = std::stoull(joinRequest.getTelephone().cStr());
          cout << "As 64-bit: " << userId << endl;

          E(mdb_txn_begin(env, NULL, 0, &txn));

          MDB_val key, data;

          key.mv_size = sizeof(userId);
          key.mv_data = &userId;

          data.mv_size = 8;
          data.mv_data = &deviceId;

          rc = mdb_put(txn, devicesForUser, &key, &data, 0);
          if (rc != 0) {
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          // TODO: Maybe we should be verifying that userForDevice, if set, matches?

          rc = mdb_put(txn, userForDevice, &data, &key, 0);
          if (rc != 0) {
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          rc = mdb_get(txn, user, &key, &data);
          if (rc != 0 && rc != MDB_NOTFOUND) {
            cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;

          } else if (rc == MDB_NOTFOUND) {
            cout << "This is a new user." << endl;

            capnp::MallocMessageBuilder userBuilder;
            User::Builder newUser = userBuilder.getRoot<User>();

            newUser.setId(userId);
            newUser.setActiveDevice(deviceId);

            key.mv_data = &deviceId;

            rc = mdb_get(txn, uuidForDevice, &key, &data);
            if (rc != 0) {
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

              auto error = responseKind.initErrorResponse();
              error.setMessage(mdb_strerror(rc));

              mdb_txn_abort(txn);
              txn = nullptr;
              throw nullptr;
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

            key.mv_data = &userId;

            data.mv_size = size;
            data.mv_data = from;

            rc = mdb_put(txn, user, &key, &data, MDB_NOOVERWRITE);
            if (rc != 0) {
              cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

              auto error = responseKind.initErrorResponse();
              error.setMessage(mdb_strerror(rc));
              throw nullptr;
            }

            auto joinResponse = responseKind.initJoinResponse();
            joinResponse.setStatus(JoinResponse::Status::NEW);

            joinResponse.setUser(newUser);

            E(mdb_txn_commit(txn));
            txn = nullptr;

          } else {
            cout << "This is an existing user." << endl;

            auto reader = makeReader(data);
            auto existingUser = reader.getRoot<User>();

            auto joinResponse = responseKind.initJoinResponse();
            joinResponse.setStatus(JoinResponse::Status::EXISTING);
              
            if (existingUser.getActiveDevice() != deviceId) {
              cout << "Device is not the activet device" << endl;

              capnp::MallocMessageBuilder builder;
              builder.setRoot(existingUser);
              User::Builder updatedUser = builder.getRoot<User>();

              updatedUser.setActiveDevice(deviceId);

              kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
              kj::ArrayPtr<kj::byte> bytes = words.asBytes();
              size_t size = bytes.size();
              char *from = (char *)(bytes.begin());

              data.mv_size = size;
              data.mv_data = from;

              rc = mdb_put(txn, user, &key, &data, 0);
              if (rc != 0) {
                cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;

                auto error = responseKind.initErrorResponse();
                error.setMessage(mdb_strerror(rc));
                throw nullptr;
              }

              joinResponse.setUser(updatedUser);

            } else {
              joinResponse.setUser(existingUser);
            }

            MDB_cursor *cursor = nullptr;

            // We assume users only have one card.
            E(mdb_cursor_open(txn, cardsForUser, &cursor));
            if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
              uint64_t foundUserId = *(uint64_t *)key.mv_data;
              if (foundUserId == userId) {
                uint64_t cardId = *(uint64_t *)data.mv_data;;
                key.mv_data = &cardId;

                rc = mdb_get(txn, card, &key, &data);
                if (rc != 0) {
                  cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;

                  auto error = responseKind.initErrorResponse();
                  error.setMessage(mdb_strerror(rc));
                  throw nullptr;
                }

                auto reader = makeReader(data);
                auto userCard = reader.getRoot<Card>();
                joinResponse.setCard(userCard);
              }
            }
            mdb_cursor_close(cursor);
            cursor = nullptr;

            // Count sections
            size_t count = 0;
            E(mdb_cursor_open(txn, sectionsForUser, &cursor));
            if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
              do { count++; } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0);
            }

            if (count > 0) {
              // Copy the sections into the JoinResponse.
              auto sectionList = joinResponse.initSections(count);
              count = 0;

              MDB_val key2, data2;
              key2.mv_size = 8;

              if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
                do {
                  key2.mv_data = data.mv_data;

                  rc = mdb_get(txn, section, &key2, &data2);
                  if (rc == 0) {
                    auto reader = makeReader(data2);
                    auto section = reader.getRoot<Section>();

                    sectionList[count].setInfo(section.getInfo());
                    sectionList[count].setCards(section.getCards());
                  }
                  count++;
                } while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0);
              }
            }

            mdb_cursor_close(cursor);
            cursor = nullptr;

            E(mdb_txn_commit(txn));
            txn = nullptr;
          }

        } catch (...) {
          if (!responseKind.hasErrorResponse()) {
            auto error = responseKind.initErrorResponse();
            error.setMessage("An unknown error occurred. (4)");
          }
        }

        if (txn != nullptr) {
          mdb_txn_abort(txn);
          txn = nullptr;
        }

      } else if (which == Request::Kind::SET_DEVICE_REQUEST) {
        auto error = responseKind.initErrorResponse();
        error.setMessage("Not implemented.");

      } else if (which == Request::Kind::BLOCK_REQUEST) {
        auto error = responseKind.initErrorResponse();
        error.setMessage("Not implemented.");

      } else if (which == Request::Kind::REMOVE_REQUEST) {
        auto error = responseKind.initErrorResponse();
        error.setMessage("Not implemented.");

      } else if (which == Request::Kind::CLIENT_SYNC_REQUEST) {
        auto error = responseKind.initErrorResponse();
        error.setMessage("Not implemented.");

      } else if (which == Request::Kind::CREATE_CARD_REQUEST) {
        int rc;
        MDB_txn *txn = nullptr;

        try {
          auto createCardRequest = requestKind.getCreateCardRequest();
          cout << "Create Card: " << requestId << endl;

          E(mdb_txn_begin(env, NULL, 0, &txn));

          uint64_t userId = 0;

          MDB_val key, data;

          key.mv_size = sizeof(deviceId);
          key.mv_data = &deviceId;

          rc = mdb_get(txn, userForDevice, &key, &data);
          if (rc != 0) {
            auto error = responseKind.initErrorResponse();
            if (rc == MDB_NOTFOUND) {
              cout << "User has not joined Blue yet" << std::endl;
              error.setMessage("User has not joined Blue yet!");

            } else {
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;
              error.setMessage(mdb_strerror(rc));
            }

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;

          } else {
            userId = *(uint64_t *)data.mv_data;
          }

          // Validation:

          // 1. User cannot have already created a card.
          key.mv_size = sizeof(userId);
          key.mv_data = &userId;

          MDB_cursor *cursor = nullptr;

          bool hasCardAlready = false;

          // We assume users only have one card.
          E(mdb_cursor_open(txn, cardsForUser, &cursor));
          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
            uint64_t foundUserId = *(uint64_t *)key.mv_data;
            if (foundUserId == userId) hasCardAlready = true;
          }
          mdb_cursor_close(cursor);
          cursor = nullptr;

          if (hasCardAlready) {
            cout << "User already has a card!" << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage("User already has a card!");

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          // 2. Card must have id = 0 and version = 0.
          auto desiredCard = createCardRequest.getCard();
          if (desiredCard.getId() != 0 || desiredCard.getVersion() != 0) {
            cout << "Card has an id or version not equal to zero" << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage("Card has an id or version not equal to zero");

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          // TODO: Additional card validation here?

          // Okay, ready to insert. First generate a new card id.
          cout << "lastCardId prior to incrementing: " << lastCardId << std::endl;
          uint64_t cardId = ++lastCardId;
          cout << "lastCardId after incrementing: " << lastCardId << std::endl;
          cout << "cardId: " << cardId << std::endl;

          // Insert the userId->cardId mapping in the database.
          data.mv_size = sizeof(cardId);
          data.mv_data = &cardId;

          rc = mdb_put(txn, cardsForUser, &key, &data, 0);
          if (rc != 0) {
            lastCardId--;  // Undo increment.
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
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

          key.mv_data = &cardId;
          data.mv_size = size;
          data.mv_data = from;

          rc = mdb_put(txn, card, &key, &data, 0);
          if (rc != 0) {
            lastCardId--;  // Undo increment.
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          cout << "Inserted card: " << cardId << std::endl;
          auto cardResponse = responseKind.initCardResponse();
          cardResponse.setCard(newCard);
          cardResponse.setStatus(CardResponse::Status::CREATED);

          // FIXME: Need to insert Branch.io requestinto Branch.io worker queue.

          E(mdb_txn_commit(txn));
          txn = nullptr;

        } catch (...) {
          if (!responseKind.hasErrorResponse()) {
            auto error = responseKind.initErrorResponse();
            error.setMessage("An unknown error occurred. (5)");
          }
        }

        if (txn != nullptr) {
          mdb_txn_abort(txn);
          txn = nullptr;
        }

      } else if (which == Request::Kind::UPDATE_CARD_REQUEST) {
        int rc;
        MDB_txn *txn = nullptr;

        try {
          auto updateCardRequest = requestKind.getUpdateCardRequest();
          cout << "Update Card: " << requestId << endl;

          E(mdb_txn_begin(env, NULL, 0, &txn));

          uint64_t userId = 0;

          MDB_val key, data;

          key.mv_size = sizeof(deviceId);
          key.mv_data = &deviceId;

          rc = mdb_get(txn, userForDevice, &key, &data);
          if (rc != 0) {
            auto error = responseKind.initErrorResponse();
            if (rc == MDB_NOTFOUND) {
              cout << "User has not joined Blue yet" << std::endl;
              error.setMessage("User has not joined Blue yet!");

            } else {
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;
              error.setMessage(mdb_strerror(rc));
            }

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;

          } else {
            userId = *(uint64_t *)data.mv_data;
          }

          // Validation:

          // 1. User must have already created a card.
          key.mv_size = sizeof(userId);
          key.mv_data = &userId;

          MDB_cursor *cursor = nullptr;

          bool hasCardAlready = false;
          uint64_t cardId = 0;

          // We assume users only have one card.
          E(mdb_cursor_open(txn, cardsForUser, &cursor));
          if ((rc = mdb_cursor_get(cursor, &key, &data, MDB_FIRST)) == 0) {
            hasCardAlready = true;
            cardId = *(uint64_t *)data.mv_data;
          }
          mdb_cursor_close(cursor);
          cursor = nullptr;

          if (!hasCardAlready) {
            cout << "User has not created a card!" << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage("User has not created a card!");

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          key.mv_data = &cardId;

          rc = mdb_get(txn, card, &key, &data);
          if (rc != 0) {
            auto error = responseKind.initErrorResponse();
            if (rc == MDB_NOTFOUND) {
              cout << "Card not found?" << std::endl;
              error.setMessage("Card not found?");

            } else {
              cout << __LINE__ << " - mdb_get() error: " << mdb_strerror(rc) << std::endl;
              error.setMessage(mdb_strerror(rc));
            }

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          auto reader = makeReader(data);
          auto existingCard = reader.getRoot<Card>();

          // 2. Updated card must have the same id and version as the existing card.
          auto desiredCard = updateCardRequest.getCard();
          if (desiredCard.getId() != cardId || desiredCard.getVersion() != existingCard.getVersion()) {
            cout << "Existing card has a different id or version." << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage("Existing card has a different id or version.");

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
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

          rc = mdb_put(txn, card, &key, &data, 0);
          if (rc != 0) {
            cout << __LINE__ << " - mdb_put() error: " << mdb_strerror(rc) << std::endl;
            auto error = responseKind.initErrorResponse();
            error.setMessage(mdb_strerror(rc));

            mdb_txn_abort(txn);
            txn = nullptr;
            throw nullptr;
          }

          cout << "Updated card: " << cardId << std::endl;
          auto cardResponse = responseKind.initCardResponse();
          cardResponse.setCard(newCard);
          cardResponse.setStatus(CardResponse::Status::UPDATED);

          E(mdb_txn_commit(txn));
          txn = nullptr;

        } catch (...) {
          if (!responseKind.hasErrorResponse()) {
            auto error = responseKind.initErrorResponse();
            error.setMessage("An unknown error occurred. (6)");
          }
        }

        if (txn != nullptr) {
          mdb_txn_abort(txn);
          txn = nullptr;
        }

      } else {
        std::cout << "Unknown request kind" << (int)which << std::endl;
        auto error = responseKind.initErrorResponse();
        error.setMessage("Unknown request kind");
      }

      // We *always* send a response.
      kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
      kj::ArrayPtr<kj::byte> bytes = words.asBytes();

      socket.send((char *)(bytes.begin()), bytes.size(), uWS::OpCode::BINARY);
    });

    server.onDisconnection([](uWS::ServerSocket socket, int code, char *message, size_t length) {
      cout << "[Disconnection] clients: " << --connections << endl;

      void *data = socket.getData();
      size_t deviceId = data == nullptr ? 0 : (size_t)data;
      if (deviceId > 0) socketMap.erase(deviceId);

      socket.setData(nullptr);
    });

    server.run();

  } catch (...) {
    cout << "ERR_LISTEN" << endl;
  }

  return 0;
}
