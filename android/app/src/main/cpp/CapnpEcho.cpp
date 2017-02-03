//
// Created by Erik van der Tier on 02/02/2017.
//

//
// Created by Erik van der Tier on 13/12/2016.
//
#include <jni.h>
#include <capnproto/capnp/message.h>
#include <follow/server/schema/card_info.capnp.h>
#include <capnproto/capnp/serialize.h>


#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "lmdbJNI::", __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif

jbyteArray Java_com_followme_blue_CapnpEcho_echoCard(
        JNIEnv *env,
        jobject obj, jbyteArray jbuffer) {

    size_t length = (size_t )env->GetArrayLength(jbuffer);
    char buf[length];
    env->GetByteArrayRegion(jbuffer, 0, env->GetArrayLength(jbuffer), (jbyte *) buf);

    kj::ArrayPtr<const capnp::word> view((const capnp::word *) buf, length / 8);
    capnp::FlatArrayMessageReader reader(view);

    CardInfo::Reader inCard = reader.getRoot<CardInfo>();

    capnp::MallocMessageBuilder builder;
    CardInfo::Builder card = builder.initRoot<CardInfo>();
    card.setId(inCard.getId());
    card.setTimestamp(inCard.getTimestamp());

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    jsize size = (jsize) bytes.size();
    jbyteArray retBuffer = env->NewByteArray(size);
    env->SetByteArrayRegion(retBuffer, 0, size, (jbyte *) bytes.begin());
    return retBuffer;
}
}