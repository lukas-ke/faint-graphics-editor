// -*- coding: us-ascii-unix -*
#include <algorithm>
#include <cassert>
#include <sstream>
#include "tests/test-util/image-table.hh"
#include "util-wx/file-path.hh"
#include "text/utf8-string.hh"

namespace faint {

using Row = std::vector<std::string>;
using Rows = std::vector<Row>;

class ImageTable::ImageTableImpl {
public:
  ImageTableImpl(const std::string& pageTitle,
    const std::vector<utf8_string>& headings)
  {
    for (auto& h : headings){
      m_columnHeadings.emplace_back(h.str());
    }
    m_pageTitle = pageTitle;
  }

  void AddCell(const std::string& s){
    assert(m_rows.size() != 0);
    Row& row = m_rows.back();
    row.emplace_back(s);
    m_numCols = std::max(m_numCols, row.size());
  }

  void AddRow(){
    m_rows.emplace_back(Row());
  }

  std::string ToHtml() const{
    std::string html = std::string("<html><head><title>") + m_pageTitle  +
      "</title></head><body><table border='0'><tr>";
    for (const auto& heading : m_columnHeadings){
      html += "<th>" + heading + "</th>";
    }
    html += "</tr>";

    for (const auto& row : m_rows){
      html += "<tr>";
      for (auto& cell : row){
        html += "<th>" + cell + "</th>";
      }
      for (size_t colNum = row.size(); colNum < m_numCols; colNum++){
        html += "<th></th>";
      }
      html += "</tr>";
    }
    html += "</table></body></html>";
    return html;
  }

private:
  std::string m_pageTitle;
  Row m_columnHeadings;
  Rows m_rows;
  size_t m_numCols = 0;
};

ImageTable::ImageTable(const std::string& title,
  const std::vector<utf8_string>& headings)
{
  m_impl = new ImageTableImpl(title, headings);
}

ImageTable::~ImageTable(){
  delete m_impl;
}

void ImageTable::AddRow(const std::string& label, const FileName& f1, const FileName& f2){
  m_impl->AddRow();


  m_impl->AddCell("<center>" + label + "</center>"); // Fixme
  std::stringstream ss;
  ss << "<img src='" << f1.Str() << "' onmouseover=\"this.src='" << f2.Str() <<
    "'\" onmouseout=\"this.src='" << f1.Str() << "'\"/>";
  m_impl->AddCell(ss.str());
  ss.str("");
  ss << "<img src='" << f2.Str() << "' onmouseover=\"this.src='" << f1.Str() <<
    "'\" onmouseout=\"this.src='" << f2.Str() << "'\"/>";
  m_impl->AddCell(ss.str());
}

std::string img_cell(const FileName& f){
  std::stringstream ss;
  ss << "<img src='" << f.Str() << "'\"/>";
  return ss.str();
}

void ImageTable::AddRow(const faint::utf8_string& label, const FileName& f1, const FileName& f2){
  AddRow(label.str(), f1, f2);
}


void ImageTable::AddRow(){
  m_impl->AddRow();
}


void ImageTable::AddCell(const std::string& cell){
  m_impl->AddCell(cell);
}

void ImageTable::AddCell(const utf8_string& cell){
  m_impl->AddCell(cell.str());
}

void ImageTable::AddCell(const FilePath& f){
  AddCell(f.StripPath());
}

void ImageTable::AddCell(const FileName& f){
  m_impl->AddCell(img_cell(f));
}

std::string ImageTable::ToHtml() const{
  return m_impl->ToHtml();
}

} // namespace
