#include "oc.h"
#include "display.h"
#include "gfx3d.h"

const int16_t sintable[] RODATA = {
0,285,571,857,1143,1429,1714,2000,2285,2570,2855,3140,3425,3709,3993,4277,4560,
4843,5126,5408,5690,5971,6252,6532,6812,7092,7371,7649,7927,8204,8480,8756,9032,
9306,9580,9853,10125,10397,10668,10938,11207,11475,11743,12009,12275,12539,12803,13066,13327,
13588,13848,14106,14364,14621,14876,15130,15383,15635,15886,16135,16384,16631,16876,17121,17364,
17606,17846,18085,18323,18559,18794,19028,19260,19491,19720,19947,20173,20398,20621,20843,21062,
21281,21497,21712,21926,22137,22347,22556,22762,22967,23170,23371,23571,23769,23964,24159,24351,
24541,24730,24916,25101,25284,25465,25644,25821,25996,26169,26340,26509,26676,26841,27004,27165,
27324,27481,27636,27788,27939,28087,28233,28377,28519,28659,28797,28932,29065,29196,29325,29451,
29575,29697,29817,29935,30050,30163,30273,30381,30487,30591,30692,30791,30888,30982,31074,31164,
31251,31336,31418,31498,31576,31651,31724,31794,31862,31928,31991,32051,32110,32165,32219,32270,
32318,32364,32408,32449,32487,32523,32557,32588,32617,32643,32666,32688,32706,32723,32736,32748,
32756,32763,32766,32767,32766,32763,32756,32748,32736,32723,32706,32688,32666,32643,32617,32588,
32557,32523,32487,32449,32408,32364,32318,32270,32219,32165,32110,32051,31991,31928,31862,31794,
31724,31651,31576,31498,31418,31336,31251,31164,31074,30982,30888,30791,30692,30591,30487,30381,
30273,30163,30050,29935,29817,29697,29575,29451,29325,29196,29065,28932,28797,28659,28519,28377,
28233,28087,27939,27788,27636,27481,27324,27165,27004,26841,26676,26509,26340,26169,25996,25821,
25644,25465,25284,25101,24916,24730,24541,24351,24159,23964,23769,23571,23371,23170,22967,22762,
22556,22347,22137,21926,21712,21497,21281,21062,20843,20621,20398,20173,19947,19720,19491,19260,
19028,18794,18559,18323,18085,17846,17606,17364,17121,16876,16631,16383,16135,15886,15635,15383,
15130,14876,14621,14364,14106,13848,13588,13327,13066,12803,12539,12275,12009,11743,11475,11207,
10938,10668,10397,10125,9853,9580,9306,9032,8756,8480,8204,7927,7649,7371,7092,6812,
6532,6252,5971,5690,5408,5126,4843,4560,4277,3993,3709,3425,3140,2855,2570,2285,
2000,1714,1429,1143,857,571,285,0,-285,-571,-857,-1143,-1429,-1714,-2000,-2285,
-2570,-2855,-3140,-3425,-3709,-3993,-4277,-4560,-4843,-5126,-5408,-5690,-5971,-6252,-6532,-6812,
-7092,-7371,-7649,-7927,-8204,-8480,-8756,-9032,-9306,-9580,-9853,-10125,-10397,-10668,-10938,-11207,
-11475,-11743,-12009,-12275,-12539,-12803,-13066,-13327,-13588,-13848,-14106,-14364,-14621,-14876,-15130,-15383,
-15635,-15886,-16135,-16384,-16631,-16876,-17121,-17364,-17606,-17846,-18085,-18323,-18559,-18794,-19028,-19260,
-19491,-19720,-19947,-20173,-20398,-20621,-20843,-21062,-21281,-21497,-21712,-21926,-22137,-22347,-22556,-22762,
-22967,-23170,-23371,-23571,-23769,-23964,-24159,-24351,-24541,-24730,-24916,-25101,-25284,-25465,-25644,-25821,
-25996,-26169,-26340,-26509,-26676,-26841,-27004,-27165,-27324,-27481,-27636,-27788,-27939,-28087,-28233,-28377,
-28519,-28659,-28797,-28932,-29065,-29196,-29325,-29451,-29575,-29697,-29817,-29935,-30050,-30163,-30273,-30381,
-30487,-30591,-30692,-30791,-30888,-30982,-31074,-31164,-31251,-31336,-31418,-31498,-31576,-31651,-31724,-31794,
-31862,-31928,-31991,-32051,-32110,-32165,-32219,-32270,-32318,-32364,-32408,-32449,-32487,-32523,-32557,-32588,
-32617,-32643,-32666,-32688,-32706,-32723,-32736,-32748,-32756,-32763,-32766,-32767,-32766,-32763,-32756,-32748,
-32736,-32723,-32706,-32688,-32666,-32643,-32617,-32588,-32557,-32523,-32487,-32449,-32408,-32364,-32318,-32270,
-32219,-32165,-32110,-32051,-31991,-31928,-31862,-31794,-31724,-31651,-31576,-31498,-31418,-31336,-31251,-31164,
-31074,-30982,-30888,-30791,-30692,-30591,-30487,-30381,-30273,-30163,-30050,-29935,-29817,-29697,-29575,-29451,
-29325,-29196,-29065,-28932,-28797,-28659,-28519,-28377,-28233,-28087,-27939,-27788,-27636,-27481,-27324,-27165,
-27004,-26841,-26676,-26509,-26340,-26169,-25996,-25821,-25644,-25465,-25284,-25101,-24916,-24730,-24541,-24351,
-24159,-23964,-23769,-23571,-23371,-23170,-22967,-22762,-22555,-22347,-22137,-21926,-21712,-21497,-21281,-21062,
-20843,-20621,-20398,-20173,-19947,-19720,-19491,-19260,-19028,-18794,-18559,-18323,-18085,-17846,-17606,-17364,
-17121,-16876,-16631,-16383,-16135,-15886,-15635,-15383,-15130,-14876,-14620,-14364,-14106,-13848,-13588,-13327,
-13066,-12803,-12539,-12275,-12009,-11742,-11475,-11207,-10938,-10668,-10397,-10125,-9853,-9580,-9306,-9032,
-8756,-8480,-8204,-7927,-7649,-7371,-7092,-6812,-6532,-6252,-5971,-5690,-5408,-5126,-4843,-4560,
-4277,-3993,-3709,-3425,-3140,-2855,-2570,-2285,-2000,-1714,-1429,-1143,-857,-571,-285,0,
285,571,857,1143,1429,1714,2000,2285,2570,2855,3140,3425,3709,3993,4277,4560,
4843,5126,5408,5690,5971,6252,6532,6812,7092,7371,7649,7927,8204,8480,8756,9032,
9306,9580,9853,10125,10397,10668,10938,11207,11475,11743,12009,12275,12539,12803,13066,13327,
13588,13848,14106,14364,14621,14876,15130,15383,15635,15886,16135,16384,16631,16876,17121,17364,
17606,17846,18085,18323,18559,18794,19028,19260,19491,19720,19947,20174,20398,20621,20843,21062,
21281,21497,21712,21926,22137,22347,22556,22762,22967,23170,23371,23571,23769,23965,24159,24351,
24541,24730,24916,25101,25284,25465,25644,25821,25996,26169,26340,26509,26676,26841,27004,27165,
27324,27481,27636,27788,27939,28087,28233,28377,28519,28659,28797,28932,29065,29196,29325,29451,
29575,29697,29817,29935,30050,30163,30273,30381,30487,30591,30692,30791,30888,30982,31074,31164,
31251,31336,31418,31498,31576,31651,31724,31794,31862,31928,31991,32051,32110,32165,32219,32270,
32318,32364,32408,32449,32487,32523,32557,32588,32617,32643,32666,32688,32706,32723,32736,32748,
32756,32763,32766 };

int16_t RWDATA sinx;
int16_t RWDATA cosx;
int16_t RWDATA siny;
int16_t RWDATA cosy;
int16_t RWDATA sinz;
int16_t RWDATA cosz;

void rotate_init(uint32_t rx, uint32_t ry, uint32_t rz) {
  sinx = sintable[rx];
  cosx = sintable[rx + 180];
  siny = sintable[ry];
  cosy = sintable[ry + 180];
  sinz = sintable[rz];
  cosz = sintable[rz + 180];
}

void transform_mesh(POINT3D *p, POINT2D *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz, int32_t ox, int32_t oy, int32_t oz, int32_t s) {
  int16_t sinx = sintable[rx];
  int16_t cosx = sintable[rx + 180];
  int16_t siny = sintable[ry];
  int16_t cosy = sintable[ry + 180];
  int16_t sinz = sintable[rz];
  int16_t cosz = sintable[rz + 180];

  for (int i = 0; i < pcnt; i++) {
    int32_t px1, px = p[i].x;
    int32_t py1, py = p[i].y;
    int32_t pz = p[i].z;
    px1 = px;
    px = (px * cosz + py * sinz) >> 15;
    py = (py * cosz - px1 * sinz) >> 15;
    px1 = px;
    px = (px * cosy + pz * siny) >> 15;
    pz = (pz * cosy - px1 * siny) >> 15;
    py1 = py;
    py = (py * cosx + pz * sinx) >> 15;
    pz = (pz * cosx - py1 * sinx) >> 15;
    pz += oz;
    px = (px * s / pz) + ox;
    py = (py * s / pz) + oy;
    pp[i].x = px;
    pp[i].y = py;
  }
}

void transform_meshz2(POINT3D *p, POINT2DZ *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz, int32_t ox, int32_t oy, int32_t oz, int32_t s) {
  int16_t sinx = sintable[rx];
  int16_t cosx = sintable[rx + 180];
  int16_t siny = sintable[ry];
  int16_t cosy = sintable[ry + 180];
  int16_t sinz = sintable[rz];
  int16_t cosz = sintable[rz + 180];

  for (int i = 0; i < pcnt; i++) {
    int32_t px1, px = p[i].x;
    int32_t py1, py = p[i].y;
    int32_t pz = p[i].z;
    px1 = px;
    px = (px * cosz + py * sinz) >> 15;
    py = (py * cosz - px1 * sinz) >> 15;
    px1 = px;
    px = (px * cosy + pz * siny) >> 15;
    pz = (pz * cosy - px1 * siny) >> 15;
    py1 = py;
    py = (py * cosx + pz * sinx) >> 15;
    pz = (pz * cosx - py1 * sinx) >> 15;
    pz += oz;
    px = (px * s / pz) + ox;
    py = (py * s / pz) + oy;
    pp[i].x = px;
    pp[i].y = py;
    pp[i].z = pz;
  }
}

#define MUL2(a, b)    ((a * b) >> 15)
#define MUL3(a, b, c) ((((a * b) >> 15) *c) >> 15)

void get_rot_matrix(uint32_t order, int32_t *m, uint32_t rx, uint32_t ry, uint32_t rz) {
  int16_t sinc = sintable[rx];
  int16_t cosc = sintable[rx + 180];
  int16_t sina = sintable[ry];
  int16_t cosa = sintable[ry + 180];
  int16_t sinb = sintable[rz];
  int16_t cosb = sintable[rz + 180];

  switch (order) {
    case ROT_XYZ:
      m[0] = MUL2(cosb, cosc);
      m[1] = -MUL2(cosb, sinc);
      m[2] = sinb;
      m[3] = MUL2(cosa, sinc) + MUL3(sina, sinb, cosc);
      m[4] = MUL2(cosa, cosc) - MUL3(sina, sinb, sinc);
      m[5] = -MUL2(sina, cosb);
      m[6] = MUL2(sina, sinc) - MUL3(cosa, sinb, cosc);
      m[7] = MUL2(sina, cosc) + MUL3(cosa, sinb, sinc);
      m[8] = MUL2(cosa, cosb);
      break;
    case ROT_XZY:
      m[0] = MUL2(cosc, cosb);
      m[1] = -sinc;
      m[2] = MUL2(cosc, sinb);
      m[3] = MUL2(sina, sinb) + MUL3(cosa, sinc, cosb);
      m[4] = MUL2(cosa, cosc);
      m[5] = -MUL2(sina, cosb) + MUL3(cosa, sinc, sinb);
      m[6] = -MUL2(cosa, sinb) + MUL3(sina, sinc, cosb);
      m[7] = MUL2(sina, cosc);
      m[8] = MUL2(cosa, cosb) + MUL3(sina, sinc, sinb);
      break;
    case ROT_YXZ:
      m[0] = MUL2(cosb, cosc) + MUL3(sinb, sina, sinc);
      m[1] = -MUL2(cosb, sinc) + MUL3(sinb, sina, cosc);
      m[2] = MUL2(sinb, cosa);
      m[3] = MUL2(cosa, sinc);
      m[4] = MUL2(cosa, cosc);
      m[5] = -sina;
      m[6] = -MUL2(sinb, cosc) + MUL3(cosb, sina, sinc);
      m[7] = MUL2(sinb, sinc) + MUL3(cosb, sina, cosc);
      m[8] = MUL2(cosb, cosa);
      break;
    case ROT_YZX:
      m[0] = MUL2(cosb, cosc);
      m[1] = MUL2(sinb, sina) - MUL3(cosb, sinc, cosa);
      m[2] = MUL2(sinb, cosa) + MUL3(cosb, sinc, sina);
      m[3] = sinc;
      m[4] = MUL2(cosc, cosa);
      m[5] = -MUL2(cosc, sina);
      m[6] = -MUL2(sinb, cosc);
      m[7] = MUL2(cosb, sina) + MUL3(sinb, sinc, cosa);
      m[8] = MUL2(cosb, cosa) - MUL3(sinb, sinc, sina);
      break;
    case ROT_ZXY:
      m[0] = MUL2(cosc, cosb) - MUL3(sinc, sina, sinb);
      m[1] = -MUL2(sinc, cosa);
      m[2] = MUL2(cosc, sinb) + MUL3(sinc, sina, cosb);
      m[3] = MUL2(sinc, cosb) + MUL3(cosc, sina, sinb);
      m[4] = MUL2(cosc, cosa);
      m[5] = MUL2(sinc, sinb) - MUL3(cosc, sina, cosb);
      m[6] = -MUL2(cosa, sinb);
      m[7] = sina;
      m[8] = MUL2(cosa, cosb);
      break;
    case ROT_ZYX:
      m[0] = MUL2(cosc, cosb);
      m[1] = -MUL2(sinc, cosa) + MUL3(cosc, sinb, sina);
      m[2] = MUL2(sinc, sina) + MUL3(cosc, sinb, cosa);
      m[3] = MUL2(sinc, cosb);
      m[4] = MUL2(cosc, cosa) + MUL3(sinc, sinb, sina);
      m[5] = -MUL2(cosc, sina) + MUL3(sinc, sinb, cosa);
      m[6] = -sinb;
      m[7] = MUL2(cosb, sina);
      m[8] = MUL2(cosb, cosa);
      break;
    default:
      for (int i = 0; i < 9; i++)
        m[i] = 0;
      m[0] = 32768;
      m[4] = 32768;
      m[8] = 32768;
      break;
  }
}

void transform_meshz(POINT3D *p, POINT2DZ *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz, int32_t ox, int32_t oy, int32_t oz, int32_t s) {
  int32_t m[9];

  get_rot_matrix(ROT_YXZ, m, rx, ry, rz);

  uint32_t *dst = (uint32_t *)pp;
  for (int i = 0; i < pcnt; i++) {
    int32_t px1, px = p[i].x;
    int32_t py1, py = p[i].y;
    int32_t pz = p[i].z;
    px1 = (px * m[0] + py * m[3] + pz * m[6]) >> 15;
    py1 = (px * m[1] + py * m[4] + pz * m[7]) >> 15;
    pz = (px * m[2] + py * m[5] + pz * m[8]) >> 15;
    pz += oz;
    px = (px1 * s / pz) + ox;
    py = (py1 * s / pz) + oy;
    //pp[i].x = px;
    //pp[i].y = py;
    //pp[i].z = pz;
    *dst++ = (px & 0xFFFF) | (py << 16);
    *dst++ = pz;
  }
}

void transform_meshz_1(POINT3D *p, POINT2DZ *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz) {
  int32_t m[9];

  get_rot_matrix(ROT_YXZ, m, rx, ry, rz);

  uint32_t *dst = (uint32_t *)pp;
  for (int i = 0; i < pcnt; i++) {
    int32_t px1, px = p[i].x;
    int32_t py1, py = p[i].y;
    int32_t pz = p[i].z;
    px1 = (px * m[0] + py * m[3] + pz * m[6]) >> 15;
    py1 = (px * m[1] + py * m[4] + pz * m[7]) >> 15;
    pz = (px * m[2] + py * m[5] + pz * m[8]) >> 15;
    //pp[i].x = px;
    //pp[i].y = py;
    //pp[i].z = pz;
    *dst++ = (px1 & 0xFFFF) | (py1 << 16);
    *dst++ = pz;
  }
}

void transform_meshz_2(POINT2DZ *pp, uint32_t pcnt, int32_t ox, int32_t oy, int32_t oz, int32_t s) {
  uint32_t *dst = (uint32_t *)pp;
  for (int i = 0; i < pcnt; i++) {
    int32_t pz = pp[i].z + oz;
    int32_t px = ((pp[i].x + ox) * s / pz) + 80;
    int32_t py = ((pp[i].y + oy) * s / pz) + 64;
    //pp[i].x = px;
    //pp[i].y = py;
    //pp[i].z = pz;
    *dst++ = (px & 0xFFFF) | (py << 16);
    *dst++ = pz;
  }
}

extern uint16_t *framebuffer;

inline void flat_xLine(int x0, int x1, uint32_t color, uint8_t *buf)
{
  if (x0 > SCREEN_WIDTH - 1)
    return;
  if (x1 < 0)
    return;
  if(x0 < 0) x0 = 0;
  if(x1 > SCREEN_WIDTH - 1) x1 = SCREEN_WIDTH - 1;

  uint8_t *dst = buf + x0;

  if ((x1 - x0) < 4) {
    for(int x = x0; x < x1; x++)
      *dst++ = color;
    return;
  }

  if (x0 & 3) {
    int xd = 4 - (x0 & 3);
    for(int x = 0; x < xd; x++)
      *dst++ = color;
    x0 += xd;
  }
  uint32_t *d4 = (uint32_t *)dst;
  for(int x = x0; x < (x1 & 0xfffc); x += 4)
    *d4++ = color;
  dst = (uint8_t *)d4;

  for(int x = 0; x < (x1 & 3); x++)
    *dst++ = color;
}

void flat_triangle(int16_t *p0, int16_t *p1, int16_t *p2, uint32_t color)
{
  if (p1[1] < p0[1]) {
    int16_t *vb = p0; p0 = p1; p1 = vb;
  }
  if (p2[1] < p1[1]) {
    int16_t *vb = p1; p1 = p2; p2 = vb;
  }
  if (p1[1] < p0[1]) {
    int16_t *vb = p0; p0 = p1; p1 = vb;
  }

  if (p0[1] > SCREEN_HEIGHT - 1)
    return;
  if (p2[1] < 0)
    return;

  int y = p0[1];
  int xac = p0[0] << 16;
  int xab = p0[0] << 16;
  int xbc = p1[0] << 16;
  int xaci = 0;
  int xabi = 0;
  int xbci = 0;
  if(p1[1] != p0[1])
    xabi = ((p1[0] - p0[0]) << 16) / (p1[1] - p0[1]);
  if(p2[1] != p0[1])
    xaci = ((p2[0] - p0[0]) << 16) / (p2[1] - p0[1]);
  if(p2[1] != p1[1])
    xbci = ((p2[0] - p1[0]) << 16) / (p2[1] - p1[1]);

  uint8_t *dst = (uint8_t *)framebuffer + y * 160;

  int y1 = p1[1];
  int y2 = p2[1];
  int ye = y1;
  if (ye >= SCREEN_HEIGHT)
    ye = SCREEN_HEIGHT - 1;
  if (ye < 0) {
    int yc = ye - y;
    xab += xabi * yc;
    xac += xaci * yc;
    dst = (uint8_t *)framebuffer;
    y = ye;
  } else {
    if (y < 0) {
      xab -= xabi * y;
      xac -= xaci * y;
      dst = (uint8_t *)framebuffer;
      y = 0;
    }

    if (xabi < xaci) {
      for(; y < ye; y++, dst += 160)
      {
        flat_xLine(xab >> 16, xac >> 16, color, dst);
        xab += xabi;
        xac += xaci;
      }
    } else {
      for(; y < ye; y++, dst += 160)
      {
        flat_xLine(xac >> 16, xab >> 16, color, dst);
        xab += xabi;
        xac += xaci;
      }
    }
  }
  ye = y2;
  if (ye >= SCREEN_HEIGHT)
    ye = SCREEN_HEIGHT - 1;
  if (y < 0) {
    xbc -= xbci * y;
    xac -= xaci * y;
    dst = (uint8_t *)framebuffer;
    y = 0;
  }

  if (xbc < xac) {
    for(; y < ye; y++, dst += 160)
    {
      flat_xLine(xbc >> 16, xac >> 16, color, dst);
      xbc += xbci;
      xac += xaci;
    }
  } else {
    for(; y < ye; y++, dst += 160)
    {
      flat_xLine(xac >> 16, xbc >> 16, color, dst);
      xbc += xbci;
      xac += xaci;
    }
  }
}
