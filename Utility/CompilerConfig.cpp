#pragma comment(lib, "dinput8")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")
#ifdef _WIN64
#pragma comment(lib, "DirectX9/Lib/x64/DxErr.lib")
#else
#pragma comment(lib, "DirectX8/Lib/D3dx8.lib")
#pragma comment(lib, "DirectX9/Lib/x86/DxErr.lib")
#endif
#pragma comment(lib, "dxguid.lib")
#pragma comment(linker, "/VERSION:3.0")