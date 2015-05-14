#include <cassert>
#include "bitmap/alpha-map.hh"
#include "bitmap/brush.hh"
#include "bitmap/draw.hh"
#include "bitmap/mask.hh"
#include "tests/test-util/text-bitmap.hh"
#include "test-sys/test.hh"

namespace faint{

template<typename T>
struct MapType{};
template<> struct MapType<Bitmap>{using value_type = Color;};
template<> struct MapType<AlphaMap>{using value_type = uchar;};
template<> struct MapType<Brush>{using value_type = uchar;};
template<> struct MapType<Mask>{using value_type = bool;};

// Type mapping a character (e.g. 'x' to the value type for a certain
// "bitmap" type (e.g. uchar for brushes, Color for Bitmap).
template<typename T>
using ValueMap = std::map<char, typename MapType<T>::value_type>;

static void set_value(Bitmap& bmp, const IntPoint& pos, const Color& c){
  put_pixel(bmp, pos, c);
}

static void set_value(Brush& brush, const IntPoint& pos, uchar alphaValue){
  brush.Set(pos, alphaValue);
}

static void set_value(Mask& mask, const IntPoint& pos, bool value){
  mask.Set(pos.x, pos.y, value);
}

uchar get_value(const AlphaMap& alphaMap, const IntPoint& pos){
  return alphaMap.Get(pos.x, pos.y);
}

Color get_value(const Bitmap& bmp, const IntPoint& pos){
  return get_color(bmp, pos);
}

template<typename T>
T create_map(const IntSize& sz,
  const std::string& s,
  const ValueMap<T>& valueMap)
{
  assert(resigned(s.size()) == area(sz));

  T object(sz);
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      size_t index = to_size_t(y * sz.w + x);
      assert(index < s.size());
      const auto& v = valueMap.at(s[index]);
      set_value(object, {x, y}, v);
    }
  }
  return object;
}

Bitmap create_bitmap(const IntSize& sz,
  const std::string& s,
  const std::map<char, faint::Color>& colorMap)
{
  return create_map<Bitmap>(sz, s, colorMap);
}

BitmapAndMask create_bitmap_and_mask(const IntSize& sz,
  const std::string& bmp,
  const std::map<char, faint::Color>& colorMap,
  const std::string& mask)
{
  return {create_map<Bitmap>(sz, bmp, colorMap), create_mask(sz, mask)};
}

Brush create_brush(const IntSize& sz,
  const std::string& s,
  const std::map<char, uchar>& charToAlpha)
{
  return create_map<Brush>(sz, s, charToAlpha);
}

Mask create_mask(const IntSize& sz, const std::string& s){
  return create_map<Mask>(sz, s, {{'#', true}, {' ', false}});
}


template<typename T>
void check_impl(const T& item, const std::string& s,
  const ValueMap<T>& valueMap)
{
  IntSize sz(item.GetSize());
  try {
    for (int y = 0; y != sz.h; y++){
      for (int x = 0; x != sz.w; x++){
        size_t index = to_size_t(y * sz.w + x);
        ASSERT(index < s.size());
        char ch = s[index];
        auto it = valueMap.find(ch);
        ASSERT(it != valueMap.end());
        ASSERT(get_value(item, {x,y}) == it->second);
      }
    }
  }
  catch (const AbortTestException&){
    // The test will have been flagged as failed.
    // No need to abort.
  }
}

void check(const Bitmap& bmp, const std::string& s,
  const bitmap_value_map& colorMap)
{
  check_impl(bmp, s, colorMap);
}

void check(const AlphaMap& alphaMap, const std::string& s,
  const alphamap_value_map& valueMap)
{
  check_impl(alphaMap, s, valueMap);
}

} // namespace
