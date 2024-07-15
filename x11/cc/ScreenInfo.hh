#pragma once

#include <impl/Library.hh>

namespace jwm
{
    struct ScreenInfo {
        long id;
        IRect bounds;
        bool isPrimary;
        jobject asJavaObject(JNIEnv* env) const;
    };
} // namespace jwm
