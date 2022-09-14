module;

#include "framework.h"
#include "Include/dinput.h"

export module core.directinputhook;

import core.apihijack;
import common.var;
import core.inputdeterminte;

typedef HRESULT(WINAPI * DirectInput8Create_t)(
    HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
typedef HRESULT(WINAPI* DirectInputCreateW_t)(
    HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter);

HRESULT WINAPI MyDirectInput8Create(
    HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
HRESULT WINAPI MyDirectInputCreateW(
    HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter);

void CreateDevice8(IDirectInputDevice8 **device);
void CreateDeviceW(IDirectInputDeviceW **device);

export SDLLHook DInput8Hook = {
    .Name = "DINPUT8.dll",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "DirectInput8Create", .HookFn = (DWORD*)MyDirectInput8Create},
        {},
    }
};

export SDLLHook DInputHook = {
    .Name = "DINPUT.dll",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "DirectInputCreate", .HookFn = (DWORD*)MyDirectInputCreateW},
        {},
    }
};

class MyDirectInputW: public IDirectInputW {
public:
    MyDirectInputW(IDirectInputW* dInput): m_dInput(dInput) {}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) {
        HRESULT t = m_dInput->QueryInterface(riid, ppvObj);
        return t;
    }
    ULONG STDMETHODCALLTYPE AddRef() {
        return m_dInput->AddRef();
    }
    ULONG STDMETHODCALLTYPE Release() {
        ULONG count = m_dInput->Release();
        if (0 == count)
            delete this;
        return count;
    }
    HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICEW* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) {
        auto hr = m_dInput->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
        if (rguid == GUID_SysKeyboard && SUCCEEDED(hr))
            CreateDeviceW(lplpDirectInputDevice);
        return hr;
    }
    HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) {
        return m_dInput->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance) {
        return m_dInput->GetDeviceStatus(rguidInstance);
    }
    HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        return m_dInput->RunControlPanel(hwndOwner, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) {
        return m_dInput->Initialize(hinst, dwVersion);
    }
private:
    IDirectInputW * m_dInput;
};

class MyDirectInput8: public IDirectInput8 {
public:
    MyDirectInput8(IDirectInput8 * dInput8): m_dInput8(dInput8) {}

    /*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) {
        return m_dInput8->QueryInterface(riid, ppvObj);
    }
    ULONG STDMETHODCALLTYPE AddRef() {
        return m_dInput8->AddRef();
    }
    ULONG STDMETHODCALLTYPE Release() {
        ULONG count = m_dInput8->Release();
        if (0 == count)
            delete this;

        return count;
    }

    /*** IDirectInput8A methods ***/
    HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8A * lplpDirectInputDevice, LPUNKNOWN pUnkOuter) {
        auto hr = m_dInput8->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
        if (rguid == GUID_SysKeyboard && SUCCEEDED(hr))
            CreateDevice8(lplpDirectInputDevice);
        return hr;
    }
    HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) {
        return m_dInput8->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance) {
        return m_dInput8->GetDeviceStatus(rguidInstance);
    }
    HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        return m_dInput8->RunControlPanel(hwndOwner, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) {
        return m_dInput8->Initialize(hinst, dwVersion);
    }
    HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance) {
        return m_dInput8->FindDevice(rguidClass, ptszName, pguidInstance);
    }
    HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags) {
        return m_dInput8->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) {
        return m_dInput8->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
    }

private:
    IDirectInput8 * m_dInput8;
};

class MyDirectInputDevice8: public IDirectInputDevice8 {
public:
#pragma region Scaffolding
    MyDirectInputDevice8(IDirectInputDevice8 * device):m_device(device) {}
    /*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) {
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
    /*** IDirectInputDevice8A methods ***/
    HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
        return m_device->GetCapabilities(lpDIDevCaps);
    }
    HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) {
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
#pragma endregion
    HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
        auto hr = m_device->GetDeviceState(cbData, lpvData);
        if (SUCCEEDED(hr) && cbData == sizeof(BYTE) * 256) {
            g_handledByDirectInput = true;
            auto keys = static_cast<BYTE*>(lpvData);
            auto gameInput = DetermineGameInput();
            if (gameInput & USE_BOMB)
                keys[DIK_X] |= 0x80;
            if (gameInput & USE_SPECIAL)
                keys[DIK_C] |= 0x80;
            if (gameInput & MOVE_LEFT)
                keys[DIK_LEFT] |= 0x80;
            if (gameInput & MOVE_RIGHT)
                keys[DIK_RIGHT] |= 0x80;
            if (gameInput & MOVE_UP)
                keys[DIK_UP] |= 0x80;
            if (gameInput & MOVE_DOWN)
                keys[DIK_DOWN] |= 0x80;
        }
        return hr;
    }
#pragma region Scaffolding
    HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
        return m_device->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) {
        c_dfDIKeyboard;
        return m_device->SetDataFormat(lpdf);
    }
    HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) {
        return m_device->SetEventNotification(hEvent);
    }
    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags) {
        return m_device->SetCooperativeLevel(hwnd, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow) {
        return m_device->GetObjectInfo(pdidoi, dwObj, dwHow);
    }
    HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCE pdidi) {
        return m_device->GetDeviceInfo(pdidi);
    }
    HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        return m_device->RunControlPanel(hwndOwner, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) {
        return m_device->Initialize(hinst, dwVersion, rguid);
    }
    HRESULT STDMETHODCALLTYPE CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT * ppdeff, LPUNKNOWN punkOuter) {
        return m_device->CreateEffect(rguid, lpeff, ppdeff, punkOuter);
    }
    HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType) {
        return m_device->EnumEffects(lpCallback, pvRef, dwEffType);
    }
    HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFOA pdei, REFGUID rguid) {
        return m_device->GetEffectInfo(pdei, rguid);
    }
    HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut) {
        return m_device->GetForceFeedbackState(pdwOut);
    }
    HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags) {
        return m_device->SendForceFeedbackCommand(dwFlags);
    }
    HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl) {
        return m_device->EnumCreatedEffectObjects(lpCallback, pvRef, fl);
    }
    HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc) {
        return m_device->Escape(pesc);
    }
    HRESULT STDMETHODCALLTYPE Poll() {
        return m_device->Poll();
    }
    HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) {
        return m_device->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
    }
    HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) {
        return m_device->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) {
        return m_device->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCTSTR lpszUserName, DWORD dwFlags) {
        return m_device->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMATA lpdiActionFormat, LPCTSTR lptszUserName, DWORD dwFlags) {
        return m_device->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
    }
    HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader) {
        return m_device->GetImageInfo(lpdiDevImageInfoHeader);
    }
private:
    IDirectInputDevice8 * m_device;
#pragma endregion
};

class MyDirectInputDeviceW: public IDirectInputDeviceW {
public:
#pragma region Scaffolding
    MyDirectInputDeviceW(IDirectInputDeviceW* device):m_device(device) {}
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
#pragma endregion
    HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
        auto hr = m_device->GetDeviceState(cbData, lpvData);
        if (SUCCEEDED(hr) && cbData == sizeof(BYTE) * 256) {
            g_handledByDirectInput = true;
            BYTE* keys = static_cast<BYTE*>(lpvData);
            auto gameInput = DetermineGameInput();
            if (gameInput & USE_BOMB)
                keys[DIK_X] |= 0x80;
            if (gameInput & USE_SPECIAL)
                keys[DIK_C] |= 0x80;
            if (gameInput & MOVE_LEFT)
                keys[DIK_LEFT] |= 0x80;
            if (gameInput & MOVE_RIGHT)
                keys[DIK_RIGHT] |= 0x80;
            if (gameInput & MOVE_UP)
                keys[DIK_UP] |= 0x80;
            if (gameInput & MOVE_DOWN)
                keys[DIK_DOWN] |= 0x80;
        }
        return hr;
    }
#pragma region Scaffolding
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
#pragma endregion
};


HRESULT WINAPI MyDirectInputCreateW(
    HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter
) {
    auto old_func = (DirectInputCreateW_t)DInputHook.Functions[0].OrigFn;
    auto hr = old_func(hinst, dwVersion, lplpDirectInput, punkOuter);
    *lplpDirectInput = new MyDirectInputW((IDirectInputW*)*lplpDirectInput);
    return hr;
}

HRESULT WINAPI MyDirectInput8Create(
    HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter
) {
    auto old_func = (DirectInput8Create_t)DInput8Hook.Functions[0].OrigFn;
    auto hr = old_func(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    *ppvOut = new MyDirectInput8((IDirectInput8*)*ppvOut);
    return hr;
}

void CreateDevice8(IDirectInputDevice8 **device) {
    *device = new MyDirectInputDevice8(*device);
}
void CreateDeviceW(IDirectInputDeviceW **device) {
    *device = new MyDirectInputDeviceW(*device);
}