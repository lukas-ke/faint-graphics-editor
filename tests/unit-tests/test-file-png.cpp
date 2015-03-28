// -*- coding: us-ascii-unix -*-
#include "png.h"
#include "test-sys/test.hh"
#include "util-wx/stream.hh"
#include "tests/test-util/file-handling.hh"
#include "text/utf8-string.hh"
#include <iostream>
#include <cstdint>
#include <csetjmp>
#include "formats/bmp/file-bmp.hh"

#ifdef _MSC_VER
#pragma warning(disable:4611) // _setjmp and C++-object destruction
#endif

// Fixme: Use only C-code in there

// Fixme: Not int for w, h
extern "C" bool read_png(const char* path, png_bytep** rowPointers,
  png_uint_32* width,
  png_uint_32* height)
{
  FILE* f = fopen(path, "rb");
  if (f == 0){
    return false;
  }

  png_byte sig[8];
  fread(sig, 1, 8, f);
  if (!png_check_sig(sig, 8)){
    return false;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
    nullptr, nullptr, nullptr);
  if (png_ptr == nullptr){
    return false;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr){
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))){
    // Fixme: Pass end_info
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return false;
  }

  png_init_io(png_ptr, f);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  png_textp textPtr;
  int numText;
  if (png_get_text(png_ptr, info_ptr, &textPtr, &numText) > 0){
    for (int i = 0; i != numText; i++){
      // std::cout << "Text: " << textPtr[i].text << std::endl;
    }
  }

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);

  png_byte colorType = png_get_color_type(png_ptr, info_ptr);
  png_byte bitDepth = png_get_bit_depth(png_ptr, info_ptr);

  int numPasses = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr))){
    // TODO: Free ptr
    return false;
  }

  (*rowPointers) = (png_bytep*) malloc(sizeof(png_bytep) * (*height));
  for (png_uint_32 y = 0; y < (*height); y++){
    (*rowPointers)[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
  }

  png_read_image(png_ptr, *rowPointers);
  // TODO: Read post-image-data

  fclose(f);
  png_destroy_read_struct(&png_ptr, nullptr, nullptr); // Maybe
  return true;
}

void test_file_png(){
  using namespace faint;
  auto path = get_test_load_path(FileName("square.png"));
  png_bytep* rowPointers;
  png_uint_32 width;
  png_uint_32 height;
  bool ok = read_png(path.Str().c_str(), &rowPointers, &width, &height);

  Bitmap bmp(IntSize(resigned(width), resigned(height)));
  auto* p = bmp.GetRaw();
  auto stride = bmp.GetStride();

  // Fixme: Handle different byte order, bit-depth etc.
  for (png_uint_32 y = 0; y < height; y++){
    for (png_uint_32 x = 0; x < width; x++){
      int i =  y * stride + x * faint::BPP;
      p[i + faint::iR] = rowPointers[y][x * 4];
      p[i + faint::iG] = rowPointers[y][x * 4 + 1];
      p[i + faint::iB] = rowPointers[y][x * 4 + 2];
      p[i + faint::iA] = rowPointers[y][x * 4 + 3];
    }
  }

  auto out = get_test_save_path(FileName("out.bmp"));
  write_bmp(out, bmp, BitmapQuality::COLOR_24BIT);

  VERIFY(ok);
}
