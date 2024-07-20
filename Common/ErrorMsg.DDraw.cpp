#include "framework.h" 
#include <cassert>
#include <vector>

#include "ErrorMsg.DDraw.h"

using namespace std;

#define ADD messages.emplace_back

namespace common::errormsg::ddraw {
    vector<ErrorMessage> messages;
    ON_INIT{
        ADD(0x80004001, "DDERR_UNSUPPORTED", null, "ddraw.h");
        ADD(0x80004005, "DDERR_GENERIC", null, "ddraw.h");
        ADD(0x800401f0, "DDERR_NOTINITIALIZED", null, "ddraw.h");
        ADD(0x8007000e, "DDERR_OUTOFMEMORY", null, "ddraw.h");
        ADD(0x80070057, "DDERR_INVALIDPARAMS", null, "ddraw.h");
        ADD(0x88760005, "DDERR_ALREADYINITIALIZED", null, "ddraw.h");
        ADD(0x8876000a, "DDERR_CANNOTATTACHSURFACE", null, "ddraw.h");
        ADD(0x88760014, "DDERR_CANNOTDETACHSURFACE", null, "ddraw.h");
        ADD(0x88760028, "DDERR_CURRENTLYNOTAVAIL", null, "ddraw.h");
        ADD(0x88760037, "DDERR_EXCEPTION", null, "ddraw.h");
        ADD(0x8876005a, "DDERR_HEIGHTALIGN", null, "ddraw.h");
        ADD(0x8876005f, "DDERR_INCOMPATIBLEPRIMARY", null, "ddraw.h");
        ADD(0x88760064, "DDERR_INVALIDCAPS", null, "ddraw.h");
        ADD(0x8876006e, "DDERR_INVALIDCLIPLIST", null, "ddraw.h");
        ADD(0x88760078, "DDERR_INVALIDMODE", null, "ddraw.h");
        ADD(0x88760082, "DDERR_INVALIDOBJECT", null, "ddraw.h");
        ADD(0x88760091, "DDERR_INVALIDPIXELFORMAT", null, "ddraw.h");
        ADD(0x88760096, "DDERR_INVALIDRECT", null, "ddraw.h");
        ADD(0x887600a0, "DDERR_LOCKEDSURFACES", null, "ddraw.h");
        ADD(0x887600aa, "DDERR_NO3D", null, "ddraw.h");
        ADD(0x887600b4, "DDERR_NOALPHAHW", null, "ddraw.h");
        ADD(0x887600b5, "DDERR_NOSTEREOHARDWARE", null, "ddraw.h");
        ADD(0x887600b6, "DDERR_NOSURFACELEFT", null, "ddraw.h");
        ADD(0x887600cd, "DDERR_NOCLIPLIST", null, "ddraw.h");
        ADD(0x887600d2, "DDERR_NOCOLORCONVHW", null, "ddraw.h");
        ADD(0x887600d4, "DDERR_NOCOOPERATIVELEVELSET", null, "ddraw.h");
        ADD(0x887600d7, "DDERR_NOCOLORKEY", null, "ddraw.h");
        ADD(0x887600dc, "DDERR_NOCOLORKEYHW", null, "ddraw.h");
        ADD(0x887600de, "DDERR_NODIRECTDRAWSUPPORT", null, "ddraw.h");
        ADD(0x887600e1, "DDERR_NOEXCLUSIVEMODE", null, "ddraw.h");
        ADD(0x887600e6, "DDERR_NOFLIPHW", null, "ddraw.h");
        ADD(0x887600f0, "DDERR_NOGDI", null, "ddraw.h");
        ADD(0x887600fa, "DDERR_NOMIRRORHW", null, "ddraw.h");
        ADD(0x887600ff, "DDERR_NOTFOUND", null, "ddraw.h");
        ADD(0x88760104, "DDERR_NOOVERLAYHW", null, "ddraw.h");
        ADD(0x8876010e, "DDERR_OVERLAPPINGRECTS", null, "ddraw.h");
        ADD(0x88760118, "DDERR_NORASTEROPHW", null, "ddraw.h");
        ADD(0x88760122, "DDERR_NOROTATIONHW", null, "ddraw.h");
        ADD(0x88760136, "DDERR_NOSTRETCHHW", null, "ddraw.h");
        ADD(0x8876013c, "DDERR_NOT4BITCOLOR", null, "ddraw.h");
        ADD(0x8876013d, "DDERR_NOT4BITCOLORINDEX", null, "ddraw.h");
        ADD(0x88760140, "DDERR_NOT8BITCOLOR", null, "ddraw.h");
        ADD(0x8876014a, "DDERR_NOTEXTUREHW", null, "ddraw.h");
        ADD(0x8876014f, "DDERR_NOVSYNCHW", null, "ddraw.h");
        ADD(0x88760154, "DDERR_NOZBUFFERHW", null, "ddraw.h");
        ADD(0x8876015e, "DDERR_NOZOVERLAYHW", null, "ddraw.h");
        ADD(0x88760168, "DDERR_OUTOFCAPS", null, "ddraw.h");
        ADD(0x8876017c, "DDERR_OUTOFVIDEOMEMORY", null, "ddraw.h");
        ADD(0x8876017e, "DDERR_OVERLAYCANTCLIP", null, "ddraw.h");
        ADD(0x88760180, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE", null, "ddraw.h");
        ADD(0x88760183, "DDERR_PALETTEBUSY", null, "ddraw.h");
        ADD(0x88760190, "DDERR_COLORKEYNOTSET", null, "ddraw.h");
        ADD(0x8876019a, "DDERR_SURFACEALREADYATTACHED", null, "ddraw.h");
        ADD(0x887601a4, "DDERR_SURFACEALREADYDEPENDENT", null, "ddraw.h");
        ADD(0x887601ae, "DDERR_SURFACEBUSY", null, "ddraw.h");
        ADD(0x887601b3, "DDERR_CANTLOCKSURFACE", null, "ddraw.h");
        ADD(0x887601b8, "DDERR_SURFACEISOBSCURED", null, "ddraw.h");
        ADD(0x887601c2, "DDERR_SURFACELOST", null, "ddraw.h");
        ADD(0x887601cc, "DDERR_SURFACENOTATTACHED", null, "ddraw.h");
        ADD(0x887601d6, "DDERR_TOOBIGHEIGHT", null, "ddraw.h");
        ADD(0x887601e0, "DDERR_TOOBIGSIZE", null, "ddraw.h");
        ADD(0x887601ea, "DDERR_TOOBIGWIDTH", null, "ddraw.h");
        ADD(0x887601fe, "DDERR_UNSUPPORTEDFORMAT", null, "ddraw.h");
        ADD(0x88760208, "DDERR_UNSUPPORTEDMASK", null, "ddraw.h");
        ADD(0x88760209, "DDERR_INVALIDSTREAM", null, "ddraw.h");
        ADD(0x88760219, "DDERR_VERTICALBLANKINPROGRESS", null, "ddraw.h");
        ADD(0x8876021c, "DDERR_WASSTILLDRAWING", null, "ddraw.h");
        ADD(0x8876021e, "DDERR_DDSCAPSCOMPLEXREQUIRED", null, "ddraw.h");
        ADD(0x88760230, "DDERR_XALIGN", null, "ddraw.h");
        ADD(0x88760231, "DDERR_INVALIDDIRECTDRAWGUID", null, "ddraw.h");
        ADD(0x88760232, "DDERR_DIRECTDRAWALREADYCREATED", null, "ddraw.h");
        ADD(0x88760233, "DDERR_NODIRECTDRAWHW", null, "ddraw.h");
        ADD(0x88760234, "DDERR_PRIMARYSURFACEALREADYEXISTS", null, "ddraw.h");
        ADD(0x88760235, "DDERR_NOEMULATION", null, "ddraw.h");
        ADD(0x88760236, "DDERR_REGIONTOOSMALL", null, "ddraw.h");
        ADD(0x88760237, "DDERR_CLIPPERISUSINGHWND", null, "ddraw.h");
        ADD(0x88760238, "DDERR_NOCLIPPERATTACHED", null, "ddraw.h");
        ADD(0x88760239, "DDERR_NOHWND", null, "ddraw.h");
        ADD(0x8876023a, "DDERR_HWNDSUBCLASSED", null, "ddraw.h");
        ADD(0x8876023b, "DDERR_HWNDALREADYSET", null, "ddraw.h");
        ADD(0x8876023c, "DDERR_NOPALETTEATTACHED", null, "ddraw.h");
        ADD(0x8876023d, "DDERR_NOPALETTEHW", null, "ddraw.h");
        ADD(0x8876023e, "DDERR_BLTFASTCANTCLIP", null, "ddraw.h");
        ADD(0x8876023f, "DDERR_NOBLTHW", null, "ddraw.h");
        ADD(0x88760240, "DDERR_NODDROPSHW", null, "ddraw.h");
        ADD(0x88760241, "DDERR_OVERLAYNOTVISIBLE", null, "ddraw.h");
        ADD(0x88760242, "DDERR_NOOVERLAYDEST", null, "ddraw.h");
        ADD(0x88760243, "DDERR_INVALIDPOSITION", null, "ddraw.h");
        ADD(0x88760244, "DDERR_NOTAOVERLAYSURFACE", null, "ddraw.h");
        ADD(0x88760245, "DDERR_EXCLUSIVEMODEALREADYSET", null, "ddraw.h");
        ADD(0x88760246, "DDERR_NOTFLIPPABLE", null, "ddraw.h");
        ADD(0x88760247, "DDERR_CANTDUPLICATE", null, "ddraw.h");
        ADD(0x88760248, "DDERR_NOTLOCKED", null, "ddraw.h");
        ADD(0x88760249, "DDERR_CANTCREATEDC", null, "ddraw.h");
        ADD(0x8876024a, "DDERR_NODC", null, "ddraw.h");
        ADD(0x8876024b, "DDERR_WRONGMODE", null, "ddraw.h");
        ADD(0x8876024c, "DDERR_IMPLICITLYCREATED", null, "ddraw.h");
        ADD(0x8876024d, "DDERR_NOTPALETTIZED", null, "ddraw.h");
        ADD(0x8876024e, "DDERR_UNSUPPORTEDMODE", null, "ddraw.h");
        ADD(0x8876024f, "DDERR_NOMIPMAPHW", null, "ddraw.h");
        ADD(0x88760250, "DDERR_INVALIDSURFACETYPE", null, "ddraw.h");
        ADD(0x88760258, "DDERR_NOOPTIMIZEHW", null, "ddraw.h");
        ADD(0x88760259, "DDERR_NOTLOADED", null, "ddraw.h");
        ADD(0x8876025a, "DDERR_NOFOCUSWINDOW", null, "ddraw.h");
        ADD(0x8876025b, "DDERR_NOTONMIPMAPSUBLEVEL", null, "ddraw.h");
        ADD(0x8876026c, "DDERR_DCALREADYCREATED", null, "ddraw.h");
        ADD(0x88760276, "DDERR_NONONLOCALVIDMEM", null, "ddraw.h");
        ADD(0x88760280, "DDERR_CANTPAGELOCK", null, "ddraw.h");
        ADD(0x88760294, "DDERR_CANTPAGEUNLOCK", null, "ddraw.h");
        ADD(0x887602a8, "DDERR_NOTPAGELOCKED", null, "ddraw.h");
        ADD(0x887602b2, "DDERR_MOREDATA", null, "ddraw.h");
        ADD(0x887602b3, "DDERR_EXPIRED", null, "ddraw.h");
        ADD(0x887602b4, "DDERR_TESTFINISHED", null, "ddraw.h");
        ADD(0x887602b5, "DDERR_NEWMODE", null, "ddraw.h");
        ADD(0x887602b6, "DDERR_D3DNOTINITIALIZED", null, "ddraw.h");
        ADD(0x887602b7, "DDERR_VIDEONOTACTIVE", null, "ddraw.h");
        ADD(0x887602b8, "DDERR_NOMONITORINFORMATION", null, "ddraw.h");
        ADD(0x887602b9, "DDERR_NODRIVERSUPPORT", null, "ddraw.h");
        ADD(0x887602bb, "DDERR_DEVICEDOESNTOWNSURFACE", null, "ddraw.h");
    };
    ON_INIT{
        for (size_t i = 1; i < messages.size(); i++) {
            assert(messages[i - 1].code <= messages[i].code);
        }
    };
}