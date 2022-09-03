// DX8Hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MyDirect3D8.h"
#include "DX8Hook.h"


DWORD* WINAPI CreateDirect3D8Detour(DWORD* d3d) {
    return (DWORD*)(new MyDirect3D8((IDirect3D8*)d3d));
}