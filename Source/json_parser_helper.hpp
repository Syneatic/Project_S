#pragma once

#include "json.h"
#include "math.hpp"
#include "color.hpp"

inline Json::Value WriteFloat2(const float2& v)
{
    Json::Value arr(Json::arrayValue);
    arr.append(v.x);
    arr.append(v.y);
    return arr;
}

inline void ReadFloat2(const Json::Value& arr, float2& const out)
{
    if (!arr.isArray() || arr.size() != 2) return;
    if (!arr[0].isNumeric() || !arr[1].isNumeric()) return;
    out.x = arr[0].asFloat();
    out.y = arr[1].asFloat();
}

inline Json::Value WriteColor(const Color& c)
{
    Json::Value arr(Json::arrayValue);
    arr.append(c.r);
    arr.append(c.g);
    arr.append(c.b);
    arr.append(c.a);
    return arr;
}

inline void ReadColor(const Json::Value& arr, Color& out)
{
    //if (!arr.isArray() || arr.size() != 4) return;
    //if (!arr[0].isNumeric() || !arr[1].isNumeric() || !arr[2].isNumeric() || !arr[3].isNumeric()) return;
    out.r = arr[0].asFloat();
    out.g = arr[1].asFloat();
    out.b = arr[2].asFloat();
    out.a = arr[3].asFloat();
}

