#pragma once

#include "global.h"
#include "Line.h"

class MyDirectInputDeviceW: public IDirectInputDeviceW {
public:
    MyDirectInputDeviceW(IDirectInputDeviceW* device):m_device(device) {
    }
    // Inherited via IDirectInputDeviceW
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID * ppvObj) {
        return m_device->QueryInterface(riid, ppvObj);
    }
    ULONG STDMETHODCALLTYPE AddRef() {
        return m_device->AddRef();
    }
    ULONG STDMETHODCALLTYPE Release() {
        ULONG count = m_device->Release();
        if (0 == count)
            delete this;
        return count;
    }
    HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
        return m_device->GetCapabilities(lpDIDevCaps);
    }
    HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) {
        return m_device->EnumObjects(lpCallback, pvRef, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph) {
        return m_device->GetProperty(rguidProp, pdiph);
    }
    HRESULT STDMETHODCALLTYPE SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) {
        return m_device->SetProperty(rguidProp, pdiph);
    }
    HRESULT STDMETHODCALLTYPE Acquire() {
        return m_device->Acquire();
    }
    HRESULT STDMETHODCALLTYPE Unacquire() {
        return m_device->Unacquire();
    }
    HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
        HRESULT hr = m_device->GetDeviceState(cbData, lpvData);
        if (SUCCEEDED(hr) && cbData == sizeof(BYTE) * 256) {
            keyBoard = TRUE;
            BYTE* keys = static_cast<BYTE*>(lpvData);
            if (g_mouseDown) {
                keys[DIK_X] |= 0x80;
                g_mouseDown = 0;
            }
            if (g_midMouseDown) {
                keys[DIK_C] |= 0x80;
                g_midMouseDown = 0;
            }
            if (dxVersion != 8 ? g_working : g_working2) {
                DWORD address = g_currentGameConfig.Posistion.Chain[0];
                if (offsetIsRelative == true) {
                    if (firstOffsetDirection == -1) address = baseOfCode - address;
                    else address += baseOfCode;
                }
                for (int i = 1; i < g_currentGameConfig.Posistion.Length; i++) {
                    address = *((DWORD*)address);
                    if (address == 0) {
                        break;
                    }
                    address += g_currentGameConfig.Posistion.Chain[i];
                }
                if (address != 0) {
                    POINT playerPos, mousePos;
                    if (g_currentGameConfig.PosDataType == Int_DataType) {
                        IntPoint * pPosition = (IntPoint*)(address);
                        playerPos.x =
                            pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                        playerPos.y =
                            pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
                    } else if (g_currentGameConfig.PosDataType == Float_DataType) {
                        FloatPoint * pPosition = (FloatPoint*)(address);
                        playerPos.x =
                            pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                        playerPos.y =
                            pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
                    } else if (g_currentGameConfig.PosDataType == Short_DataType) {
                        ShortPoint * pPosition = (ShortPoint*)(address);
                        playerPos.x =
                            pPosition->X / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.X;
                        playerPos.y =
                            pPosition->Y / g_currentGameConfig.PixelRate + g_currentGameConfig.PixelOffset.Y;
                    }
                    GetCursorPos(&mousePos);
                    if ((dxVersion != 8 ? windowed : windowed2) == TRUE)
                        ScreenToClient(dxVersion != 8 ? g_hFocusWindow : g_hFocusWindow2, &mousePos);
                    auto shiftPressed = (keys[DIK_LSHIFT] & 0x80) || (keys[DIK_RSHIFT] & 0x80);
                    shiftPressed = false;
                    if (shiftPressed ? playerPos.x < mousePos.x : playerPos.x < mousePos.x - 1) {
                        keys[DIK_RIGHT] = 0x80;
                    } else if (shiftPressed ? playerPos.x > mousePos.x : playerPos.x > mousePos.x + 1) {
                        keys[DIK_LEFT] = 0x80;
                    }

                    if (shiftPressed ? playerPos.y < mousePos.y : playerPos.y < mousePos.y - 1) {
                        keys[DIK_DOWN] = 0x80;
                    } else if (shiftPressed ? playerPos.y > mousePos.y : playerPos.y > mousePos.y + 1) {
                        keys[DIK_UP] = 0x80;
                    }
                }
            }
        }
        return hr;
    }
    HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
        return m_device->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) {
        return m_device->SetDataFormat(lpdf);
    }
    HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) {
        return m_device->SetEventNotification(hEvent);
    }
    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags) {
        return m_device->SetCooperativeLevel(hwnd, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEW pdidoi, DWORD dwObj, DWORD dwHow) {
        return m_device->GetObjectInfo(pdidoi, dwObj, dwHow);
    }
    HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi) {
        return m_device->GetDeviceInfo(pdidi);
    }
    HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        return m_device->RunControlPanel(hwndOwner, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) {
        return m_device->Initialize(hinst, dwVersion, rguid);
    }
private:
    IDirectInputDeviceW* m_device;
};