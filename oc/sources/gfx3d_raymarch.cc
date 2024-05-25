#include "oc.h"
#include "display.h"
#include "gfx3d.h"

static unsigned isqrt(unsigned long val) {
    unsigned long temp, g=0, b = 0x80, bshft = 7;
    do {
        if (val >= (temp = (((g << 1) + b)<<bshft--))) {
           g += b;
           val -= temp;
        }
    } while (b >>= 1);
    return g;
}

#define ABS(x)  (((x) < 0)?-(x):(x))

void shader(uint32_t rx, uint32_t ry, uint32_t rz, uint16_t *framebuffer) {
  uint16_t *fb = framebuffer;
  /*for (int y = 0; y < 128; y++) {
    for (int x = 0; x < 160; x+=2) {
      *fb++ = 0xf8;
      *fb++ = 0x1f00;
    }
  }
  return;*/

  int ox = 0;
  int oy = 0;
  int oz = 150;
  rotate_init(rx, ry, rz);
  ROTATE_POINT(ox, oy, oz);
  for (int y = 0; y < 128; y++) {
    for (int x = 0; x < 160; x++) {
      int dy = y - 64;
      int dx = x - 80;
      int dz = -100;
      int d = 100, t = 16, cnt = 0;
      int dl = isqrt(dx * dx + dy * dy + dz * dz);
      int px, py, pz;
      dx = (dx << 8) / dl;
      dy = (dy << 8) / dl;
      dz = (dz << 8) / dl;

      ROTATE_POINT(dx, dy, dz);

      while (t < 256 && cnt < 15) {
        px = ox + ((t * dx) >> 8);
        py = oy + ((t * dy) >> 8);
        pz = oz + ((t * dz) >> 8);
        //px = (px % 2047) - 1024;
        //py = (py % 2047) - 1024;
        //pz = (pz % 2047) - 1024;

        // CUBE
        int bx = ABS(px) - 20;
        int by = ABS(py) - 30;
        //int bz = ABS(pz) - 40;
        bx = bx < 0 ? 0 : bx;
        by = by < 0 ? 0 : by;
        //bz = bz < 0 ? 0 : bz;
        //d = isqrt(bx * bx + by * by + bz * bz) - 7;
        d = isqrt(bx * bx + by * by) - 7;
        if (d < 0)
          d = 1;

        // DONUT
        //int bx = isqrt(px * px + pz * pz) - 40;
        //int by = py;
        //d = isqrt(bx * bx + by * by) - 15;

        /*if (by > bx)
          bx = by;
        if (bz > bx)
          bx = bz;
        d = bx - 7;*/
        if (d < 3)
          break;

        t += d;
        cnt++;
      }
      /*if (t > 1024) {
        *fb++ = 0xE007;
        continue;
      }
      if (d > 2) {
        *fb++ = 0xE003;
        continue;
      }
      int cx = px * dx;
      int cy = py * dy;
      int cz = pz * dz;
      int c = cx + cy + cz;
      if (c < 0)
        c = -c;*/
      //cnt = c / (5800 / 32);
      //if (cnt > 31)
      //  cnt = 31;
      cnt = 31 - (cnt << 1);
      uint16_t pc = (cnt << 11) | (cnt << 6) | cnt;
      if (d > 2)
        pc &= 0x7E0;
      *fb++ = (pc >> 8) | (pc << 8);
//      *fb++ = (pc >> 8) | (pc << 8);
    }
  }
}
