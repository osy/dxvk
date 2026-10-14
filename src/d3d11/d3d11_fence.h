#pragma once

#include "../dxvk/dxvk_fence.h"
#include "../dxvk/dxvk_gpu_query.h"

#include "../util/util_shared_res.h"

#ifndef _WIN32
#include <directx-native/dxn_shared.h>
#endif

#include "d3d11_device_child.h"

namespace dxvk {
  
  class D3D11Fence : public D3D11DeviceChild<ID3D11Fence> {

  public:
    
    D3D11Fence(
            D3D11Device*        pDevice,
            UINT64              InitialValue,
            D3D11_FENCE_FLAG    Flags,
            HANDLE              hFence);

    ~D3D11Fence();
    
    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID              riid,
            void**              ppvObject);

    HRESULT STDMETHODCALLTYPE CreateSharedHandle(
      const SECURITY_ATTRIBUTES* pAttributes,
            DWORD               dwAccess,
            LPCWSTR             lpName,
            HANDLE*             pHandle);

    HRESULT STDMETHODCALLTYPE SetEventOnCompletion(
            UINT64              Value,
            HANDLE              hEvent);

    UINT64 STDMETHODCALLTYPE GetCompletedValue();

    Rc<DxvkFence> GetFence() const {
      return m_fence;
    }
    
  private:

    Rc<DxvkFence>           m_fence;
    D3D11_FENCE_FLAG        m_flags = D3D11_FENCE_FLAG_NONE;

    D3DDestructionNotifier  m_destructionNotifier;

#ifndef _WIN32
    // Native shared-fence descriptor; fd < 0 when not exported.
    // Owns its fd for the fence's lifetime.
    dxn_shared_fence m_sharedDescriptor = { 0u, 0u, 0u, -1, 0u, 0u, 0u };
#endif

  };
  
}
