#pragma once

#include <cstdint>

#include "./com/com_include.h"

#include <d3d11_4.h>

#include "../../include/native/dxvk_shared_resource.h"

namespace dxvk {

    HANDLE openKmtHandle(HANDLE kmt_handle);

    bool setSharedMetadata(HANDLE handle, void *buf, uint32_t bufSize);
    bool getSharedMetadata(HANDLE handle, void *buf, uint32_t bufSize, uint32_t *metadataSize);

    using DxvkSharedTextureMetadata = ::DxvkSharedTextureMetadata;

}
