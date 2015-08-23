import os
import subprocess
import shutil
join_path = os.path.join

# Fixme: This should be done by the regular build.py
# Fixme: Replace hard coded paths.

PY_EXT_DIR = os.getcwd()
assert(PY_EXT_DIR.endswith("ext"))

ROOT_DIR = os.path.abspath(join_path(PY_EXT_DIR, "../"))
OUT_DIR = join_path(PY_EXT_DIR, "out")
TARGET = join_path(OUT_DIR, "faint.pyd")

INCLUDE_FOLDERS = [
    "c:/python34/include", # Fixme: Use build.cfg
    ROOT_DIR
]

LIBS = [
    "c:/python34/libs/python34.lib", # Fixme: Use build.cfg
]

LIBS_WX = [
    "wxbase31u.lib",
    "wxbase31u_net.lib",
    "wxbase31u_xml.lib",
    "wxexpat.lib",
    "wxjpeg.lib",
    "wxmsw31u_adv.lib",
    "wxmsw31u_aui.lib",
    "wxmsw31u_core.lib",
    "wxmsw31u_gl.lib",
    "wxmsw31u_html.lib",
    "wxmsw31u_media.lib",
    "wxmsw31u_propgrid.lib",
    "wxmsw31u_qa.lib",
    "wxmsw31u_richtext.lib",
    "wxmsw31u_stc.lib",
    "wxmsw31u_xrc.lib",
    "wxpng.lib",
    "wxregexu.lib",
    "wxtiff.lib",
    "wxzlib.lib"]

LIBS_MISC = [
    "comctl32.lib",
    "rpcrt4.lib",
    "shell32.lib",
    "gdi32.lib",
    "kernel32.lib",
    "gdiplus.lib",
    "comdlg32.lib",
    "user32.lib",
    "Advapi32.lib",
    "Ole32.lib",
    "Oleaut32.lib",
    "Winspool.lib",

    # FIXME: Use build.cfg
    "c:/dev/faintdep/gtk/cairo-dev_1.10.2-2_win32/lib/cairo.lib",
    "c:/dev/faintdep/gtk/pango-dev_1.29.4-1_win32/lib/pango-1.0.lib",
    "c:/dev/faintdep/gtk/pango-dev_1.29.4-1_win32/lib/pangocairo-1.0.lib",
    "c:/dev/faintdep/gtk/pango-dev_1.29.4-1_win32/lib/pangoft2-1.0.lib",
    "c:/dev/faintdep/gtk/pango-dev_1.29.4-1_win32/lib/pangowin32-1.0.lib",
    "c:/dev/faintdep/gtk/glib-dev_2.28.8-1_win32/lib/gio-2.0.lib",
    "c:/dev/faintdep/gtk/glib-dev_2.28.8-1_win32/lib/glib-2.0.lib",
    "c:/dev/faintdep/gtk/glib-dev_2.28.8-1_win32/lib/gmodule-2.0.lib",
    "c:/dev/faintdep/gtk/glib-dev_2.28.8-1_win32/lib/gobject-2.0.lib",
    "c:/dev/faintdep/gtk/glib-dev_2.28.8-1_win32/lib/gthread-2.0.lib",
]

LIB_PATHS = [
    "c:/dev/faintdep/gtk/cairo-dev_1.10.2-2_win32/lib",
    "c:/dev/faintdep/gtk/pango-dev_1.29.4-1_win32/lib",
    "c:/dev/faintdep/gtk/glib-dev_2.28.8-1_win32/lib"]

OBJS = [
    "aa-line.obj",
    "accelerator-entry.obj",
    "active-canvas.obj",
    "add-object-cmd.obj",
    "add-point-cmd.obj",
    "adjust.obj",
    "alpha-dialog.obj",
    "alpha-map.obj",
    "angle.obj",
    "app-getter-util.obj",
    "apply-target.obj",
    "arc.obj",
    "arrowhead.obj",
    "art.obj",
    "auto-complete.obj",
    "auto-crop.obj",
    "bezier.obj",
    "bitmap-cmd.obj",
    "bitmap-data-object.obj",
    "bitmap-list-ctrl.obj",
    "bitmap.obj",
    "blit-bitmap-cmd.obj",
    "bmp-errors.obj",
    "bmp-types.obj",
    "bound-object.obj",
    "brightness-contrast-dialog.obj",
    "brush-tool.obj",
    "brush.obj",
    "build-info.obj",
    "cairo-context.obj",
    "calibrate-cmd.obj",
    "calibrate-draw-line.obj",
    "calibrate-enter-measure.obj",
    "calibrate-idle.obj",
    "calibrate-tool.obj",
    "canvas-change-event.obj",
    "canvas-handle.obj",
    "canvas-panel-contexts.obj",
    "canvas-panel.obj",
    "change-setting-cmd.obj",
    "channel.obj",
    "clipboard-util.obj",
    "clipboard.obj",
    "color-balance-dialog.obj",
    "color-bitmap-util.obj",
    "color-choice.obj",
    "color-counting.obj",
    "color-data-object.obj",
    "color-list.obj",
    "color-panel.obj",
    "color-span.obj",
    "color.obj",
    "command-bunch.obj",
    "command-history.obj",
    "command-line.obj",
    "command-util.obj",
    "command.obj",
    "context-commands.obj",
    "convenience.obj",
    "convert-wx.obj",
    "cpp-setting-id.obj",
    "cut-and-paste.obj",
    "default-settings.obj",
    "delete-object-cmd.obj",
    "delete-rect-cmd.obj",
    "dialog-context.obj",
    "drag-value-ctrl.obj",
    "draw-object-cmd.obj",
    "draw.obj",
    "drop-down-ctrl.obj",
    "drop-source.obj",
    "dual-slider.obj",
    "egif_lib.obj",
    "ellipse-tool.obj",
    "events.obj",
    "exception.obj",
    "faint-about-dialog.obj",
    "faint-cairo-stride.obj",
    "faint-common-cursors.obj",
    "faint-dc.obj",
    "faint-fopen.obj",
    "faint-resize-dialog-context.obj",
    "faint-slider-cursors.obj",
    "faint-tool-actions.obj",
    "faint-window-app-context.obj",
    "faint-window.obj",
    "faint.res",
    "file-auto-complete.obj",
    "file-bmp.obj",
    "file-format-util.obj",
    "file-gif.obj",
    "file-ico.obj",
    "file-image-wx.obj",
    "file-path-util.obj",
    "file-path.obj",
    "file-png.obj",
    "fill-tool.obj",
    "filter.obj",
    "flip-rotate-cmd.obj",
    "font-ctrl.obj",
    "format-bmp.obj",
    "format-cur.obj",
    "format-gif.obj",
    "format-ico.obj",
    "format-png.obj",
    "format-util.obj",
    "format-wx.obj",
    "format.obj",
    "formatting.obj",
    "frame-cmd.obj",
    "frame-ctrl.obj",
    "frame-props.obj",
    "frame.obj",
    "function-cmd.obj",
    "fwd-bind.obj",
    "fwd-wx.obj",
    "gaussian-blur-exact.obj",
    "gaussian-blur-fast.obj",
    "geo-func.obj",
    "geo-list-points.obj",
    "gifalloc.obj",
    "gif_err.obj",
    "gif_hash.obj",
    "gradient-panel.obj",
    "gradient-slider.obj",
    "gradient.obj",
    "grid-ctrl.obj",
    "grid-dialog.obj",
    "grid.obj",
    "group-objects-cmd.obj",
    "gui-util.obj",
    "help-frame.obj",
    "histogram.obj",
    "hit-test.obj",
    "hot-spot-tool.obj",
    "hsl-panel.obj",
    "html-bind-tag.obj",
    "image-info.obj",
    "image-list.obj",
    "image-props.obj",
    "image-toggle-ctrl.obj",
    "image-util.obj",
    "image.obj",
    "index.obj",
    "int-point.obj",
    "int-range.obj",
    "int-rect.obj",
    "int-size.obj",
    "interpreter-ctrl.obj",
    "interpreter-frame.obj",
    "key-codes.obj",
    "key-press.obj",
    "layout-wx.obj",
    "level-tool.obj",
    "line-editor.obj",
    "line-tool.obj",
    "line.obj",
    "linear-gradient-display.obj",
    "load-resources.obj",
    "mask.obj",
    "math-text-ctrl.obj",
    "measure.obj",
    "menu-bar.obj",
    "menu-predicate.obj",
    "mouse-capture.obj",
    "mouse.obj",
    "move-point-cmd.obj",
    "multi-tool.obj",
    "null-task.obj",
    "objcomposite.obj",
    "object-data-object.obj",
    "object-handle.obj",
    "object-selection-tool.obj",
    "object-util.obj",
    "object.obj",
    "objellipse.obj",
    "objline.obj",
    "objpath.obj",
    "objpolygon.obj",
    "objraster.obj",
    "objrectangle.obj",
    "objspline.obj",
    "objtext.obj",
    "objtri.obj",
    "old-command.obj",
    "one-instance.obj",
    "order-object-cmd.obj",
    "overlay-dc-wx.obj",
    "overlay.obj",
    "padding.obj",
    "paint-canvas.obj",
    "paint-dialog.obj",
    "paint-map.obj",
    "paint.obj",
    "palette-ctrl.obj",
    "parse-math-string.obj",
    "path-tool.obj",
    "pathpt.obj",
    "pattern-panel.obj",
    "pattern.obj",
    "pen-tool.obj",
    "picker-tool.obj",
    "pinch-whirl-dialog.obj",
    "pixel-snap.obj",
    "pixelize-dialog.obj",
    "placement.obj",
    "point.obj",
    "points.obj",
    "polygon-tool.obj",
    "pos-info.obj",
    "primitive.obj",
    "put-pixel-cmd.obj",
    "py-active-settings.obj",
    "py-app.obj",
    "py-bitmap.obj",
    "py-canvas.obj",
    "py-clipboard.obj",
    "py-dialog-functions.obj",
    "py-exception.obj",
    "py-format.obj",
    "py-frame-props.obj",
    "py-frame.obj",
    "py-function-error.obj",
    "py-functions.obj",
    "py-global-functions.obj",
    "py-grid.obj",
    "py-image-props.obj",
    "py-include.obj",
    "py-initialize-ifaint.obj",
    "py-interface.obj",
    "py-interpreter.obj",
    "py-key-press.obj",
    "py-linear-gradient.obj",
    "py-object-geo.obj",
    "py-palette.obj",
    "py-parse.obj",
    "py-pattern.obj",
    "py-radial-gradient.obj",
    "py-settings.obj",
    "py-something.obj",
    "py-tri.obj",
    "py-util.obj",
    "py-window.obj",
    "quantize.obj",
    "radial-gradient-display.obj",
    "radii.obj",
    "range.obj",
    "raster-selection-tool.obj",
    "raster-selection.obj",
    "read-libpng.obj",
    "rect.obj",
    "rectangle-tool.obj",
    "remove-point-cmd.obj",
    "render-brush.obj",
    "render-text.obj",
    "rescale-cmd.obj",
    "resize-canvas-tool.obj",
    "resize-cmd.obj",
    "resize-dialog-options.obj",
    "resize-dialog.obj",
    "rotate-dialog.obj",
    "rotate-nearest.obj",
    "rotation-adjustment.obj",
    "save-result.obj",
    "save.obj",
    "scale-bicubic.obj",
    "scale-bilinear.obj",
    "scale-nearest.obj",
    "scale.obj",
    "scoped-error-log.obj",
    "select-object-idle.obj",
    "select-object-move-point.obj",
    "select-object-move.obj",
    "select-object-rectangle.obj",
    "select-object-resize.obj",
    "select-object-rotate.obj",
    "select-object-set-name.obj",
    "select-raster-base.obj",
    "select-raster-idle.obj",
    "select-raster-move.obj",
    "select-raster-rectangle.obj",
    "selected-color-ctrl.obj",
    "selection-tool.obj",
    "serialize-bmp-pixel-data.obj",
    "set-bitmap-cmd.obj",
    "set-object-name-cmd.obj",
    "set-raster-selection-cmd.obj",
    "setting-events.obj",
    "setting-util.obj",
    "settings.obj",
    "sharpness-dialog.obj",
    "size.obj",
    "slice-generic.obj",
    "slice-wx.obj",
    "slice.obj",
    "slider-alpha-background.obj",
    "slider-common.obj",
    "slider-histogram-background.obj",
    "slider.obj",
    "spin-button.obj",
    "spin-ctrl.obj",
    "spline-tool.obj",
    "split-string.obj",
    "standard-task.obj",
    "standard-tool.obj",
    "static-bitmap.obj",
    "status-button.obj",
    "stream.obj",
    "string-util.obj",
    "system-colors.obj",
    "tab-ctrl.obj",
    "tablet-error-message.obj",
    "tablet-event.obj",
    "tablet-init.obj",
    "tape-measure-tool.obj",
    "task.obj",
    "text-buffer.obj",
    "text-edit.obj",
    "text-entry-cmd.obj",
    "text-entry-util.obj",
    "text-expression-cmds.obj",
    "text-expression-conversions.obj",
    "text-expression.obj",
    "text-geo.obj",
    "text-idle.obj",
    "text-info-dc.obj",
    "text-line.obj",
    "text-make-box.obj",
    "text-select.obj",
    "text-tool.obj",
    "threshold-dialog.obj",
    "tool-bar.obj",
    "tool-drop-down-button.obj",
    "tool-id.obj",
    "tool-panel.obj",
    "tool-setting-ctrl.obj",
    "tool-setting-panel.obj",
    "tool-util.obj",
    "tool-wrapper.obj",
    "tool.obj",
    "transparency-style.obj",
    "tri-cmd.obj",
    "tri.obj",
    "utf8-char.obj",
    "utf8-string-iterator.obj",
    "utf8-string.obj",
    "utf8.obj",
    "util-wx.obj",
    "write-exception-log.obj",
    "write-giflib.obj",
    "write-libpng.obj",
    "zoom-ctrl.obj",
    "zoom-level.obj",
]

SOURCES = [
    "faint-py-ext.cpp",
]

def prefix_all_paths(folder, file_list):
    return [os.path.join(folder, f) for f in file_list]

def include_folders(includes):
    return ["/I%s" % include for include in includes]

def objs_fq(objs):
    return ["c:/dev/faint-graphics-editor/build/objs-release/%s" % o for o in objs]

def libs_wx(libs):
    return ["C:/dev/faintdep/wxWidgets2/lib/vc_lib/%s" % l for l in libs]

CC = "cl"

cl_common_switches = [
    "EHsc", # Exception mode sc
    "GF", # String pooling
    "GT", # Fiber-safe thread local storage
    "Gm-", # Disable minimal rebuild
    "Gy", # Disable function level linking (why?)
    "Oi", # Intrinsic functions (faster but larger)
    "W4", # Warning level
    "WX-", # Treat warnings as errors
    "Y-", # Ignore precompiled header options
    "fp:precise", # Floating point model
    "nologo", # No startup banner
    #"Gd", # cdecl calling convention
    "analyze-",
    "errorReport:queue",
    "Zc:forScope", # ISO-Conformance
    "Zc:wchar_t", # ISO-Conformance

    # /Za disables Microsoft extensions. Not used here because it causes many
    # warnings in wxWidgets headers and Microsoft's last developer says its
    # broken:
    # http://clang-developers.42468.n3.nabble.com/MSVC-Za-considered-harmful-td4024306.html
]

cl_release_switches = [
    "MD",
    #"GL", # Whole program optimization, requires LTCG for linker
    "GS-", # Disable Buffer overrun check
    "Ot", # Favor fast code
    "Ox", # Optimize, Favor speed
    #"P", # Keep preprocessed file
    #"Oy-", # Do not suppress frame pointer (old 386 optimization?)
]

cl_dll_switches = [
    "LD", # Fixme: Document
]

FLAGS = cl_common_switches + cl_release_switches + cl_dll_switches

def format_flags(flags):
    return ["/%s" % f for f in flags]

def format_lib_paths(paths):
    return ["/LIBPATH:%s" % p for p in paths]

cmd_l = ([CC,] +
         format_flags(FLAGS) +
         prefix_all_paths(PY_EXT_DIR, SOURCES) +
         include_folders(INCLUDE_FOLDERS) +
         LIBS +
         libs_wx(LIBS_WX)+
         LIBS_MISC +
         objs_fq(OBJS))
cmd = " ".join(cmd_l)

if not os.path.exists(OUT_DIR):
    os.mkdir(OUT_DIR)

if os.path.exists(TARGET):
    os.remove(TARGET)

try:
    os.chdir("out")
    result = subprocess.call(cmd)
    if result == 0:
        shutil.move(join_path(OUT_DIR, "faint-py-ext.dll"), TARGET)
        shutil.copy(TARGET, "c:/python34/DLLs/")
    exit(result)

finally:
    os.chdir(PY_EXT_DIR)
