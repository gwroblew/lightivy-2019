#include <stdlib.h>
#include <string.h>
#include "oc.h"
#include "display.h"
#include "control.h"
#include "config.h"

#include "gfx3d.h"

extern SYSCALLS * RWDATA gsys;
extern uint16_t * RWDATA framebuffer;

extern int32_t * RWDATA divmap;

extern void scan_draw_init();
extern void buffer_clear(uint16_t *zbuf);

extern void scan_triangle3(int32_t *p0, int32_t *p1, int32_t *p2, uint32_t color);
extern void scan_triangle4(int32_t *p0, int32_t *p1, int32_t *p2, uint32_t color, uint32_t uv1, uint32_t uv2, uint32_t uv3);

#define RAND    (seed = (((seed ^ 0xBACADA55) + 0x9123) >> 11) ^ (seed + 0x7432))

extern uint32_t RWDATA mx;
extern uint32_t RWDATA my;
extern int32_t RWDATA height;
extern int32_t RWDATA horizon;
extern int32_t RWDATA angle;
extern int32_t RWDATA roll;

extern uint32_t RWDATA audio_ptr;
extern uint32_t RWDATA audio_start;
extern uint32_t RWDATA audio_size;

extern void fill_audio();

OBJINST RWDATA objects[10];
uint32_t RWDATA objcnt;

uint32_t RWDATA fadecnt;
uint32_t RWDATA objidx;

extern uint32_t RWDATA screen_ymax;

#define P565TORGB(p) { uint16_t t = (p>>8) | (p<<8); r = (t>>11)<<3; g = (t>>3)&0xFC; b = (t<<3)&0xF8; }

void genfade(uint16_t *pal) {
  uint16_t *tmppal = gsys->GetMemBlock(0);
  int32_t r, g, b, r0, g0, b0;
  P565TORGB(pal[0]);
  r0 = r; g0 = g; b0 = b;
  for (int j = 0; j < 256; j++) {
    P565TORGB(pal[j]);
    for (int i = 1; i < 33; i++) {
      int32_t dr = r + (((r0 - r) * i) >> 5);
      int32_t dg = g + (((g0 - g) * i) >> 5);
      int32_t db = b + (((b0 - b) * i) >> 5);
      uint16_t c = ((dr << 8) & 0xF800) | ((dg << 3) & 0x7E0) | ((db >> 3) & 0x1F);
      tmppal[(i-1)*256+j] = (c << 8) | (c >> 8);
    }
  }
  gsys->FlashErase(0x7C000, 16384);
  gsys->FlashWrite(0x7C000, tmppal, 16384);
}

void loadobj(char *fn, uint32_t faddr, uint32_t raddr) {
  if (raddr != 0) {
    load_file(fn, 0x40200000 + raddr, 0x800000, gsys->GetMemBlock(0), 32768);
    OBJECT3D tmpobj;
    gsys->FlashRead(raddr + 4, &tmpobj, sizeof(OBJECT3D));
    load_file(fn, 0x40200000 + faddr, 4 + sizeof(OBJECT3D) + tmpobj.point_cnt * sizeof(POINT3D) + tmpobj.face_cnt * sizeof(FACE3D), gsys->GetMemBlock(0), 32768);
  } else {
    load_file(fn, 0x40200000 + faddr, 0x80000, gsys->GetMemBlock(0), 32768);
  }
  OBJINST *o = &objects[objcnt];
  objcnt++;

  o->animfr = 0;
  o->animcnt = *(uint32_t *)FLASH_ADDR(faddr);

  o->object = FLASH_ADDR(faddr + 4);
  o->points = (POINT3D *)(o->object + 1);
  o->faces = (FACE3D *)(o->points + o->object->point_cnt);

  if (raddr != 0) {
    o->points = raddr + 4 + sizeof(OBJECT3D);
  }
  o->startpoints = o->points;
  o->rx = o->ry = o->rz = 0;
  o->dx = 0;
  o->dy = 0;
  o->dz = 4260;
  o->shading = 0;
  o->shift1 = 5;
  o->shift2 = 15;
  o->shift3 = -70;
  o->norm = 0;
}

void getpoints(POINT2DZ *pp, int i) {
  if (objects[i].points > 0x800000) {
    transform_meshz_1(objects[i].points, pp, objects[i].object->point_cnt, objects[i].rx, objects[i].ry, objects[i].rz);
    return;
  }
  POINT3D *ptr = gsys->GetMemBlock(1) + 8192;
  uint32_t pcnt = objects[i].object->point_cnt;
  uint32_t src = objects[i].points;
  while (pcnt > 0) {
    uint32_t chunk = pcnt;
    if (chunk > 170)
      chunk = 170;
    gsys->FlashRead(src, ptr, chunk * sizeof(POINT3D));
    transform_meshz_1(ptr, pp, chunk, objects[i].rx, objects[i].ry, objects[i].rz);
    src += chunk * sizeof(POINT3D);
    pp += chunk;
    pcnt -= chunk;
  }
}

void nextframe(int i) {
  /*if (animfr == 0)
    testobject = (OBJECT3D *)(testfaces + testobject->face_cnt);
  else
    testobject = (OBJECT3D *)(testpoints + testobject->point_cnt);
  testpoints = (POINT3D *)(testobject + 1);*/
  if (objects[i].animfr == 0)
    objects[i].points = (POINT3D *)((uint8_t *)objects[i].points + sizeof(OBJECT3D) + objects[i].object->point_cnt * sizeof(POINT3D) + objects[i].object->face_cnt * sizeof(FACE3D));
  else
    objects[i].points = (POINT3D *)((uint8_t *)objects[i].points + sizeof(OBJECT3D) + objects[i].object->point_cnt * sizeof(POINT3D));
  objects[i].animfr++;
  if (objects[i].animfr == objects[i].animcnt) {
    objects[i].animfr = 0;
    //testobject = FLASH_ADDR(0x80004);
    //testpoints = (POINT3D *)(testobject + 1);
    objects[i].points = objects[i].startpoints;
  }
}

inline unsigned isqrt2(unsigned long val) {
    unsigned long temp, g=0, b = 0x8000, bshft = 15;
    do {
        if (val >= (temp = (((g << 1) + b)<<bshft--))) {
           g += b;
           val -= temp;
        }
    } while (b >>= 1);
    return g;
}

#define ABS(x)  (((x) < 0)?-(x):(x))

void render3d() {
  POINT2DZ *pp = gsys->GetMemBlock(1);
  uint8_t *fc = (uint8_t *)framebuffer + 40960 - 3840;
  int16_t *fn = framebuffer + 10240 - 1920;
  for (int k = 0; k < objcnt; k++) {
    getpoints(pp, k);

    if (objects[k].shading) {
      uint8_t s1 = objects[k].shift1;
      uint8_t s2 = objects[k].shift2;
      int32_t s3 = objects[k].shift3;

      if (objects[k].norm == 0) {
        for (int i = 0; i < objects[k].object->face_cnt; i++) {
          POINT2DZ *p1 = &pp[objects[k].faces[i].p1];
          POINT2DZ *p2 = &pp[objects[k].faces[i].p2];
          POINT2DZ *p3 = &pp[objects[k].faces[i].p3];
          int32_t dx1 = p2->x - p1->x;
          int32_t dy1 = p2->y - p1->y;
          int32_t dz1 = p2->z - p1->z;
          int32_t dx2 = p3->x - p1->x;
          int32_t dy2 = p3->y - p1->y;
          int32_t dz2 = p3->z - p1->z;
          int32_t vx = (dy2 * dz1 - dy1 * dz2) >> s1;
          int32_t vy = (dx1 * dz2 - dx2 * dz1) >> s1;
          int32_t vz = (dx2 * dy1 - dx1 * dy2) >> s1;
          int32_t vl = (vx * vx + vy * vy + vz * vz) >> 12;
          if (vl == 0)
            vl = 1;
          fn[i] = 65536 / isqrt2(vl);// divmap[isqrt2(vl)];
        }
        objects[k].norm = 1;
      }
      for (int i = 0; i < objects[k].object->face_cnt; i++) {
        POINT2DZ *p1 = &pp[objects[k].faces[i].p1];
        POINT2DZ *p2 = &pp[objects[k].faces[i].p2];
        POINT2DZ *p3 = &pp[objects[k].faces[i].p3];
        int32_t dx1 = p2->x - p1->x;
        int32_t dy1 = p2->y - p1->y;
        int32_t dx2 = p3->x - p1->x;
        int32_t dy2 = p3->y - p1->y;
        int32_t vz = (dx2 * dy1 - dx1 * dy2) >> s1;
        int32_t n = ((vz * fn[i]) >> s2) + s3;
        if (n < 0)
          n = 0;
        if (n > 63)
          n = 63;
        fc[i] = n + 192;
      }
    }

    transform_meshz_2(pp, objects[k].object->point_cnt, objects[k].dx, objects[k].dy, objects[k].dz, 100);
    fill_audio();

    for (int i = 0; i < objects[k].object->face_cnt; i++) {
      if (objects[k].faces[i].p1 == 0 && objects[k].faces[i].p2 == 0)
        break;
      POINT2DZ *p1 = &pp[objects[k].faces[i].p1];
      POINT2DZ *p2 = &pp[objects[k].faces[i].p2];
      POINT2DZ *p3 = &pp[objects[k].faces[i].p3];
      int32_t dx1 = p2->x - p1->x;
      int32_t dy1 = p2->y - p1->y;
      int32_t dx2 = p3->x - p1->x;
      int32_t dy2 = p3->y - p1->y;
      int32_t n = dx2 * dy1 - dx1 * dy2;
      if (n > 0) {
        if (objects[k].shading)
          scan_triangle3(p1, p2, p3, fc[i]);
        else
          scan_triangle3(p1, p2, p3, objects[k].faces[i].color);
      }
    }
    nextframe(k);
  }
}

void nextobj() {
  uint32_t rr = 0;
  if (objidx > 0) {
    objects[objidx].rz = objects[0].rz;
    rr = objects[0].rz;
  }
  objidx++;
  if (objidx == 10)
    objidx = 1;
  objects[0] = objects[objidx];
  objects[0].rz = rr;
  fadecnt = 0;
}

void main_loop2() {
  fill_audio();
  volatile uint32_t ticks = gsys->GetMsTicks();

  uint32_t seed = 0x12345678;

  uint32_t k = gsys->GetKeys();

  uint16_t *zbuf = (uint16_t *)(framebuffer + 10240);

  buffer_clear(zbuf);
  render3d();
  objects[0].rz += 5;
  if (objects[0].rz >= 720)
    objects[0].rz -= 720;
  objects[0].dz = objects[objidx].dz + (sintable[objects[0].rz] >> 4);
  objects[0].ry = (sintable[objects[0].rz] >> 9) + 360;
  objects[0].dy = (-sintable[objects[0].rz] >> 5) + objects[objidx].dy;

  fill_audio();

  uint16_t *pal = (uint16_t *)gsys->GetMemBlock(1) + 5 * 1024;
  memcpy(framebuffer + 10240, pal, 512);
  uint32_t fc = fadecnt;
  if (fc < 31)
    fc = 31 - fc;
  else if (fc < 100)
    fc = 0;
  else
    fc -= 100;
  fadecnt++;
  if (fadecnt == 131)
    nextobj();
  memcpy(framebuffer + 10240, FLASH_ADDR(0x7C000 + fc * 512), 512);
  gsys->UpdateScreen8(framebuffer - 6 * 160, framebuffer + 10240, 12, 127-12, 1);
#ifndef OCEMU
  //gsys->Printf("%d %d %d\n", ticks, ticks2, ticks3);
#endif

#ifdef OCEMU
  gsys->SleepMs(20);
#else
  gsys->SleepMs(1);
#endif
}

void main_start2() {
  gsys->Printf("Loading music...\n");
  audio_size = load_file("0:/romeo.raw", 0, 0x800000, gsys->GetMemBlock(0), 32768);

  uint16_t *pal = (uint16_t *)gsys->GetMemBlock(1) + 5 * 1024;
  LOAD_FILE("0:/palette.bin", pal, 512, gsys->GetMemBlock(0), 32768);
  pal[0] = 0x1234;
  //pal[0] = 0xFF67;

  for (int i = 0; i < 64; i++) {
    uint16_t c = (i << 5);
    pal[i + 192] = (c << 8) | (c >> 8);
  }
  genfade(pal);

  loadobj("0://male_walk.3d", 0x80000, 0x100000);
  objects[0].ry = 360;
  objects[0].dz = 4260;
  objects[0].shading = 1;
  //objects[0].shift1 = 2;
  //objects[0].shift2 = 16;
  //objects[0].shift3 = -10;
  objects[0].shift1 = 3;
  objects[0].shift2 = 16;
  objects[0].shift3 = -20;
  loadobj("0://male_run.3d", 0x88000, 0x200000);
  objects[1].ry = 360;
  objects[1].dz = 4260;
  objects[1].shading = 1;
  objects[1].shift1 = 3;
  objects[1].shift2 = 16;
  objects[1].shift3 = -20;
  loadobj("0://boy.3d", 0x90000, 0x240000);
  objects[2].ry = 360;
  objects[2].dz = 4260;
  loadobj("0://cat.3d", 0x86000, 0);
  objects[3].ry = 360;
  objects[3].dy = -1500;
  objects[3].dz = 6260;
  loadobj("0://horse.3d", 0x98000, 0x2C0000);
  objects[4].ry = 360;
  objects[4].dy = -1000;
  objects[4].dz = 4260;
  objects[4].shading = 1;
  objects[4].shift1 = 4;
  objects[4].shift2 = 16;
  objects[4].shift3 = -10;
  loadobj("0://tree.3d", 0xA0000, 0);
  objects[5].ry = 360;
  objects[5].dz = 4260;
  loadobj("0://corgi1.3d", 0xA4000, 0x320000);
  objects[6].ry = 360;
  objects[6].dy = -1000;
  objects[6].dz = 4260;
  objects[6].shading = 1;
  objects[6].shift1 = 4;
  objects[6].shift2 = 16;
  objects[6].shift3 = -10;
  loadobj("0://beagle2.3d", 0xA8000, 0x340000);
  objects[7].ry = 360;
  objects[7].dy = -1000;
  objects[7].dz = 6260;
  objects[7].shading = 1;
  objects[7].shift1 = 4;
  objects[7].shift2 = 16;
  objects[7].shift3 = -10;
  loadobj("0://french1.3d", 0xAC000, 0x370000);
  objects[8].ry = 360;
  objects[8].dy = -1000;
  objects[8].dz = 4260;
  objects[8].shading = 1;
  objects[8].shift1 = 4;
  objects[8].shift2 = 15;
  objects[8].shift3 = -70;

  objects[9] = objects[0];
  objcnt = 1;
  objidx = 0;
  nextobj();

  scan_draw_init();

  mx = 0;
  my = 0;
  height = 160;
  horizon = 32;
  angle = 0;
  roll = 0;
  screen_ymax = 127 - 24;

  gsys->ClearScreen(0);

  audio_start = audio_ptr = 0;
  gsys->PlayAudio(22050);
}
