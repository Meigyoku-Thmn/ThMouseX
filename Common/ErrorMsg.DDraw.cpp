#include <Windows.h>
#include <cassert>
#include <vector>

#include "ErrorMsg.DDraw.h"

using namespace std;

#define ADD messages.emplace_back

namespace common::errormsg::ddraw {
    vector<ErrorMessage> messages;
    ON_INIT{
        ADD(0x80004001, "DDERR_UNSUPPORTED", nil, "ddraw.h");
        ADD(0x80004005, "DDERR_GENERIC", nil, "ddraw.h");
        ADD(0x800401f0, "DDERR_NOTINITIALIZED", nil, "ddraw.h");
        ADD(0x8007000e, "DDERR_OUTOFMEMORY", nil, "ddraw.h");
        ADD(0x80070057, "DDERR_INVALIDPARAMS", nil, "ddraw.h");
        ADD(0x88760005, "DDERR_ALREADYINITIALIZED", nil, "ddraw.h");
        ADD(0x8876000a, "DDERR_CANNOTATTACHSURFACE", nil, "ddraw.h");
        ADD(0x88760014, "DDERR_CANNOTDETACHSURFACE", nil, "ddraw.h");
        ADD(0x88760028, "DDERR_CURRENTLYNOTAVAIL", nil, "ddraw.h");
        ADD(0x88760037, "DDERR_EXCEPTION", nil, "ddraw.h");
        ADD(0x8876005a, "DDERR_HEIGHTALIGN", nil, "ddraw.h");
        ADD(0x8876005f, "DDERR_INCOMPATIBLEPRIMARY", nil, "ddraw.h");
        ADD(0x88760064, "DDERR_INVALIDCAPS", nil, "ddraw.h");
        ADD(0x8876006e, "DDERR_INVALIDCLIPLIST", nil, "ddraw.h");
        ADD(0x88760078, "DDERR_INVALIDMODE", nil, "ddraw.h");
        ADD(0x88760082, "DDERR_INVALIDOBJECT", nil, "ddraw.h");
        ADD(0x88760091, "DDERR_INVALIDPIXELFORMAT", nil, "ddraw.h");
        ADD(0x88760096, "DDERR_INVALIDRECT", nil, "ddraw.h");
        ADD(0x887600a0, "DDERR_LOCKEDSURFACES", nil, "ddraw.h");
        ADD(0x887600aa, "DDERR_NO3D", nil, "ddraw.h");
        ADD(0x887600b4, "DDERR_NOALPHAHW", nil, "ddraw.h");
        ADD(0x887600b5, "DDERR_NOSTEREOHARDWARE", nil, "ddraw.h");
        ADD(0x887600b6, "DDERR_NOSURFACELEFT", nil, "ddraw.h");
        ADD(0x887600cd, "DDERR_NOCLIPLIST", nil, "ddraw.h");
        ADD(0x887600d2, "DDERR_NOCOLORCONVHW", nil, "ddraw.h");
        ADD(0x887600d4, "DDERR_NOCOOPERATIVELEVELSET", nil, "ddraw.h");
        ADD(0x887600d7, "DDERR_NOCOLORKEY", nil, "ddraw.h");
        ADD(0x887600dc, "DDERR_NOCOLORKEYHW", nil, "ddraw.h");
        ADD(0x887600de, "DDERR_NODIRECTDRAWSUPPORT", nil, "ddraw.h");
        ADD(0x887600e1, "DDERR_NOEXCLUSIVEMODE", nil, "ddraw.h");
        ADD(0x887600e6, "DDERR_NOFLIPHW", nil, "ddraw.h");
        ADD(0x887600f0, "DDERR_NOGDI", nil, "ddraw.h");
        ADD(0x887600fa, "DDERR_NOMIRRORHW", nil, "ddraw.h");
        ADD(0x887600ff, "DDERR_NOTFOUND", nil, "ddraw.h");
        ADD(0x88760104, "DDERR_NOOVERLAYHW", nil, "ddraw.h");
        ADD(0x8876010e, "DDERR_OVERLAPPINGRECTS", nil, "ddraw.h");
        ADD(0x88760118, "DDERR_NORASTEROPHW", nil, "ddraw.h");
        ADD(0x88760122, "DDERR_NOROTATIONHW", nil, "ddraw.h");
        ADD(0x88760136, "DDERR_NOSTRETCHHW", nil, "ddraw.h");
        ADD(0x8876013c, "DDERR_NOT4BITCOLOR", nil, "ddraw.h");
        ADD(0x8876013d, "DDERR_NOT4BITCOLORINDEX", nil, "ddraw.h");
        ADD(0x88760140, "DDERR_NOT8BITCOLOR", nil, "ddraw.h");
        ADD(0x8876014a, "DDERR_NOTEXTUREHW", nil, "ddraw.h");
        ADD(0x8876014f, "DDERR_NOVSYNCHW", nil, "ddraw.h");
        ADD(0x88760154, "DDERR_NOZBUFFERHW", nil, "ddraw.h");
        ADD(0x8876015e, "DDERR_NOZOVERLAYHW", nil, "ddraw.h");
        ADD(0x88760168, "DDERR_OUTOFCAPS", nil, "ddraw.h");
        ADD(0x8876017c, "DDERR_OUTOFVIDEOMEMORY", nil, "ddraw.h");
        ADD(0x8876017e, "DDERR_OVERLAYCANTCLIP", nil, "ddraw.h");
        ADD(0x88760180, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE", nil, "ddraw.h");
        ADD(0x88760183, "DDERR_PALETTEBUSY", nil, "ddraw.h");
        ADD(0x88760190, "DDERR_COLORKEYNOTSET", nil, "ddraw.h");
        ADD(0x8876019a, "DDERR_SURFACEALREADYATTACHED", nil, "ddraw.h");
        ADD(0x887601a4, "DDERR_SURFACEALREADYDEPENDENT", nil, "ddraw.h");
        ADD(0x887601ae, "DDERR_SURFACEBUSY", nil, "ddraw.h");
        ADD(0x887601b3, "DDERR_CANTLOCKSURFACE", nil, "ddraw.h");
        ADD(0x887601b8, "DDERR_SURFACEISOBSCURED", nil, "ddraw.h");
        ADD(0x887601c2, "DDERR_SURFACELOST", nil, "ddraw.h");
        ADD(0x887601cc, "DDERR_SURFACENOTATTACHED", nil, "ddraw.h");
        ADD(0x887601d6, "DDERR_TOOBIGHEIGHT", nil, "ddraw.h");
        ADD(0x887601e0, "DDERR_TOOBIGSIZE", nil, "ddraw.h");
        ADD(0x887601ea, "DDERR_TOOBIGWIDTH", nil, "ddraw.h");
        ADD(0x887601fe, "DDERR_UNSUPPORTEDFORMAT", nil, "ddraw.h");
        ADD(0x88760208, "DDERR_UNSUPPORTEDMASK", nil, "ddraw.h");
        ADD(0x88760209, "DDERR_INVALIDSTREAM", nil, "ddraw.h");
        ADD(0x88760219, "DDERR_VERTICALBLANKINPROGRESS", nil, "ddraw.h");
        ADD(0x8876021c, "DDERR_WASSTILLDRAWING", nil, "ddraw.h");
        ADD(0x8876021e, "DDERR_DDSCAPSCOMPLEXREQUIRED", nil, "ddraw.h");
        ADD(0x88760230, "DDERR_XALIGN", nil, "ddraw.h");
        ADD(0x88760231, "DDERR_INVALIDDIRECTDRAWGUID", nil, "ddraw.h");
        ADD(0x88760232, "DDERR_DIRECTDRAWALREADYCREATED", nil, "ddraw.h");
        ADD(0x88760233, "DDERR_NODIRECTDRAWHW", nil, "ddraw.h");
        ADD(0x88760234, "DDERR_PRIMARYSURFACEALREADYEXISTS", nil, "ddraw.h");
        ADD(0x88760235, "DDERR_NOEMULATION", nil, "ddraw.h");
        ADD(0x88760236, "DDERR_REGIONTOOSMALL", nil, "ddraw.h");
        ADD(0x88760237, "DDERR_CLIPPERISUSINGHWND", nil, "ddraw.h");
        ADD(0x88760238, "DDERR_NOCLIPPERATTACHED", nil, "ddraw.h");
        ADD(0x88760239, "DDERR_NOHWND", nil, "ddraw.h");
        ADD(0x8876023a, "DDERR_HWNDSUBCLASSED", nil, "ddraw.h");
        ADD(0x8876023b, "DDERR_HWNDALREADYSET", nil, "ddraw.h");
        ADD(0x8876023c, "DDERR_NOPALETTEATTACHED", nil, "ddraw.h");
        ADD(0x8876023d, "DDERR_NOPALETTEHW", nil, "ddraw.h");
        ADD(0x8876023e, "DDERR_BLTFASTCANTCLIP", nil, "ddraw.h");
        ADD(0x8876023f, "DDERR_NOBLTHW", nil, "ddraw.h");
        ADD(0x88760240, "DDERR_NODDROPSHW", nil, "ddraw.h");
        ADD(0x88760241, "DDERR_OVERLAYNOTVISIBLE", nil, "ddraw.h");
        ADD(0x88760242, "DDERR_NOOVERLAYDEST", nil, "ddraw.h");
        ADD(0x88760243, "DDERR_INVALIDPOSITION", nil, "ddraw.h");
        ADD(0x88760244, "DDERR_NOTAOVERLAYSURFACE", nil, "ddraw.h");
        ADD(0x88760245, "DDERR_EXCLUSIVEMODEALREADYSET", nil, "ddraw.h");
        ADD(0x88760246, "DDERR_NOTFLIPPABLE", nil, "ddraw.h");
        ADD(0x88760247, "DDERR_CANTDUPLICATE", nil, "ddraw.h");
        ADD(0x88760248, "DDERR_NOTLOCKED", nil, "ddraw.h");
        ADD(0x88760249, "DDERR_CANTCREATEDC", nil, "ddraw.h");
        ADD(0x8876024a, "DDERR_NODC", nil, "ddraw.h");
        ADD(0x8876024b, "DDERR_WRONGMODE", nil, "ddraw.h");
        ADD(0x8876024c, "DDERR_IMPLICITLYCREATED", nil, "ddraw.h");
        ADD(0x8876024d, "DDERR_NOTPALETTIZED", nil, "ddraw.h");
        ADD(0x8876024e, "DDERR_UNSUPPORTEDMODE", nil, "ddraw.h");
        ADD(0x8876024f, "DDERR_NOMIPMAPHW", nil, "ddraw.h");
        ADD(0x88760250, "DDERR_INVALIDSURFACETYPE", nil, "ddraw.h");
        ADD(0x88760258, "DDERR_NOOPTIMIZEHW", nil, "ddraw.h");
        ADD(0x88760259, "DDERR_NOTLOADED", nil, "ddraw.h");
        ADD(0x8876025a, "DDERR_NOFOCUSWINDOW", nil, "ddraw.h");
        ADD(0x8876025b, "DDERR_NOTONMIPMAPSUBLEVEL", nil, "ddraw.h");
        ADD(0x8876026c, "DDERR_DCALREADYCREATED", nil, "ddraw.h");
        ADD(0x88760276, "DDERR_NONONLOCALVIDMEM", nil, "ddraw.h");
        ADD(0x88760280, "DDERR_CANTPAGELOCK", nil, "ddraw.h");
        ADD(0x88760294, "DDERR_CANTPAGEUNLOCK", nil, "ddraw.h");
        ADD(0x887602a8, "DDERR_NOTPAGELOCKED", nil, "ddraw.h");
        ADD(0x887602b2, "DDERR_MOREDATA", nil, "ddraw.h");
        ADD(0x887602b3, "DDERR_EXPIRED", nil, "ddraw.h");
        ADD(0x887602b4, "DDERR_TESTFINISHED", nil, "ddraw.h");
        ADD(0x887602b5, "DDERR_NEWMODE", nil, "ddraw.h");
        ADD(0x887602b6, "DDERR_D3DNOTINITIALIZED", nil, "ddraw.h");
        ADD(0x887602b7, "DDERR_VIDEONOTACTIVE", nil, "ddraw.h");
        ADD(0x887602b8, "DDERR_NOMONITORINFORMATION", nil, "ddraw.h");
        ADD(0x887602b9, "DDERR_NODRIVERSUPPORT", nil, "ddraw.h");
        ADD(0x887602bb, "DDERR_DEVICEDOESNTOWNSURFACE", nil, "ddraw.h");
    };
    ON_INIT{
        for (size_t i = 1; i < messages.size(); i++) {
            assert(messages[i - 1].code <= messages[i].code);
        }
    };
}