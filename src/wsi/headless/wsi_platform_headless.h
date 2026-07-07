#pragma once

#include "../wsi_platform.h"

namespace dxvk::wsi {

  /* No-op WsiDriver carrier for headless operation.  DXVK constructs a
   * WsiDriver unconditionally at instance creation (wsi::init), but in
   * the Neptune host renderer there is no window and no Vulkan WSI
   * surface: the guest owns its own swapchain and rendered frames leave
   * the host via shared-resource export, not a presenter.  This class
   * only satisfies the WsiDriver vtable so instance/device creation and
   * D3D mode-enumeration succeed.  Selected via DXVK_WSI_DRIVER=Headless. */
  class HeadlessWsiDriver : public WsiDriver {
  public:
    HeadlessWsiDriver() = default;
    ~HeadlessWsiDriver() = default;

    std::vector<const char *> getInstanceExtensions() override { return {}; }

    HMONITOR getDefaultMonitor() override;
    HMONITOR enumMonitors(uint32_t index) override;
    HMONITOR enumMonitors(const LUID *adapterLUID[], uint32_t numLUIDs, uint32_t index) override;

    bool getDisplayName(HMONITOR hMonitor, WCHAR (&Name)[32]) override;
    bool getDesktopCoordinates(HMONITOR hMonitor, RECT* pRect) override;
    bool getDisplayMode(HMONITOR hMonitor, uint32_t modeNumber, WsiMode* pMode) override;
    bool getCurrentDisplayMode(HMONITOR hMonitor, WsiMode* pMode) override;
    bool getDesktopDisplayMode(HMONITOR hMonitor, WsiMode* pMode) override;
    WsiEdidData getMonitorEdid(HMONITOR hMonitor) override;

    void getWindowSize(HWND hWindow, uint32_t* pWidth, uint32_t* pHeight) override;
    void resizeWindow(HWND hWindow, DxvkWindowState* pState, uint32_t width, uint32_t height) override { }
    void saveWindowState(HWND hWindow, DxvkWindowState* pState, bool saveStyle) override { }
    void restoreWindowState(HWND hWindow, DxvkWindowState* pState, bool restoreCoordinates) override { }
    bool setWindowMode(HMONITOR hMonitor, HWND hWindow, DxvkWindowState* pState, const WsiMode& mode) override { return true; }
    bool enterFullscreenMode(HMONITOR hMonitor, HWND hWindow, DxvkWindowState* pState, [[maybe_unused]] bool modeSwitch) override { return true; }
    bool leaveFullscreenMode(HWND hWindow, DxvkWindowState* pState) override { return true; }
    bool restoreDisplayMode() override { return true; }
    HMONITOR getWindowMonitor(HWND hWindow) override;
    bool isWindow(HWND hWindow) override { return hWindow != nullptr; }
    bool isMinimized(HWND hWindow) override { return false; }
    bool isOccluded(HWND hWindow) override { return false; }
    void updateFullscreenWindow(HMONITOR hMonitor, HWND hWindow, bool forceTopmost) override { }
    VkResult createSurface(HWND hWindow, PFN_vkGetInstanceProcAddr, VkInstance, VkSurfaceKHR* pSurface) override {
      *pSurface = VK_NULL_HANDLE;
      return VK_SUCCESS;
    }
  };

  // HMONITOR = (uintptr_t)displayId + 1, so a NULL HMONITOR maps to "no display".
  inline int32_t fromHmonitor(HMONITOR hMonitor) {
    return static_cast<int32_t>(reinterpret_cast<intptr_t>(hMonitor)) - 1;
  }
  inline HMONITOR toHmonitor(int32_t displayId) {
    return reinterpret_cast<HMONITOR>(static_cast<intptr_t>(displayId + 1));
  }

}
