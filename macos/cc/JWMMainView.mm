#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"

#include <algorithm>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <Foundation/Foundation.h>
#include <jni.h>
#include "impl/JNILocal.hh"
#include "impl/Library.hh"
#include "KeyModifier.hh"
#include "Log.hh"
#include "JWMMainView.hh"
#include "WindowMac.hh"
#include "Util.hh"

#include <stdio.h>

namespace jwm {
Key kKeyTable[128];
KeyLocation kKeyLocations[128];
TouchType kTouchTypes[2];
BOOL kPressAndHoldEnabled;

void initKeyTable() {
    std::fill(kKeyTable, kKeyTable + 128, Key::UNDEFINED);
    std::fill(kKeyLocations, kKeyLocations + 128, KeyLocation::DEFAULT);

    kKeyTable[kVK_CapsLock] = Key::CAPS_LOCK;
    kKeyTable[kVK_Shift] = Key::SHIFT;
    kKeyTable[kVK_RightShift] = Key::SHIFT;
    kKeyTable[kVK_Control] = Key::CONTROL;
    kKeyTable[kVK_RightControl] = Key::CONTROL;
    // Key::Alt
    // Key::WIN_LOGO
    // Key::LINUX_META
    // Key::LINUX_SUPER
    kKeyTable[kVK_Command] = Key::MAC_COMMAND;
    kKeyTable[kVK_RightCommand] = Key::MAC_COMMAND;
    kKeyTable[kVK_Option] = Key::MAC_OPTION;
    kKeyTable[kVK_RightOption] = Key::MAC_OPTION;
    kKeyTable[kVK_Function] = Key::MAC_FN;

    kKeyTable[kVK_Return] = Key::ENTER;
    kKeyTable[kVK_ANSI_KeypadEnter] = Key::ENTER;
    kKeyTable[kVK_Delete] = Key::BACKSPACE;
    kKeyTable[kVK_Tab] = Key::TAB;
    // Key::CANCEL
    kKeyTable[kVK_ANSI_KeypadClear] = Key::CLEAR;
    // Key::PAUSE
    kKeyTable[kVK_Escape] = Key::ESCAPE;
    kKeyTable[kVK_Space] = Key::SPACE;
    kKeyTable[kVK_PageUp] = Key::PAGE_UP;
    kKeyTable[kVK_PageDown] = Key::PAGE_DOWN;
    kKeyTable[kVK_End] = Key::END;
    kKeyTable[kVK_Home] = Key::HOME;
    kKeyTable[kVK_LeftArrow] = Key::LEFT;
    kKeyTable[kVK_UpArrow] = Key::UP;
    kKeyTable[kVK_RightArrow] = Key::RIGHT;
    kKeyTable[kVK_DownArrow] = Key::DOWN;
    kKeyTable[kVK_ANSI_Comma] = Key::COMMA;
    kKeyTable[kVK_ANSI_Minus] = Key::MINUS;
    kKeyTable[kVK_ANSI_Period] = Key::PERIOD;
    kKeyTable[kVK_ANSI_Slash] = Key::SLASH;
    kKeyTable[kVK_ANSI_0] = Key::DIGIT0;
    kKeyTable[kVK_ANSI_1] = Key::DIGIT1;
    kKeyTable[kVK_ANSI_2] = Key::DIGIT2;
    kKeyTable[kVK_ANSI_3] = Key::DIGIT3;
    kKeyTable[kVK_ANSI_4] = Key::DIGIT4;
    kKeyTable[kVK_ANSI_5] = Key::DIGIT5;
    kKeyTable[kVK_ANSI_6] = Key::DIGIT6;
    kKeyTable[kVK_ANSI_7] = Key::DIGIT7;
    kKeyTable[kVK_ANSI_8] = Key::DIGIT8;
    kKeyTable[kVK_ANSI_9] = Key::DIGIT9;
    kKeyTable[kVK_ANSI_Semicolon] = Key::SEMICOLON;
    kKeyTable[kVK_ANSI_Equal] = Key::EQUALS;
    kKeyTable[kVK_ANSI_KeypadEquals] = Key::EQUALS;
    kKeyTable[kVK_ANSI_A] = Key::A;
    kKeyTable[kVK_ANSI_B] = Key::B;
    kKeyTable[kVK_ANSI_C] = Key::C;
    kKeyTable[kVK_ANSI_D] = Key::D;
    kKeyTable[kVK_ANSI_E] = Key::E;
    kKeyTable[kVK_ANSI_F] = Key::F;
    kKeyTable[kVK_ANSI_G] = Key::G;
    kKeyTable[kVK_ANSI_H] = Key::H;
    kKeyTable[kVK_ANSI_I] = Key::I;
    kKeyTable[kVK_ANSI_J] = Key::J;
    kKeyTable[kVK_ANSI_K] = Key::K;
    kKeyTable[kVK_ANSI_L] = Key::L;
    kKeyTable[kVK_ANSI_M] = Key::M;
    kKeyTable[kVK_ANSI_N] = Key::N;
    kKeyTable[kVK_ANSI_O] = Key::O;
    kKeyTable[kVK_ANSI_P] = Key::P;
    kKeyTable[kVK_ANSI_Q] = Key::Q;
    kKeyTable[kVK_ANSI_R] = Key::R;
    kKeyTable[kVK_ANSI_S] = Key::S;
    kKeyTable[kVK_ANSI_T] = Key::T;
    kKeyTable[kVK_ANSI_U] = Key::U;
    kKeyTable[kVK_ANSI_V] = Key::V;
    kKeyTable[kVK_ANSI_W] = Key::W;
    kKeyTable[kVK_ANSI_X] = Key::X;
    kKeyTable[kVK_ANSI_Y] = Key::Y;
    kKeyTable[kVK_ANSI_Z] = Key::Z;
    kKeyTable[kVK_ANSI_LeftBracket] = Key::OPEN_BRACKET;
    kKeyTable[kVK_ANSI_Backslash] = Key::BACK_SLASH;
    kKeyTable[kVK_ANSI_RightBracket] = Key::CLOSE_BRACKET;
    kKeyTable[kVK_ANSI_Keypad0] = Key::DIGIT0;
    kKeyTable[kVK_ANSI_Keypad1] = Key::DIGIT1;
    kKeyTable[kVK_ANSI_Keypad2] = Key::DIGIT2;
    kKeyTable[kVK_ANSI_Keypad3] = Key::DIGIT3;
    kKeyTable[kVK_ANSI_Keypad4] = Key::DIGIT4;
    kKeyTable[kVK_ANSI_Keypad5] = Key::DIGIT5;
    kKeyTable[kVK_ANSI_Keypad6] = Key::DIGIT6;
    kKeyTable[kVK_ANSI_Keypad7] = Key::DIGIT7;
    kKeyTable[kVK_ANSI_Keypad8] = Key::DIGIT8;
    kKeyTable[kVK_ANSI_Keypad9] = Key::DIGIT9;
    kKeyTable[kVK_ANSI_KeypadMultiply] = Key::MULTIPLY;
    kKeyTable[kVK_ANSI_KeypadPlus] = Key::ADD;
    // Key::SEPARATOR
    kKeyTable[kVK_ANSI_KeypadMinus] = Key::MINUS;
    kKeyTable[kVK_ANSI_KeypadDecimal] = Key::PERIOD;
    kKeyTable[kVK_ANSI_KeypadDivide] = Key::SLASH;
    kKeyTable[kVK_ForwardDelete] = Key::DEL;
    // Key::NUM_LOCK
    // Key::SCROLL_LOCK
    kKeyTable[kVK_F1] = Key::F1;
    kKeyTable[kVK_F2] = Key::F2;
    kKeyTable[kVK_F3] = Key::F3;
    kKeyTable[kVK_F4] = Key::F4;
    kKeyTable[kVK_F5] = Key::F5;
    kKeyTable[kVK_F6] = Key::F6;
    kKeyTable[kVK_F7] = Key::F7;
    kKeyTable[kVK_F8] = Key::F8;
    kKeyTable[kVK_F9] = Key::F9;
    kKeyTable[kVK_F10] = Key::F10;
    kKeyTable[kVK_F11] = Key::F11;
    kKeyTable[kVK_F12] = Key::F12;
    kKeyTable[kVK_F13] = Key::F13;
    kKeyTable[kVK_F14] = Key::F14;
    kKeyTable[kVK_F15] = Key::F15;
    kKeyTable[kVK_F16] = Key::F16;
    kKeyTable[kVK_F17] = Key::F17;
    kKeyTable[kVK_F18] = Key::F18;
    kKeyTable[kVK_F19] = Key::F19;
    kKeyTable[kVK_F20] = Key::F20;
    // Key::F21
    // Key::F22
    // Key::F23
    // Key::F24
    // Key::PRINTSCREEN
    // Key::INSERT
    kKeyTable[kVK_Help] = Key::HELP;
    kKeyTable[kVK_ANSI_Grave] = Key::BACK_QUOTE;
    kKeyTable[kVK_ANSI_Quote] = Key::QUOTE;
    // Key::MENU

    /* ISO keyboards only*/
    // kKeyTable[kVK_ISO_Section] = Key::

    /* JIS keyboards only*/
    // kKeyTable[kVK_JIS_Yen] = Key::
    // kKeyTable[kVK_JIS_Underscore] = Key::
    // kKeyTable[kVK_JIS_KeypadComma] = Key::
    // kKeyTable[kVK_JIS_Eisu] = Key::
    kKeyTable[kVK_JIS_Kana] = Key::KANA;

    kKeyTable[kVK_VolumeUp] = Key::VOLUME_UP;
    kKeyTable[kVK_VolumeDown] = Key::VOLUME_DOWN;
    kKeyTable[kVK_Mute] = Key::MUTE;

    // Locations
    kKeyLocations[kVK_RightShift] = KeyLocation::RIGHT;
    kKeyLocations[kVK_RightControl] = KeyLocation::RIGHT;
    kKeyLocations[kVK_RightCommand] = KeyLocation::RIGHT;
    kKeyLocations[kVK_RightOption] = KeyLocation::RIGHT;
    kKeyLocations[kVK_ANSI_KeypadEnter] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadClear] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadEquals] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad0] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad1] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad2] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad3] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad4] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad5] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad6] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad7] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad8] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_Keypad9] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadMultiply] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadPlus] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadMinus] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadDecimal] = KeyLocation::KEYPAD;
    kKeyLocations[kVK_ANSI_KeypadDivide] = KeyLocation::KEYPAD;

    kTouchTypes[NSTouchTypeDirect] = TouchType::DIRECT;
    kTouchTypes[NSTouchTypeIndirect] = TouchType::INDIRECT;

    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    kPressAndHoldEnabled = nullptr == [userDefaults objectForKey:@"ApplePressAndHoldEnabled"] ? YES : [userDefaults boolForKey:@"ApplePressAndHoldEnabled"];
}

jint modifierMask(NSEventModifierFlags flags) {
    jint mask = 0;
    if ((flags & NSEventModifierFlagShift) != 0)
        mask |= static_cast<int>(KeyModifier::SHIFT);
    if ((flags & NSEventModifierFlagControl) != 0)
        mask |= static_cast<int>(KeyModifier::CONTROL);
    if ((flags & NSEventModifierFlagOption) != 0)
        mask |= static_cast<int>(KeyModifier::MAC_OPTION);
    if ((flags & NSEventModifierFlagCommand) != 0)
        mask |= static_cast<int>(KeyModifier::MAC_COMMAND);
    if ((flags & NSEventModifierFlagFunction) != 0)
        mask |= static_cast<int>(KeyModifier::MAC_FN);
    if ((flags & NSEventModifierFlagCapsLock) != 0)
        mask |= static_cast<int>(KeyModifier::CAPS_LOCK);
    return mask;
}

void onMouseMoved(jwm::WindowMac* window, NSEvent* event, CGPoint* lastPos) {
    NSView* view = window->fNSWindow.contentView;
    CGFloat scale = window->getScale();

    const NSPoint pos = [event locationInWindow];
    const NSRect rect = [view frame];
    lastPos->x = pos.x * scale;
    lastPos->y = (rect.size.height - pos.y) * scale;

    jwm::JNILocal<jobject> eventObj(window->fEnv, jwm::classes::EventMouseMove::make(
        window->fEnv,
        (jint) lastPos->x,
        (jint) lastPos->y,
        (jint)[event deltaX],
        (jint)[event deltaY],
        [NSEvent pressedMouseButtons],
        jwm::modifierMask([event modifierFlags])));
    window->dispatch(eventObj.get());
}

void onMouseButton(jwm::WindowMac* window, NSEvent* event, CGPoint& lastPos) {
    jwm::MouseButton button = jwm::kMouseButtonValues.begin()[[event buttonNumber]];
    bool clicked = [event pressure] == 1;
    jint modifierMask = jwm::modifierMask([event modifierFlags]);
    jwm::JNILocal<jobject> eventObj(window->fEnv, jwm::classes::EventMouseButton::make(window->fEnv, button, clicked, lastPos.x, lastPos.y, modifierMask));
    window->dispatch(eventObj.get());
}

void removeTouch(jwm::WindowMac* window, NSTouchPhase phase, id identity, NSUInteger touchId, NSMutableDictionary* touchIds, NSUInteger& touchCount) {
    [touchIds removeObjectForKey:identity];
    if ([touchIds count] == 0) {
        touchCount = 0;
    }
    if (phase == NSTouchPhaseEnded) {
        jwm::JNILocal<jobject> eventObj(window->fEnv, jwm::classes::EventTouchEnd::make(window->fEnv, (jint)touchId));
        window->dispatch(eventObj.get());
    } else if (phase == NSTouchPhaseCancelled) {
        jwm::JNILocal<jobject> eventObj(window->fEnv, jwm::classes::EventTouchCancel::make(window->fEnv, (jint)touchId));
        window->dispatch(eventObj.get());
    }
}

void onTouch(jwm::WindowMac* window, NSEvent* event, NSTouchPhase phase, NSMutableDictionary* touchIds, NSMutableDictionary* touchDeviceIds, NSUInteger& touchCount) {
    // NOTE:  Mouse events originating outside the view are still tracked if the window is in focus.
    //        But touch events here are only triggered if the cursor originates inside the view.
    //        Is this related to fTrackingArea?

    // Cancel absent touches
    NSSet* activeTouches = [event touchesMatchingPhase:NSTouchPhaseAny inView:nil];
    for (id identity in [touchIds allKeys]) {
        BOOL present = NO;
        for (NSTouch* activeTouch in activeTouches) {
            if (identity == activeTouch.identity) {
                present = YES;
                break;
            }
        }
        if (!present) {
            NSUInteger touchId = [[touchIds objectForKey:identity] unsignedIntegerValue];
            removeTouch(window, NSTouchPhaseCancelled, identity, touchId, touchIds, touchCount);
        }
    }

    NSSet* touches = [event touchesMatchingPhase:phase inView:nil];
    for (NSTouch* touch in touches) {
        // Keep convention: (0, 0) in top left corner
        const NSPoint _pos = [touch normalizedPosition];
        const CGFloat x = _pos.x;
        const CGFloat y = 1.0 - _pos.y;

        // Get or create new touch ID
        NSNumber* touchIdNum = [touchIds objectForKey:touch.identity];
        NSUInteger touchId;
        if (touchIdNum) {
            touchId = [touchIdNum unsignedIntegerValue];
        } else {
            touchId = ++touchCount;
            touchIdNum = [NSNumber numberWithUnsignedInteger:touchId];
            [touchIds setObject:touchIdNum forKey:touch.identity];
        }
        [touchIdNum release];

        if (phase == NSTouchPhaseBegan) {
            // create a Device ID number and associate it with this touch’s device object address
            const NSNumber* deviceKey = [NSNumber numberWithUnsignedInteger:(NSUInteger)touch.device];
            NSNumber* deviceId = [touchDeviceIds objectForKey:deviceKey];
            if (deviceId == nil) {
                const NSUInteger deviceCount = [touchDeviceIds count];
                deviceId = [NSNumber numberWithUnsignedInteger:deviceCount + 1];
                [touchDeviceIds setObject:deviceId forKey:deviceKey];
            }

            const NSSize size = [touch deviceSize];
            jwm::JNILocal<jobject> eventObj(window->fEnv, jwm::classes::EventTouchStart::make(
                window->fEnv, (jint)touchId, x, y,
                (jint)[deviceId unsignedIntegerValue],
                size.width,
                size.height,
                jwm::kTouchTypes[touch.type]));
            window->dispatch(eventObj.get());

            [deviceKey release];
            [deviceId release];
        } else if (phase == NSTouchPhaseMoved) {
            jwm::JNILocal<jobject> eventObj(window->fEnv, jwm::classes::EventTouchMove::make(window->fEnv, (jint)touchId, x, y));
            window->dispatch(eventObj.get());
        } else if (phase != NSTouchPhaseStationary) {
            removeTouch(window, phase, touch.identity, touchId, touchIds, touchCount);
        }
    }
}

} // namespace jwm

static const NSRange kEmptyRange = { NSNotFound, 0 };

@implementation JWMMainView {
    jwm::WindowMac* fWindow;
    // A TrackingArea prevents us from capturing events outside the view
    NSTrackingArea* fTrackingArea;
    NSMutableAttributedString* fMarkedText;
    // We keep track of the state of the modifier keys on each event in order to synthesize
    // key-up/down events for each modifier.
    NSEventModifierFlags fLastFlags;
    CGPoint fLastPos;
    NSMutableDictionary* fTouchIds;
    NSMutableDictionary* fTouchDeviceIds;
    NSUInteger fTouchCount;
}

- (JWMMainView*)initWithWindow:(jwm::WindowMac*)initWindow {
    self = [super init];

    fWindow = initWindow;
    fTrackingArea = nil;
    fMarkedText = [[NSMutableAttributedString alloc] init];
    fLastFlags = 0;

    fTouchIds = [[NSMutableDictionary alloc] init];
    fTouchDeviceIds = [[NSMutableDictionary alloc] init];
    fTouchCount = 0;

    [self updateTrackingAreas];

    return self;
}

- (void)dealloc {
    [fTrackingArea release];
    [fMarkedText release];
    [fTouchIds release];
    [fTouchDeviceIds release];
    [super dealloc];
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (NSTouchTypeMask)allowedTouchTypes {
    // Direct = on-screen
    // Indirect = off-screen (e.g. trackpad)
    return NSTouchTypeMaskIndirect; // | NSTouchTypeMaskDirect;
}

- (float)getScrollScaling {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *scrollScaling = [defaults objectForKey:@"com.apple.scrollwheel.scaling"]; 

    // If never set, the value will be null. Return scale factor of 1.0
    if (scrollScaling == nil) {
        return 1.0; // No scaling.
    }

    // Value is 0->1.0. Return a "scale factor" to multiply delta by.
    return [scrollScaling floatValue] + 1.0; 
}

- (BOOL)wantsRestingTouches {
    // we set this to YES to prevent resting touches from triggering the cancel event
    return YES;
}

- (BOOL)acceptsTouchEvents {
    // deprecated according to: https://developer.apple.com/documentation/appkit/nsview/1483739-acceptstouchevents?language=objc
    // but touch events are not caught unless this property is set to YES
    return YES;
}

- (void)touchesBeganWithEvent:(NSEvent *)event {
    jwm::onTouch(fWindow, event, NSTouchPhaseBegan, fTouchIds, fTouchDeviceIds, fTouchCount);
}

- (void)touchesMovedWithEvent:(NSEvent *)event {
    jwm::onTouch(fWindow, event, NSTouchPhaseMoved, fTouchIds, fTouchDeviceIds, fTouchCount);
}

- (void)touchesEndedWithEvent:(NSEvent *)event {
    jwm::onTouch(fWindow, event, NSTouchPhaseEnded, fTouchIds, fTouchDeviceIds, fTouchCount);
}

- (void)touchesCancelledWithEvent:(NSEvent *)event {
    jwm::onTouch(fWindow, event, NSTouchPhaseCancelled, fTouchIds, fTouchDeviceIds, fTouchCount);
}

- (void)updateTrackingAreas {
    if (fTrackingArea != nil) {
        [self removeTrackingArea:fTrackingArea];
        [fTrackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    fTrackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                 options:options
                                                   owner:self
                                                userInfo:nil];

    [self addTrackingArea:fTrackingArea];
    [super updateTrackingAreas];
}

- (void)mouseMoved:(NSEvent *)event {
    onMouseMoved(fWindow, event, &fLastPos);
}

- (void)mouseDragged:(NSEvent *)event {
    onMouseMoved(fWindow, event, &fLastPos);
}

- (void)rightMouseDragged:(NSEvent *)event {
    onMouseMoved(fWindow, event, &fLastPos);
}

- (void)otherMouseDragged:(NSEvent *)event {
    onMouseMoved(fWindow, event, &fLastPos);
}

- (void)mouseDown:(NSEvent *)event {
    onMouseButton(fWindow, event, fLastPos);
}

- (void)mouseUp:(NSEvent *)event {
    onMouseButton(fWindow, event, fLastPos);
}

- (void)rightMouseDown:(NSEvent *)event {
    onMouseButton(fWindow, event, fLastPos);
}

- (void)rightMouseUp:(NSEvent *)event {
    onMouseButton(fWindow, event, fLastPos);
}

- (void)otherMouseDown:(NSEvent *)event {
    onMouseButton(fWindow, event, fLastPos);
}

- (void)otherMouseUp:(NSEvent *)event {
    onMouseButton(fWindow, event, fLastPos);
}

- (void)cursorUpdate:(NSEvent *)event {
    [fWindow->fCursor set];
}

- (void)scrollWheel:(NSEvent *)event {
    jint modifierMask = jwm::modifierMask([event modifierFlags]);
    CGFloat deltaX;
    CGFloat deltaY;
    // Retrieve the scrolling speed
    if ([event hasPreciseScrollingDeltas]) {
        deltaX = [event scrollingDeltaX];
        deltaY = [event scrollingDeltaY];
    } else {
        // https://github.com/chromium/chromium/blob/2dc93b871d2b02f895ada7f1a6fbb642cb6ec9da/ui/events/cocoa/events_mac.mm#L138-L157
        deltaX = [event deltaX] * 40.0;
        deltaY = [event deltaY] * 40.0;
    }
    CGFloat scale = fWindow->getScale();
    deltaX *= scale;
    deltaY *= scale;

    // Accessibility scaling.
    float scrollScaling = [self getScrollScaling];
    deltaX *= scrollScaling;
    deltaY *= scrollScaling;

    // deltaLines
    CGFloat deltaLines;
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    if (source) {
        double pixelsPerLine = CGEventSourceGetPixelsPerLine(source);
        deltaLines = deltaY / pixelsPerLine;
        CFRelease(source);
    }

    jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventMouseScroll::make(fWindow->fEnv, deltaX, deltaY, 0, deltaLines, 0, fLastPos.x, fLastPos.y, modifierMask));
    fWindow->dispatch(eventObj.get());
}

- (void)keyDown:(NSEvent *)event {
    fKeyEventsNeeded = YES;
    unsigned short keyCode = [event keyCode];

    [NSCursor setHiddenUntilMouseMoves:YES];

    if (jwm::kPressAndHoldEnabled && [event willBeHandledByComplexInputMethod] && !fInPressAndHold) {
        fInPressAndHold = YES;
    }

    BOOL wasInPressAndHold = fInPressAndHold;

    if (wasInPressAndHold) {
        fKeyEventsNeeded = NO;
    }

    // Allow TSM to look at the event and potentially send back NSTextInputClient messages.
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];

    if (wasInPressAndHold) {
        switch(keyCode) {
        case kVK_Return:
        case kVK_Escape:
        case kVK_PageUp:
        case kVK_PageDown:
        case kVK_DownArrow:
        case kVK_UpArrow:
        case kVK_Home:
        case kVK_End:
            [self abandonInput];
        default:
            if (!fKeyEventsNeeded)
                return;
        }
    }

    jwm::Key key = keyCode < 128 ? jwm::kKeyTable[keyCode] : jwm::Key::UNDEFINED;
    jint modifierMask = jwm::modifierMask([event modifierFlags]);
    jwm::KeyLocation location = keyCode < 128 ? jwm::kKeyLocations[keyCode] : jwm::KeyLocation::DEFAULT;
    jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, key, (jboolean) true, modifierMask, location));
    fWindow->dispatch(eventObj.get());
}

- (void)keyUp:(NSEvent *)event {
    unsigned short keyCode = [event keyCode];
    jwm::Key key = keyCode < 128 ? jwm::kKeyTable[keyCode] : jwm::Key::UNDEFINED;
    jint modifierMask = jwm::modifierMask([event modifierFlags]);
    jwm::KeyLocation location = keyCode < 128 ? jwm::kKeyLocations[keyCode] : jwm::KeyLocation::DEFAULT;
    jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, key, (jboolean) false, modifierMask, location));
    fWindow->dispatch(eventObj.get());
}

- (void)flagsChanged:(NSEvent *)event {
    NSEventModifierFlags flags = [event modifierFlags];
    jint modifierMask = jwm::modifierMask([event modifierFlags]);

    if ((fLastFlags & NSEventModifierFlagShift) == 0 && (flags & NSEventModifierFlagShift) != 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::SHIFT, (jboolean) true, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagShift) != 0 && (flags & NSEventModifierFlagShift) == 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::SHIFT, (jboolean) false, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagControl) == 0 && (flags & NSEventModifierFlagControl) != 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::CONTROL, (jboolean) true, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagControl) != 0 && (flags & NSEventModifierFlagControl) == 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::CONTROL, (jboolean) false, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagOption) == 0 && (flags & NSEventModifierFlagOption) != 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::MAC_OPTION, (jboolean) true, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagOption) != 0 && (flags & NSEventModifierFlagOption) == 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::MAC_OPTION, (jboolean) false, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagCommand) == 0 && (flags & NSEventModifierFlagCommand) != 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::MAC_COMMAND, (jboolean) true, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagCommand) != 0 && (flags & NSEventModifierFlagCommand) == 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::MAC_COMMAND, (jboolean) false, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagFunction) == 0 && (flags & NSEventModifierFlagFunction) != 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::MAC_FN, (jboolean) true, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagFunction) != 0 && (flags & NSEventModifierFlagFunction) == 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::MAC_FN, (jboolean) false, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagCapsLock) == 0 && (flags & NSEventModifierFlagCapsLock) != 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::CAPS_LOCK, (jboolean) true, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    if ((fLastFlags & NSEventModifierFlagCapsLock) != 0 && (flags & NSEventModifierFlagCapsLock) == 0) {
        jwm::JNILocal<jobject> eventObj(fWindow->fEnv, jwm::classes::EventKey::make(fWindow->fEnv, jwm::Key::CAPS_LOCK, (jboolean) false, modifierMask));
        fWindow->dispatch(eventObj.get());
    }

    fLastFlags = flags;
}

// NSTextInputClient

- (BOOL)hasMarkedText {
    // printf("hasMarkedText %lu\n", [fMarkedText length]);
    // fflush(stdout);
    return [fMarkedText length] > 0;
}

- (NSRange)markedRange {
    // printf("markedRange %lu\n", [fMarkedText length]);
    // fflush(stdout);
    if ([fMarkedText length] > 0)
        return NSMakeRange(0, [fMarkedText length] - 1);
    else
        return kEmptyRange;
}

// watasinonamaehanakanodesu
- (NSRange)selectedRange {
    JNIEnv* env = fWindow->fEnv;
    jwm::JNILocal<jobject> client(env, fWindow->getTextInputClient());
    if (!client.get()) {
        return kEmptyRange;
    } else {
        jwm::IRange range = jwm::classes::TextInputClient::getSelectedRange(env, client.get());
        NSRange result = NSMakeRange(range.fStart, range.getLength());
        return result;
    }
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange {
    // NSLog(@"setMarkedText: %@ start: %lu len: %lu", string, replacementRange.location, replacementRange.length);
    [fMarkedText release];
    if ([string isKindOfClass:[NSAttributedString class]])
        fMarkedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        fMarkedText = [[NSMutableAttributedString alloc] initWithString:string];

    JNIEnv* env = fWindow->fEnv;
    jwm::JNILocal<jstring> jtext(env, jwm::nsStringToJava(env, [fMarkedText string]));
    jint replacementStart = replacementRange.location == LONG_MAX ? -1 : replacementRange.location;
    jint replacementEnd = replacementStart + replacementRange.length;
    jwm::JNILocal<jobject> inputEvent(env, jwm::classes::EventTextInputMarked::make(env, jtext.get(), selectedRange.location, selectedRange.location + selectedRange.length, replacementStart, replacementEnd));
    fWindow->dispatch(inputEvent.get());

    if ([fMarkedText length] == 0) {
        fKeyEventsNeeded = NO;
    }
}

- (void)unmarkText {
    // printf("unmarkText\n");
    // fflush(stdout);
    [[fMarkedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText {
    // printf("validAttributesForMarkedText\n");
    // fflush(stdout);
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange {
    // printf("attributedSubstringForProposedRange (%lu %lu)\n", range.location, range.length);
    // fflush(stdout);
    JNIEnv* env = fWindow->fEnv;
    jwm::JNILocal<jobject> client(env, fWindow->getTextInputClient());
    if (!client.get()) {
        return nil;
    }
    jint start = range.location;
    jint end = start + range.length;
    jwm::JNILocal<jstring> substring(env, jwm::classes::TextInputClient::getSubstring(env, client.get(), start, end));
    if (!substring.get()) {
        return nil;
    }
    NSString* nsSubstring = jwm::nsStringFromJava(env, substring.get());
    if (actualRange) {
        *actualRange = NSMakeRange(start, [nsSubstring length]);
    }
    return [[[NSAttributedString alloc] initWithString:nsSubstring] autorelease];
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    // printf("characterIndexForPoint (%f, %f)\n", point.x, point.y);
    // fflush(stdout);
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange {
    JNIEnv* env = fWindow->fEnv;
    const NSRect frame = [fWindow->fNSWindow.contentView frame];
    CGFloat scale = fWindow->getScale();
    jwm::JNILocal<jobject> client(env, fWindow->getTextInputClient());

    NSRect nsRect;
    if (!client.get()) {
        nsRect = NSMakeRect(frame.size.width / 2.0, frame.size.height / 2.0, 0.0, 0.0);
    } else {
        jwm::IRect rect = jwm::classes::TextInputClient::getRectForMarkedRange(env, client.get(), range.location, range.location + range.length);
        nsRect = NSMakeRect(
            rect.fLeft / scale,
            frame.size.height - rect.fBottom / scale,
            (rect.fRight - rect.fLeft) / scale,
            (rect.fBottom - rect.fTop) / scale
        );
    }
    return [fWindow->fNSWindow convertRectToScreen:nsRect];
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    // NSLog(@"insertText: %@ start: %lu len: %lu", string, replacementRange.location, replacementRange.length);
    fInPressAndHold = NO;

    NSString* characters;
    NSEvent* event = [NSApp currentEvent];

    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*) string;

    JNIEnv* env = fWindow->fEnv;
    jwm::JNILocal<jstring> jtext(env, jwm::nsStringToJava(env, characters));
    jint replacementStart = replacementRange.location == LONG_MAX ? -1 : replacementRange.location;
    jint replacementEnd = replacementStart + replacementRange.length;
    jwm::JNILocal<jobject> inputEvent(env, jwm::classes::EventTextInput::make(env, jtext.get(), replacementStart, replacementEnd));
    fWindow->dispatch(inputEvent.get());

    // Abandon input to reset IM and unblock input after entering accented
    // symbols
    [self abandonInput];
}

- (void)doCommandBySelector:(SEL)selector {
    // printf("doCommandBySelector %s\n", [NSStringFromSelector(selector) UTF8String]);
    // fflush(stdout);
    // [super doCommandBySelector:selector];
    if (@selector(insertNewline:) == selector || @selector(insertTab:) == selector || @selector(deleteBackward:) == selector) {
        fKeyEventsNeeded = YES;
    }
}

- (void)abandonInput {
    [self unmarkText];
}

@end
