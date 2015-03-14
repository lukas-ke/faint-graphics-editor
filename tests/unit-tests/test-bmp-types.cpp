// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "formats/bmp/serialize-bmp-types.hh"

void test_bmp_types(){
  using namespace faint;

  {
    // Test the struct_lengths function
    EQUAL(struct_lengths<BitmapFileHeader>(), 14);
    EQUAL(struct_lengths<BitmapInfoHeader>(), 40);
    size_t sum1 = struct_lengths<BitmapFileHeader, BitmapInfoHeader>();
    size_t sum2 = StructInfo<BitmapFileHeader>::bytes +
      StructInfo<BitmapInfoHeader>::bytes;

    EQUAL(sum1, sum2);
    EQUAL(sum1, 14 + 40);
  }

  {
    BitmapFileHeader h1 = {0x424d,
                           500,
                           0,
                           0,
                           0x1000};
    auto a = serialize_struct(h1);
    auto h2 = deserialize_struct<BitmapFileHeader>(a);
    EQUAL(h1.fileType, h2.fileType);
    EQUAL(h1.fileLength, h2.fileLength);
    EQUAL(h1.reserved1, h2.reserved1);
    EQUAL(h1.reserved2, h2.reserved2);
    EQUAL(h1.dataOffset, h2.dataOffset);
  }

  {
    BitmapInfoHeader h1 = {
      40,
      640, 480,
      1,
      32,
      Compression::BI_RGB,
      1000,
      1,
      1,
      0,
      0};
    auto a = serialize_struct(h1);
    auto h2 = deserialize_struct<BitmapInfoHeader>(a);
    EQUAL(h1.headerLen, h2.headerLen);
    EQUAL(h1.width, h2.width);
    EQUAL(h1.height, h2.height);
    EQUAL(h1.colorPlanes, h2.colorPlanes);
    EQUAL(h1.bpp, h2.bpp);
    EQUALF(h1.compression, h2.compression, enum_str);
    EQUAL(h1.rawDataSize, h2.rawDataSize);
    EQUAL(h1.horizontalResolution, h2.horizontalResolution);
    EQUAL(h1.verticalResolution, h2.verticalResolution);
    EQUAL(h1.paletteColors, h2.paletteColors);
    EQUAL(h1.importantColors, h2.importantColors);
  }

  {
    IconDir h1 = {0,
                  IconType::ICO,
                  1};
    auto a = serialize_struct(h1);
    auto h2 = deserialize_struct<IconDir>(a);
    EQUAL(h1.reserved, h2.reserved);
    EQUALF(h1.imageType, h2.imageType, enum_str);
    EQUAL(h1.imageCount, h2.imageCount);
  }

  {
    IconDirEntry h1 = {128,
                       128,
                       0,
                       0,
                       1,
                       24,
                       1000,
                       2000};
    auto a = serialize_struct(h1);
    auto h2 = deserialize_struct<IconDirEntry>(a);
    EQUAL(h1.width, h2.width);
    EQUAL(h1.height, h2.height);
    EQUAL(h1.colorCount, h2.colorCount);
    EQUAL(h1.reserved, h2.reserved);
    EQUAL(h1.colorPlanes, h2.colorPlanes);
    EQUAL(h1.bpp, h2.bpp);
    EQUAL(h1.bytes, h2.bytes);
    EQUAL(h1.offset, h2.offset);
  }
}
