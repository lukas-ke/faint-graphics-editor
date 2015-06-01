// -*- coding: us-ascii-unix -*-
#include <stdexcept>
#include <fstream>
#include "wx/dir.h"
#include "formats/png/file-png.hh"
#include "test-sys/test-name.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"

namespace faint{

FilePath get_test_load_path(const FileName& filename){
  return make_absolute_file_path("test-data/" + filename.Str()).Visit(
    [](const FilePath& path){
      return path;
    },
    []() -> FilePath{
      throw std::logic_error("Could not create absolute path");
    });
}

static OrError<Bitmap> load_png(const FilePath& path){
  if (path.Extension() == FileExtension("png")){
    return read_png(path);
  }
  return space_sep("Unsupported extension:", path.Extension().Str());
}

Bitmap load_test_image(const FileName& fileName){
  const auto path = get_test_load_path(fileName);
  return load_png(path).Visit(
    [](const Bitmap& bmp){
      return bmp;
    },
    [&](const utf8_string&) -> Bitmap{
      throw std::runtime_error(std::string("load_test_image failed loading \"") +
        path.Str().c_str() + "\"");
    });
}

static SaveResult save_as_png(const Bitmap& bmp, const FilePath& path){
  return write_png(path, bmp, PngColorType::RGB_ALPHA);
}

FilePath save_test_image(const Bitmap& bmp, const FileName& filename){
  const auto outPath = get_test_save_path(filename);
  save_as_png(bmp, outPath);
  return outPath;
}

FilePath get_test_save_path(const FileName& fileName){
  DirPath out("out");
  if (!exists(out)){
    make_dir(out);
  }

  DirPath testDir(wxString("out/") + get_test_name());
  if (!exists(testDir)){
    make_dir(testDir);
  }

  return make_absolute_file_path(utf8_string("out/" +
      get_test_name() + "/" +
      fileName.Str().str())).Get();
}

void save_image_table(const ImageTable& table){
  std::ofstream f(get_test_save_path(FileName("index.html")).Str().c_str());
  f << table.ToHtml();
}

utf8_string u8_chars(){
  utf8_string s;
  s += utf8_char(hyphen);
  s += utf8_char(246);
  return s;
}

FileName suffix_u8_chars(const FileName& f){
  auto p = split_extension(f);
  return FileName(p.first + u8_chars() + p.second);
}

} // namespace
