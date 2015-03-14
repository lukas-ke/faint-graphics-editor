// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#include <cassert>
#include "bitmap/bitmap.hh"
#include "bitmap/pattern.hh"
#include "util/id-types.hh"

namespace faint{

using PatternId = FaintID<110>;
using ref_map_t = std::map<PatternId, int>;
using pat_map_t = std::map<PatternId, Bitmap>;

class PatternRef{
public:
  PatternRef(const PatternId&);
  PatternRef(const PatternRef&);
  ~PatternRef();
  const Bitmap& GetBitmap() const;
  bool operator==(const PatternRef&) const;
  bool operator!=(const PatternRef&) const;
  bool operator<(const PatternRef&) const;
  PatternRef operator=(const PatternRef&) = delete;
private:
  PatternId m_id;
};

class PatternHandler{
public:
  PatternRef Add(const Bitmap& pattern){
    PatternId newId;
    m_refCounts[newId] = 0;
    m_patterns[newId] = pattern;
    return PatternRef(newId);
  }

  void Increment(const PatternId& id){
    ref_map_t::iterator it = m_refCounts.find(id);
    assert(it != m_refCounts.end());
    it->second++;
  }

  void Decrement(const PatternId& id){
    ref_map_t::iterator it = m_refCounts.find(id);
    assert(it != m_refCounts.end());
    assert(it->second >= 1);
    it->second--;
    if (it->second == 0){
      Erase(id);
    }
  }

  const Bitmap& Get(const PatternId& id) const{
    pat_map_t::const_iterator it(m_patterns.find(id));
    assert(it != m_patterns.end());
    return it->second;
  }

  std::map<int, int> GetStatus() const{
    std::map<int, int> statusMap;
    for (const auto& idToCount : m_refCounts){
      statusMap[idToCount.first.Raw()] = idToCount.second;
    }
    return statusMap;
  }
private:
  void Erase(const PatternId& id){
    ref_map_t::iterator refIt = m_refCounts.find(id);
    assert(refIt != m_refCounts.end()); // Erase of non existing pattern
    assert(refIt->second == 0); // Erased pattern with non-zero ref-count
    m_refCounts.erase(refIt);

    pat_map_t::iterator patIt = m_patterns.find(id);
    assert(patIt != m_patterns.end()); // Reference counted pattern has no bitmap
    m_patterns.erase(patIt);
  }

  ref_map_t m_refCounts;
  pat_map_t m_patterns;
};

static PatternHandler& get_pattern_handler(){
  static PatternHandler p;
  return p;
}

PatternRef::PatternRef(const PatternId& id)
  : m_id(id)
{
  get_pattern_handler().Increment(m_id);
}

PatternRef::PatternRef(const PatternRef& other)
  : m_id(other.m_id)
{
  get_pattern_handler().Increment(m_id);
}

PatternRef::~PatternRef(){
  get_pattern_handler().Decrement(m_id);
}

bool PatternRef::operator==(const PatternRef& other) const{
  return m_id == other.m_id;
}

bool PatternRef::operator!=(const PatternRef& other) const{
  return m_id != other.m_id;
}

bool PatternRef::operator<(const PatternRef& other) const{
  return m_id < other.m_id;
}

const Bitmap& PatternRef::GetBitmap() const{
  return get_pattern_handler().Get(m_id);
}

Pattern::Pattern(const Bitmap& bmp)
  : Pattern(bmp, IntPoint(0,0), object_aligned_t(false))
{}

Pattern::Pattern(const Bitmap& bmp,
  const IntPoint& anchor,
  const object_aligned_t& objectAligned)
  : m_anchor(anchor),
    m_objectAligned(objectAligned.Get()),
    m_ref(new PatternRef(get_pattern_handler().Add(bmp)))
{}

Pattern::Pattern(const Pattern& other)
  : m_anchor(other.m_anchor),
    m_objectAligned(other.m_objectAligned),
    m_ref(new PatternRef(*other.m_ref))
{}

Pattern::~Pattern(){
  delete m_ref;
}

const Bitmap& Pattern::GetBitmap() const{
  return m_ref->GetBitmap();
}

IntPoint Pattern::GetAnchor() const{
  return m_anchor;
}

bool Pattern::GetObjectAligned() const{
  return m_objectAligned;
}

IntSize Pattern::GetSize() const{
  return m_ref->GetBitmap().GetSize();
}

void Pattern::SetAnchor(const IntPoint& anchor){
  m_anchor = anchor;
}

void Pattern::SetBitmap(const Bitmap& bmp){
  delete m_ref;
  m_ref = new PatternRef(get_pattern_handler().Add(bmp));
}

void Pattern::SetObjectAligned(bool objectAligned){
  m_objectAligned = objectAligned;
}

Pattern& Pattern::operator=(const Pattern& other){
  if (this == &other){
    return *this;
  }
  delete m_ref;
  m_anchor = other.m_anchor;
  m_objectAligned = other.m_objectAligned;
  m_ref = new PatternRef(*other.m_ref);
  return *this;
}

bool Pattern::operator==(const Pattern& other) const{
  return (*m_ref == *other.m_ref &&
    m_anchor == other.m_anchor &&
    m_objectAligned == other.m_objectAligned);
}

bool Pattern::operator<(const Pattern& other) const{
  if (*m_ref < *other.m_ref){
    return true;
  }
  else if (*m_ref != *other.m_ref){
    return false;
  }
  else if (m_anchor < other.m_anchor){
    return true;
  }
  else if (m_anchor != other.m_anchor){
    return false;
  }
  return m_objectAligned < other.m_objectAligned;
}

bool Pattern::operator>(const Pattern& other) const{
  return !operator==(other) && !operator<(other);
}

Pattern offsat(const Pattern& p, const IntPoint& delta){
  Pattern p2(p);
  p2.SetAnchor(p.GetAnchor() + delta);
  return p2;
}

std::map<int, int> pattern_status(){
  return get_pattern_handler().GetStatus();
}

} // namespace
