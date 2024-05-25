#include <stdlib.h>
#include <string.h>
#include "oc.h"
#include "display.h"
#include "control.h"
#include "config.h"

#include "gfx3d.h"

extern SYSCALLS * RWDATA gsys;
extern uint16_t * RWDATA framebuffer;

extern OBJINST RWDATA objects[16];

/*
insert
draw with z-buffer @ screen update

find idx for z > zprev && z < znext
insert
draw back to front

find idx for x0 > x0prev && x0 < x0next
insert
draw while tracking x and z on stack @ screen update

next:
if x0pop < x0next
  pop x0 x1
else
  x0 = x0next
  x1 = x1next
  z = znext
loop:
if x1 > x0next
  if z > znext
    draw x0 x0next-1
    if x1 > x1next
      push x1next+1 x1
    goto next
  if x1 < x1next
    push x1+1 x1next
  skip next
  goto loop
draw x0 x1
goto next
*/

int32_t * RWDATA divmap;

void scan_draw_init() {
  int32_t *dst = (int32_t *)gsys->GetMemBlock(1) + (10 * 1024 + 512) / 4;
  divmap = dst;

  *dst++ = 65536;
  for (int i = 1; i < 384; i++) {
    *dst++ = 65536 / i;
  }
}

void buffer_clear(uint16_t *zbuf) {
  uint32_t *dst = (uint32_t *)zbuf;
  for (int i = 0; i < 5120; i++)
    *dst++ = 0x7FFF7FFF;
}

inline void scan_xLine3(int x0, int x1, int z1, int z2, uint32_t color, uint8_t *buf, uint16_t *zbuf) {
  if (x0 > SCREEN_WIDTH - 1)
    return;
  if (x1 < 0)
    return;
  if(x0 < 0) {
    int dx = x1 - x0;
    if (dx > 1)
      z1 += (z1 - z2) * x0 / dx;
    x0 = 0;
  }
  if(x1 > SCREEN_WIDTH - 1) {
    int dx = x1 - x0;
    if (dx > 1)
      z2 -= (z1 - z2) * (SCREEN_WIDTH - x1) / dx;
    x1 = SCREEN_WIDTH - 1;
  }

  int dx = x1 - x0 + 1;
  int dz = ((z2 - z1) * divmap[dx]) >> 7;
  z1 <<= 8;
  uint8_t *dst = buf + x0;
  uint16_t *zb = zbuf + (x0 >> 1);

  if (x0 & 1) {
    if (*zb > (z1 >> 8)) {
      *dst = color;
    }
    x0++;
    dst++;
    zb++;
    z1 += (dz >> 1);
  }
  while (x0 <= x1) {
    if (*zb > (z1 >> 8)) {
      *dst = color;
      if (x0 == x1)
        break;
      dst[1] = color;
      *zb = z1 >> 8;
    }
    x0 += 2;
    dst += 2;
    zb++;
    z1 += dz;
  }
}

#define STA   (8)
#define SWAP(p1, p2)  { int tx = p1##x, ty = p1##y, tz = p1##z; p1##x = p2##x; p1##y = p2##y; p1##z = p2##z; p2##x = tx; p2##y = ty; p2##z = tz; }

uint32_t RWDATA screen_ymax;

void scan_triangle3(int32_t *p0, int32_t *p1, int32_t *p2, uint32_t color) {
  int p0x = p0[0];
  int p0y = p0x >> 16;
  int p0z = p0[1];
  int p1x = p1[0];
  int p1y = p1x >> 16;
  int p1z = p1[1];
  int p2x = p2[0];
  int p2y = p2x >> 16;
  int p2z = p2[1];
  int ymax = screen_ymax;

  if (p1y < p0y)
    SWAP(p0, p1)
  if (p2y < p1y)
    SWAP(p1, p2)
  if (p1y < p0y)
    SWAP(p0, p1)
  if (p0y > ymax)
    return;
  if (p2y < 0)
    return;

  p0x = (int16_t)p0x;
  p1x = (int16_t)p1x;
  p2x = (int16_t)p2x;

  int y = p0y;
  int xac = p0x << STA;
  int xab = p0x << STA;
  int xbc = p1x << STA;
  int xaci = 0;
  int xabi = 0;
  int xbci = 0;
  int zac = p0z << STA;
  int zab = p0z << STA;
  int zbc = p1z << STA;
  int zaci = 0;
  int zabi = 0;
  int zbci = 0;
  if(p1y != p0y) {
    int dy = p1y - p0y;
    xabi = ((p1x - p0x) * divmap[dy]) >> (16 - STA);
    zabi = ((p1z - p0z) * divmap[dy]) >> (16 - STA);
  }
  if(p2y != p0y) {
    int dy = p2y - p0y;
    xaci = ((p2x - p0x) * divmap[dy]) >> (16 - STA);
    zaci = ((p2z - p0z) * divmap[dy]) >> (16 - STA);
  }
  if(p2y != p1y) {
    int dy = p2y - p1y;
    xbci = ((p2x - p1x) * divmap[dy]) >> (16 - STA);
    zbci = ((p2z - p1z) * divmap[dy]) >> (16 - STA);
  }

  uint8_t *buf = (uint8_t *)framebuffer;
  uint8_t *dst = buf + y * SCREEN_WIDTH;
  uint16_t *zbuf = (uint16_t *)(framebuffer + 10240);
  uint16_t *zdst = zbuf + y * (SCREEN_WIDTH / 2);

  int y1 = p1y;
  int y2 = p2y;
  int ye = y1;
  if (ye > ymax)
    ye = ymax;
  if (ye < 0) {
    int yc = ye - y;
    xac += xaci * yc;
    zac += zaci * yc;
    dst = buf;
    zdst = zbuf;
    y = ye;
  } else {
    if (y < 0) {
      xab -= xabi * y;
      xac -= xaci * y;
      zab -= zabi * y;
      zac -= zaci * y;
      dst = buf;
      zdst = zbuf;
      y = 0;
    }

    if (xabi < xaci) {
      for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
      {
        scan_xLine3(xab >> STA, xac >> STA, zab >> STA, zac >> STA, color, dst, zdst);
        xab += xabi;
        xac += xaci;
        zab += zabi;
        zac += zaci;
      }
    } else {
      for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
      {
        scan_xLine3(xac >> STA, xab >> STA, zac >> STA, zab >> STA, color, dst, zdst);
        xab += xabi;
        xac += xaci;
        zab += zabi;
        zac += zaci;
      }
    }
  }
  ye = y2;
  if (ye > ymax)
    ye = ymax;
  if (y < 0) {
    xbc -= xbci * y;
    xac -= xaci * y;
    zbc -= zbci * y;
    zac -= zaci * y;
    dst = buf;
    zdst = zbuf;
    y = 0;
  }

  if (xbc < xac) {
    for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
    {
      scan_xLine3(xbc >> STA, xac >> STA, zbc >> STA, zac >> STA, color, dst, zdst);
      xbc += xbci;
      xac += xaci;
      zbc += zbci;
      zac += zaci;
    }
  } else {
    for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
    {
      scan_xLine3(xac >> STA, xbc >> STA, zac >> STA, zbc >> STA, color, dst, zdst);
      xbc += xbci;
      xac += xaci;
      zbc += zbci;
      zac += zaci;
    }
  }
}

void scan_triangle4(int32_t *p0, int32_t *p1, int32_t *p2, uint32_t color, uint32_t uv1, uint32_t uv2, uint32_t uv3) {
  int p0x = p0[0];
  int p0y = p0x >> 16;
  int p0z = p0[1];
  int p1x = p1[0];
  int p1y = p1x >> 16;
  int p1z = p1[1];
  int p2x = p2[0];
  int p2y = p2x >> 16;
  int p2z = p2[1];

  if (p1y < p0y)
    SWAP(p0, p1)
  if (p2y < p1y)
    SWAP(p1, p2)
  if (p1y < p0y)
    SWAP(p0, p1)
  if (p0y > SCREEN_HEIGHT - 1)
    return;
  if (p2y < 0)
    return;

  p0x = (int16_t)p0x;
  p1x = (int16_t)p1x;
  p2x = (int16_t)p2x;

  int y = p0y;
  int xac = p0x << STA;
  int xab = p0x << STA;
  int xbc = p1x << STA;
  int xaci = 0;
  int xabi = 0;
  int xbci = 0;
  int zac = p0z << STA;
  int zab = p0z << STA;
  int zbc = p1z << STA;
  int zaci = 0;
  int zabi = 0;
  int zbci = 0;
  if(p1y != p0y) {
    int dy = p1y - p0y;
    xabi = ((p1x - p0x) * divmap[dy]) >> (16 - STA);
    zabi = ((p1z - p0z) * divmap[dy]) >> (16 - STA);
  }
  if(p2y != p0y) {
    int dy = p2y - p0y;
    xaci = ((p2x - p0x) * divmap[dy]) >> (16 - STA);
    zaci = ((p2z - p0z) * divmap[dy]) >> (16 - STA);
  }
  if(p2y != p1y) {
    int dy = p2y - p1y;
    xbci = ((p2x - p1x) * divmap[dy]) >> (16 - STA);
    zbci = ((p2z - p1z) * divmap[dy]) >> (16 - STA);
  }

  uint8_t *buf = (uint8_t *)framebuffer;
  uint8_t *dst = buf + y * SCREEN_WIDTH;
  uint16_t *zbuf = (uint16_t *)(framebuffer + 10240);
  uint16_t *zdst = zbuf + y * (SCREEN_WIDTH / 2);

  int y1 = p1y;
  int y2 = p2y;
  int ye = y1;
  if (ye >= SCREEN_HEIGHT)
    ye = SCREEN_HEIGHT - 1;
  if (ye < 0) {
    int yc = ye - y;
    xac += xaci * yc;
    zac += zaci * yc;
    dst = buf;
    zdst = zbuf;
    y = ye;
  } else {
    if (y < 0) {
      xab -= xabi * y;
      xac -= xaci * y;
      zab -= zabi * y;
      zac -= zaci * y;
      dst = buf;
      zdst = zbuf;
      y = 0;
    }

    if (xabi < xaci) {
      for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
      {
        scan_xLine3(xab >> STA, xac >> STA, zab >> STA, zac >> STA, color, dst, zdst);
        xab += xabi;
        xac += xaci;
        zab += zabi;
        zac += zaci;
      }
    } else {
      for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
      {
        scan_xLine3(xac >> STA, xab >> STA, zac >> STA, zab >> STA, color, dst, zdst);
        xab += xabi;
        xac += xaci;
        zab += zabi;
        zac += zaci;
      }
    }
  }
  ye = y2;
  if (ye >= SCREEN_HEIGHT)
    ye = SCREEN_HEIGHT - 1;
  if (y < 0) {
    xbc -= xbci * y;
    xac -= xaci * y;
    zbc -= zbci * y;
    zac -= zaci * y;
    dst = buf;
    zdst = zbuf;
    y = 0;
  }

  if (xbc < xac) {
    for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
    {
      scan_xLine3(xbc >> STA, xac >> STA, zbc >> STA, zac >> STA, color, dst, zdst);
      xbc += xbci;
      xac += xaci;
      zbc += zbci;
      zac += zaci;
    }
  } else {
    for(; y < ye; y++, dst += SCREEN_WIDTH, zdst += SCREEN_WIDTH / 2)
    {
      scan_xLine3(xac >> STA, xbc >> STA, zac >> STA, zbc >> STA, color, dst, zdst);
      xbc += xbci;
      xac += xaci;
      zbc += zbci;
      zac += zaci;
    }
  }
}

#define USE_FLASH   1

#define RAND    (seed = (((seed ^ 0xBACADA55) + 0x9123) >> 11) ^ (seed + 0x7432))

uint32_t RWDATA mx;
uint32_t RWDATA my;
int32_t RWDATA height;
int32_t RWDATA horizon;
int32_t RWDATA angle;
int32_t RWDATA roll;

int32_t RWDATA testh;

typedef struct {
  uint16_t start;
  uint16_t cstart;
  uint16_t cend;
} __attribute((__packed__)) VOXELRAY;

typedef struct {
  int8_t x;
  int8_t y;
  uint16_t h;
} __attribute((__packed__)) VOXELCOL;

typedef struct {
  int16_t off;
  uint16_t h;
} __attribute((__packed__)) VOXELCOL2;

uint32_t gen_ray(VOXELRAY *ray, int32_t sx, int32_t sy, VOXELCOL *cols, uint32_t cc) {
  int i;
  uint8_t *fb = (uint8_t *)framebuffer;
  for (i = 0; i < cc; i++) {
    if (sx >= 0 && sx <= 159 && sy >= 0 && sy <= 127) {
      ray->cend = i - 1;
      break;
    }
    sx += cols[i].x;
    sy += cols[i].y;
  }
  if (i == cc)
    return 0;
  for (;i < cc; i++) {
    if (!(sx >= 0 && sx <= 159 && sy >= 0 && sy <= 127))
      break;
    //if (i < cc / 2)
    //  *(fb + sy * 160 + sx) = 0xFF;
    sx += cols[i].x;
    sy += cols[i].y;
  }
  i--;
  sx -= cols[i].x;
  sy -= cols[i].y;
  ray->start = sy * 160 + sx;
  ray->cstart = i - 1;

  return 1;
}

typedef struct {
  uint16_t cc;
  uint16_t rc;
  int32_t rx0;
  int32_t rx1;
} VOXELRAYS;

uint32_t precalc_ray(uint32_t angle, VOXELCOL2 *cols2, VOXELRAY *rays, VOXELRAYS *vr) {
  VOXELCOL cols[300];

  int16_t sinr = sintable[angle];
  int16_t cosr = sintable[angle + 180];
  int32_t botx = (208 * sinr) >> 16;
  int32_t boty = (208 * cosr) >> 16;
  int32_t topx = (-208 * sinr) >> 16;
  int32_t topy = (-208 * cosr) >> 16;

  int32_t vdx = botx - topx;
  int32_t vdy = boty - topy;
  int32_t xinc = 1;
  int32_t yinc = 1;
  int32_t e;
  int32_t vx = botx, vy = boty;
  uint32_t cc = 0;

	if (vdx < 0) {
		xinc = -1;
		vdx   = -vdx;
	}
	if (vdy < 0) {
		yinc = -1;
		vdy   = -vdy;
	}

	if (vdx > vdy) {
		e = vdy - vdx;
		for (int i = 0; i <= vdx; i++) {
      cols[cc].y = 0;
			if (e >= 0) {
				e -= vdx;
        vy += yinc;
        cols[cc].x = 0;
        cols[cc].h = (i * 209) / vdx;
        cols[cc++].y = yinc;
        cols[cc].y = 0;
			}
			e += vdy;
      vx += xinc;
      cols[cc].h = (i * 209) / vdx;
      cols[cc++].x = xinc;
		}
	} else {
		e = vdx - vdy;
		for (int i = 0; i <= vdy; i++) {
      cols[cc].x = 0;
			if (e >= 0) {
				e -= vdy;
				vx += xinc;
        cols[cc].x = xinc;
        cols[cc].h = (i * 209) / vdy;
        cols[cc++].y = 0;
        cols[cc].x = 0;
			}
			e += vdx;
			vy += yinc;
      cols[cc].h = (i * 209) / vdy;
      cols[cc++].y = yinc;
		}
	}

  int32_t leftx = (-120 * cosr - 104 * sinr) >> 0;
  int32_t lefty = (-104 * cosr + 120 * sinr) >> 0;
  int32_t rightx = (120 * cosr - 104 * sinr) >> 0;
  int32_t righty = (-104 * cosr - 120 * sinr) >> 0;

  int32_t dx = rightx - leftx;
  int32_t dy = righty - lefty;
  int32_t hx = leftx, hy = lefty;
  int32_t rc = 0;
  int32_t ardx = dx >= 0 ? dx : -dx;
  int32_t ardy = dy >= 0 ? dy : -dy;
  int32_t dist = ardy >> 15;

  if ((ardx >> 15) == (ardy >> 15)) {
    if (dx > 0)
      dx = 1 << 15;
    else
      dx = -(1 << 15);
    if (dy > 0)
      dy = 1 << 15;
    else
      dy = -(1 << 15);
  } else if (ardx > ardy) {
    dist = ardx >> 15;
    dy = dy / (ardx >> 15);
    if (dx > 0)
      dx = 1 << 15;
    else
      dx = -(1 << 15);
  } else {
    dx = dx / (ardy >> 15);
    if (dy > 0)
      dy = 1 << 15;
    else
      dy = -(1 << 15);
  }

  int32_t rx0 = 0, rx1 = 0;
  for (int i = 0; i <= dist; i++, hx += dx, hy += dy) {
    uint32_t dc = gen_ray(&rays[rc], (hx >> 15) + 80, (hy >> 15) + 64, cols, cc);
    if (rx0 == 0 && dc != 0)
      rx0 = (((i * 2 - dist) * 241) << 5) / dist;
    if (dc != 0)
      rx1 = (((i * 2 - dist) * 241) << 5) / dist;
    rc += dc;
  }

  vr->cc = cc;
  vr->rc = rc;
  vr->rx0 = rx0;
  vr->rx1 = rx1;

  for (int i = 0; i < cc; i++) {
    cols2[i].h = cols[i].h;
    cols2[i].off = cols[i].y * 160 + cols[i].x;
  }

  return 300 * 4 + 200 * 6;

  /*int32_t leftx = (-120 * cosr - 104 * sinr) >> 10;
  int32_t lefty = (-104 * cosr + 120 * sinr) >> 10;
  int32_t rightx = (120 * cosr - 104 * sinr) >> 10;
  int32_t righty = (-104 * cosr - 120 * sinr) >> 10;

  int32_t dx = rightx - leftx;
  int32_t dy = righty - lefty;
  int32_t hx = leftx >> 5, hy = lefty >> 5;
  int32_t rc = 0;
  int32_t rx0 = 0, rx1 = 0;
  xinc = yinc = 1;

	if (dx < 0) {
		xinc = -1;
		dx   = -dx;
	}
	if (dy < 0) {
		yinc = -1;
		dy   = -dy;
	}

	if (dx > dy) {
		e = dy - dx;
		for (int i = 0; i <= (dx >> 5); i++) {
      uint32_t dc = gen_ray(&rays[rc], hx + 80, hy + 64, cols, cc);
      int32_t dx2 = dx >> 5;
      if (rx0 == 0 && dc != 0)
        rx0 = (((i * 2 - dx2) * 241) << 5) / dx2;
      if (dc != 0)
        rx1 = (((i * 2 - dx2) * 241) << 5) / dx2;
      rc += dc;
			if (e >= 0) {
				e -= dx;
        hy += yinc;
			}
			e += dy;
      hx += xinc;
		}
	} else {
		e = dx - dy;
		for (int i = 0; i <= (dy >> 5); i++) {
      uint32_t dc = gen_ray(&rays[rc], hx + 80, hy + 64, cols, cc);
      int32_t dy2 = dy >> 5;
      if (rx0 == 0 && dc != 0)
        rx0 = (((i * 2 - dy2) * 241) << 5) / dy2;
      if (dc != 0)
        rx1 = (((i * 2 - dy2) * 241) << 5) / dy2;
      rc += dc;
			if (e >= 0) {
				e -= dy;
				hx += xinc;
			}
			e += dx;
			hy += yinc;
		}
	}
  vr->cc = cc;
  vr->rc = rc;
  vr->rx0 = rx0;
  vr->rx1 = rx1;

  for (int i = 0; i < cc; i++) {
    cols2[i].h = cols[i].h;
    cols2[i].off = cols[i].y * 160 + cols[i].x;
  }

  return 300 * 4 + 200 * 6;*/
}

#define VOXELDBL    1

uint32_t RWDATA oldroll;

void render_voxel() {
  VOXELCOL2 cols2[300];
  VOXELRAY rays[200];
  int16_t ha[200];
  int16_t hi[320];
  int32_t a = angle - 80;
  if (a < 0)
    a += 720;

  int32_t cc = 0, rc = 0;
  int32_t rx0 = 0, rx1 = 0;

  if (roll != oldroll) {
    VOXELRAYS vr;
    precalc_ray(roll, cols2, rays, &vr);
    cc = vr.cc;
    rc = vr.rc;
    rx0 = vr.rx0;
    rx1 = vr.rx1;
    oldroll = roll;
    gsys->RamMode();
    gsys->RamWrite(roll * 12, &vr, 12);
    gsys->RamWrite(40960 + roll * 2400, cols2, 1200);
    gsys->RamWrite(40960 + roll * 2400 + 1200, rays, 1200);
    /*uint8_t *buf = gsys->GetMemBlock(1);
    memcpy(buf, &vr, 12);
    memcpy(buf + 12, cols2, 1200);
    memcpy(buf + 1212, rays, 1200);*/
    fill_audio();
  } else {
    VOXELRAYS vr;
    gsys->RamMode();
    gsys->RamRead(roll * 12, &vr, 12);
    gsys->RamRead(40960 + roll * 2400, cols2, 1200);
    gsys->RamRead(40960 + roll * 2400 + 1200, rays, 1200);
    /*uint8_t *buf = gsys->GetMemBlock(1);
    memcpy(&vr, buf, 12);
    memcpy(cols2, buf + 12, 1200);
    memcpy(rays, buf + 1212, 1200);*/
    cc = vr.cc;
    rc = vr.rc;
    rx0 = vr.rx0;
    rx1 = vr.rx1;
    fill_audio();
  }

  for (int i = 0; i < rc; i++) {
    ha[i] = cols2[rays[i].cstart].h;
  }

  for (int i = 0, j = 0; i < cc; i++) {
    while (j < cols2[i].h)
      hi[j++] = i;
  }

  int16_t sina = sintable[a];
  int16_t cosa = sintable[a + 180];
  uint8_t *fb = (uint8_t *)framebuffer;
  uint16_t *zb = framebuffer + 10 * 1024;

  int32_t zdepth = 160;
  int32_t zz = zdepth << 6;
  int32_t zz1 = -(rx0 * zdepth) / 80;
  int32_t zz2 = (rx1 * zdepth) / 80;
  int32_t lx = (-zz1 * cosa - zz * sina) >> 6;
  int32_t ly = (-zz * cosa + zz1 * sina) >> 6;
  int32_t rx = (zz2 * cosa - zz * sina) >> 6;
  int32_t ry = (-zz * cosa - zz2 * sina) >> 6;
  int32_t dx = (rx - lx) / rc;
  int32_t dy = (ry - ly) / rc;
  int32_t cx = mx + 0x7FF8000;
  int32_t cy = my + 0x7FF8000;

  lx += cx;
  ly += cy;
  rc &= 0xFFFE;
#ifdef  VOXELDBL
  for (int x = 0; x < rc; x+=2, lx += dx+dx, ly += dy+dy) {
#else
  for (int x = 0; x < rc; x++, lx += dx, ly += dy) {
#endif
    int32_t z = 0, dz;
    int32_t rdx = lx - cx;
    int32_t rdy = ly - cy;
    int32_t rx = cx;
    int32_t ry = cy;
    int32_t hx = ha[x];
    uint32_t cend = rays[x].cend;
#ifdef  VOXELDBL
    int32_t hx2 = ha[x+1];
    uint32_t cend2 = rays[x+1].cend;
#endif
    int32_t ardx = rdx >= 0 ? rdx : -rdx;
    int32_t ardy = rdy >= 0 ? rdy : -rdy;
    int32_t dist = ardy;

    if (ardx > ardy) {
      dist = ardx;
      rdy = ((rdy >> 2) * divmap[ardx >> 15]) >> 14;
      //rdy = rdy / (ardx >> 15);
      if (rdx > 0)
        rdx = 1 << 15;
      else
        rdx = -(1 << 15);
    } else {
      rdx = ((rdx >> 2) * divmap[ardy >> 15]) >> 14;
      //rdx = rdx / (ardy >> 15);
      if (rdy > 0)
        rdy = 1 << 15;
      else
        rdy = -(1 << 15);
    }
    int32_t drdx = rdx >> 7, drdy = rdy >> 7;

    int32_t ze = zdepth << 16;
    dz = ze / (dist >> 15);
    uint8_t *dst = fb + rays[x].start;
    uint32_t ci = rays[x].cstart;
#ifdef  VOXELDBL
    uint8_t *dst2 = fb + rays[x+1].start;
    uint32_t ci2 = rays[x+1].cstart;
#endif
    z += dz << 3;
    rx += rdx << 3;
    ry += rdy << 3;
    for (; z < ze; rx += rdx, ry += rdy, z += dz) {
      if (z > zdepth << 14)
        dz += 512, rdx += drdx, rdy += drdy;
      uint32_t j0 = ((ry >> 15) & 3);
      uint32_t k0 = ((rx >> 15) & 3);
      uint32_t j1 = (ry >> 17) & 0x3F;
      uint32_t k1 = (rx >> 17) & 0x3F;
      uint32_t v;
#ifdef  USE_FLASH
      uint32_t addr = 0x68000 + j1 * 512 * 4 + k1 * 32 + j0 * 8 + k0 * 2;
#ifdef  OCEMU
      gsys->FlashRead(addr & 0xFFFFFFFC, &v, 4);
#else
      v = *(uint32_t *)((addr & 0xFFFFFFFC) + 0x40200000);
#endif
      if (addr & 2)
        v >>= 16;
      else
        v &= 0xFFFF;
#else
      uint32_t addr = 0x0000 + j1 * 512 * 4 + k1 * 32 + j0 * 8 + k0 * 2;
      uint32_t addrc = addr & 0xFFFFFFC0;
      if (addrc != cacheaddr) {
        cacheaddr = addrc;
        gsys->RamRead(addrc, cachedata, 64);
      }
      v = cachedata[(addr & 63) >> 1];
#endif

      int h = (v >> 8);
      uint32_t zz = z >> 16;
      h = ((((height - h) * testh) * divmap[zz]) >> 16) + horizon;
      uint8_t c = v + 2;
      if (h < 0)
        h = 0;
#ifdef  VOXELDBL
      if (h < hx2) {
        uint32_t he = hi[h];
        if (he < cend2)
          he = cend2;
        for (; ci2 > he; dst2 -= cols2[ci2].off, ci2--) {
          //if (dst2 < framebuffer)
          //  printf("#\n");
          *dst2 = c;
        }
        if (he == cend2)
          break;
        hx2 = h;
      }
#endif
      if (h >= hx)
        continue;
      uint32_t he = hi[h];
      if (he < cend)
        he = cend;
      //uint32_t zzz = z >> 10;
      for (; ci > he; dst -= cols2[ci].off, ci--) {
          //if (dst < framebuffer)
          //  printf("#\n");
        *dst = c;
#ifdef  OCEMU
        //*(uint16_t *)((uint64_t)(dst + 20480) & 0xFFFFFFFFFFFFFFFE) = zzz;
#else
        //*(uint16_t *)((uint32_t)(dst + 20480) & 0xFFFFFFFE) = zzz;
#endif
      }
      /*for (; ci >= he; dst -= cols2[ci].off, ci--) {
        *dst = c;
      }*/
      if (he == cend)
        break;
      hx = h;
    }
  }
  //printf("%d\n", cnt / rc);
}

// MOVE divmap to stack

uint32_t RWDATA audio_ptr;
uint32_t RWDATA audio_start;
uint32_t RWDATA audio_size;

void fill_audio() {
  uint32_t *sndbuf;
  int8_t src[256];
  while ((sndbuf = gsys->GetAudioBuffer()) != NULL) {
    if ((audio_ptr + 256) > audio_size)
      audio_ptr = audio_start;
    gsys->RamRead(audio_ptr, src, 256);
    for (int i = 0; i < 256; i++) {
      sndbuf[i] = src[i] << 7;
    }
    audio_ptr += 256;
    // 16kHz
    /*if ((audio_ptr + 192) > audio_size)
      audio_ptr = 0;
    gsys->RamRead(audio_ptr, src, 192);
    uint32_t *dst = sndbuf;
    for (int i = 0; i < 192; i++) {
      *dst++ = src[i++] << 7;
      *dst++ = src[i++] << 7;
      *dst++ = src[i] << 7;
      *dst++ = src[i] << 7;
    }
    audio_ptr += 192;*/
  }
}

void fill_silence() {
  uint32_t *sndbuf;
  uint32_t bufcnt = 0;
  while (bufcnt != 4) {
    while ((sndbuf = gsys->GetAudioBuffer()) != NULL) {
      for (int i = 0; i < 256; i++) {
        sndbuf[i] = 0;
      }
      bufcnt++;
    }
    gsys->SleepMs(1);
  }
}

extern void render3d();
extern void loadobj(char *fn, uint32_t faddr, uint32_t raddr);

int32_t RWDATA mapx;
int32_t RWDATA mapy;
uint32_t RWDATA mapidx;

const char * RODATA map1 = "0:/map1x.bin";
const char * RODATA map2 = "0:/map7x.bin";
const char * RODATA map3 = "0:/map10x.bin";
const char * RODATA map4 = "0:/map18x.bin";
const char * RODATA map5 = "0:/map24x.bin";

void loadmap() {
  uint16_t *pal = (uint16_t *)gsys->GetMemBlock(1) + 5 * 1024;
  const char *maps[] = { map3, map4, map5, map2, map1 };

  fill_silence();
  gsys->LcdMode();
  gsys->Printf("   Loading Map...   \n");
#ifdef  USE_FLASH
  load_file(maps[mapidx], 0x40268000, 2 * 256 * 256 + 512, gsys->GetMemBlock(0), 32768);
  gsys->FlashRead(0x68000 + 2 * 256 * 256, pal + 2, 252);
#else
  load_file("0:/map18s.bin", 0, 2 * 256 * 256 + 512, gsys->GetMemBlock(0), 32768);
  gsys->RamRead(0 + 2 * 256 * 256, pal, 512);
#endif
  pal[0] = 0x1234;
  memset(framebuffer, 0, 20480);
}

void main_loop1() {
  fill_audio();
  volatile uint32_t ticks = gsys->GetMsTicks();

  int32_t px = -3072*2, py = 3072*2, pz = 0;

  rotate_init(0, 0, 719 - roll);
  ROTATE_POINT(px, py, pz);
  mapx += px;
  mapy += py;
  mx = mapx << 3;
  my = mapy << 3;
  angle = objects[0].rz;
  objects[1].rz = objects[0].rz;
  objects[2].rz = objects[0].rz;
  objects[0].rx = 719 - roll;
  objects[1].rx = objects[0].rx;
  objects[2].rx = objects[0].rx;
  objects[0].dy = (horizon - 100) * 30 + 500;
  objects[1].dy = objects[0].dy - 1500;
  objects[2].dy = objects[0].dy - 1500;

  uint32_t k = gsys->GetKeys();

  ONKEY(KEY_UP, horizon++);
  ONKEY(KEY_DOWN, horizon--);
  ONKEY(KEY_LEFT, objects[0].rz-=4; objects[0].rz = objects[0].rz >= 720 ? objects[0].rz + 720 : objects[0].rz);
  ONKEY(KEY_RIGHT, objects[0].rz+=4; objects[0].rz = objects[0].rz >= 720 ? objects[0].rz - 720 : objects[0].rz);
  ONKEY(KEY_A, roll++);
  ONKEY(KEY_B, roll--);
  ONKEY(KEY_X, mapidx++; mapidx = mapidx > 4 ? 0 : mapidx; loadmap());
//  ONKEY(KEY_X | KEY_UP, testh++; printf("%d\n", testh));
//  ONKEY(KEY_X | KEY_DOWN, testh--; printf("%d\n", testh));
  //ONKEY(KEY_Y | KEY_UP, height++; printf("%d\n", height));
  //ONKEY(KEY_Y | KEY_DOWN, height--; printf("%d\n", height));
  if (angle >= 720)
    angle -= 720;
  if (angle < 0)
    angle += 720;
  if (roll >= 720)
    roll -= 720;
  if (roll < 0)
    roll += 720;
  if (roll > 120 && roll < 360)
    roll = 120;
  if (roll > 360 && roll < 600)
    roll = 600;

  uint16_t *zbuf = (uint16_t *)(framebuffer + 10240);

  buffer_clear(zbuf);

  render_voxel();
  fill_audio();

  render3d();
  fill_audio();

  uint16_t *pal = (uint16_t *)gsys->GetMemBlock(1) + 5 * 1024;
  memcpy(framebuffer + 10240, pal, 512);
  gsys->UpdateScreen8(framebuffer, framebuffer + 10240, 0, 127, 1);

#ifdef OCEMU
  gsys->SleepMs(20);
#else
  gsys->SleepMs(1);
#endif
}

void main_start1() {
  oldroll = 0xFFFF;

  gsys->Printf("Loading music...\n");
  audio_size = load_file("0:/khaled.raw", 0x200000, 0x800000, gsys->GetMemBlock(0), 32768);

  uint16_t *pal = (uint16_t *)gsys->GetMemBlock(1) + 5 * 1024;
  LOAD_FILE("0:/palette.bin", pal, 512, gsys->GetMemBlock(0), 32768);

  for (int i = 0; i < 64; i++) {
    uint16_t c = (i << 5);
    pal[i + 192] = (c << 8) | (c >> 8);
  }

  loadobj("0://parrot.3d", 0xAC000, 0);
  objects[0].ry = 360;
  objects[0].rz = 360;
  objects[0].dy = 500;
  loadobj("0://stork.3d", 0xD9000, 0);
  objects[1].ry = 360;
  objects[1].rz = 360;
  objects[1].dy = -1000;
  objects[1].dx = -1500;
  loadobj("0://flamingo.3d", 0x89000, 0);
  objects[2].ry = 360;
  objects[2].rz = 360;
  objects[2].dy = -1000;
  objects[2].dx = 1500;

  scan_draw_init();

  mapx = 0;
  mapy = 0;
  height = 160;
  horizon = 100;
  angle = 0;
  roll = 0;
  testh = 12;

  audio_start = audio_ptr = 0x200000;
  audio_size += audio_start;
  gsys->PlayAudio(22050);

  mapidx = 0;
  loadmap();
}
