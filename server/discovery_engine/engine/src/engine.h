//
//  //discovery_engine/engine/engine.h
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//

#ifndef DISCOVERY_ENGINE_ENGINE_H_
#define DISCOVERY_ENGINE_ENGINE_H_

#include "generic.h"
#include "session_db.h"
#include "graph_db.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"
#include "clock/clock.h"

namespace DISCOVERY_ENGINE {
namespace ENGINE {

struct info_record {
  int64_t time;
  int64_t id;
};

// higher time is more recent (LT)
// to make sure values with the same timestamp but different id's are sorted
//    lower to higher id values, this should not occur but better be safe than
//    sorry. Has no performance impact at all as this code will not be reached
//    unless somethings wrong
auto InfoCmp(const MDB_val *val1, const MDB_val *val2) -> int;

template <typename ClockSourcePolicy>
class engine {
 public:
  engine(ErrorPtr &e, LMDBEnv &env, xy::clock<ClockSourcePolicy> &clock);
  virtual ~engine(){};

  void beginTransaction(ErrorPtr &e, TransBody body);
  void commitTransaction(ErrorPtr &e, Transaction &txn);

  void handleDiscoverRequest(ErrorPtr &e, int64_t deviceID,
                             DiscoverRequest::Reader request, Transaction &txn);

  void handleSyncResponse(ErrorPtr &e, int64_t deviceID,
                          SyncResponse::Reader response, Transaction &txn);

  void handleDiscoverAckResponse(ErrorPtr &e, int64_t deviceID,
                                 DiscoverAckResponse::Reader, Transaction &txn);

  void onSendSyncRequest(std::function<void(ErrorPtr &e, int64_t deviceID,
                                            SyncRequest::Reader request)>
                             callback);  // register callback
  void onSendDiscoverResponse(std::function<
      void(ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response)>
                                  callback);  // register callback

 private:
  LMDBEnv &env_;
  SessionDB sessionDB_;
  GraphDB graphDB_;
  DatabasePtr<DBVal, DBVal> discoverdDevicesDB_;
  DatabasePtr<DBVal, DBVal> advertisedCards_;
  DatabasePtr<DBVal, DBVal> recentCards_;

  // DeviceDB deviceDB_;

  std::function<void(ErrorPtr &e, int64_t deviceID,
                     SyncRequest::Reader request)> onSendSyncRequest_ = nullptr;

  std::function<void(ErrorPtr &e, int64_t deviceID,
                     DiscoverResponse::Reader response)>
      onSendDiscoverResponse_ = nullptr;

  void handleDeactivateDevice(ErrorPtr &e, int64_t deviceID,
                              DiscoverInfo::Reader request,
                              SyncInfo::Reader syncInfo, Transaction &txn);
  void handleDiscoverCards(ErrorPtr &e, int64_t deviceID,
                           DiscoverInfo::Reader discInfo,
                           SyncInfo::Reader syncInfo, Transaction &txn);
  void processCachedResponse(ErrorPtr &e, int64_t deviceID, int64_t timeStamp,
                             Transaction &txn);
  void processAckResponseSection(ErrorPtr &e, int64_t deviceID,
                                 DiscoverResponse::Reader response,
                                 TimestampSection::Reader section,
                                 Transaction &txn);
  void sendSyncRequest(ErrorPtr &e, int64_t deviceID,
                       SyncRequest::Reader request);
  void sendDiscoverResponse(ErrorPtr &e, int64_t deviceID,
                            DiscoverResponse::Reader response);

};  // class engine

template <typename ClockSourcePolicy>
engine<ClockSourcePolicy>::engine(ErrorPtr &e, LMDBEnv &env,
                                  xy::clock<ClockSourcePolicy> &clock)
    : env_(env), sessionDB_(e, env), graphDB_(e, env) {
  discoverdDevicesDB_ = env_.openDatabase<DBVal, DBVal>(
      e, "discoveredDevices", MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERKEY);
  discoverdDevicesDB_->setDubKeyCompareFunction(e, InfoCmp);

  advertisedCards_ = env_.openDatabase<DBVal, DBVal>(
      e, "advertisedCards",
      MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERKEY | MDB_INTEGERDUP);

  recentCards_ = env_.openDatabase<DBVal, DBVal>(
      e, "recentCards", MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERKEY);
  recentCards_->setDubKeyCompareFunction(e, InfoCmp);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::beginTransaction(ErrorPtr &e, TransBody body) {
  ASSERT_ERROR_RESET(e);

  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  body(e, txn);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::commitTransaction(ErrorPtr &e,
                                                  Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  env_.commitTransaction(e, txn);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::handleDiscoverRequest(
    ErrorPtr &e, int64_t deviceID, DiscoverRequest::Reader request,
    Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("we got a discover request!")

  capnp::MallocMessageBuilder builder;

  // check for session data for request.deviceID,
  auto SessionVal = sessionDB_.getSession(e, deviceID, request, txn);

  // if we have a 'timestamp' on the request but no session, we ignore the
  // timestamp! so no 'ack' is implied
  // perform initial sync for new sesssion
  ERROR_RESET(e);
  if (SessionVal == nullptr) {
    // build SyncRequest
    auto SRequest = builder.initRoot<SyncRequest>();
    SRequest.setDiscoverInfo(request.getDiscoverInfo());
    SRequest.setLatestRecentCard(0);
    SRequest.setWantsCardsAdvertisedByDevice(true);

    LOG("we're sending new session sync request!")
    sendSyncRequest(e, deviceID, SRequest);
    ON_ERROR_RETURN(e);
    return;
  }

  // we already had a session object.
  // we always ignore the timestamp on the session object:
  //   this timestamp was only there as an 'ack' in the original
  //   DiscoveryRequest
  //   that we used as our session object.

  // check on sync! (we don't act on this just yet)

  auto Session = make_reader(*SessionVal).template getRoot<DiscoverRequest>();
  // we have session now check syndinfo
  auto RequestSyncInfo = request.getSyncInfo();
  auto SessionSyncInfo = Session.getSyncInfo();

  // compare request sync with session sync
  bool needCardsSync =
      RequestSyncInfo.getCardsVersion() > SessionSyncInfo.getCardsVersion();
  bool needDeviceUpdate = RequestSyncInfo.getLastDeviceUpdateTimestamp() >
                          SessionSyncInfo.getLastDeviceUpdateTimestamp();
  bool needLatestRecentCard = RequestSyncInfo.getLastCardTimestamp() >
                              SessionSyncInfo.getLastCardTimestamp();

  // check if I have a uncommited cached response object
  auto CachedResponseVal = sessionDB_.getResponse(e, deviceID, txn);
  ON_ERROR_RETURN(e);

  if (CachedResponseVal != nullptr) {
    auto CachedResponse =
        make_reader(*CachedResponseVal).template getRoot<DiscoverResponse>();

    // we have a response, let's process it
    // if the cached timestamp == request timestamp, commit Response

    if (CachedResponse.getTimestamp() == request.getTimestamp()) {
      // we commit the response
      processCachedResponse(e, deviceID, request.getTimestamp(), txn);
    } else if (CachedResponse.getTimestamp() > request.getTimestamp() &&
               !(needCardsSync || needDeviceUpdate || needLatestRecentCard)) {
      // if the CachedResponse is newer, check sync info, resend
      // if sync info is the same in this case, the other side is out of sync on
      // the
      // response.
      sendDiscoverResponse(e, deviceID, CachedResponse);
    } else {
      ERROR_RETURN_CODE(e, INCORRECT_SYNC_ON_RESPONSE);
    }

  }  // endif have cached response

  if (needCardsSync || needDeviceUpdate || needLatestRecentCard) {
    // we're out of sync, send sync request

    auto SRequest = builder.initRoot<SyncRequest>();
    SRequest.setDiscoverInfo(request.getDiscoverInfo());
    SRequest.setLatestRecentCard(SessionSyncInfo.getLastCardTimestamp());
    SRequest.setWantsCardsAdvertisedByDevice(needCardsSync);
    SRequest.setLatestDiscoveredDevice(
        SessionSyncInfo.getLastDeviceUpdateTimestamp());

    LOG("we're sending regular discover request sync request!")

    sendSyncRequest(e, deviceID, SRequest);
    ON_ERROR_RETURN(e);
    return;
  }
  // we should now be in sync!

  // find out what we need to do in this request
  auto RDiscoverInfo = request.getDiscoverInfo();

  switch (RDiscoverInfo.getType()) {
    case DiscoverInfo::Type::DISCOVER:
      handleDiscoverCards(e, deviceID, RDiscoverInfo, Session.getSyncInfo(),
                          txn);
      break;
    case DiscoverInfo::Type::DEACTIVATE_DEVICE:
      handleDeactivateDevice(e, deviceID, RDiscoverInfo, Session.getSyncInfo(),
                             txn);
      break;
    default:
      ERROR_SET_CODE(e, ERROR_CODE::UNKNOWN_DISC_TYPE);
      // set error code
      break;
  }
  ON_ERROR_RETURN(e);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::handleSyncResponse(
    ErrorPtr &e, int64_t deviceID, SyncResponse::Reader response,
    Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("we got a sync response!")
  capnp::MallocMessageBuilder builder;
  auto Key = DBVal(deviceID);

  // get discovered devices from the response
  auto Devices = response.getDiscoveredDevices();
  for (auto device : Devices) {
    info_record DevRec = {device.getTimestamp(), device.getId()};
    auto Record = DBVal(DevRec);
    discoverdDevicesDB_->add(e, Key, Record, txn, true);
    ON_ERROR_RETURN(e);
  }

  // add/update devices to graph
  // ASSUMPTION: for updating the deviceID timestamp in the graph we take the
  //  current time: now()
  graphDB_.update(e, deviceID, now(), Devices, txn);
  ON_ERROR_RETURN(e);

  auto DeviceCards = response.getCardsAdvertisedByDevice();
  for (auto card : DeviceCards) {
    advertisedCards_->add(e, Key, card, txn, true);
    ON_ERROR_RETURN(e);
  }

  auto RecentCards = response.getRecentCards();
  for (auto card : RecentCards) {
    LOG("card: " << card.getId() << ", " << card.getTimestamp())
    info_record CardRec = {card.getTimestamp(), card.getId()};
    auto Record = DBVal(CardRec);
    recentCards_->add(e, Key, Record, txn, true);
    ON_ERROR_RETURN(e);
  }

  // store syncinfo in session
  auto dummySession = builder.initRoot<DiscoverRequest>();
  auto SessionVal = sessionDB_.getSession(e, deviceID, dummySession, txn);
  ON_ERROR_RETURN(e);
  auto Session = make_reader(*SessionVal).template getRoot<DiscoverRequest>();
  capnp::MallocMessageBuilder SessionBuilder;
  SessionBuilder.setRoot(Session);

  // we update the session, overwriting only the 'SyncInfo' to our new status
  auto WriteSession = SessionBuilder.getRoot<DiscoverRequest>();
  WriteSession.setDiscoverInfo(Session.getDiscoverInfo());
  WriteSession.setTimestamp(Session.getTimestamp());
  WriteSession.setSyncInfo(response.getSyncInfo());
  sessionDB_.updateSession(e, deviceID, Session, txn);
  ON_ERROR_RETURN(e);

  // check discover info to figure out next steps
  // if we already have a DiscoverRequestion in the current session:
  // we perform that now we're in sync
  auto RDiscoverInfo = response.getDiscoverInfo();

  switch (RDiscoverInfo.getType()) {
    case DiscoverInfo::Type::DISCOVER:
      handleDiscoverCards(e, deviceID, RDiscoverInfo,
                          WriteSession.getSyncInfo(), txn);
      break;
    case DiscoverInfo::Type::DEACTIVATE_DEVICE:
      handleDeactivateDevice(e, deviceID, RDiscoverInfo,
                             WriteSession.getSyncInfo(), txn);
      break;
    default:
      ERROR_SET_CODE(e, ERROR_CODE::UNKNOWN_DISC_TYPE);
      break;
  }
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::handleDiscoverCards(
    ErrorPtr &e, int64_t deviceID, DiscoverInfo::Reader discInfo,
    SyncInfo::Reader syncInfo, Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("we're discovery cards and doing section work!")

  capnp::MallocMessageBuilder ResponseBuilder;

  auto CurrentTime = now();

  auto Response = ResponseBuilder.initRoot<DiscoverResponse>();
  Response.setTimestamp(CurrentTime);
  Response.setDiscoverInfo(discInfo);
  Response.setSyncInfo(syncInfo);
  auto Section = Response.initSection();

  // discover cards
  LOG("Discovering Cards")

  auto DiscoverdDevices = graphDB_.discoverDevices(e, deviceID, txn);
  ON_ERROR_RETURN(e);

  auto RecentCardCursor = recentCards_->getCursor(e, txn);
  ON_ERROR_RETURN(e);
  auto AdvertizedCardCursor = advertisedCards_->getCursor(e, txn);
  ON_ERROR_RETURN(e);

  LOG("Creating new recent cards list")

  std::vector<int64_t> TempRecentCardsList;
  for (auto discDevice : DiscoverdDevices) {
    // we compare the discovered cards to the advertized cards of the device
    auto Card = AdvertizedCardCursor->getFirstDup(e, discDevice);
    while (hasVal(Card)) {
      auto Record = DBVal(Card->value);

      auto RecentCard = RecentCardCursor->getDup(e, discDevice, Record);

      // we only add cards that aren't already in our list
      if (hasVal(RecentCard)) break;

      ON_ERROR_RETURN(e);

      // we don't have this card yet, and not error -> add to temp recent list
      TempRecentCardsList.push_back(dbValAs<int64_t>(Card->value));

      Card = AdvertizedCardCursor->getNextDup(e, discDevice);
    }
    ON_ERROR_RETURN(e);

  }  // for device in DiscoverdDevices

  // insert cards into Response, now we know the size of the new list we can add
  auto DiscoveredRecentCards = Response.initCards(TempRecentCardsList.size());
  int CardsCount = 0;
  std::for_each(
      TempRecentCardsList.cbegin(), TempRecentCardsList.cend(),
      [&DiscoveredRecentCards, &CardsCount, &CurrentTime](auto &card) {
        DiscoveredRecentCards[CardsCount].setId(card);

        // set the timestamp to the CurrentTime + one second per next card
        DiscoveredRecentCards[CardsCount].setTimestamp(CurrentTime +
                                                       CardsCount);
        CardsCount++;
      });

  if (!discInfo.getShouldSuppressSectionCreation()) {
    // figure out which cards to move out of recents
    // we don't actually perform the move
    const auto HourDuration = 3600000;
    const auto QuarterHourDuration = 900000;

    ON_ERROR_RETURN(e);
    auto CardBuffer = RecentCardCursor->getFirstDup(e, deviceID);
    ON_ERROR_RETURN(e);

    auto SectionKind = Section.initKind();

    if (hasVal(CardBuffer)) {
      auto Card = *static_cast<info_record *>(CardBuffer->value.buffer());
      auto CardTime = Card.time;
      LOG("First card to maybe move: " << Card.id << ", " << Card.time)
      auto found = false;

      // if we actually discoved any new devices and the first card is over an
      // hour
      // old we move all cards to section
      if ((CurrentTime - CardTime) > HourDuration &&
          DiscoverdDevices.size() > 0) {
        found = true;
      } else {
        auto LastTime = CardTime;
        do {
          CardBuffer = RecentCardCursor->getNextDup(e, deviceID);
          if (hasVal(CardBuffer)) {
            Card = *static_cast<info_record *>(CardBuffer->value.buffer());

            auto CardTime = Card.time;
            LOG("next card to maybe move: " << Card.id << ", " << Card.time)

            if ((LastTime - CardTime) > QuarterHourDuration) {
              found = true;
            }
          } else {
            break;
          }
        } while (!found);
        ON_ERROR_RETURN(e);
      }
      if (found) {
        LOG("LastCard: " << Card.id << ", time: " << Card.time)

        auto TimeStampSection = SectionKind.initTimestampSection();
        TimeStampSection.setTimestamp(Card.time);
        TimeStampSection.setLastCard(Card.id);
      }
    }  // end if hasVal(CardBuffer)

  }  // end if (discInfo.getShouldSuppressSectionCreation())
  // cache response

  sessionDB_.cacheResponse(e, deviceID, Response, txn);
  ON_ERROR_RETURN(e);
  // send response

  LOG("we're sending discover response request!")
  sendDiscoverResponse(e, deviceID, Response);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::handleDeactivateDevice(
    ErrorPtr &e, int64_t deviceID, DiscoverInfo::Reader discInfo,
    SyncInfo::Reader syncInfo, Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("BEGIN: handleDeactivateDevice")
  auto CurrentTime = now();

  // build DiscoveryResponse object and send
  capnp::MallocMessageBuilder ResponseBuilder;
  auto Response = ResponseBuilder.initRoot<DiscoverResponse>();
  Response.setTimestamp(CurrentTime);
  Response.setDiscoverInfo(discInfo);
  Response.setSyncInfo(syncInfo);
  auto SectionKind = Response.initSection().getKind();

  auto RecentCardCursor = recentCards_->getCursor(e, txn);
  ON_ERROR_RETURN(e);
  auto CardBuffer = RecentCardCursor->getFirstDup(e, deviceID);
  ON_ERROR_RETURN(e);

  // assuming that if there is no card, the Section should just be default
  if (hasVal(CardBuffer)) {
    auto TimeStampSection = SectionKind.initTimestampSection();
    auto Card = *static_cast<info_record *>(CardBuffer->value.buffer());
    auto CardTime = Card.time;

    TimeStampSection.setTimestamp(CardTime);
    TimeStampSection.setLastCard(syncInfo.getLastCardTimestamp());
  }

  sendDiscoverResponse(e, deviceID, Response);
  LOG("END: handleDeactivateDevice")
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::handleDiscoverAckResponse(
    ErrorPtr &e, int64_t deviceID, DiscoverAckResponse::Reader response,
    Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  // pass it along to the process method

  processCachedResponse(e, deviceID, response.getTimestamp(), txn);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::processCachedResponse(ErrorPtr &e,
                                                      int64_t deviceID,
                                                      int64_t timeStamp,
                                                      Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("BEGIN: processCachedResponse\n")

  // read the cached discover response,

  auto ResponseVal = sessionDB_.getResponse(e, deviceID, txn);
  if (ResponseVal == nullptr) {
    ERROR_RETURN_CODE(e, ERROR_CODE::MISSING_RESPONSE_CACHE);
  } else
    ON_ERROR_RETURN(e);

  // we have a response, let's process it
  auto Response =
      make_reader(*ResponseVal).template getRoot<DiscoverResponse>();

  // check for validity of time stamp on the Ack.
  if (timeStamp != Response.getTimestamp())
    ERROR_RETURN_CODE(e, MISMATCH_RESPONSE_ACK_TIMESTAMP);

  //  get the section info and purge the list.
  auto SectionKind = Response.getSection().getKind();
  if (SectionKind.which() == SectionInfo::Kind::TIMESTAMP_SECTION) {
    auto Section = SectionKind.getTimestampSection();
    processAckResponseSection(e, deviceID, Response, Section, txn);
    ON_ERROR_RETURN(e);
  }
  // clean the graph as well if needed, for now only on deactivate!
  auto DiscInfo = Response.getDiscoverInfo();
  if (DiscInfo.getType() == DiscoverInfo::Type::DEACTIVATE_DEVICE) {
    graphDB_.clean(e, deviceID, now(), txn);
    ON_ERROR_RETURN(e);
  }

  //  get the card list and apply to recents

  for (auto card : Response.getCards()) {
    info_record CardRec = {card.getTimestamp(), card.getId()};
    auto Record = DBVal(CardRec);
    recentCards_->add(e, deviceID, Record, txn, true);
    ON_ERROR_RETURN(e);
  }

  // delete cached response

  sessionDB_.deleteResponse(e, deviceID, Response, txn);
  ON_ERROR_RETURN(e);
  // reset session, only sync info is maintained
  capnp::MallocMessageBuilder Builder;
  auto dummySession = Builder.initRoot<DiscoverRequest>();
  auto SessionVal = sessionDB_.getSession(e, deviceID, dummySession, txn);
  ON_ERROR_RETURN(e);
  auto Session = make_reader(*SessionVal).template getRoot<DiscoverRequest>();
  capnp::MallocMessageBuilder SessionBuilder;
  SessionBuilder.setRoot(Session);
  auto WriteSession = SessionBuilder.template getRoot<DiscoverRequest>();
  WriteSession.setSyncInfo(Session.getSyncInfo());
  sessionDB_.updateSession(e, deviceID, Session, txn);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::processAckResponseSection(
    ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response,
    TimestampSection::Reader section, Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("BEGIN processAckResponseSection\n")

  auto RecentCardsCursor = recentCards_->getCursor(e, txn);
  ON_ERROR_RETURN(e);
  auto ID = section.getLastCard();
  info_record CardRec = {section.getTimestamp(), ID};

  LOG("LastCard: " << CardRec.id << ", time: " << CardRec.time);

  auto Record = DBVal(CardRec);
  auto Card = RecentCardsCursor->getDup(e, deviceID, Record);
  std::vector<info_record> RemoveCards;
  while (Card.is_initialized()) {
    LOG("removing card: " << (*static_cast<info_record *>(Card->value.buffer()))
                                 .time)

    RemoveCards.push_back(*static_cast<info_record *>(Card->value.buffer()));
    Card = RecentCardsCursor->getNextDup(e, deviceID);
    ON_ERROR_RETURN(e);
  }
  ON_ERROR_RETURN(e);
  for (auto recentCard : RemoveCards) {
    auto Record = DBVal(recentCard);

    LOG("RemoveCard: " << recentCard.id << ", time: " << recentCard.time)

    recentCards_->del(e, deviceID, Record, txn);
    ON_ERROR_RETURN(e);
  }

  auto DiscoveredDevicesCursor = discoverdDevicesDB_->getCursor(e, txn);
  ON_ERROR_RETURN(e);

  info_record DeviceRec = {section.getTimestamp(), 0};

  LOG("DiscoveredDevice time: " << DeviceRec.time)

  auto DeviceRecord = DBVal(DeviceRec);

  auto DiscDevice =
      DiscoveredDevicesCursor->getDupKeyGreaterEqual(e, deviceID, DeviceRecord);
  std::vector<info_record> RemoveDevices;
  while (DiscDevice.is_initialized()) {
    LOG("removing card: " << (*static_cast<info_record *>(
                                  DiscDevice->value.buffer())).time)

    RemoveDevices.push_back(
        *static_cast<info_record *>(DiscDevice->value.buffer()));
    DiscDevice = DiscoveredDevicesCursor->getNextDup(e, deviceID);
    ON_ERROR_RETURN(e);
  }
  for (auto device : RemoveDevices) {
    auto Record = DBVal(device);

    LOG("RemoveCard: " << device.id << ", time: " << device.time)

    discoverdDevicesDB_->del(e, deviceID, Record, txn);
    ON_ERROR_RETURN(e);
  }

  graphDB_.clean(e, deviceID, section.getTimestamp(), txn);
  ON_ERROR_RETURN(e);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::sendSyncRequest(ErrorPtr &e, int64_t deviceID,
                                                SyncRequest::Reader request) {
  ASSERT_ERROR_RESET(e);
  // create SyncRequest with request
  //   do we need DeviceCards? add to request
  //   do we need Sync? add to request
  if (onSendSyncRequest_ == nullptr)
    ERROR_RETURN_CODE(e, ERROR_CODE::MISSING_ON_SEND_SYNC_REQUEST_HANLDER);

  onSendSyncRequest_(e, deviceID, request);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::sendDiscoverResponse(
    ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response) {
  ASSERT_ERROR_RESET(e);
  if (onSendDiscoverResponse_ == nullptr)
    ERROR_RETURN_CODE(e, ERROR_CODE::MISSING_ON_SEND_DISCOVER_RESPONSE_HANLDER);

  onSendDiscoverResponse_(e, deviceID, response);
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::onSendSyncRequest(std::function<void(
    ErrorPtr &e, int64_t deviceID, SyncRequest::Reader response)> callback) {
  onSendSyncRequest_ = callback;
}

template <typename ClockSourcePolicy>
void engine<ClockSourcePolicy>::onSendDiscoverResponse(
    std::function<void(ErrorPtr &e, int64_t deviceID,
                       DiscoverResponse::Reader response)> callback) {
  onSendDiscoverResponse_ = callback;
}

}  // namespace ENGINE

}  // namespace DISCOVERY_ENGINE

#endif  // DISCOVERY_ENGINE_ENGINE_H_
