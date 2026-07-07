#if defined(DXVK_WSI_HEADLESS)

#include "wsi_platform_headless.h"

namespace dxvk::wsi {

  void HeadlessWsiDriver::getWindowSize(HWND, uint32_t* pWidth, uint32_t* pHeight) {
    // Headless: there is no window.  The guest owns the swapchain and
    // publishes the real extent out-of-band, so report zero here.
    if (pWidth)  *pWidth  = 0;
    if (pHeight) *pHeight = 0;
  }

  HMONITOR HeadlessWsiDriver::getWindowMonitor(HWND) {
    return toHmonitor(0);
  }

}

#endif
