// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/render_handler_cpptoc.h"
#include "libcef_dll/ctocpp/browser_ctocpp.h"
#include "libcef_dll/ctocpp/drag_data_ctocpp.h"


// MEMBER FUNCTIONS - Body may be edited by hand.

int CEF_CALLBACK render_handler_get_root_screen_rect(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    cef_rect_t* rect) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return 0;
  // Verify param: rect; type: simple_byref
  DCHECK(rect);
  if (!rect)
    return 0;

  // Translate param: rect; type: simple_byref
  CefRect rectVal = rect?*rect:CefRect();

  // Execute
  bool _retval = CefRenderHandlerCppToC::Get(self)->GetRootScreenRect(
      CefBrowserCToCpp::Wrap(browser),
      rectVal);

  // Restore param: rect; type: simple_byref
  if (rect)
    *rect = rectVal;

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK render_handler_get_view_rect(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    cef_rect_t* rect) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return 0;
  // Verify param: rect; type: simple_byref
  DCHECK(rect);
  if (!rect)
    return 0;

  // Translate param: rect; type: simple_byref
  CefRect rectVal = rect?*rect:CefRect();

  // Execute
  bool _retval = CefRenderHandlerCppToC::Get(self)->GetViewRect(
      CefBrowserCToCpp::Wrap(browser),
      rectVal);

  // Restore param: rect; type: simple_byref
  if (rect)
    *rect = rectVal;

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK render_handler_get_screen_point(
    struct _cef_render_handler_t* self, cef_browser_t* browser, int viewX,
    int viewY, int* screenX, int* screenY) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return 0;
  // Verify param: screenX; type: simple_byref
  DCHECK(screenX);
  if (!screenX)
    return 0;
  // Verify param: screenY; type: simple_byref
  DCHECK(screenY);
  if (!screenY)
    return 0;

  // Translate param: screenX; type: simple_byref
  int screenXVal = screenX?*screenX:0;
  // Translate param: screenY; type: simple_byref
  int screenYVal = screenY?*screenY:0;

  // Execute
  bool _retval = CefRenderHandlerCppToC::Get(self)->GetScreenPoint(
      CefBrowserCToCpp::Wrap(browser),
      viewX,
      viewY,
      screenXVal,
      screenYVal);

  // Restore param: screenX; type: simple_byref
  if (screenX)
    *screenX = screenXVal;
  // Restore param: screenY; type: simple_byref
  if (screenY)
    *screenY = screenYVal;

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK render_handler_get_screen_info(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    struct _cef_screen_info_t* screen_info) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return 0;
  // Verify param: screen_info; type: struct_byref
  DCHECK(screen_info);
  if (!screen_info)
    return 0;

  // Translate param: screen_info; type: struct_byref
  CefScreenInfo screen_infoObj;
  if (screen_info)
    screen_infoObj.AttachTo(*screen_info);

  // Execute
  bool _retval = CefRenderHandlerCppToC::Get(self)->GetScreenInfo(
      CefBrowserCToCpp::Wrap(browser),
      screen_infoObj);

  // Restore param: screen_info; type: struct_byref
  if (screen_info)
    screen_infoObj.DetachTo(*screen_info);

  // Return type: bool
  return _retval;
}

void CEF_CALLBACK render_handler_on_popup_show(
    struct _cef_render_handler_t* self, cef_browser_t* browser, int show) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return;

  // Execute
  CefRenderHandlerCppToC::Get(self)->OnPopupShow(
      CefBrowserCToCpp::Wrap(browser),
      show?true:false);
}

void CEF_CALLBACK render_handler_on_popup_size(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    const cef_rect_t* rect) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return;
  // Verify param: rect; type: simple_byref_const
  DCHECK(rect);
  if (!rect)
    return;

  // Translate param: rect; type: simple_byref_const
  CefRect rectVal = rect?*rect:CefRect();

  // Execute
  CefRenderHandlerCppToC::Get(self)->OnPopupSize(
      CefBrowserCToCpp::Wrap(browser),
      rectVal);
}

void CEF_CALLBACK render_handler_on_paint(struct _cef_render_handler_t* self,
    cef_browser_t* browser, cef_paint_element_type_t type,
    size_t dirtyRectsCount, cef_rect_t const* dirtyRects, const void* buffer,
    int width, int height) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return;
  // Verify param: dirtyRects; type: simple_vec_byref_const
  DCHECK(dirtyRectsCount == 0 || dirtyRects);
  if (dirtyRectsCount > 0 && !dirtyRects)
    return;
  // Verify param: buffer; type: simple_byaddr
  DCHECK(buffer);
  if (!buffer)
    return;

  // Translate param: dirtyRects; type: simple_vec_byref_const
  std::vector<CefRect > dirtyRectsList;
  if (dirtyRectsCount > 0) {
    for (size_t i = 0; i < dirtyRectsCount; ++i) {
      dirtyRectsList.push_back(dirtyRects[i]);
    }
  }

  // Execute
  CefRenderHandlerCppToC::Get(self)->OnPaint(
      CefBrowserCToCpp::Wrap(browser),
      type,
      dirtyRectsList,
      buffer,
      width,
      height);
}

void CEF_CALLBACK render_handler_on_cursor_change(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    cef_cursor_handle_t cursor, cef_cursor_type_t type,
    const struct _cef_cursor_info_t* custom_cursor_info) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return;
  // Verify param: custom_cursor_info; type: struct_byref_const
  DCHECK(custom_cursor_info);
  if (!custom_cursor_info)
    return;

  // Translate param: custom_cursor_info; type: struct_byref_const
  CefCursorInfo custom_cursor_infoObj;
  if (custom_cursor_info)
    custom_cursor_infoObj.Set(*custom_cursor_info, false);

  // Execute
  CefRenderHandlerCppToC::Get(self)->OnCursorChange(
      CefBrowserCToCpp::Wrap(browser),
      cursor,
      type,
      custom_cursor_infoObj);
}

int CEF_CALLBACK render_handler_start_dragging(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    cef_drag_data_t* drag_data, cef_drag_operations_mask_t allowed_ops, int x,
    int y) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return 0;
  // Verify param: drag_data; type: refptr_diff
  DCHECK(drag_data);
  if (!drag_data)
    return 0;

  // Execute
  bool _retval = CefRenderHandlerCppToC::Get(self)->StartDragging(
      CefBrowserCToCpp::Wrap(browser),
      CefDragDataCToCpp::Wrap(drag_data),
      allowed_ops,
      x,
      y);

  // Return type: bool
  return _retval;
}

void CEF_CALLBACK render_handler_update_drag_cursor(
    struct _cef_render_handler_t* self, cef_browser_t* browser,
    cef_drag_operations_mask_t operation) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return;

  // Execute
  CefRenderHandlerCppToC::Get(self)->UpdateDragCursor(
      CefBrowserCToCpp::Wrap(browser),
      operation);
}

void CEF_CALLBACK render_handler_on_scroll_offset_changed(
    struct _cef_render_handler_t* self, cef_browser_t* browser, double x,
    double y) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: browser; type: refptr_diff
  DCHECK(browser);
  if (!browser)
    return;

  // Execute
  CefRenderHandlerCppToC::Get(self)->OnScrollOffsetChanged(
      CefBrowserCToCpp::Wrap(browser),
      x,
      y);
}


// CONSTRUCTOR - Do not edit by hand.

CefRenderHandlerCppToC::CefRenderHandlerCppToC(CefRenderHandler* cls)
    : CefCppToC<CefRenderHandlerCppToC, CefRenderHandler, cef_render_handler_t>(
        cls) {
  struct_.struct_.get_root_screen_rect = render_handler_get_root_screen_rect;
  struct_.struct_.get_view_rect = render_handler_get_view_rect;
  struct_.struct_.get_screen_point = render_handler_get_screen_point;
  struct_.struct_.get_screen_info = render_handler_get_screen_info;
  struct_.struct_.on_popup_show = render_handler_on_popup_show;
  struct_.struct_.on_popup_size = render_handler_on_popup_size;
  struct_.struct_.on_paint = render_handler_on_paint;
  struct_.struct_.on_cursor_change = render_handler_on_cursor_change;
  struct_.struct_.start_dragging = render_handler_start_dragging;
  struct_.struct_.update_drag_cursor = render_handler_update_drag_cursor;
  struct_.struct_.on_scroll_offset_changed =
      render_handler_on_scroll_offset_changed;
}

#ifndef NDEBUG
template<> base::AtomicRefCount CefCppToC<CefRenderHandlerCppToC,
    CefRenderHandler, cef_render_handler_t>::DebugObjCt = 0;
#endif

