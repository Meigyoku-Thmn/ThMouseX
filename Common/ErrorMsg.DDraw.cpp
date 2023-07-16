#include "framework.h" 
#include <cassert>
#include <vector>

#include "ErrorMsg.DDraw.h"

using namespace std;

#define ADD messages.emplace_back

namespace common::errormsg::ddraw {
    vector<ErrorMessage> messages;
    ON_INIT{
        ADD(0x80004001, "DDERR_UNSUPPORTED", nullptr, "ddraw.h");
        ADD(0x80004005, "DDERR_GENERIC", nullptr, "ddraw.h");
        ADD(0x800401f0, "DDERR_NOTINITIALIZED", nullptr, "ddraw.h");
        ADD(0x8007000e, "DDERR_OUTOFMEMORY", nullptr, "ddraw.h");
        ADD(0x80070057, "DDERR_INVALIDPARAMS", nullptr, "ddraw.h");
        ADD(0x88760005, "DDERR_ALREADYINITIALIZED", nullptr, "ddraw.h");
        ADD(0x8876000a, "DDERR_CANNOTATTACHSURFACE", nullptr, "ddraw.h");
        ADD(0x88760014, "DDERR_CANNOTDETACHSURFACE", nullptr, "ddraw.h");
        ADD(0x88760028, "DDERR_CURRENTLYNOTAVAIL", nullptr, "ddraw.h");
        ADD(0x88760037, "DDERR_EXCEPTION", nullptr, "ddraw.h");
        ADD(0x8876005a, "DDERR_HEIGHTALIGN", nullptr, "ddraw.h");
        ADD(0x8876005f, "DDERR_INCOMPATIBLEPRIMARY", nullptr, "ddraw.h");
        ADD(0x88760064, "DDERR_INVALIDCAPS", nullptr, "ddraw.h");
        ADD(0x8876006e, "DDERR_INVALIDCLIPLIST", nullptr, "ddraw.h");
        ADD(0x88760078, "DDERR_INVALIDMODE", nullptr, "ddraw.h");
        ADD(0x88760082, "DDERR_INVALIDOBJECT", nullptr, "ddraw.h");
        ADD(0x88760091, "DDERR_INVALIDPIXELFORMAT", nullptr, "ddraw.h");
        ADD(0x88760096, "DDERR_INVALIDRECT", nullptr, "ddraw.h");
        ADD(0x887600a0, "DDERR_LOCKEDSURFACES", nullptr, "ddraw.h");
        ADD(0x887600aa, "DDERR_NO3D", nullptr, "ddraw.h");
        ADD(0x887600b4, "DDERR_NOALPHAHW", nullptr, "ddraw.h");
        ADD(0x887600b5, "DDERR_NOSTEREOHARDWARE", nullptr, "ddraw.h");
        ADD(0x887600b6, "DDERR_NOSURFACELEFT", nullptr, "ddraw.h");
        ADD(0x887600cd, "DDERR_NOCLIPLIST", nullptr, "ddraw.h");
        ADD(0x887600d2, "DDERR_NOCOLORCONVHW", nullptr, "ddraw.h");
        ADD(0x887600d4, "DDERR_NOCOOPERATIVELEVELSET", nullptr, "ddraw.h");
        ADD(0x887600d7, "DDERR_NOCOLORKEY", nullptr, "ddraw.h");
        ADD(0x887600dc, "DDERR_NOCOLORKEYHW", nullptr, "ddraw.h");
        ADD(0x887600de, "DDERR_NODIRECTDRAWSUPPORT", nullptr, "ddraw.h");
        ADD(0x887600e1, "DDERR_NOEXCLUSIVEMODE", nullptr, "ddraw.h");
        ADD(0x887600e6, "DDERR_NOFLIPHW", nullptr, "ddraw.h");
        ADD(0x887600f0, "DDERR_NOGDI", nullptr, "ddraw.h");
        ADD(0x887600fa, "DDERR_NOMIRRORHW", nullptr, "ddraw.h");
        ADD(0x887600ff, "DDERR_NOTFOUND", nullptr, "ddraw.h");
        ADD(0x88760104, "DDERR_NOOVERLAYHW", nullptr, "ddraw.h");
        ADD(0x8876010e, "DDERR_OVERLAPPINGRECTS", nullptr, "ddraw.h");
        ADD(0x88760118, "DDERR_NORASTEROPHW", nullptr, "ddraw.h");
        ADD(0x88760122, "DDERR_NOROTATIONHW", nullptr, "ddraw.h");
        ADD(0x88760136, "DDERR_NOSTRETCHHW", nullptr, "ddraw.h");
        ADD(0x8876013c, "DDERR_NOT4BITCOLOR", nullptr, "ddraw.h");
        ADD(0x8876013d, "DDERR_NOT4BITCOLORINDEX", nullptr, "ddraw.h");
        ADD(0x88760140, "DDERR_NOT8BITCOLOR", nullptr, "ddraw.h");
        ADD(0x8876014a, "DDERR_NOTEXTUREHW", nullptr, "ddraw.h");
        ADD(0x8876014f, "DDERR_NOVSYNCHW", nullptr, "ddraw.h");
        ADD(0x88760154, "DDERR_NOZBUFFERHW", nullptr, "ddraw.h");
        ADD(0x8876015e, "DDERR_NOZOVERLAYHW", nullptr, "ddraw.h");
        ADD(0x88760168, "DDERR_OUTOFCAPS", nullptr, "ddraw.h");
        ADD(0x8876017c, "DDERR_OUTOFVIDEOMEMORY", nullptr, "ddraw.h");
        ADD(0x8876017e, "DDERR_OVERLAYCANTCLIP", nullptr, "ddraw.h");
        ADD(0x88760180, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE", nullptr, "ddraw.h");
        ADD(0x88760183, "DDERR_PALETTEBUSY", nullptr, "ddraw.h");
        ADD(0x88760190, "DDERR_COLORKEYNOTSET", nullptr, "ddraw.h");
        ADD(0x8876019a, "DDERR_SURFACEALREADYATTACHED", nullptr, "ddraw.h");
        ADD(0x887601a4, "DDERR_SURFACEALREADYDEPENDENT", nullptr, "ddraw.h");
        ADD(0x887601ae, "DDERR_SURFACEBUSY", nullptr, "ddraw.h");
        ADD(0x887601b3, "DDERR_CANTLOCKSURFACE", nullptr, "ddraw.h");
        ADD(0x887601b8, "DDERR_SURFACEISOBSCURED", nullptr, "ddraw.h");
        ADD(0x887601c2, "DDERR_SURFACELOST", nullptr, "ddraw.h");
        ADD(0x887601cc, "DDERR_SURFACENOTATTACHED", nullptr, "ddraw.h");
        ADD(0x887601d6, "DDERR_TOOBIGHEIGHT", nullptr, "ddraw.h");
        ADD(0x887601e0, "DDERR_TOOBIGSIZE", nullptr, "ddraw.h");
        ADD(0x887601ea, "DDERR_TOOBIGWIDTH", nullptr, "ddraw.h");
        ADD(0x887601fe, "DDERR_UNSUPPORTEDFORMAT", nullptr, "ddraw.h");
        ADD(0x88760208, "DDERR_UNSUPPORTEDMASK", nullptr, "ddraw.h");
        ADD(0x88760209, "DDERR_INVALIDSTREAM", nullptr, "ddraw.h");
        ADD(0x88760219, "DDERR_VERTICALBLANKINPROGRESS", nullptr, "ddraw.h");
        ADD(0x8876021c, "DDERR_WASSTILLDRAWING", nullptr, "ddraw.h");
        ADD(0x8876021e, "DDERR_DDSCAPSCOMPLEXREQUIRED", nullptr, "ddraw.h");
        ADD(0x88760230, "DDERR_XALIGN", nullptr, "ddraw.h");
        ADD(0x88760231, "DDERR_INVALIDDIRECTDRAWGUID", nullptr, "ddraw.h");
        ADD(0x88760232, "DDERR_DIRECTDRAWALREADYCREATED", nullptr, "ddraw.h");
        ADD(0x88760233, "DDERR_NODIRECTDRAWHW", nullptr, "ddraw.h");
        ADD(0x88760234, "DDERR_PRIMARYSURFACEALREADYEXISTS", nullptr, "ddraw.h");
        ADD(0x88760235, "DDERR_NOEMULATION", nullptr, "ddraw.h");
        ADD(0x88760236, "DDERR_REGIONTOOSMALL", nullptr, "ddraw.h");
        ADD(0x88760237, "DDERR_CLIPPERISUSINGHWND", nullptr, "ddraw.h");
        ADD(0x88760238, "DDERR_NOCLIPPERATTACHED", nullptr, "ddraw.h");
        ADD(0x88760239, "DDERR_NOHWND", nullptr, "ddraw.h");
        ADD(0x8876023a, "DDERR_HWNDSUBCLASSED", nullptr, "ddraw.h");
        ADD(0x8876023b, "DDERR_HWNDALREADYSET", nullptr, "ddraw.h");
        ADD(0x8876023c, "DDERR_NOPALETTEATTACHED", nullptr, "ddraw.h");
        ADD(0x8876023d, "DDERR_NOPALETTEHW", nullptr, "ddraw.h");
        ADD(0x8876023e, "DDERR_BLTFASTCANTCLIP", nullptr, "ddraw.h");
        ADD(0x8876023f, "DDERR_NOBLTHW", nullptr, "ddraw.h");
        ADD(0x88760240, "DDERR_NODDROPSHW", nullptr, "ddraw.h");
        ADD(0x88760241, "DDERR_OVERLAYNOTVISIBLE", nullptr, "ddraw.h");
        ADD(0x88760242, "DDERR_NOOVERLAYDEST", nullptr, "ddraw.h");
        ADD(0x88760243, "DDERR_INVALIDPOSITION", nullptr, "ddraw.h");
        ADD(0x88760244, "DDERR_NOTAOVERLAYSURFACE", nullptr, "ddraw.h");
        ADD(0x88760245, "DDERR_EXCLUSIVEMODEALREADYSET", nullptr, "ddraw.h");
        ADD(0x88760246, "DDERR_NOTFLIPPABLE", nullptr, "ddraw.h");
        ADD(0x88760247, "DDERR_CANTDUPLICATE", nullptr, "ddraw.h");
        ADD(0x88760248, "DDERR_NOTLOCKED", nullptr, "ddraw.h");
        ADD(0x88760249, "DDERR_CANTCREATEDC", nullptr, "ddraw.h");
        ADD(0x8876024a, "DDERR_NODC", nullptr, "ddraw.h");
        ADD(0x8876024b, "DDERR_WRONGMODE", nullptr, "ddraw.h");
        ADD(0x8876024c, "DDERR_IMPLICITLYCREATED", nullptr, "ddraw.h");
        ADD(0x8876024d, "DDERR_NOTPALETTIZED", nullptr, "ddraw.h");
        ADD(0x8876024e, "DDERR_UNSUPPORTEDMODE", nullptr, "ddraw.h");
        ADD(0x8876024f, "DDERR_NOMIPMAPHW", nullptr, "ddraw.h");
        ADD(0x88760250, "DDERR_INVALIDSURFACETYPE", nullptr, "ddraw.h");
        ADD(0x88760258, "DDERR_NOOPTIMIZEHW", nullptr, "ddraw.h");
        ADD(0x88760259, "DDERR_NOTLOADED", nullptr, "ddraw.h");
        ADD(0x8876025a, "DDERR_NOFOCUSWINDOW", nullptr, "ddraw.h");
        ADD(0x8876025b, "DDERR_NOTONMIPMAPSUBLEVEL", nullptr, "ddraw.h");
        ADD(0x8876026c, "DDERR_DCALREADYCREATED", nullptr, "ddraw.h");
        ADD(0x88760276, "DDERR_NONONLOCALVIDMEM", nullptr, "ddraw.h");
        ADD(0x88760280, "DDERR_CANTPAGELOCK", nullptr, "ddraw.h");
        ADD(0x88760294, "DDERR_CANTPAGEUNLOCK", nullptr, "ddraw.h");
        ADD(0x887602a8, "DDERR_NOTPAGELOCKED", nullptr, "ddraw.h");
        ADD(0x887602b2, "DDERR_MOREDATA", nullptr, "ddraw.h");
        ADD(0x887602b3, "DDERR_EXPIRED", nullptr, "ddraw.h");
        ADD(0x887602b4, "DDERR_TESTFINISHED", nullptr, "ddraw.h");
        ADD(0x887602b5, "DDERR_NEWMODE", nullptr, "ddraw.h");
        ADD(0x887602b6, "DDERR_D3DNOTINITIALIZED", nullptr, "ddraw.h");
        ADD(0x887602b7, "DDERR_VIDEONOTACTIVE", nullptr, "ddraw.h");
        ADD(0x887602b8, "DDERR_NOMONITORINFORMATION", nullptr, "ddraw.h");
        ADD(0x887602b9, "DDERR_NODRIVERSUPPORT", nullptr, "ddraw.h");
        ADD(0x887602bb, "DDERR_DEVICEDOESNTOWNSURFACE", nullptr, "ddraw.h");
    };
    ON_INIT{
        for (size_t i = 1; i < messages.size(); i++) {
            assert(messages[i - 1].code <= messages[i].code);
        }
    };
}