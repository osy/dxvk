#include "d3d11_fence.h"
#include "d3d11_device.h"
#include "../util/util_win32_compat.h"

#ifndef _WIN32
#include <unistd.h>
#endif

namespace dxvk {

  D3D11Fence::D3D11Fence(
          D3D11Device*        pDevice,
          UINT64              InitialValue,
          D3D11_FENCE_FLAG    Flags,
          HANDLE              hFence)
  : D3D11DeviceChild<ID3D11Fence>(pDevice),
    m_flags(Flags), m_destructionNotifier(this) {
    DxvkFenceCreateInfo fenceInfo = { };
    fenceInfo.initialValue = InitialValue;

    if (Flags & D3D11_FENCE_FLAG_SHARED) {
#ifdef _WIN32
      fenceInfo.sharedType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_FENCE_BIT;

      if (!hFence)
        hFence = INVALID_HANDLE_VALUE;

      fenceInfo.sharedHandle = hFence;
#else
      // Native: shared fences are opaque-fd timeline semaphores; the
      // HANDLE is a pointer to a DxvkSharedFenceDescriptor (see
      // include/native/dxvk_shared_resource.h).
      fenceInfo.sharedType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

      if (hFence && hFence != INVALID_HANDLE_VALUE) {
        auto descriptor = reinterpret_cast<const DxvkSharedFenceDescriptor*>(hFence);

        if (descriptor->magic != DXVK_SHARED_DESCRIPTOR_FENCE
         || descriptor->version != DXVK_SHARED_DESCRIPTOR_VERSION
         || descriptor->structSize != sizeof(*descriptor)
         || descriptor->fd < 0)
          throw DxvkError("D3D11: Cannot open shared fence: invalid descriptor");

        fenceInfo.sharedFd = descriptor->fd;
      }
#endif
    }

    if (Flags & ~D3D11_FENCE_FLAG_SHARED)
      Logger::err(str::format("Fence flags 0x", std::hex, Flags, " not supported"));

    m_fence = pDevice->GetDXVKDevice()->createFence(fenceInfo);

#ifndef _WIN32
    // Build the export descriptor eagerly so creation fails loudly
    // if a newly created shared fence cannot actually be exported;
    // opened fences keep working even if re-export is unavailable.
    if (Flags & D3D11_FENCE_FLAG_SHARED) {
      int fd = m_fence->sharedFd();

      if (fd < 0 && fenceInfo.sharedFd < 0)
        throw DxvkError("D3D11: Failed to export shared fence");

      if (fd >= 0) {
        m_sharedDescriptor.magic = DXVK_SHARED_DESCRIPTOR_FENCE;
        m_sharedDescriptor.version = DXVK_SHARED_DESCRIPTOR_VERSION;
        m_sharedDescriptor.structSize = sizeof(m_sharedDescriptor);
        m_sharedDescriptor.fd = fd;
      }
    }
#endif
  }


  D3D11Fence::~D3D11Fence() {
#ifndef _WIN32
    if (m_sharedDescriptor.fd >= 0)
      close(m_sharedDescriptor.fd);
#endif
  }


  HRESULT STDMETHODCALLTYPE D3D11Fence::QueryInterface(
          REFIID              riid,
          void**              ppvObject) {
    if (ppvObject == nullptr)
      return E_POINTER;

    *ppvObject = nullptr;
    
    if (riid == __uuidof(IUnknown)
     || riid == __uuidof(ID3D11DeviceChild)
     || riid == __uuidof(ID3D11Fence)) {
      *ppvObject = ref(this);
      return S_OK;
    }

    if (riid == __uuidof(ID3DDestructionNotifier)) {
      *ppvObject = ref(&m_destructionNotifier);
      return S_OK;
    }

    if (logQueryInterfaceError(__uuidof(ID3D11Fence), riid)) {
      Logger::warn("D3D11Fence: Unknown interface query");
      Logger::warn(str::format(riid));
    }

    return E_NOINTERFACE;
  }


  HRESULT STDMETHODCALLTYPE D3D11Fence::CreateSharedHandle(
    const SECURITY_ATTRIBUTES* pAttributes,
          DWORD               dwAccess,
          LPCWSTR             lpName,
          HANDLE*             pHandle) {
    InitReturnPtr(pHandle);
    if (!(m_flags & D3D11_FENCE_FLAG_SHARED))
      return E_INVALIDARG;

#ifndef _WIN32
    // Native: return a pointer to the fence's opaque-fd descriptor,
    // owned by the fence
    if (pAttributes || dwAccess || lpName)
      Logger::warn("CreateSharedHandle: attributes, access and name not handled on native");

    if (!pHandle || m_sharedDescriptor.fd < 0)
      return E_INVALIDARG;

    *pHandle = reinterpret_cast<HANDLE>(&m_sharedDescriptor);
    return S_OK;
#endif

    OBJECT_ATTRIBUTES attr = { };
    attr.Length = sizeof(attr);
    attr.SecurityDescriptor = const_cast<SECURITY_ATTRIBUTES*>(pAttributes);

    WCHAR buffer[MAX_PATH];
    UNICODE_STRING name_str;
    if (lpName) {
        DWORD session, len, name_len = dxvk::str::wcslen(lpName);

        ProcessIdToSessionId(GetCurrentProcessId(), &session);
        len = dxvk::str::swprintf(buffer, ARRAYSIZE(buffer), "\\Sessions\\%u\\BaseNamedObjects\\", session);
        memcpy(buffer + len, lpName, (name_len + 1) * sizeof(WCHAR));
        name_str.MaximumLength = name_str.Length = (len + name_len) * sizeof(WCHAR);
        name_str.MaximumLength += sizeof(WCHAR);
        name_str.Buffer = buffer;

        attr.ObjectName = &name_str;
        attr.Attributes = OBJ_CASE_INSENSITIVE;
    }

    D3DKMT_HANDLE local = m_fence->kmtLocal();
    if (!D3DKMTShareObjects(1, &local, &attr, dwAccess, pHandle))
      return S_OK;

    /* try legacy Proton shared resource implementation */

    if (pAttributes)
      Logger::warn(str::format("CreateSharedHandle: attributes ", pAttributes, " not handled"));
    if (dwAccess)
      Logger::warn(str::format("CreateSharedHandle: access ", dwAccess, " not handled"));
    if (lpName)
      Logger::warn(str::format("CreateSharedHandle: name ", dxvk::str::fromws(lpName), " not handled"));

    HANDLE sharedHandle = m_fence->sharedHandle();
    if (sharedHandle == INVALID_HANDLE_VALUE)
      return E_INVALIDARG;

    *pHandle = sharedHandle;
    return S_OK;
  }


  HRESULT STDMETHODCALLTYPE D3D11Fence::SetEventOnCompletion(
          UINT64              Value,
          HANDLE              hEvent) {
    if (hEvent) {
      m_fence->enqueueWait(Value, [hEvent] {
        SetEvent(hEvent);
      });
    } else {
      m_fence->wait(Value);
    }
    return S_OK;
  }


  UINT64 STDMETHODCALLTYPE D3D11Fence::GetCompletedValue() {
    // TODO in the case of rewinds, the stored value may be higher.
    // For shared fences, calling vkGetSemaphoreCounterValue here could alleviate the issue.

    return m_fence->getValue();
  }
  
}
