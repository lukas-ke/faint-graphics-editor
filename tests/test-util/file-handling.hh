// -*- coding: us-ascii-unix -*-

#ifndef FAINT_TEST_FILE_HANDLING_HH
#define FAINT_TEST_FILE_HANDLING_HH
#include "bitmap/bitmap.hh"
#include "util-wx/file-path.hh"

namespace faint{

FilePath get_test_save_path(const FileName&);
FilePath save_test_image(const Bitmap&, const FileName&);

FilePath get_test_load_path(const FileName&);
Bitmap load_test_image(const FileName&);

class ImageTable;
void save_image_table(const ImageTable&);

} // namespace

#endif
