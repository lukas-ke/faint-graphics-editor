// -*- coding: us-ascii-unix -*-
#include <stdexcept>
#include <fstream>
#include "formats/wx/file-image-wx.hh"
#include "test-sys/test-name.hh"
#include "text/formatting.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"
#include "wx/dir.h"

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
    return read_image_wx(path, wxBITMAP_TYPE_PNG);
  }

  return space_sep("Unsupported extension:", path.Extension().Str());
}

Bitmap load_test_image(const FileName& fileName){
  return load_png(get_test_load_path(fileName)).Visit(
    [](const Bitmap& bmp){
      return bmp;
    },
    [](const utf8_string&) -> Bitmap{
      // Todo: Throw fail-test-exception
      throw std::logic_error("Failed loading");
    });
}

static SaveResult save_as_png(const Bitmap& bmp, const FilePath& path){
  return write_image_wx(bmp, wxBITMAP_TYPE_PNG, path);
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

} // namespace
