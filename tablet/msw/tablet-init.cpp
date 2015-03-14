// -*- coding: us-ascii-unix -*-
#ifndef _WIN32
#error tablet-init.cpp compiled with non-VC compiler
#endif

#undef _UNICODE
#undef UNICODE
#define MBCS
#include <cassert>
#include <windows.h>
#include <windowsx.h>
#include "tablet/msw/tablet-init.hh"
#include "tablet/msw/wintab/wintab.h"

#define PACKETDATA (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_CURSOR | PK_ORIENTATION)
#define PACKETMODE 0
#include "tablet/msw/wintab/pktdef.h"

// Disable encounter at FARPROC warning
#pragma warning(disable:4191)

void unload_wintab32_dll();

typedef BOOL (API * WTENABLE) (HCTX, BOOL);
typedef BOOL (API * WTOVERLAP) (HCTX, BOOL);
typedef BOOL (API * WTPACKET) (HCTX, UINT, LPVOID);
typedef HCTX (API * WTOPENA)(HWND, LPLOGCONTEXTA, BOOL);
typedef UINT (API * WTINFOA) (UINT, UINT, LPVOID);

HINSTANCE g_winTabInstance = nullptr;
WTINFOA g_WTInfoA = nullptr;
WTOPENA g_WTOpenA = nullptr;
WTPACKET g_WTPacket = nullptr;
WTENABLE g_WTEnable = nullptr;
WTOVERLAP g_WTOverlap = nullptr;

namespace faint{namespace tablet{
class InitError{
public:
  InitError(InitResult errorCode)
    : m_errorCode(errorCode)
  {
    assert(m_errorCode != InitResult::OK);
  }

  InitResult ErrorCode() const{
    return m_errorCode;
  }

private:
  InitResult m_errorCode;
};

void load_wintab32_dll(){
  g_winTabInstance = LoadLibraryA("Wintab32.dll");

  if (!g_winTabInstance){
    auto errorCode = GetLastError();
    if (errorCode == ERROR_MOD_NOT_FOUND){
      throw InitError(InitResult::DLL_NOT_FOUND);
    }
    else{
      throw InitError(InitResult::OTHER_ERROR);
    }
  }

  g_WTOpenA = (WTOPENA)GetProcAddress(g_winTabInstance, "WTOpenA");
  if (g_WTOpenA == nullptr){
    throw InitError(InitResult::OTHER_ERROR);
  }

  g_WTInfoA = (WTINFOA)GetProcAddress(g_winTabInstance, "WTInfoA");
  if (g_WTInfoA == nullptr){
    throw InitError(InitResult::OTHER_ERROR);
  }

  g_WTPacket = (WTPACKET)GetProcAddress(g_winTabInstance, "WTPacket");
  if (g_WTPacket == nullptr){
    throw InitError(InitResult::OTHER_ERROR);
  }

  g_WTEnable = (WTENABLE)GetProcAddress(g_winTabInstance, "WTEnable");
  if (g_WTEnable == nullptr){
    throw InitError(InitResult::OTHER_ERROR);
  }

  g_WTOverlap = (WTOVERLAP)GetProcAddress(g_winTabInstance, "WTOverlap");
  if (g_WTOverlap == nullptr){
    throw InitError(InitResult::OTHER_ERROR);
  }
}

void unload_wintab32_dll(){
  if (g_winTabInstance != nullptr){
    FreeLibrary(g_winTabInstance);
    g_winTabInstance = nullptr;
  }
  g_WTOpenA = nullptr;
  g_WTInfoA = nullptr;
  g_WTPacket = nullptr;
  g_WTEnable = nullptr;
  g_WTOverlap = nullptr;
}

static LOGCONTEXT glogContext = {0};
HINSTANCE g_hInst;

HCTX g_winTabContext = nullptr;

HCTX static NEAR tablet_init(HWND hWnd){
  // Move system cursor before getting default system context
  glogContext.lcOptions |= CXO_SYSTEM;

  // Open default system context to read screen coordinates, not
  // tablet coordinates
  UINT logContextSize = g_WTInfoA(WTI_DEFSYSCTX, 0, &glogContext);
  if (logContextSize != sizeof(LOGCONTEXT)){
    throw InitError(InitResult::ERROR_LOGCONTEXT_SIZE);
  }
  assert(glogContext.lcOptions & CXO_SYSTEM); // CXO_SYSTEM still set

  wsprintf(glogContext.lcName, "Faint Tablet (%x)", g_hInst);

  // Enable WT_PACKET messaging
  glogContext.lcOptions |= CXO_MESSAGES;

  // Specify data to be sent in packets
  glogContext.lcPktData = PACKETDATA;

  // Specified packet items (sub set of PACKETDATA) will be expressed
  // as a delta values between updates (relative)
  glogContext.lcPktMode = PACKETMODE;

  // Specifies which packet fields trigger packet messages
  glogContext.lcMoveMask = PACKETDATA;

  // Specify button events to handle in this context
  glogContext.lcBtnUpMask = glogContext.lcBtnDnMask;

  // Set the entire tablet as active
  AXIS TabletX = {0};
  UINT sizeDeviceX = g_WTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
  assert(sizeDeviceX == sizeof(AXIS));
  AXIS TabletY = {0};
  UINT sizeDeviceY = g_WTInfoA(WTI_DEVICES, DVC_Y, &TabletY);
  assert(sizeDeviceY == sizeof(AXIS));

  glogContext.lcInOrgX = 0;
  glogContext.lcInOrgY = 0;
  glogContext.lcInExtX = TabletX.axMax;
  glogContext.lcInExtY = TabletY.axMax;

  // Specify output in screen coordinates
  glogContext.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
  glogContext.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
  glogContext.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN);

  // Default Wintab origin is lower left, change to upper left
  glogContext.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);

  // Open the region (initially disabled, which is required when using
  // cursor masks)
  HCTX ctx{g_WTOpenA(hWnd, &glogContext, FALSE)};
  if (ctx == nullptr){
    throw InitError(InitResult::WTOPENA_FAILED);
  }
  return ctx;
}

InitResult initialize(HINSTANCE hInst, HWND hWnd){
  g_hInst = hInst;
  try{
    load_wintab32_dll();

    g_winTabContext = tablet_init(hWnd);
    assert(g_winTabContext != nullptr);
  }
  catch (const InitError& e){
    return e.ErrorCode();
  }

  return InitResult::OK;
}

void uninitialize(){
  unload_wintab32_dll();
}

bool is_wt_packet(unsigned int cmd){
  return g_winTabContext != nullptr && cmd == WT_PACKET;
}

#pragma warning(disable:4100)
void msg_activate(WPARAM wParam, LPARAM lParam){
  if (g_winTabContext) {
    g_WTEnable(g_winTabContext, GET_WM_ACTIVATE_STATE(wParam, lParam));
    if (GET_WM_ACTIVATE_STATE(wParam, lParam)){
      g_WTOverlap(g_winTabContext, TRUE);
    }
  }
}
#pragma warning(default:4100)

WTP get_wt_packet(WPARAM wParam, LPARAM lParam){
  PACKET wtPacket;
  WTP faintPacket;
  faintPacket.x = faintPacket.y = faintPacket.pressure = faintPacket.cursor = 0;
  if (g_WTPacket((HCTX)lParam, wParam, &wtPacket)){
    faintPacket.x = wtPacket.pkX;
    faintPacket.y = wtPacket.pkY;
    faintPacket.pressure = wtPacket.pkNormalPressure;
    faintPacket.cursor = wtPacket.pkCursor;
  }
  return faintPacket;
}

void activate(bool enable){
  if (g_winTabContext) {
    g_WTEnable(g_winTabContext, enable ? TRUE : FALSE);
    g_WTOverlap(g_winTabContext, enable ? TRUE : FALSE);
  }
}

}} // namespace
