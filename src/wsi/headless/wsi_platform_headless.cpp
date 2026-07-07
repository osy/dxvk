#if defined(DXVK_WSI_HEADLESS)

#include "wsi_platform_headless.h"
#include "../../util/util_error.h"

namespace dxvk::wsi {

  static bool createHeadlessWsiDriver(WsiDriver **driver) {
    try {
      *driver = new HeadlessWsiDriver();
    } catch (const DxvkError&) {
      return false;
    }
    return true;
  }

  WsiBootstrap HeadlessWSI = {
    "Headless",
    createHeadlessWsiDriver,
  };

}

#endif
