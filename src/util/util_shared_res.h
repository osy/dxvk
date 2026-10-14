#pragma once

#include <cstdint>

#include "./com/com_include.h"

#include <d3d11_4.h>


namespace dxvk {

    HANDLE openKmtHandle(HANDLE kmt_handle);

    bool setSharedMetadata(HANDLE handle, void *buf, uint32_t bufSize);
    bool getSharedMetadata(HANDLE handle, void *buf, uint32_t bufSize, uint32_t *metadataSize);

    /* The metadata Wine attaches to a shared resource (win32 builds).
     * Fixed-width fields, so the layout is the same as the D3D types'
     * (each a 32-bit enum or UINT). */
    struct DxvkSharedTextureMetadata {
      uint32_t Width;
      uint32_t Height;
      uint32_t MipLevels;
      uint32_t ArraySize;
      uint32_t Format;              /* DXGI_FORMAT */
      struct {
        uint32_t Count;
        uint32_t Quality;
      } SampleDesc;                 /* DXGI_SAMPLE_DESC */
      uint32_t Usage;               /* D3D11_USAGE */
      uint32_t BindFlags;
      uint32_t CPUAccessFlags;
      uint32_t MiscFlags;
      uint32_t TextureLayout;       /* D3D11_TEXTURE_LAYOUT */
    };

}
