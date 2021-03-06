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

#ifndef FAINT_COMMON_FWD_HH
#define FAINT_COMMON_FWD_HH
#include <memory>
#include <string>
#include <vector>
#include "commands/command-ptr.hh"
#include "geo/geo-fwd.hh"
#include "geo/primitive.hh"
#include "util/objects.hh"

namespace faint{

class AlphaMap;
class AlphaMapRef;
class AppContext;
class Bitmap;
class Brush;
class Calibration;
class Canvas;
class ClosedIntRange;
class ColRGB;
class Color;
class ColorSpan;
class ColorStop;
class Command;
class DirPath;
class ExpressionContext;
class FaintDC;
class FilePath;
class Format;
class Frame;
class Gradient;
class Grid;
class Image;
class ImageInfo;
class ImageList;
class ImageProps;
class Index;
class Interval;
class KeyPress;
class LinearGradient;
class Mod;
class Object;
class Overlays;
class Paint;
class PaintMap;
class Pattern;
class PosInfo;
class PythonContext;
class RadialGradient;
class RasterSelection;
class SelectionOptions;
class SelectionState;
class Settings;
class Task;
class TextLine;
class Tool;
class ToolModifiers;
class ZoomLevel;

class utf8_char;
class utf8_string;
enum class Cursor;
enum class Layer;
enum class MouseButton;

using tris_t = std::vector<Tri>;
using strings_t = std::vector<std::string>;
using FileList = std::vector<FilePath>;

} // namespace

namespace faint::sel{
  class Existing;
} // namespace

#endif
