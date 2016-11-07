//
//  test_main.cpp
//  Blue
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <map>

#include "gtest/gtest.h"

#include "server.h"

using namespace std;

#define DEVICE(id) DeviceModel device##id(id, uuid##id(), telephone##id(), email##id(), digitsId##id(), fullname##id())

struct DeviceModel {
  DeviceModel(uint64_t id_, unsigned char *uuid_, string telephone_, string email_, string digitsId_, string fullname_)
    : id(id_), requestId(0), telephone(telephone_), email(email_), emailVerified(true), digitsId(digitsId_), fullname(fullname_), cardId(0), cardVersion(0), lastDiscoveryTimestamp(0)
  {
    memcpy(uuid, uuid_, 16);
  }

  void reset() {
    requestId = 0;
  }

  auto nextRequestId() -> int64_t {
    return ++requestId;
  }

  auto getUserId() -> uint64_t {
    return std::stoull(telephone.c_str());
  }

  uint64_t id;
  int64_t requestId;
  unsigned char uuid[16];

  string telephone;
  string email;
  bool   emailVerified;
  string digitsId;
  string fullname;

  uint64_t cardId;
  uint32_t cardVersion;

  int64_t lastDiscoveryTimestamp;

  vector<pair<int64_t, uint64_t>> recentCards;                    // vector<timestamp, cardId>
  vector<pair<int64_t, uint64_t>> recentDevices;                  // vector<timestamp, deviceId>
  map<uint64_t, int64_t> sectionInfos;                            // sectionId -> timestamp
  map<uint64_t, vector<pair<int64_t, uint64_t>>> sectionsToCards; // sectionId -> vector<timestamp, cardId>

  void discover(int64_t timestamp, DeviceModel *device) {
    bool haveCard = false;
    for (auto recentCard : recentCards) {
      if (recentCard.second == device->cardId) {
        haveCard = true;
        break;
      }
    }

    if (not haveCard) {
      recentCards.push_back(make_pair(timestamp, device->cardId));
    }

    bool haveDevice = false;
    pair<int64_t, uint64_t> existingDevice;
    for (auto recentDevice : recentDevices) {
      if (recentDevice.second == device->id) {
        existingDevice = recentDevice;
        haveDevice = true;
        break;
      }
    }

    if (haveDevice) {
      auto it = find(recentDevices.begin(), recentDevices.end(), existingDevice);
      recentDevices.erase(it);
    }

    recentDevices.push_back(make_pair(timestamp, device->id));
  }
};

void ServerCallback::setDevicePublicKey(uint64_t deviceId, unsigned char *pk) {
  EXPECT_NE(deviceId, 0);

  m_callbackBuffers.deviceId = deviceId;
  m_callbackBuffers.didSetPublicKey = true;
  memcpy(m_callbackBuffers.devicePublicKey, pk, 32);
}

bool ServerCallback::sendEngineMessage(uint8_t *data, size_t length) {
  if (length > CALLBACK_BUFFER_SIZE) throw "Message too big";

  m_callbackBuffers.engineMessageBufferLength = length;
  memcpy(m_callbackBuffers.engineMessageBuffer, data, length);

  return true;
}

void ServerCallback::sendBranchMessage(const char *msg, size_t len) {
  if (len > CALLBACK_BUFFER_SIZE) throw "Message too big";

  m_callbackBuffers.branchMessageBufferLength = len;
  memcpy(m_callbackBuffers.branchMessageBuffer, msg, len);
}

void ServerCallback::sendTraceMessage(uint8_t *data, size_t length) {
  if (length > CALLBACK_BUFFER_SIZE) throw "Message too big";

  m_callbackBuffers.traceMessageBufferLength = length;
  memcpy(m_callbackBuffers.traceMessageBuffer, data, length);
}

void ServerCallback::sendDeviceMessage(uint64_t deviceId, const char *msg, size_t len) {
  if (len > CALLBACK_BUFFER_SIZE) throw "Message too big";

  m_callbackBuffers.messageDeviceId = deviceId;
  m_callbackBuffers.deviceMessageBufferLength = len;
  memcpy(m_callbackBuffers.deviceMessageBuffer, msg, len);
}

class ServerTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    char dir_template[] = "/tmp/lmdbXXXXXX";
    char *path = mkdtemp(dir_template);

    if (path == nullptr) cout << "errno: " << errno << endl;

    m_server = new Server(path);
    m_timestamp = 0;
  }

  virtual void TearDown() {
    delete m_server;
  }

  uint64_t getDeviceId() {
    return m_serverCallback.m_callbackBuffers.deviceId;
  }

  uint64_t getMessageDeviceId() {
    return m_serverCallback.m_callbackBuffers.messageDeviceId;
  }

  const capnp::word *getDeviceMessageBytes() {
    return (const capnp::word *)m_serverCallback.m_callbackBuffers.deviceMessageBuffer;
  }

  size_t getDeviceMessageSize() {
    EXPECT_EQ(0, m_serverCallback.m_callbackBuffers.deviceMessageBufferLength % 8);
    return m_serverCallback.m_callbackBuffers.deviceMessageBufferLength / 8;
  }

  bool didSetDevicePublicKey() {
    return m_serverCallback.m_callbackBuffers.didSetPublicKey;
  }

  bool didSendDeviceMessage() {
    return m_serverCallback.m_callbackBuffers.deviceMessageBufferLength > 0;
  }

  void resetCallbackBuffers() {
    m_serverCallback.m_callbackBuffers.deviceId = 0;
    m_serverCallback.m_callbackBuffers.messageDeviceId = 0;
    m_serverCallback.m_callbackBuffers.didSetPublicKey = false;
    m_serverCallback.m_callbackBuffers.engineMessageBufferLength = 0;
    m_serverCallback.m_callbackBuffers.branchMessageBufferLength = 0;
    m_serverCallback.m_callbackBuffers.traceMessageBufferLength  = 0;
    m_serverCallback.m_callbackBuffers.deviceMessageBufferLength = 0;
  }

  auto uuid1() -> unsigned char * {
    static unsigned char uuid[16] = { 0xDA, 0x42, 0x27, 0x8F, 0xC0, 0x95, 0x49, 0x6F, 0xA0, 0xAE, 0xA9, 0xB2, 0x53, 0x4E, 0xAC, 0x21 };
    return uuid;
  }

  auto uuid2() -> unsigned char * {
    static unsigned char uuid[16] = { 0x18, 0x81, 0xA1, 0x4F, 0x7F, 0x5A, 0x47, 0x4A, 0x93, 0x31, 0x10, 0xDD, 0x0C, 0x1E, 0x48, 0x48 };
    return uuid;
  }

  auto uuid3() -> unsigned char * {
    static unsigned char uuid[16] = { 0x84, 0x6B, 0xD7, 0x7C, 0x05, 0xF4, 0x42, 0x24, 0xB5, 0x28, 0xEA, 0x39, 0x25, 0x8A, 0x60, 0x9A };
    return uuid;
  }

  auto uuid4() -> unsigned char * {
    static unsigned char uuid[16] = { 0x71, 0x20, 0x9A, 0x5E, 0x60, 0x0A, 0x47, 0x63, 0x94, 0xBB, 0x59, 0x65, 0xF5, 0xF5, 0x74, 0x21 };
    return uuid;
  }

  auto uuid5() -> unsigned char * {
    static unsigned char uuid[16] = { 0x39, 0x98, 0x9A, 0xD6, 0x58, 0x2A, 0x41, 0xF7, 0x99, 0x85, 0xB2, 0x64, 0x9C, 0xF8, 0x5D, 0xB5 };
    return uuid;
  }

  auto telephone1() -> string { return "13235551212"; }
  auto telephone2() -> string { return "13235551213"; }
  auto telephone3() -> string { return "13235551214"; }
  auto telephone4() -> string { return "13235551215"; }
  auto telephone5() -> string { return "13235551216"; }

  auto email1() -> string { return "13235551212@example.com"; }
  auto email2() -> string { return "13235551213@example.com"; }
  auto email3() -> string { return "13235551214@example.com"; }
  auto email4() -> string { return "13235551215@example.com"; }
  auto email5() -> string { return "13235551216@example.com"; }

  auto digitsId1() -> string { return "digits13235551212"; }
  auto digitsId2() -> string { return "digits13235551213"; }
  auto digitsId3() -> string { return "digits13235551214"; }
  auto digitsId4() -> string { return "digits13235551215"; }
  auto digitsId5() -> string { return "digits13235551216"; }

  auto fullname1() -> string { return "Erich Ocean"; }
  auto fullname2() -> string { return "Lauren Ocean"; }
  auto fullname3() -> string { return "Erik van der Tier"; }
  auto fullname4() -> string { return "Mickey Mouse"; }
  auto fullname5() -> string { return "Minnie Mouse"; }

  void sendHelloRequest(DeviceModel &device) {
    resetCallbackBuffers();

    unsigned char publicKeyBytes[32] = { 0 };

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    HelloRequest::Builder helloRequest = request.getKind().initHelloRequest();
    auto data = helloRequest.initUuid(16);

    for (int idx=0, len=16; idx<len; ++idx) {
      data[idx] = device.uuid[idx];
    }

    helloRequest.setVersion(4);

    auto pk = helloRequest.initPublicKey(32);
    for (int idx=0, len=32; idx<len; ++idx) {
      pk[idx] = publicKeyBytes[idx];
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(0, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  void sendJoinRequest(DeviceModel &device) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    JoinRequest::Builder joinRequest = request.getKind().initJoinRequest();
    joinRequest.setTelephone(device.telephone);

    joinRequest.setEmail(device.email);
    joinRequest.setEmailVerified(device.emailVerified);
    joinRequest.setDigitsId(device.digitsId);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  void sendCreateCardRequest(DeviceModel &device) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    CreateCardRequest::Builder createCardRequest = request.getKind().initCreateCardRequest();
    auto card = createCardRequest.initCard();
    card.setFullName(device.fullname);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);

    // Gather the card response, if successful.
    {
      kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
      capnp::FlatArrayMessageReader responseReader(view);

      Response::Reader response = responseReader.getRoot<Response>();
      auto kind = response.getKind();
      auto which = kind.which();

      if (which != Response::Kind::CARD_RESPONSE) return;

      auto cardResponse = kind.getCardResponse();
      if (not cardResponse.hasCard()) return;

      auto card = cardResponse.getCard();
      device.cardId = card.getId();
      device.cardVersion = card.getVersion();
    }
  }

  void sendUpdateCardRequest(DeviceModel &device, uint64_t cardId, uint32_t version, string fullname) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    UpdateCardRequest::Builder updateCardRequest = request.getKind().initUpdateCardRequest();
    auto card = updateCardRequest.initCard();
    card.setId(cardId);
    card.setVersion(version);
    card.setFullName(fullname);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  void sendBranchRequest() {
    resetCallbackBuffers();

    stringstream ss;
    ss << 0;
    string request = ss.str();
    const char *cRequest = request.c_str();

    m_server->handleBranchMessage(cRequest, strlen(cRequest), &m_serverCallback);
  }

  void sendBranchResponse(uint64_t branchRequest) {
    resetCallbackBuffers();

    stringstream ss;
    ss << branchRequest;
    string request = ss.str();
    const char *cRequest = request.c_str();

    m_server->handleBranchMessage(cRequest, strlen(cRequest), &m_serverCallback);
  }

  auto getBranchResponse() -> char * {
    return strndup((char *)m_serverCallback.m_callbackBuffers.branchMessageBuffer, m_serverCallback.m_callbackBuffers.branchMessageBufferLength);
  }

  void sendDiscoveryRequest(DeviceModel &device) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    DiscoveryRequest::Builder discoveryRequest = request.getKind().initDiscoveryRequest();
    discoveryRequest.setLastDiscoveryTimestamp(device.lastDiscoveryTimestamp);

    vector<pair<int64_t, uint64_t>> newDevices;
    for (auto devicePair : device.recentDevices) {
      if (devicePair.first > device.lastDiscoveryTimestamp) {
        newDevices.push_back(devicePair);
      }
    }

    if (not newDevices.empty()) {
      auto devices = discoveryRequest.initDevices(newDevices.size());
      for (size_t idx=0, len=newDevices.size(); idx<len; ++idx) {
        auto devicePair = newDevices[idx];
        devices[0].setTimestamp(devicePair.first);
        devices[0].setId(devicePair.second);
      }
    }

    vector<pair<int64_t, uint64_t>> newCards;
    for (auto cardPair : device.recentCards) {
      if (cardPair.first > device.lastDiscoveryTimestamp) {
        newCards.push_back(cardPair);
      }
    }

    if (not newCards.empty()) {
      auto cards = discoveryRequest.initCards(newCards.size());
      for (size_t idx=0, len=newCards.size(); idx<len; ++idx) {
        auto cardPair = newCards[idx];
        cards[0].setTimestamp(cardPair.first);
        cards[0].setId(cardPair.second);
      }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  void sendCreateBackupRequest(DeviceModel &device, int64_t timestamp, int64_t previousBackup) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    CreateBackupRequest::Builder createBackupRequest = request.getKind().initCreateBackupRequest();
    createBackupRequest.setPreviousBackup(previousBackup);

    auto backup = createBackupRequest.initBackup();
    backup.setTimestamp(timestamp);

    auto data = backup.initData(16);
    for (int idx=0, len=16; idx<len; ++idx) {
      data[idx] = device.uuid[idx];
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  void sendBackupListRequest(DeviceModel &device) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    request.getKind().initBackupListRequest();

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  void sendBackupRequest(DeviceModel &device, int64_t timestamp) {
    resetCallbackBuffers();

    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(device.nextRequestId());

    BackupRequest::Builder backupRequest = request.getKind().initBackupRequest();
    backupRequest.setTimestamp(timestamp);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    m_server->handleClientMessage(device.id, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);
  }

  auto getTimestamp(int64_t amountToAdvanceInMilliseconds = 0) -> int64_t {
    m_timestamp += amountToAdvanceInMilliseconds;
    return m_timestamp;
  }

  Server *m_server;
  ServerCallback m_serverCallback;

  int64_t m_timestamp;
};

TEST_F(ServerTest, Empty) {
  EXPECT_NE(0, m_server->m_databaseId);
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(0, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);
}

TEST_F(ServerTest, HelloRequest) {
  DEVICE(1);
  DEVICE(2);

  sendHelloRequest(device1);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device1.id, getDeviceId());
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_TRUE(helloResponse.getIsClientCompatible());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }

  // Sending Hello a second time should'n change anything.
  sendHelloRequest(device1);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device1.id, getDeviceId());
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_TRUE(helloResponse.getIsClientCompatible());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }

  sendHelloRequest(device2);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device2.id, getDeviceId());
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_TRUE(helloResponse.getIsClientCompatible());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }

  // Sending Hello again on the second device shouldn't change anything.
  sendHelloRequest(device2);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device2.id, getDeviceId());
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_TRUE(helloResponse.getIsClientCompatible());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }

  // Sending Hello again on the first device shouldn't change anything.
  sendHelloRequest(device1);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device1.id, getDeviceId());
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_TRUE(helloResponse.getIsClientCompatible());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }
}

TEST_F(ServerTest, JoinRequest) {
  DEVICE(1);
  DEVICE(2);

  sendHelloRequest(device1);
  sendJoinRequest(device1);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::JOIN_RESPONSE, which);

    auto joinResponse = kind.getJoinResponse();
    EXPECT_EQ(JoinResponse::Status::NEW, joinResponse.getStatus());
    EXPECT_FALSE(joinResponse.hasCard());

    auto user = joinResponse.getUser();
    EXPECT_EQ(1, user.getId());
    EXPECT_EQ(device1.id, user.getActiveDevice());
    EXPECT_STREQ(device1.telephone.c_str(), user.getTelephone().cStr());
    EXPECT_STREQ(device1.email.c_str(), user.getEmail().cStr());
    EXPECT_EQ(device1.emailVerified, user.getEmailVerified());

    auto data = user.getActiveDeviceUUID();
    for (int idx=0, len=16; idx<len; ++idx) {
      EXPECT_EQ(device1.uuid[idx], data[idx]);
    }

    EXPECT_EQ(0, user.getCard());
  }

  // After we've joined, when we say Hello with the same device as the one we
  // joined with earlier, we do not need to join again with that device.
  device1.reset();
  sendHelloRequest(device1);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_FALSE(helloResponse.getJoinRequired());
  }

  // Saying hello with a new device for the first time should still require a join.
  sendHelloRequest(device2);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device2.id, getDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_TRUE(helloResponse.getIsClientCompatible());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }

  // Joining with the second device should replace the previous device, and the
  // join response should indicate an existing user.
  device2.telephone = device1.telephone;
  device2.digitsId = device1.digitsId;
  device2.email = device1.email;
  sendJoinRequest(device2);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device2.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::JOIN_RESPONSE, which);

    auto joinResponse = kind.getJoinResponse();
    EXPECT_EQ(JoinResponse::Status::EXISTING, joinResponse.getStatus());
    EXPECT_FALSE(joinResponse.hasCard());

    auto user = joinResponse.getUser();
    EXPECT_EQ(1, user.getId());
    EXPECT_EQ(device2.id, user.getActiveDevice());
    EXPECT_STREQ(device2.telephone.c_str(), user.getTelephone().cStr());
    EXPECT_STREQ(device2.email.c_str(), user.getEmail().cStr());
    EXPECT_EQ(device2.emailVerified, user.getEmailVerified());

    auto data = user.getActiveDeviceUUID();
    for (int idx=0, len=16; idx<len; ++idx) {
      EXPECT_EQ(device2.uuid[idx], data[idx]);
    }

    EXPECT_EQ(0, user.getCard());
  }

  // Reconnecting after joining with the second device should indicate that a
  // join is not required.
  device2.reset();
  sendHelloRequest(device2);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device2.id, getDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_FALSE(helloResponse.getJoinRequired());
  }

  // If we try and join when not requiring, we should still get back a correct join response.
  sendJoinRequest(device2);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device2.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::JOIN_RESPONSE, which);

    auto joinResponse = kind.getJoinResponse();
    EXPECT_EQ(JoinResponse::Status::EXISTING, joinResponse.getStatus());
    EXPECT_FALSE(joinResponse.hasCard());

    auto user = joinResponse.getUser();
    EXPECT_EQ(1, user.getId());
    EXPECT_EQ(device2.id, user.getActiveDevice());
    EXPECT_STREQ(device2.telephone.c_str(), user.getTelephone().cStr());
    EXPECT_STREQ(device2.email.c_str(), user.getEmail().cStr());
    EXPECT_EQ(device2.emailVerified, user.getEmailVerified());

    auto data = user.getActiveDeviceUUID();
    for (int idx=0, len=16; idx<len; ++idx) {
      EXPECT_EQ(device2.uuid[idx], data[idx]);
    }

    EXPECT_EQ(0, user.getCard());
  }

  // If we say hello with a previous device, it should indicate that we do need to re-join.
  device1.reset();
  sendHelloRequest(device1);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(device1.id, getDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }

  // We can still re-join with the previous device.
  sendJoinRequest(device1);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::JOIN_RESPONSE, which);

    auto joinResponse = kind.getJoinResponse();
    EXPECT_EQ(JoinResponse::Status::EXISTING, joinResponse.getStatus());
    EXPECT_FALSE(joinResponse.hasCard());

    auto user = joinResponse.getUser();
    EXPECT_EQ(1, user.getId());
    EXPECT_EQ(device1.id, user.getActiveDevice());

    auto data = user.getActiveDeviceUUID();
    for (int idx=0, len=16; idx<len; ++idx) {
      EXPECT_EQ(device1.uuid[idx], data[idx]);
    }

    EXPECT_EQ(0, user.getCard());
  }

  // If we say hello again with the second device, it should indicate that we do need to re-join.
  device2.reset();
  sendHelloRequest(device2);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(2, getDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(2, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }
}

TEST_F(ServerTest, UpgradeRequired) {
  int64_t requestId = 1;
  unsigned char uuid[16] = { 0xDA, 0x42, 0x27, 0x8F, 0xC0, 0x95, 0x49, 0x6F, 0xA0, 0xAE, 0xA9, 0xB2, 0x53, 0x4E, 0xAC, 0x21 };
  unsigned char publicKeyBytes[32] = { 0 };

  capnp::MallocMessageBuilder builder;
  Request::Builder request = builder.initRoot<Request>();
  request.setId(requestId);

  HelloRequest::Builder helloRequest = request.getKind().initHelloRequest();
  auto data = helloRequest.initUuid(16);

  for (int idx=0, len=16; idx<len; ++idx) {
    data[idx] = uuid[idx];
  }

  helloRequest.setVersion(1); // <= This is the important value!

  auto pk = helloRequest.initPublicKey(32);
  for (int idx=0, len=32; idx<len; ++idx) {
    pk[idx] = publicKeyBytes[idx];
  }

  kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
  kj::ArrayPtr<kj::byte> bytes = words.asBytes();

  m_server->handleClientMessage(0, (char *)bytes.begin(), bytes.size(), false, &m_serverCallback);

  EXPECT_TRUE(didSetDevicePublicKey());
  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(getDeviceId(), getMessageDeviceId());
  EXPECT_EQ(1, getDeviceId());
  EXPECT_EQ(0, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(0, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(0, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(requestId, responseId);
    EXPECT_EQ(Response::Kind::HELLO_RESPONSE, which);

    auto helloResponse = kind.getHelloResponse();
    EXPECT_FALSE(helloResponse.hasDiscoveryResponse());
    EXPECT_EQ(m_server->m_databaseId, helloResponse.getDatabaseID());
    EXPECT_FALSE(helloResponse.getIsClientCompatible()); // <= This is the key result!
    EXPECT_TRUE(helloResponse.getJoinRequired());
  }
}

TEST_F(ServerTest, CreateCardRequest) {
  DEVICE(1);

  string telephone = "13235551212";
  unsigned long long userId = 1;
  string fullname = "Erich Ocean";

  sendHelloRequest(device1);
  sendJoinRequest(device1);

  sendBranchRequest();
  char *response = getBranchResponse();
  EXPECT_STREQ("{}", response);

  sendCreateCardRequest(device1);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(1, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(1, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::CARD_RESPONSE, which);

    auto cardResponse = kind.getCardResponse();
    EXPECT_EQ(cardResponse.getStatus(), CardResponse::Status::CREATED);
    EXPECT_TRUE(cardResponse.hasCard());

    auto card = cardResponse.getCard();
    EXPECT_EQ(1, card.getId());
    EXPECT_STREQ(fullname.c_str(), card.getFullName().cStr());
  }

  // Verify the Branch.io request was added successfully.
  {
    int rc;
    MDB_txn *txn = nullptr;
    MDB_val key, data;

    rc = mdb_txn_begin(m_server->m_env, NULL, MDB_RDONLY, &txn);
    EXPECT_EQ(0, rc);

    uint64_t branchRequest = 1;

    key.mv_size = sizeof(branchRequest);
    key.mv_data = &branchRequest;

    rc = mdb_get(txn, m_server->m_branchRequest, &key, &data);
    EXPECT_EQ(0, rc);

    char *request = strndup((char *)data.mv_data, data.mv_size);
    EXPECT_STREQ("{ \"cardId\": 1, \"fullName\": \"Erich Ocean\" }", request);
    free(request);

    mdb_txn_abort(txn);
  }

  sendBranchRequest();
  response = getBranchResponse();
  EXPECT_STREQ("{ \"id\": 1, \"request\": { \"cardId\": 1, \"fullName\": \"Erich Ocean\" } }", response);

  // Simulate fullfilling the Branch.io request.
  sendBranchResponse(1);
  response = getBranchResponse();
  EXPECT_STREQ("OK", response);

  {
    int rc;
    MDB_txn *txn = nullptr;
    MDB_val key, data;

    rc = mdb_txn_begin(m_server->m_env, NULL, MDB_RDONLY, &txn);
    EXPECT_EQ(0, rc);

    uint64_t branchRequest = 1;

    key.mv_size = sizeof(branchRequest);
    key.mv_data = &branchRequest;

    rc = mdb_get(txn, m_server->m_branchRequest, &key, &data);
    EXPECT_EQ(MDB_NOTFOUND, rc);

    mdb_txn_abort(txn);
  }

  // If we send a join request now, we should get the card we just created back.
  sendJoinRequest(device1);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(1, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(1, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::JOIN_RESPONSE, which);

    auto joinResponse = kind.getJoinResponse();
    EXPECT_EQ(joinResponse.getStatus(), JoinResponse::Status::EXISTING);
    EXPECT_TRUE(joinResponse.hasUser());
    EXPECT_TRUE(joinResponse.hasCard());

    auto user = joinResponse.getUser();
    EXPECT_EQ(userId, user.getId());
    EXPECT_EQ(device1.id, user.getActiveDevice());
    EXPECT_EQ(1, user.getCard());

    auto card = joinResponse.getCard();
    EXPECT_EQ(1, card.getId());
    EXPECT_EQ(1, card.getVersion());
    EXPECT_STREQ(fullname.c_str(), card.getFullName().cStr());
  }
}

TEST_F(ServerTest, UpdateCardRequest) {
  DEVICE(1);

  string telephone = "13235551212";
  string fullname = "Erich Ocean";
  string fullname2 = "Erich Atlas Ocean";

  sendHelloRequest(device1);
  sendJoinRequest(device1);
  sendCreateCardRequest(device1);

  uint64_t cardId = 1;
  uint32_t version = 1;

  sendUpdateCardRequest(device1, cardId, version, fullname2);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastUserId);
  EXPECT_EQ(0, m_server->m_lastBackupId);
  EXPECT_EQ(1, m_server->m_lastCardId);
  EXPECT_EQ(1, m_server->m_lastDeviceId);
  EXPECT_EQ(0, m_server->m_lastSectionId);
  EXPECT_EQ(2, m_server->m_lastBranchRequest);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::CARD_RESPONSE, which);

    auto cardResponse = kind.getCardResponse();
    EXPECT_EQ(cardResponse.getStatus(), CardResponse::Status::UPDATED);
    EXPECT_TRUE(cardResponse.hasCard());

    auto card = cardResponse.getCard();
    EXPECT_EQ(cardId, card.getId());
    EXPECT_EQ(version + 1, card.getVersion());
    EXPECT_STREQ(fullname2.c_str(), card.getFullName().cStr());
  }
}

TEST_F(ServerTest, DeviceModel) {
  DEVICE(1);
  DEVICE(2);
  DEVICE(3);
  DEVICE(4);
  DEVICE(5);

  // Set up each device so that they all have cards.
  vector<DeviceModel *> devices = { &device1, &device2, &device3, &device4, &device5 };
  for (auto device : devices) {
    sendHelloRequest(*device);
    sendJoinRequest(*device);
    sendCreateCardRequest(*device);

    EXPECT_EQ(0, device->lastDiscoveryTimestamp);
    EXPECT_TRUE(device->recentDevices.empty());
    EXPECT_TRUE(device->recentCards.empty());
    EXPECT_TRUE(device->sectionInfos.empty());
    EXPECT_TRUE(device->sectionsToCards.empty());
  }

  // Discovering new cards/device should work.
  device1.discover(getTimestamp(), &device2);

  EXPECT_TRUE(device1.sectionInfos.empty());
  EXPECT_TRUE(device1.sectionsToCards.empty());

  EXPECT_EQ(1, device1.recentDevices.size());
  EXPECT_EQ(1, device1.recentCards.size());
  EXPECT_TRUE(find(device1.recentDevices.begin(), device1.recentDevices.end(), make_pair(getTimestamp(), device2.id)) != device1.recentDevices.end());
  EXPECT_TRUE(find(device1.recentCards.begin(), device1.recentCards.end(), make_pair(getTimestamp(), device2.cardId)) != device1.recentCards.end());

  device1.discover(getTimestamp(480000), &device3);

  EXPECT_EQ(2, device1.recentDevices.size());
  EXPECT_EQ(2, device1.recentCards.size());
  EXPECT_TRUE(find(device1.recentDevices.begin(), device1.recentDevices.end(), make_pair(getTimestamp(), device3.id)) != device1.recentDevices.end());
  EXPECT_TRUE(find(device1.recentCards.begin(), device1.recentCards.end(), make_pair(getTimestamp(), device3.cardId)) != device1.recentCards.end());

  // Rediscovering an existing card/device should increment the device timestamp and leave the card alone.
  device1.discover(getTimestamp(480000), &device2);

  EXPECT_EQ(2, device1.recentDevices.size());
  EXPECT_EQ(2, device1.recentCards.size());
  EXPECT_TRUE(find(device1.recentDevices.begin(), device1.recentDevices.end(), make_pair(getTimestamp(), device2.id)) != device1.recentDevices.end());
  EXPECT_TRUE(find(device1.recentCards.begin(), device1.recentCards.end(), make_pair(getTimestamp() - 960000, device2.cardId)) != device1.recentCards.end());
}

TEST_F(ServerTest, DiscoveryRequest) {
  DEVICE(1);
  DEVICE(2);
  DEVICE(3);
  DEVICE(4);
  DEVICE(5);

  // Set up each device so that they all have cards.
  vector<DeviceModel *> devices = { &device1, &device2, &device3, &device4, &device5 };
  for (auto device : devices) {
    sendHelloRequest(*device);
    sendJoinRequest(*device);
    sendCreateCardRequest(*device);
  }

  // Discover some cards/devices.
  device1.discover(getTimestamp(1500), &device2);
  device1.discover(getTimestamp(480000), &device3);

  // Send a discovery request.
  sendDiscoveryRequest(device1);
}

TEST_F(ServerTest, Backup) {
  DEVICE(1);
  DEVICE(2);

  // Set up each device so that they all have cards.
  vector<DeviceModel *> devices = { &device1, &device2 };
  for (auto device : devices) {
    sendHelloRequest(*device);
    sendJoinRequest(*device);
    sendCreateCardRequest(*device);
  }

  EXPECT_EQ(0, m_server->m_lastBackupId);

  // Asking for a backup for a new user should return a timestamp of zero,
  // and no data.
  sendBackupRequest(device1, 0);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(0, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_RESPONSE, which);

    auto backupResponse = kind.getBackupResponse();
    EXPECT_TRUE(backupResponse.hasBackup());

    auto backup = backupResponse.getBackup();
    EXPECT_EQ(0, backup.getTimestamp());
    EXPECT_FALSE(backup.hasData());
  }

  // Send a create backup request.
  sendCreateBackupRequest(device1, 1500, 0);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_RESPONSE, which);

    auto backupResponse = kind.getBackupResponse();
    EXPECT_TRUE(backupResponse.hasBackup());

    auto backup = backupResponse.getBackup();
    EXPECT_EQ(1500, backup.getTimestamp());
    EXPECT_FALSE(backup.hasData());
  }

  // Sending a second backup with a new timestamp, and an incorrect previousBackup, should error
  sendCreateBackupRequest(device1, 2500, 42);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(1, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::ERROR_RESPONSE, which);

    auto errorResponse = kind.getErrorResponse();
    EXPECT_EQ(ErrorResponse::Code::PREVIOUS_BACKUP_NOT_APPLIED, errorResponse.getCode());
  }

  // Sending a second backup with a new timestamp, and correct previousBackup, should work
  sendCreateBackupRequest(device1, 2500, 1500);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(2, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_RESPONSE, which);

    auto backupResponse = kind.getBackupResponse();
    EXPECT_TRUE(backupResponse.hasBackup());

    auto backup = backupResponse.getBackup();
    EXPECT_EQ(2500, backup.getTimestamp());
    EXPECT_FALSE(backup.hasData());
  }

  // Asking for an exact backup should work, even if it's not the most recent.
  sendBackupRequest(device1, 1500);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(2, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_RESPONSE, which);

    auto backupResponse = kind.getBackupResponse();
    EXPECT_TRUE(backupResponse.hasBackup());

    auto backup = backupResponse.getBackup();
    EXPECT_EQ(1500, backup.getTimestamp());
    EXPECT_TRUE(backup.hasData());

    auto data = backup.getData();
    EXPECT_EQ(16, data.size());
    for (int idx=0, len=16; idx<len; ++idx) {
      EXPECT_EQ(device1.uuid[idx], data[idx]);
    }
  }

  // Asking for the latest backup should work.
  sendBackupRequest(device1, 0);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(2, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_RESPONSE, which);

    auto backupResponse = kind.getBackupResponse();
    EXPECT_TRUE(backupResponse.hasBackup());

    auto backup = backupResponse.getBackup();
    EXPECT_EQ(2500, backup.getTimestamp());
    EXPECT_TRUE(backup.hasData());

    auto data = backup.getData();
    EXPECT_EQ(16, data.size());
    for (int idx=0, len=16; idx<len; ++idx) {
      EXPECT_EQ(device1.uuid[idx], data[idx]);
    }
  }

  // Asking for a list of backups should return them, and in the correct order.
  sendBackupListRequest(device1);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device1.id, getMessageDeviceId());
  EXPECT_EQ(2, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device1.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_LIST_RESPONSE, which);

    auto backupListResponse = kind.getBackupListResponse();
    EXPECT_TRUE(backupListResponse.hasBackups());

    auto backups = backupListResponse.getBackups();
    EXPECT_EQ(2, backups.size());

    auto backup1 = backups[0];
    EXPECT_EQ(1500, backup1.getTimestamp());
    EXPECT_FALSE(backup1.hasData());

    auto backup2 = backups[1];
    EXPECT_EQ(2500, backup2.getTimestamp());
    EXPECT_FALSE(backup2.hasData());
  }

  // Asking for a list of backups should return an empty list, if there are none.
  sendBackupListRequest(device2);

  EXPECT_TRUE(didSendDeviceMessage());
  EXPECT_EQ(device2.id, getMessageDeviceId());
  EXPECT_EQ(2, m_server->m_lastBackupId);

  {
    kj::ArrayPtr<const capnp::word> view(getDeviceMessageBytes(), getDeviceMessageSize());
    capnp::FlatArrayMessageReader responseReader(view);

    Response::Reader response = responseReader.getRoot<Response>();
    auto responseId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    EXPECT_EQ(device2.requestId, responseId);
    EXPECT_EQ(Response::Kind::BACKUP_LIST_RESPONSE, which);

    auto backupListResponse = kind.getBackupListResponse();
    EXPECT_TRUE(backupListResponse.hasBackups());

    auto backups = backupListResponse.getBackups();
    EXPECT_EQ(0, backups.size());
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
