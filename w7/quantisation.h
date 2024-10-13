#pragma once
#include "mathUtils.h"
#include <limits>

struct Vec2
{
  float x;
  float y;
  Vec2(float x_i, float y_i) : x(x_i), y(y_i) {};
};

struct Vec3
{
  float x;
  float y;
  float z;
  Vec3(float x_i, float y_i, float z_i) : x(x_i), y(y_i), z(z_i) {};
};

template <typename T>
T pack_float(float v, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1; // std::numeric_limits<T>::max();
  return range * ((clamp(v, lo, hi) - lo) / (hi - lo));
}

template <typename T>
float unpack_float(T c, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1; // std::numeric_limits<T>::max();
  return float(c) / range * (hi - lo) + lo;
}

template <typename T, int num_bits>
struct PackedFloat
{
  T packedVal;

  PackedFloat(float v, float lo, float hi) { pack(v, lo, hi); }
  PackedFloat(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v, float lo, float hi) { packedVal = pack_float<T>(v, lo, hi, num_bits); }
  float unpack(float lo, float hi) { return unpack_float<T>(packedVal, lo, hi, num_bits); }
};

typedef PackedFloat<uint8_t, 4> float4bitsQuantized;

template <typename T, int num_bits_h, int num_bits_l>
struct PackedVec2
{
  T packedVal;

  PackedVec2(Vec2 v, Vec2 lo, Vec2 hi) { pack(v, lo, hi); };
  PackedVec2(T pv) : packedVal(pv) {};

  void pack(Vec2 v, Vec2 lo, Vec2 hi)
  {
    packedVal = pack_float<T>(v.x, lo.x, hi.x, num_bits_h);
    packedVal <<= num_bits_l;
    packedVal += pack_float<T>(v.y, lo.y, hi.y, num_bits_l);
  }

  Vec2 unpack(Vec2 lo, Vec2 hi)
  {
    return {unpack_float<T>(packedVal >> num_bits_l, lo.x, hi.x, num_bits_h),
            unpack_float<T>(packedVal & (1 << num_bits_l) - 1, lo.y, hi.y, num_bits_l)};
  }
};

template <typename T, int num_bits_h, int num_bits_m, int num_bits_l>
struct PackedVec3
{
  T packedVal;

  PackedVec3(Vec3 v, Vec3 lo, Vec3 hi) { pack(v, lo, hi); };
  PackedVec3(T pv) : packedVal(pv) {};

  void pack(Vec3 v, Vec3 lo, Vec3 hi)
  {
    packedVal = pack_float<T>(v.x, lo.x, hi.x, num_bits_h);
    packedVal <<= num_bits_m;
    packedVal += pack_float<T>(v.y, lo.y, hi.y, num_bits_m);
    packedVal <<= num_bits_l;
    packedVal += pack_float<T>(v.z, lo.z, hi.z, num_bits_l);
  }

  Vec3 unpack(Vec3 lo, Vec3 hi)
  {
    return {unpack_float<T>(packedVal >> num_bits_l + num_bits_m, lo.x, hi.x, num_bits_h),
            unpack_float<T>(packedVal >> num_bits_l & (1 << num_bits_m) - 1, lo.y, hi.y, num_bits_m),
            unpack_float<T>(packedVal & (1 << num_bits_l) - 1, lo.z, hi.z, num_bits_l)};
  }
};
