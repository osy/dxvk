#if defined(DXVK_WSI_HEADLESS)

#include "wsi_platform_headless.h"

#include "../../util/util_string.h"

namespace dxvk::wsi {

  // Headless: expose a single 1920x1080 @ 60 Hz pseudo-display so D3D
  // mode-enumeration paths see something.  The rendered extent is driven
  // by the guest swapchain, not these numbers.

  HMONITOR HeadlessWsiDriver::getDefaultMonitor() {
    return toHmonitor(0);
  }

  HMONITOR HeadlessWsiDriver::enumMonitors(uint32_t index) {
    return index == 0 ? toHmonitor(0) : nullptr;
  }

  HMONITOR HeadlessWsiDriver::enumMonitors(const LUID*[], uint32_t, uint32_t index) {
    return enumMonitors(index);
  }

  bool HeadlessWsiDriver::getDisplayName(HMONITOR hMonitor, WCHAR (&Name)[32]) {
    if (fromHmonitor(hMonitor) != 0)
      return false;
    std::memset(Name, 0, sizeof(Name));
    str::swprintf(Name, ARRAYSIZE(Name), "\\\\.\\DISPLAY1");
    return true;
  }

  bool HeadlessWsiDriver::getDesktopCoordinates(HMONITOR hMonitor, RECT* pRect) {
    if (fromHmonitor(hMonitor) != 0)
      return false;
    pRect->left = 0; pRect->top = 0; pRect->right = 1920; pRect->bottom = 1080;
    return true;
  }

  bool HeadlessWsiDriver::getDisplayMode(HMONITOR hMonitor, uint32_t modeNumber, WsiMode* pMode) {
    if (fromHmonitor(hMonitor) != 0 || modeNumber > 0)
      return false;
    pMode->width        = 1920;
    pMode->height       = 1080;
    pMode->refreshRate  = WsiRational{ 60000, 1000 };
    pMode->bitsPerPixel = 32;
    pMode->interlaced   = false;
    return true;
  }

  bool HeadlessWsiDriver::getCurrentDisplayMode(HMONITOR hMonitor, WsiMode* pMode) {
    return getDisplayMode(hMonitor, 0, pMode);
  }

  bool HeadlessWsiDriver::getDesktopDisplayMode(HMONITOR hMonitor, WsiMode* pMode) {
    return getDisplayMode(hMonitor, 0, pMode);
  }

  WsiEdidData HeadlessWsiDriver::getMonitorEdid(HMONITOR) {
    return {};
  }

}

#endif
