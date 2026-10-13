#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * dxvk-native: caller-owned storage for DYNAMIC D3D11 buffers.
 *
 * dxvk_d3d11_buffer_bind_external_dmabuf adds a dma-buf, tagged with a
 * caller-chosen cookie, to the slices a Map(WRITE_DISCARD) on the
 * immediate context may rename the buffer onto.  It does not change the
 * buffer's current contents.  A DISCARD takes such a slice only once the
 * GPU is done with it, and otherwise allocates as usual, so the caller
 * learns where a Map landed from dxvk_d3d11_buffer_external_cookie on
 * the pointer the Map returned.  Writes to the dma-buf from any mapping
 * are what the GPU reads after that DISCARD.
 *
 * The buffer must be DYNAMIC and CPU-writable, and the dma-buf at least
 * as large as the buffer's storage.  DXVK dup()s the fd; the caller keeps
 * its own.  Both calls take the ID3D11Buffer the device created.
 *
 * Returns an HRESULT: S_OK, E_INVALIDARG for an unsuitable buffer or
 * argument, E_NOTIMPL when the device cannot import dma-bufs, E_FAIL
 * when the import fails.
 */
int32_t dxvk_d3d11_buffer_bind_external_dmabuf(void *d3d11_buffer, int fd,
                                               uint64_t length,
                                               uint32_t cookie);

/* 0 and *out_cookie set when mapped_ptr is the start of a bound slice,
 * -1 otherwise. */
int dxvk_d3d11_buffer_external_cookie(void *d3d11_buffer,
                                      const void *mapped_ptr,
                                      uint32_t *out_cookie);

#ifdef __cplusplus
}
#endif
