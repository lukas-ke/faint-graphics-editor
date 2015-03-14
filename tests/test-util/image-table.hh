// -*- coding: us-ascii-unix -*
#ifndef FAINT_TEST_IMAGE_TABLE_HH
#define FAINT_TEST_IMAGE_TABLE_HH
#include <string>
#include <vector>
#include "text/utf8-string.hh"

namespace faint{

class FileName;
class FilePath;

class ImageTable{
public:
  ImageTable(const std::string& title);
  ImageTable(const std::string& title, const std::vector<utf8_string>& headings);
  ~ImageTable();
  void AddRow(const std::string&, const FileName&, const FileName&);
  void AddRow(const faint::utf8_string&, const FileName&, const FileName&);

  template<class... Args>
  void AddRow(Args... args){
    AddRow();
    AddCells(args...);
  }

  void AddRow();
  void AddCell(const std::string&);
  void AddCell(const utf8_string&);
  void AddCell(const FilePath&);
  void AddCell(const FileName&);
  std::string ToHtml() const;
private:
  class ImageTableImpl;
  ImageTableImpl* m_impl;

  template<class T1, class... Args>
  void AddCells(T1 v1, Args... args){
    AddCell(v1);
    AddCells(args...);
  }

  void AddCells(){
  }
};

} // namespace

#endif
