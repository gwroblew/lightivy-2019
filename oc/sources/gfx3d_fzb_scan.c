#include "oc.h"
#include "display.h"
#include "gfx3d.h"

extern int32_t *divmap;
extern uint16_t *framebuffer;

void scan_clear(SCANLINE *scanbuf) {
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    scanbuf[i].cnt = 0;
  }
}

void scan_draw11(SCANLINE *sb, uint8_t *fb) {
  int32_t zbuf[SCREEN_WIDTH];
  for (int y = 0; y < SCREEN_HEIGHT; y++, sb++, fb += SCREEN_WIDTH) {
    uint32_t *fp = (uint32_t *)fb;
    for (int i = 0; i < SCREEN_WIDTH / 4; i++)
      *fp++ = 0;
    if (sb->cnt == 0) {
      continue;
    }
    for (int i = 0; i < SCREEN_WIDTH; i++)
      zbuf[i] = 32767;
    for (int i = 0; i < sb->cnt; i++) {
      int x0 = sb->frags[i].x1;
      int x1 = sb->frags[i].x2;
      int z = sb->frags[i].z;
      uint32_t color = sb->frags[i].color;
      uint8_t *dst = fb + x0;
      int32_t *zb = zbuf + x0;
      color |= color << 16;

      if ((x1 - x0) < 4) {
        for(int x = x0; x < x1; x++, dst++, zb++) {
          if (*zb < z)
            continue;
          *dst = color;
          *zb = z;
        }
        continue;
      }

      if (x0 & 3) {
        int xd = 4 - (x0 & 3);
        for(int x = 0; x < xd; x++, dst++, zb++) {
          if (*zb < z)
            continue;
          *dst = color;
          *zb = z;
        }
        x0 += xd;
      }
      uint32_t *d4 = (uint32_t *)dst;
      for(int x = x0; x < (x1 & 0xfffc); x += 4, d4++, zb += 4) {
        if (*zb < z)
          continue;
        *d4 = color;
        zb[0] = z;
        zb[1] = z;
        zb[2] = z;
        zb[3] = z;
      }
      dst = (uint8_t *)d4;

      for(int x = 0; x < (x1 & 3); x++, dst++, zb++) {
        if (*zb < z)
          continue;
        *dst = color;
        *zb = z;
      }
    }
  }
}

void scan_draw1(SCANLINE *sb, uint8_t *fb) {
  int32_t zbuf[SCREEN_WIDTH];
  for (int y = 0; y < SCREEN_HEIGHT; y++, sb++, fb += SCREEN_WIDTH) {
    uint32_t *fp = (uint32_t *)fb;
    for (int i = 0; i < SCREEN_WIDTH / 4; i++)
      *fp++ = 0;
    if (sb->cnt == 0) {
      continue;
    }
    for (int i = 0; i < SCREEN_WIDTH; i++)
      zbuf[i] = 32767;
    for (int i = 0; i < sb->cnt; i++) {
      int x0 = sb->frags[i].x1;
      int x1 = sb->frags[i].x2;
      int z = sb->frags[i].z;
      uint32_t color = sb->frags[i].color;
      uint8_t *dst = fb + x0;
      int32_t *zb = zbuf + x0;

      for(int x = x0; x <= x1; x++, dst++, zb++) {
        if (*zb < z)
          continue;
        *dst = color;
        *zb = z;
      }
    }
  }
}

inline void scan_xLine(int x0, int x1, int z, uint32_t color, SCANLINE *buf) {
  if (x0 > SCREEN_WIDTH - 1)
    return;
  if (x1 < 0)
    return;
  if(x0 < 0) x0 = 0;
  if(x1 > SCREEN_WIDTH - 1) x1 = SCREEN_WIDTH - 1;

  if (buf->cnt == MAX_SCAN_FRAGS) {
    //gsys->Printf("X");
    return;
  }
  SCANENT *se = &buf->frags[buf->cnt];
  buf->cnt++;
  se->x1 = x0;
  se->x2 = x1;
  se->color = color;
  se->z = z;
}

#define STA   (8)
#define SWAP(p1, p2)  { int tx = p1##x, ty = p1##y, tz = p1##z; p1##x = p2##x; p1##y = p2##y; p1##z = p2##z; p2##x = tx; p2##y = ty; p2##z = tz; }

void scan_triangle1(int32_t *p0, int32_t *p1, int32_t *p2, uint32_t color)
{
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
  uint32_t zd = (zabi + zaci) >> 1;

  SCANLINE *buf = (SCANLINE *)(framebuffer + 256);
  SCANLINE *dst = buf + y;

  int y1 = p1y;
  int y2 = p2y;
  int ye = y1;
  if (ye >= SCREEN_HEIGHT)
    ye = SCREEN_HEIGHT - 1;
  if (ye < 0) {
    int yc = ye - y;
    xac += xaci * yc;
    zac += zd * yc;
    dst = buf;
    y = ye;
  } else {
    if (y < 0) {
      xab -= xabi * y;
      xac -= xaci * y;
      zac -= zd * y;
      dst = buf;
      y = 0;
    }

    if (xabi < xaci) {
      for(; y < ye; y++, dst++)
      {
        scan_xLine(xab >> STA, xac >> STA, zac >> STA, color, dst);
        xab += xabi;
        xac += xaci;
        zac += zd;
      }
    } else {
      for(; y < ye; y++, dst++)
      {
        scan_xLine(xac >> STA, xab >> STA, zac >> STA, color, dst);
        xab += xabi;
        xac += xaci;
        zac += zd;
      }
    }
  }
  zd = (zbci + zaci) >> 1;
  ye = y2;
  if (ye >= SCREEN_HEIGHT)
    ye = SCREEN_HEIGHT - 1;
  if (y < 0) {
    xbc -= xbci * y;
    xac -= xaci * y;
    zac -= zd * y;
    dst = buf;
    y = 0;
  }

  if (xbc < xac) {
    for(; y < ye; y++, dst++)
    {
      scan_xLine(xbc >> STA, xac >> STA, zac >> STA, color, dst);
      xbc += xbci;
      xac += xaci;
      zac += zd;
    }
  } else {
    for(; y < ye; y++, dst++)
    {
      scan_xLine(xac >> STA, xbc >> STA, zac >> STA, color, dst);
      xbc += xbci;
      xac += xaci;
      zac += zd;
    }
  }
}
