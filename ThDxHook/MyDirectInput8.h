#pragma once

#include "stdafx.h"
#include "MyDirectInputDevice8.h"
#include "MyDirectInputDeviceW.h"

class MyDirectInputW: public IDirectInputW {
public:
    MyDirectInputW(IDirectInputW* dInput): m_dInput(dInput) {
    }
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
        HRESULT hr = m_dInput->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
        if (rguid == GUID_SysKeyboard) {
            if (SUCCEEDED(hr))
                *lplpDirectInputDevice = new MyDirectInputDeviceW(*lplpDirectInputDevice);
        }
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
    MyDirectInput8(IDirectInput8 * dInput8): m_dInput8(dInput8) {
    }

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
        //char dbBuff[128];
        //sprintf_s(dbBuff, sizeof(dbBuff), "MyDirectInput8: CreateDevice called. rguid = 0x%08X\n", rguid);GUID_SysKeyboard
        //WriteToLog( dbBuff );
        HRESULT hr = m_dInput8->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
        if (rguid == GUID_SysKeyboard) {
            if (SUCCEEDED(hr))
                *lplpDirectInputDevice = new MyDirectInputDevice8(*lplpDirectInputDevice);
        }
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