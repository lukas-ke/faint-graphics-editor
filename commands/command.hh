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

#ifndef FAINT_COMMAND_HH
#define FAINT_COMMAND_HH
#include "commands/command-constants.hh"
#include "util/id-types.hh"
#include "util/index.hh"
#include "util/objects.hh"
#include "util/pending.hh"

namespace faint{

class Bitmap;
class Command;
class FaintDC;
class Image;
class IntPoint;
class IntSize;
class Object;
class Point;
class RasterSelection;
class SelectionOptions;
class SelectionState;
class utf8_string;

// True if the command affects the raster layer in a document-changing
// way (not merely selecting a region for example).
bool affects_raster(const Command*);
bool fully_reversible(CommandType);
bool somewhat_reversible(CommandType);

class CommandContext{
  // Interface passed to Commands to let them modify an image.
  // Note: Anything the CommandContext returns by reference
  // (e.g. GetDC, GetFrame) has uncertain life-time and may be
  // invalidated by other CommandContext methods.
public:
  virtual ~CommandContext() = default;
  virtual void Add(Object*, const select_added&, const deselect_old&) = 0;
  virtual void Add(Object*, int z, const select_added&, const deselect_old&) = 0;
  virtual void AddFrame(Image*) = 0;
  virtual void AddFrame(Image*, const Index&) = 0;
  virtual const Bitmap& GetBitmap() const = 0;
  virtual FaintDC& GetDC() = 0;
  virtual Image& GetFrame() = 0;
  virtual Image& GetFrame(const Index&) = 0;
  virtual RasterSelection& GetRasterSelection() = 0;
  virtual IntSize GetImageSize() const = 0;
  virtual const objects_t& GetObjects() = 0;
  virtual int GetObjectZ(const Object*) = 0;
  virtual Bitmap& GetRawBitmap() = 0;
  virtual bool HasObjects() const = 0;
  virtual void MoveRasterSelection(const IntPoint& topLeft) = 0;

  // Offset selections, objects and hot spots by the specified offset
  // (for example due to image resizing)
  virtual void OffsetOrigin(const IntPoint& delta) = 0;
  virtual void Remove(Object*) = 0;
  virtual void RemoveFrame(const Index&) = 0;
  virtual void RemoveFrame(Image*) = 0;
  virtual void ReorderFrame(const NewIndex&, const OldIndex&) = 0;
  virtual void SetBitmap(const Bitmap&) = 0;
  virtual void SetBitmap(Bitmap&&) = 0;
  virtual void SetRasterSelection(const SelectionState&) = 0;
  virtual void SetRasterSelectionOptions(const SelectionOptions&) = 0;
  virtual void SetObjectZ(Object*, int z) = 0;
};

class Command{
public:
  Command(CommandType);
  virtual ~Command() = default;
  virtual void Do(CommandContext&) = 0;
  // Do only the Raster part of the command
  // (Calls Do(...) by default, but must be overridden for Hybrid commands)
  virtual void DoRaster(CommandContext&);

  // True if the passed in command could be merged with this command.
  // sameFrame specifies if the command was targetting the same frame
  // as this command.
  virtual bool Merge(Command*, bool sameFrame);

  // "Do What I Mean" - returns an alternate command if available.
  // Should only be called after HasDWIM() returns true
  virtual Command* GetDWIM();
  virtual CommandId GetId() const;
  virtual bool HasDWIM() const;

  // Whether the command modifies the image, which most commands do.
  // Some selection actions do not change the image, and should not
  // flag the image has dirty - but should still support undo/redo.
  virtual bool ModifiesState() const;
  virtual utf8_string Name() const = 0;
  // Commands that change the image size (e.g. cropping, scaling) can
  // translate a point, expressed in image coordinates, relative to
  // the transformation.
  virtual Point Translate(const Point&) const;
  CommandType Type() const;
  virtual void Undo(CommandContext&);
  virtual Point UndoTranslate(const Point&) const;
private:
  CommandType m_type;
  CommandId m_id;
};

// Manages memory for a not-yet-performed command. Releases the memory
// for the command if it is not Retrieve():ed before destruction or
// before another command is set.
using PendingCommand = Pending<Command>;

class Operation{
public:
  virtual ~Operation() = default;
  virtual Command* DoImage() const = 0;
  virtual Command* DoObjects(const objects_t&) const = 0;
  virtual Command* DoRasterSelection(const Image&) const = 0;
};

} // namespace

#endif
