#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Public C contract for DXVK's native shared resources.
 *
 * On dxvk-native, a D3D11 shared-resource HANDLE is a pointer to one of
 * the self-contained descriptors below.  The descriptor bytes plus the
 * dma-buf / opaque fd they carry are the transportable unit: any
 * consumer that ships {descriptor bytes, fd} to another process (e.g.
 * over SCM_RIGHTS) can reconstruct the resource there by pointing a
 * rebuilt descriptor at the received fd and passing its address to
 * ID3D11Device::OpenSharedResource / OpenSharedFence.  The pointer
 * itself is process-local and carries no meaning across processes.
 *
 * Ownership rules:
 *
 * - Exporter (GetSharedHandle / ID3D11Fence::CreateSharedHandle): the
 *   returned descriptor and its fd are owned by the exporting texture
 *   or fence and stay valid until that object is destroyed.  Callers
 *   must not free the descriptor or close the fd; dup() the fd to
 *   extend its lifetime beyond the exporter's.
 *
 * - Importer (OpenSharedResource / OpenSharedFence): DXVK reads the
 *   caller's descriptor synchronously during the call and dup()s the
 *   fd; the caller retains ownership of both and may release them as
 *   soon as the call returns.
 *
 * Descriptors are validated on import: a HANDLE whose magic, version
 * or structSize does not match fails with E_INVALIDARG instead of
 * being dereferenced further.
 */

#define DXVK_SHARED_DESCRIPTOR_TEXTURE 0x74584444u /* 'DDXt' */
#define DXVK_SHARED_DESCRIPTOR_FENCE   0x66584444u /* 'DDXf' */
#define DXVK_SHARED_DESCRIPTOR_VERSION 1u

#define DXVK_SHARED_MAX_PLANES 4u

/*
 * Native-only vendor MiscFlags bit for ID3D11Device::CreateTexture2D.
 * Combined with D3D11_RESOURCE_MISC_SHARED it forces the dmabuf export
 * to DRM_FORMAT_MOD_LINEAR (no modifier negotiation), so the buffer
 * can be consumed by scanout paths without modifier plumbing.  The bit
 * is stripped from the texture's public description.
 */
#define DXVK_D3D11_RESOURCE_MISC_LINEAR_EXPORT 0x40000000u

/*
 * D3D11-level description of a shared texture.  Fields mirror
 * D3D11_TEXTURE2D_DESC plus the texture layout; fixed-width types keep
 * the struct usable from C consumers that do not include D3D headers.
 * Layout-compatible with the D3D11 types (each field is a 32-bit
 * enum/UINT).
 */
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

/*
 * Shared-texture descriptor: D3D11-level metadata plus the dmabuf-level
 * facts an importer needs to reconstruct the image.
 */
struct DxvkSharedTextureDescriptor {
  uint32_t magic;               /* DXVK_SHARED_DESCRIPTOR_TEXTURE */
  uint32_t version;             /* DXVK_SHARED_DESCRIPTOR_VERSION */
  uint32_t structSize;          /* sizeof(struct DxvkSharedTextureDescriptor) */
  struct DxvkSharedTextureMetadata meta;
  /* dmabuf-level description */
  uint64_t drmFormatModifier;
  uint32_t planeCount;
  struct {
    uint64_t offset;
    uint64_t pitch;
  } planes[DXVK_SHARED_MAX_PLANES];
  uint64_t allocationSize;      /* exporter's memory size; import validation */
  int      fd;                  /* dma-buf; see ownership rules above */
};

/*
 * Shared-fence descriptor.  The fd is an opaque fd exported from a
 * timeline semaphore; importing binds the consumer's semaphore to the
 * same payload, so values signaled on one device are observed on all.
 * Opaque fds require the same Vulkan driver on both ends.
 */
struct DxvkSharedFenceDescriptor {
  uint32_t magic;               /* DXVK_SHARED_DESCRIPTOR_FENCE */
  uint32_t version;             /* DXVK_SHARED_DESCRIPTOR_VERSION */
  uint32_t structSize;          /* sizeof(struct DxvkSharedFenceDescriptor) */
  int      fd;                  /* opaque fd, timeline semaphore */
};

#ifdef __cplusplus
} /* extern "C" */
#endif
