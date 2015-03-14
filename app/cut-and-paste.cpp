// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include "app/app-context.hh"
#include "app/canvas.hh"
#include "app/cut-and-paste.hh"
#include "bitmap/paint.hh"
#include "geo/geo-func.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "objects/objraster.hh"
#include "tools/tool-id.hh"
#include "util-wx/clipboard.hh"
#include "util/command-util.hh"
#include "util/default-settings.hh"
#include "util/image-props.hh"
#include "util/image-util.hh"
#include "util/image.hh"
#include "util/object-util.hh"

namespace faint{

class category_clipboard;
using erase_copied = Distinct<bool, category_clipboard, 0>;

static bool selection_to_clipboard(AppContext& app,
  const erase_copied& eraseCopied)
{
  Canvas& canvas = app.GetActiveCanvas();
  const Image& image = canvas.GetImage();
  ToolInterface& tool = canvas.GetTool();
  auto maybeText = eraseCopied.Get() ?
    tool.CutText() :
    tool.CopyText();

  if (maybeText.IsSet()){
    Clipboard clipboard;
    if (!clipboard.Good()){
      return false;
    }
    clipboard.SetText(maybeText.Get());
    return true;
  }

  const RasterSelection& selection = canvas.GetRasterSelection();
  if (tool.GetLayerType() == Layer::RASTER && selection.Exists()){
    Clipboard clipboard;
    if (!clipboard.Good()){
      return false;
    }
    if (selection.Floating()){
      Paint bg = bitmap_to_clipboard(app, clipboard, selection.GetBitmap());
      if (eraseCopied.Get()){
        canvas.RunCommand(get_delete_raster_selection_command(image, bg));
      }
    }
    else{
      // The non-floating selection can be dragged outside the image
      // with the right mouse, so clip it to the data
      IntRect rect(intersection(selection.GetRect(), image_rect(image)));
      if (empty(rect)){
        return false;
      }
      Paint bg = bitmap_to_clipboard(app, clipboard, subbitmap(image, rect));
      if (eraseCopied.Get()){
        canvas.RunCommand(delete_rect_command(rect, bg));
        canvas.Refresh();
      }
    }
    return true;
  }

  if (tool.GetLayerType() == Layer::OBJECT){
    const objects_t& objects = image.GetObjectSelection();
    if (objects.empty()){
      return false;
    }

    Clipboard clipboard;
    if (!clipboard.Good()){
      return false;
    }
    clipboard.SetObjects(objects);

    if (eraseCopied.Get()){
      canvas.RunCommand(get_delete_objects_command(objects, image, "Cut"));
      canvas.Refresh();
    }
    return true;
  }
  return false;
}

void cut_selection(AppContext& app){
  selection_to_clipboard(app, erase_copied(true));
}

void copy_selection(AppContext& app){
  selection_to_clipboard(app, erase_copied(false));
}

void paste_to_active_canvas(AppContext& app){
  Clipboard clipboard;
  if (!clipboard.Good()){
    return;
  }
  Canvas& canvas = app.GetActiveCanvas();
  ToolInterface& tool = canvas.GetTool();

  if (tool.AcceptsPastedText()){
    if (auto text = clipboard.GetText()){
      tool.Paste(text.Take());
    }
    else{
      // Stop further pasting when text paste failed. It would be
      // surprising if a bitmap was pasted while editing text.
      return;
    }
  }

  const bool rasterPaste = tool.GetLayerType() == Layer::RASTER;
  const bool objectPaste = !rasterPaste;
  if (objectPaste){
    if (auto maybeObjects = clipboard.GetObjects()){
      objects_t objects(maybeObjects.Take());

      // Find the correct offset to place the pasted objects
      // with relative positions intact and anchored at the view-start
      Point minObj = bounding_rect(objects).TopLeft();

      const Point viewStart = canvas.GetImageViewStart();
      offset_by(objects, viewStart - minObj);
      canvas.RunCommand(get_add_objects_command(objects, select_added(false),
        "Paste"));
      app.SelectTool(ToolId::SELECTION);
      canvas.SelectObjects(objects, deselect_old(true));
      return;
    }
  }

  if (auto maybeBmp = clipboard.GetBitmap()){
    if (rasterPaste){
      canvas.RunCommand(
        get_insert_raster_bitmap_command(maybeBmp.Take(),
          floored(canvas.GetImageViewStart()),
          canvas.GetRasterSelection(),
          app.GetToolSettings(),
          "Paste Bitmap"));
      app.SelectTool(ToolId::SELECTION);
    }
    else{
      Bitmap bitmap(maybeBmp.Take());
      Settings s(default_raster_settings());
      s.Update(app.GetToolSettings());
      auto tri = tri_for_bmp(canvas.GetImageViewStart(), bitmap);
      Object* rasterObj = new ObjRaster(tri, bitmap, s);
      canvas.RunCommand(add_object_command(rasterObj, select_added(false),
        "Paste"));
      canvas.SelectObject(rasterObj, deselect_old(true));
      app.SelectTool(ToolId::SELECTION);
    }
    return;
  }

  // No suitable format on clipboard
  return;
}

void paste_to_new_image(AppContext& app){
  Clipboard clipboard;
  if (!clipboard.Good()){
    return;
  }

  if (auto bmp = clipboard.GetBitmap()){
    app.NewDocument(ImageProps(bmp.Take()));
    return;
  }

  if (auto maybeObjects = clipboard.GetObjects()){
    objects_t objects(maybeObjects.Take());
    auto objRect = bounding_rect(objects);
    offset_by(objects, -objRect.TopLeft());
    app.NewDocument(ImageProps(rounded(objRect.GetSize()), objects));
    return;
  }
}

Paint bitmap_to_clipboard(AppContext& app,
  Clipboard& clipboard,
  const Bitmap& bmp)
{
  // If the bg-color is a color, use it as the background for
  // blending alpha when pasting outside Faint.
  Paint bgPaint = app.GetToolSettings().Get(ts_Bg);
  Color bgCol = get_color_default(bgPaint, color_white);
  clipboard.SetBitmap(bmp, strip_alpha(bgCol));
  return bgPaint;
}

} // namespace
