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
#include <stdlib.h>
#include <string>
#include <thread>

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/util/CommandOptionParser.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"

#include "libuv/uv.h"
#include "tweetnacl/tweetnacl.h"
#include "uWebSockets/uWS.h"

#include "server.h"

using namespace std;
using namespace aeron;
using namespace aeron::util;

const static string  DEFAULT_SERVER_CHANNEL       = "aeron:udp?endpoint=localhost:40123";
const static string  DEFAULT_ENGINE_CHANNEL       = "aeron:udp?endpoint=localhost:40124";
const static string  DEFAULT_TRACE_CHANNEL        = "aeron:udp?endpoint=localhost:40125";
const static int32_t DEFAULT_SERVER_STREAM_ID     = 10;
const static int32_t DEFAULT_ENGINE_STREAM_ID     = 10;
const static int32_t DEFAULT_TRACE_STREAM_ID      = 10;
const static int     DEFAULT_FRAGMENT_COUNT_LIMIT = 10;

static atomic<bool> shouldPoll (false);

static shared_ptr<Publication>  serverPublication  = nullptr;
static shared_ptr<Subscription> engineSubscription = nullptr;
static shared_ptr<Publication>  tracePublication  = nullptr;

static const char optHelp = 'h';

// Aeron options
static const char optPrefix         = 'p';
static const char optFrags          = 'f';
static const char optServerChannel  = 's';
static const char optServerStreamId = 'S';
static const char optEngineChannel  = 'e';
static const char optEngineStreamId = 'E';
static const char optTraceChannel   = 't';
static const char optTraceStreamId  = 'T';

struct Settings {
  string  dirPrefix          = "/Users/erich/Desktop/aeron";
  int32_t fragmentCountLimit = DEFAULT_FRAGMENT_COUNT_LIMIT;

  string  serverChannel      = DEFAULT_SERVER_CHANNEL;
  int32_t serverStreamId     = DEFAULT_SERVER_STREAM_ID;

  string  engineChannel      = DEFAULT_ENGINE_CHANNEL;
  int32_t engineStreamId     = DEFAULT_ENGINE_STREAM_ID;

  string  traceChannel       = DEFAULT_TRACE_CHANNEL;
  int32_t traceStreamId      = DEFAULT_TRACE_STREAM_ID;
};

static Settings settings;

auto parseCmdLine(CommandOptionParser& cp, int argc, char** argv) -> Settings {
  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.dirPrefix          = cp.getOption(optPrefix)        .getParam(0, s.dirPrefix);
  s.fragmentCountLimit = cp.getOption(optFrags)         .getParamAsInt(0, 1, INT32_MAX, s.fragmentCountLimit);

  s.serverChannel      = cp.getOption(optServerChannel) .getParam(0, s.serverChannel);
  s.serverStreamId     = cp.getOption(optServerStreamId).getParamAsInt(0, 1, INT32_MAX, s.serverStreamId);

  s.engineChannel      = cp.getOption(optEngineChannel) .getParam(0, s.engineChannel);
  s.engineStreamId     = cp.getOption(optEngineStreamId).getParamAsInt(0, 1, INT32_MAX, s.engineStreamId);

  s.traceChannel       = cp.getOption(optTraceChannel)  .getParam(0, s.traceChannel);
  s.traceStreamId      = cp.getOption(optTraceStreamId) .getParamAsInt(0, 1, INT32_MAX, s.traceStreamId);

  return s;
}

static std::map<uint64_t, uv_poll_t *> socketMap; // maps a deviceId to a pointer that can be used to create a uWS::Socket object

static Server *server = nullptr;
static ServerCallback serverCallback;

static int connections = 0;

static uv_poll_t *branchSocket = nullptr;

void engineMessageHandler(AtomicBuffer& buffer, index_t offset, index_t length) {
  server->handleEngineMessage(buffer.buffer() + offset, length, &serverCallback);
}

static uv_timer_t timerHandle;

void timerCallback(uv_timer_t* handle) {
  if (!shouldPoll) return;

  // Poll for messages from discovery engine.
  FragmentAssembler fragmentAssembler([&](AtomicBuffer& buffer, index_t offset, index_t length, Header& header) {
    engineMessageHandler(buffer, offset, length);
  });

  engineSubscription->poll(fragmentAssembler.handler(), settings.fragmentCountLimit);
}

static unsigned char *sk = nullptr;
static unsigned char *branchPublicKey = nullptr;

void configureCrypto() {
  sk = (unsigned char *)malloc(32);

#ifdef NDEBUG
  FILE *f = fopen("/Users/erich/Desktop/Follow/follow/key_gen/SECRET_KEY", "r");

  if (!f || fread((void *)sk, 1, 32, f) != 32) {
    fclose(f);
    exit(1);
  }

  fclose(f);

#else
  // This is the debugging-only secret key.
  unsigned char debug_sk[32] = {
    0x38, 0x84, 0x3e, 0xfb, 0xb9, 0x88, 0xc8, 0xb8,
    0x9a, 0x84, 0x1e, 0xeb, 0x60, 0xb1, 0xa5, 0x06,
    0x3b, 0x45, 0xe9, 0xa4, 0x98, 0xab, 0xc1, 0x26,
    0xf0, 0x82, 0xbf, 0x58, 0xd5, 0xf9, 0xa3, 0x34
  };

  memcpy(sk, debug_sk, 32);
#endif

  // This is the Branch.io public key setup.
  branchPublicKey = (unsigned char *)malloc(32);

  unsigned char branch_pk[32] = {
    0xb1, 0x67, 0xd8, 0x3e, 0x31, 0x4e, 0xfd, 0x81,
    0x87, 0x21, 0x8e, 0x9c, 0x86, 0xf4, 0x1c, 0xd2,
    0x8a, 0x25, 0xc0, 0x77, 0x46, 0x4a, 0xc1, 0x3c,
    0x01, 0x0b, 0x41, 0xf5, 0xcd, 0x97, 0x04, 0x7e
  };

  memcpy(branchPublicKey, branch_pk, 32);
}

bool ServerCallback::sendEngineMessage(uint8_t *data, size_t length) {
  concurrent::AtomicBuffer srcBuffer(data, length);
  const std::int64_t result = serverPublication->offer(srcBuffer, 0, length);
  if (result < 0L) {
    switch(result) {
      case NOT_CONNECTED:
        cout << "sendEngineMessage() failed: No subscriber is connected to the engine Publication." << endl;
        break;
      case BACK_PRESSURED:
        cout << "sendEngineMessage() failed: The message was not sent due to back pressure from Subscribers, but can be retried if desired." << endl;
        break;
      case ADMIN_ACTION:
        cout << "sendEngineMessage() failed: The message was not sent due to an administration action, such as log rotation, but can be retried if desired." << endl;
        break;
      case PUBLICATION_CLOSED:
        cout << "sendEngineMessage() failed: The message was not sent due to the engine Publication being closed. This is a permanent error." << endl;
        break;
      default:
        cout << "sendEngineMessage() failed for an unknown reason." << endl;
    }

    return false;

  } else {
    return true;
  }
}

void ServerCallback::sendBranchMessage(const char *msg, size_t len) {
  if (branchSocket == nullptr) return;

  uWS::ServerSocket socket(branchSocket);

  // We always encrypt messages.
  // TODO: We can optimize the memory allocations here.

  // Create the nonce.
  unsigned char *nonce = (unsigned char *)malloc(crypto_box_NONCEBYTES);
  randombytes(nonce, crypto_box_NONCEBYTES);

  // Create a buffer to encrypt message from and into.
  const unsigned char *encryptionSource = (unsigned char *)malloc(len + 32);
  const unsigned char *encryptionDestination = (unsigned char *)malloc(len + 32);

  // Create the final packet.
  const unsigned char *packet = (unsigned char *)malloc(len + 32 + 24);

  try {
    // NOTE: First 32 bytes MUST be zero filled.
    bzero((void *)encryptionSource, 32);
    bzero((void *)encryptionDestination, 32);

    // Copy in the message. Leave 32 zero bytes in the front.
    memcpy((void *)(encryptionSource + 32), msg, len);

    // Encrypt message
    int result = crypto_box((unsigned char *)encryptionDestination,
                            (unsigned char *)encryptionSource,
                            len + 32,
                            nonce,
                            (unsigned char *)branchPublicKey,
                            sk);

    if (result != 0) {
      cout << "Encryption error: " << result << endl;
    }

    // Copy in the 24 random nonce bytes.
    memcpy((void *)packet, nonce, 24);

    // Copy in the encrypted message plus the 16 zero bytes at the beginning.
    memcpy((void *)(packet + 24), encryptionDestination, len + 32);

    socket.send((char *)packet, len + 32 + 24, uWS::OpCode::BINARY);

  } catch (...) {
    cout << "Unexpected exception in sendMessage()" << endl;
  }

  free((void *)nonce);
  free((void *)encryptionSource);
  free((void *)encryptionDestination);
  free((void *)packet);
}

void ServerCallback::sendTraceMessage(uint8_t *data, size_t length) {
  concurrent::AtomicBuffer srcBuffer(data, length);
  const std::int64_t result = tracePublication->offer(srcBuffer, 0, length);
  if (result < 0L) {
    switch(result) {
      case NOT_CONNECTED:
        cout << "sendTraceMessage() failed: No subscriber is connected to the trace Publication." << endl;
        break;
      case BACK_PRESSURED:
        cout << "sendTraceMessage() failed: The message was not sent due to back pressure from Subscribers, but can be retried if desired." << endl;
        break;
      case ADMIN_ACTION:
        cout << "sendTraceMessage() failed: The message was not sent due to an administration action, such as log rotation, but can be retried if desired." << endl;
        break;
      case PUBLICATION_CLOSED:
        cout << "sendTraceMessage() failed: The message was not sent due to the trace Publication being closed. This is a permanent error." << endl;
        break;
      default:
        cout << "sendTraceMessage() failed for an unknown reason." << endl;
    }
  }
}

void ServerCallback::setDevicePublicKey(uint64_t deviceId, unsigned char *pk) {
  assert(deviceId != 0 && "deviceId should not be zero here");

  uWS::ServerSocket socket(socketMap[0]); // HACK: We stashed the "current" socket under 0 earlier.

  socket.setData((void *)deviceId);
  socket.setPublicKey(pk);
  
  socketMap[deviceId] = socket.getSocket();
}

void ServerCallback::sendDeviceMessage(uint64_t deviceId, const char *msg, size_t len) {
  if (socketMap.find(deviceId) == socketMap.end()) {
    return; // This device does not have an active socket right now.
  }

  uWS::ServerSocket socket(socketMap[deviceId]);

  // We always encrypt messages.
  // TODO: We can optimize the memory allocations here.

  // Create the nonce.
  unsigned char *nonce = (unsigned char *)malloc(crypto_box_NONCEBYTES);
  randombytes(nonce, crypto_box_NONCEBYTES);

  // Create a buffer to encrypt message from and into.
  const unsigned char *encryptionSource = (unsigned char *)malloc(len + 32);
  const unsigned char *encryptionDestination = (unsigned char *)malloc(len + 32);

  // Create the final packet.
  const unsigned char *packet = (unsigned char *)malloc(len + 32 + 24);

  try {
    // NOTE: First 32 bytes MUST be zero filled.
    bzero((void *)encryptionSource, 32);
    bzero((void *)encryptionDestination, 32);

    // Copy in the message. Leave 32 zero bytes in the front.
    memcpy((void *)(encryptionSource + 32), msg, len);

    // Encrypt message
    int result = crypto_box((unsigned char *)encryptionDestination,
                            (unsigned char *)encryptionSource,
                            len + 32,
                            nonce,
                            (unsigned char *)socket.getPublicKey(),
                            sk);

    if (result != 0) {
      cout << "Encryption error: " << result << endl;
    }

    // Copy in the 24 random nonce bytes.
    memcpy((void *)packet, nonce, 24);

    // Copy in the encrypted message plus the 16 zero bytes at the beginning.
    memcpy((void *)(packet + 24), encryptionDestination, len + 32);
    
    socket.send((char *)packet, len + 32 + 24, uWS::OpCode::BINARY);

  } catch (...) {
    cout << "Unexpected exception in sendMessage()" << endl;
  }

  free((void *)nonce);
  free((void *)encryptionSource);
  free((void *)encryptionDestination);
  free((void *)packet);
}

int main(int argc, char** argv) {
  assert(sizeof(unsigned long long) == sizeof(void *) && "Expect sizeof(unsigned long long) == sizeof(void *)");

  configureCrypto();

  // Configure Aeron.
  // NOTE: We need to do this here (as opposed to a function) because the objects
  // we're creating are on the stack, and we need them to continue to live.
  CommandOptionParser cp;
  cp.addOption(CommandOption (optHelp,           0, 0, "                Displays help information."));

  cp.addOption(CommandOption (optPrefix,         1, 1, "dir             Prefix directory for aeron driver."));
  cp.addOption(CommandOption (optFrags,          1, 1, "limit           Fragment Count Limit."));

  cp.addOption(CommandOption (optServerChannel,  1, 1, "channel         Server Channel."));
  cp.addOption(CommandOption (optServerStreamId, 1, 1, "streamId        Server Stream ID."));

  cp.addOption(CommandOption (optEngineChannel,  1, 1, "channel         Engine Channel."));
  cp.addOption(CommandOption (optEngineStreamId, 1, 1, "streamId        Engine Stream ID."));

  cp.addOption(CommandOption (optTraceChannel,   1, 1, "channel         Trace Channel."));
  cp.addOption(CommandOption (optTraceStreamId,  1, 1, "streamId        Trace Stream ID."));

  settings = parseCmdLine(cp, argc, argv);

  cout << "Publishing Server at " << settings.serverChannel << " on Stream ID " << settings.serverStreamId << endl;
  cout << "Subscribing Engine at " << settings.engineChannel << " on Stream ID " << settings.engineStreamId << endl;
  cout << "Publishing Trace at " << settings.traceChannel << " on Stream ID " << settings.traceStreamId << endl;

  aeron::Context context;
  int64_t serverPublicationId;
  int64_t engineSubscriptionId;
  int64_t tracePublicationId;

  if (settings.dirPrefix != "") {
    context.aeronDir(settings.dirPrefix);
  }

  context.newPublicationHandler([](const string& channel, int32_t streamId, int32_t sessionId, int64_t correlationId) {
    cout << "Got Publication: " << channel << " " << correlationId << ":" << streamId << ":" << sessionId << endl;
  });

  context.newSubscriptionHandler([](const string& channel, int32_t streamId, int64_t correlationId) {
    cout << "Got Subscription: " << channel << " " << correlationId << ":" << streamId << endl;
  });

  context.availableImageHandler([&engineSubscriptionId](Image &image) {
    cout << "Available image correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
    cout << " at position=" << image.position() << " from " << image.sourceIdentity() << endl;

    if (image.subscriptionRegistrationId() != engineSubscriptionId) return;

    shouldPoll = true;
  });

  context.unavailableImageHandler([&engineSubscriptionId](Image &image) {
    cout << "Unavailable image on correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
    cout << " at position=" << image.position() << " from " << image.sourceIdentity() << endl;

    if (image.subscriptionRegistrationId() != engineSubscriptionId) return;

    shouldPoll = false;
  });

  Aeron aeron(context);

  serverPublicationId = aeron.addPublication(settings.serverChannel, settings.serverStreamId);
  engineSubscriptionId = aeron.addSubscription(settings.engineChannel, settings.engineStreamId);
  tracePublicationId = aeron.addPublication(settings.traceChannel, settings.traceStreamId);

  serverPublication = aeron.findPublication(serverPublicationId);
  while (!serverPublication) {
    this_thread::yield();
    serverPublication = aeron.findPublication(serverPublicationId);
  }

  engineSubscription = aeron.findSubscription(engineSubscriptionId);
  while (!engineSubscription) {
    this_thread::yield();
    engineSubscription = aeron.findSubscription(engineSubscriptionId);
  }

  tracePublication = aeron.findPublication(tracePublicationId);
  while (!tracePublication) {
    this_thread::yield();
    tracePublication = aeron.findPublication(tracePublicationId);
  }

  // Set up libuv timer polling for Aeron.
  uv_timer_init(uv_default_loop(), &timerHandle);
  uv_timer_start(&timerHandle, timerCallback, 0, 1);

  cout << "Aeron is up and running." << endl;

  server = new Server("/Users/erich/Desktop/testdb");

  try { // Configure µWebSockets
    uWS::Server client(3000);
    uWS::Server branch(3001);

    // Branch.io WebSocket
    branch.onConnection([](uWS::ServerSocket socket) {
      cout << "BRANCH: [Connection]" << endl;

//      if (branchSocket) {
//        cout << "BRANCH: Closing new connection because we already have a branchSocket." << endl;
//        socket.close();
//        return;
//      }

      branchSocket = socket.getSocket();
    });

    branch.onMessage([](uWS::ServerSocket socket, const char *msg, size_t len, uWS::OpCode opCode) {
      const unsigned char *plainText = nullptr;

      try {
        const unsigned char *nonce = (unsigned char *)msg;
        const unsigned char *cipherText = (unsigned char *)msg + 24;

        plainText = (unsigned char *)malloc(len - 24);
        bzero((void *)plainText, 32);

        int result = crypto_box_open((unsigned char *)plainText,
                                     (unsigned char *)cipherText,
                                     len - 24,
                                     (unsigned char *)nonce,
                                     (unsigned char *)branchPublicKey,
                                     (unsigned char *)sk);

        if (result == -1) {
          cout << "Branch.io socket decryption failed" << endl;

          const char *response = "{}";
          serverCallback.sendBranchMessage(response, 2);

          socket.close(); // We shouldn't be having decryption problems with our own server.

        } else {
          const char *decrytedData = (const char *)plainText + 32;
          size_t decryptedLength = len - 24 - 32;

          server->handleBranchMessage(decrytedData, decryptedLength, &serverCallback);
        }

      } catch (...) {
        cout << "Unexpected exception during handleBranchMessage()" << endl;
      }

      if (plainText != nullptr) {
        free((void *)plainText);
        plainText = nullptr;
      }
    });

    branch.onDisconnection([](uWS::ServerSocket socket, int code, char *message, size_t length) {
      cout << "BRANCH: [Disconnection]" << endl;
      branchSocket = nullptr;
    });

    // Client WebSocket
    client.onConnection([](uWS::ServerSocket socket) {
      cout << "[Connection] clients: " << ++connections << endl;
    });

    client.onMessage([](uWS::ServerSocket socket, const char *msg, size_t len, uWS::OpCode opCode) {
      void *data = socket.getData();
      size_t deviceId = data == nullptr ? 0 : (size_t)data;

      // cout << "Got request from " << deviceId << endl;

      const char *decrytedData = nullptr;
      size_t decryptedLength = 0;
      const unsigned char *plainText = nullptr;

      // We only decrypt if the deviceId is unknown.
      if (deviceId != 0) {
        const unsigned char *nonce = (unsigned char *)msg;
        const unsigned char *cipherText = (unsigned char *)msg + 24;

        plainText = (unsigned char *)malloc(len - 24);
        bzero((void *)plainText, 32);

        int result = crypto_box_open((unsigned char *)plainText,
                                     (unsigned char *)cipherText,
                                     len - 24,
                                     (unsigned char *)nonce,
                                     (unsigned char *)socket.getPublicKey(),
                                     (unsigned char *)sk);

        if (result == -1) {
          free((void *)plainText);

          // We *always* send a response.
          capnp::MallocMessageBuilder builder;
          Response::Builder response = builder.initRoot<Response>();
//              response.setId(requestId); // FIXME: We don't know the requestId!
          auto responseKind = response.getKind();

          cout << "Decryption failed" << endl;

          auto error = responseKind.initErrorResponse();
          error.setMessage("Decryption failed");

          kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
          kj::ArrayPtr<kj::byte> bytes = words.asBytes();

          socket.send((char *)(bytes.begin()), bytes.size(), uWS::OpCode::BINARY);
          return;
        }

        decrytedData = (const char *)plainText + 32;
        decryptedLength = len - 24 - 32;

      } else {
        // HACK: Stash the current socket under 0 so we can update it later in setDevicePublicKey().
        socketMap[0] = socket.getSocket();

        decrytedData = msg;
        decryptedLength = len;
      }

      try {
        server->handleClientMessage(deviceId, decrytedData, decryptedLength, shouldPoll, &serverCallback);

      } catch (...) {
        cout << "Unexpected exception during handleClientMessage()" << endl;
      }
      
      if (plainText != nullptr) {
        free((void *)plainText);
      }
    });

    client.onDisconnection([](uWS::ServerSocket socket, int code, char *message, size_t length) {
      cout << "[Disconnection] clients: " << --connections << endl;

      void *data = socket.getData();
      size_t deviceId = data == nullptr ? 0 : (size_t)data;
      if (deviceId > 0) socketMap.erase(deviceId);

      socket.setData(nullptr);
    });

    // Start µWebSockets run loop.
    client.run();

  } catch (const std::exception& e) {
    cout << e.what() << endl;
  }

  return 0;
}
