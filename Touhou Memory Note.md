Measurements from Touhou 11.
## Initial position
* X = 0
* Y = 51200
* Cheat Engine AOB pattern: `00 00 00 00   00 C8 00 00`
## Bottom Left
* X = -23552
* Y = 55296
* Cheat Engine AOB pattern: `00 A4 FF FF   00 D8 00 00`
## Bottom Right
* X = 23552
* Y = 55296
* Cheat Engine AOB pattern: `00 5C 00 00   00 D8 00 00`
## Scale factor
* 0.0078125 (1/128)
## Range
* X: -23552 ~ 23552, real: -184.f ~ 184.f
* Y: 4096 ~ 55296, real: 32.f ~ 432.f

## Remark

From Touhou 6 to Touhou 9.5, ZUN used 640x480 as logical graphics coordinate.

But for some reasons, in Touhou 10, he used 64000x48000 instead, but scaled down by the factor 0.01 (1/100) then reupscaled to window size when rendering (I'm discussing about sprite positions, not about texture's resolutions).

From Touhou 11, he changed to 81920x61440 and factor 0.0078125 (1/128).

That means in all cases, the real logical graphics coordinate is still 640x480.

Maybe because floating-point type is inaccurate but DirectX still needs floating-point type, so he just made an interger type wrapper over it and maintained the real logical graphics coordinate till the time this note was written.
