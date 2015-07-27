// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_STUB_COMMAND_CONTEXT_HH
#define FAINT_STUB_COMMAND_CONTEXT_HH
#include <stdexcept>
#include "commands/command.hh"
#include "geo/int-size.hh"

namespace faint{ class Object; }

namespace test{

using namespace faint;

class StubCommandContext : public CommandContext{
public:
  StubCommandContext(){}
  void Add(Object*, const select_added&, const deselect_old&) override{}
  void Add(Object*, int, const select_added&, const deselect_old&) override{}
  void AddFrame(Image*) override{}
  void AddFrame(Image*, const Index&) override{}
  const Bitmap& GetBitmap() const override{
    throw std::exception();
  }

  FaintDC& GetDC() override{
    throw std::exception();
  }

  Image& GetFrame() override{
    throw std::exception();
  }

  Image& GetFrame(const Index&) override{
    throw std::exception();
  }

  RasterSelection& GetRasterSelection() override{
    throw std::exception();
  }

  IntSize GetImageSize() const override{
    return {0,0};
  }

  const objects_t& GetObjects() override{
    throw std::exception();
  }

  int GetObjectZ(const Object*) override{
    return 0;
  }

  Bitmap& GetRawBitmap() override{
    throw std::exception();
  }

  bool HasObjects() const override{
    return false;
  }

  void MoveRasterSelection(const IntPoint&) override{}
  void OffsetOrigin(const IntPoint&) override{}
  void Remove(Object*) override{}
  void RemoveFrame(const Index&) override{}
  void RemoveFrame(Image*) override{}
  void ReorderFrame(const NewIndex&, const OldIndex&) override{}
  void SetBitmap(const Bitmap&) override{}
  void SetBitmap(Bitmap&&) override{}
  void SetRasterSelection(const SelectionState&) override{}
  void SetRasterSelectionOptions(const SelectionOptions&) override{}
  void SetObjectZ(Object*, int) override{}
};

} // namespace

#endif
