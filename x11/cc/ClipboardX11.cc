#include <jni.h>
#include <StringUTF16.hh>
#include <impl/Library.hh>
#include <impl/JNILocal.hh>
#include "AppX11.hh"
#include <cstring>

namespace jwm {
    class ClipboardX11 {
    public:
        static ClipboardX11& inst() {
            static ClipboardX11 s;
            return s;
        }
        
        jobjectArray getFormats(JNIEnv* env) const {
            auto formats = jwm::app.getWindowManager().getClipboardFormats();
            if (formats.empty()) {
                return nullptr;
            }

            std::vector<jobject> formatObjs;

            for (auto& format : formats) {
                auto js = StringUTF16(format.c_str()).toJString(env);
                formatObjs.push_back(classes::Clipboard::registerFormat(env, js.get()));
            }
            jobjectArray jniFormats = env->NewObjectArray(static_cast<jsize>(formats.size()), classes::ClipboardFormat::kCls, nullptr);

            // fill java array
            for (jsize i = 0; i < static_cast<jsize>(formatObjs.size()); ++i) {
                env->SetObjectArrayElement(jniFormats, i, formatObjs[i]);
            }

            return jniFormats;
        }

        jobject get(JNIEnv* env, jobjectArray formats) {
            jsize formatsSize = env->GetArrayLength(formats);
            for (jsize i = 0; i < formatsSize; ++i) {
                jobject format = env->GetObjectArrayElement(formats, i);
                if (format) {
                    jwm::StringUTF16 formatId = jwm::StringUTF16::makeFromJString(env, classes::ClipboardFormat::getFormatId(env, format));


                    ByteBuf contents;
                    // HACK: prefer UTF8_STRING over text/plain and convert it to utf16
                    if (formatId == "text/plain") {
                        contents = app.getWindowManager().getClipboardContents("UTF8_STRING");
                    }
                    // TODO add another formats
                    if (contents.empty()) {
                        return nullptr;
                    }
                    JNILocal<jbyteArray> data(env, env->NewByteArray(static_cast<jsize>(contents.size())));
                    jbyte* bytes = env->GetByteArrayElements(data.get(), nullptr);
                    std::memcpy(bytes, contents.data(), contents.size());

                    env->ReleaseByteArrayElements(data.get(), bytes, 0);


                    JNILocal<jobject> entry(env, classes::ClipboardEntry::make(env, format, data.get()));
                    return env->NewGlobalRef(entry.get());
                }
            }
            classes::Throwable::exceptionThrown(env);
            return nullptr;
        }


        void set(JNIEnv* env, jobjectArray entries) {
            jsize size = env->GetArrayLength(entries);
            std::map<std::string, ByteBuf> contents;
            for (jsize i = 0; i < size; ++i) {
                jobject entry = env->GetObjectArrayElement(entries, i);

                if (entry) {
                    jobject format = classes::ClipboardEntry::getFormat(env, entry);
                    jbyteArray data = classes::ClipboardEntry::getData(env, entry);
                    jsize dataSize = env->GetArrayLength(data);
                    
                    StringUTF16 formatId = StringUTF16::makeFromJString(env, classes::ClipboardFormat::getFormatId(env, format));

                    ByteBuf resultBuffer;
                    {
                        jbyte* dataBytes = env->GetByteArrayElements(data, nullptr);
                        resultBuffer.insert(resultBuffer.end(), dataBytes, dataBytes + dataSize);
                        env->ReleaseByteArrayElements(data, dataBytes, JNI_ABORT);
                    }
               
                    contents[formatId.toAscii()] = std::move(resultBuffer);       
                }
            }
            jwm::app.getWindowManager().setClipboardContents(std::move(contents));
        }
    };
}


// JNI

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_Clipboard__1nSet
        (JNIEnv* env, jclass jclass, jobjectArray entries) {
    return jwm::ClipboardX11::inst().set(env, entries);
}

extern "C" JNIEXPORT jobject JNICALL Java_io_github_humbleui_jwm_Clipboard__1nGet
        (JNIEnv* env, jclass jclass, jobjectArray formats) {
    return jwm::ClipboardX11::inst().get(env, formats);
}

extern "C" JNIEXPORT jobjectArray JNICALL Java_io_github_humbleui_jwm_Clipboard__1nGetFormats
        (JNIEnv* env, jclass jclass) {
    return jwm::ClipboardX11::inst().getFormats(env);
}

extern "C" JNIEXPORT void JNICALL Java_io_github_humbleui_jwm_Clipboard__1nClear
        (JNIEnv* env, jclass jclass) {

}

extern "C" JNIEXPORT jboolean JNICALL Java_io_github_humbleui_jwm_Clipboard__1nRegisterFormat
        (JNIEnv* env, jclass jclass, jstring formatId) {
   return true;
}