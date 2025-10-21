/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#ifdef ENABLE_SCRIPTING_REFACTOR

    #include "ScParkMessage.hpp"

    #include "../../../Context.h"
    #include "../../../GameState.h"
    #include "../../../core/String.hpp"
    #include "../../../entity/Peep.h"
    #include "../../../management/Finance.h"
    #include "../../../management/NewsItem.h"
    #include "../../../windows/Intent.h"
    #include "../../../world/Park.h"
    #include "../../ScriptEngine.h"

namespace OpenRCT2::Scripting
{
    using OpaqueParkMessageData = struct
    {
        size_t index;
    };

    News::Item* ScParkMessage::GetMessage(JSValue thisVal)
    {
        auto index = gScParkMessage.GetOpaque<OpaqueParkMessageData*>(thisVal)->index;
        return &getGameState().newsItems[index];
    }

    JSValue ScParkMessage::isArchived_get(JSContext* ctx, JSValue thisVal)
    {
        auto index = gScParkMessage.GetOpaque<OpaqueParkMessageData*>(thisVal)->index;
        return JS_NewBool(ctx, index >= News::ItemHistoryStart);
    }

    JSValue ScParkMessage::month_get(JSContext* ctx, JSValue thisVal)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            return JS_NewUint32(ctx, msg->monthYear);
        }
        return JS_NewUint32(ctx, 0);
    }

    JSValue ScParkMessage::month_set(JSContext* ctx, JSValue thisVal, JSValue jsValue)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            JS_UNPACK_UINT32(value, ctx, jsValue);
            msg->monthYear = static_cast<uint16_t>(value);
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::day_get(JSContext* ctx, JSValue thisVal)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            return JS_NewUint32(ctx, msg->day);
        }
        return JS_NewUint32(ctx, 0);
    }

    JSValue ScParkMessage::day_set(JSContext* ctx, JSValue thisVal, JSValue jsValue)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            JS_UNPACK_UINT32(value, ctx, jsValue);
            msg->day = static_cast<uint8_t>(value);
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::tickCount_get(JSContext* ctx, JSValue thisVal)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            return JS_NewUint32(ctx, msg->ticks);
        }
        return JS_NewUint32(ctx, 0);
    }

    JSValue ScParkMessage::tickCount_set(JSContext* ctx, JSValue thisVal, JSValue jsValue)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            JS_UNPACK_UINT32(value, ctx, jsValue);
            msg->ticks = static_cast<uint16_t>(value);
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::type_get(JSContext* ctx, JSValue thisVal)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            return JSFromStdString(ctx, GetParkMessageType(msg->type));
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::type_set(JSContext* ctx, JSValue thisVal, JSValue jsValue)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            JS_UNPACK_STR(value, ctx, jsValue);
            msg->type = GetParkMessageType(value);
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::subject_get(JSContext* ctx, JSValue thisVal)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            return JS_NewUint32(ctx, msg->assoc);
        }
        return JS_NewUint32(ctx, 0);
    }

    JSValue ScParkMessage::subject_set(JSContext* ctx, JSValue thisVal, JSValue jsValue)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            JS_UNPACK_UINT32(value, ctx, jsValue);
            msg->assoc = value;
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::text_get(JSContext* ctx, JSValue thisVal)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            return JSFromStdString(ctx, msg->text);
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::text_set(JSContext* ctx, JSValue thisVal, JSValue jsValue)
    {
        auto msg = GetMessage(thisVal);
        if (msg != nullptr)
        {
            JS_UNPACK_STR(value, ctx, jsValue);
            msg->text = value;
        }
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::remove(JSContext*, JSValue thisVal, int, JSValue*)
    {
        auto index = gScParkMessage.GetOpaque<OpaqueParkMessageData*>(thisVal)->index;
        News::RemoveItem(static_cast<int32_t>(index));
        return JS_UNDEFINED;
    }

    JSValue ScParkMessage::New(JSContext* ctx, size_t index)
    {
        static constexpr JSCFunctionListEntry funcs[] = {
            JS_CGETSET_DEF("isArchived", ScParkMessage::isArchived_get, nullptr),
            JS_CGETSET_DEF("month", ScParkMessage::month_get, ScParkMessage::month_set),
            JS_CGETSET_DEF("day", ScParkMessage::day_get, ScParkMessage::day_set),
            JS_CGETSET_DEF("tickCount", ScParkMessage::tickCount_get, ScParkMessage::tickCount_set),
            JS_CGETSET_DEF("type", ScParkMessage::type_get, ScParkMessage::type_set),
            JS_CGETSET_DEF("subject", ScParkMessage::subject_get, ScParkMessage::subject_set),
            JS_CGETSET_DEF("text", ScParkMessage::text_get, ScParkMessage::text_set),
            JS_CFUNC_DEF("remove", 0, ScParkMessage::remove),
        };

        return MakeWithOpaque(ctx, funcs, nullptr);
    }

    void ScParkMessage::Register(JSContext* ctx)
    {
        RegisterBaseStr(ctx, "ParkMessage");
    }

    void ScParkMessage::Finalize(JSRuntime*, JSValue thisVal)
    {
        auto* data = gScParkMessage.GetOpaque<OpaqueParkMessageData*>(thisVal);
        if (data)
            delete data;
    }

} // namespace OpenRCT2::Scripting

#endif
