#ifndef __GFX3D_H
#define __GFX3D_H

typedef struct {
  int16_t x;
  int16_t y;
} POINT2D;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
  int16_t dummy;
} POINT2DZ;

typedef struct {
  int32_t x;
  int32_t y;
  int32_t z;
} POINT3D;

typedef struct {
  uint16_t p1;
  uint16_t p2;
  uint16_t p3;
  uint16_t color;
} FACE3D;

typedef struct {
  uint16_t p1;
  uint16_t p2;
  uint16_t p3;
  uint16_t color;
  uint32_t uv1;
  uint32_t uv2;
  uint32_t uv3;
} FACE3DT;

typedef struct {
  uint16_t point_cnt;
  uint16_t face_cnt;
} OBJECT3D;

typedef struct {
  uint16_t z;
  uint8_t x1;
  uint8_t x2;
  uint8_t color;
} __attribute((__packed__)) SCANENT;

typedef struct {
  uint8_t x1;
  uint8_t x2;
  uint16_t color;
  uint16_t z1;
  uint16_t z2;
} __attribute((__packed__)) SCANENT2;

#define MAX_SCAN_FRAGS    (62)
#define MAX_SCAN_FRAGS2   (39)

typedef struct {
  uint32_t cnt;
  SCANENT frags[MAX_SCAN_FRAGS];
  uint16_t dummy;
} SCANLINE;

typedef struct {
  uint32_t cnt;
  SCANENT2 frags[MAX_SCAN_FRAGS2];
} SCANLINE2;

#define ROT_XYZ     (1)
#define ROT_XZY     (2)
#define ROT_YXZ     (3)
#define ROT_YZX     (4)
#define ROT_ZXY     (5)
#define ROT_ZYX     (6)

extern const int16_t sintable[];

extern int16_t sinx;
extern int16_t cosx;
extern int16_t siny;
extern int16_t cosy;
extern int16_t sinz;
extern int16_t cosz;

#define ROTATE_POINT(px, py, pz) {      \
  int32_t px1, py1;                     \
  px1 = px;                             \
  px = (px * cosz + py * sinz) >> 15;   \
  py = (py * cosz - px1 * sinz) >> 15;  \
  px1 = px;                             \
  px = (px * cosy + pz * siny) >> 15;   \
  pz = (pz * cosy - px1 * siny) >> 15;  \
  py1 = py;                             \
  py = (py * cosx + pz * sinx) >> 15;   \
  pz = (pz * cosx - py1 * sinx) >> 15;  \
}

void transform_mesh(POINT3D *p, POINT2D *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz, int32_t ox, int32_t oy, int32_t oz, int32_t s);
void transform_meshz(POINT3D *p, POINT2DZ *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz, int32_t ox, int32_t oy, int32_t oz, int32_t s);
void rotate_init(uint32_t rx, uint32_t ry, uint32_t rz);

typedef struct {
  uint16_t animfr;
  uint16_t animcnt;
  OBJECT3D *object;
  POINT3D *points;
  FACE3D *faces;
  POINT3D *startpoints;
  uint16_t rx;
  uint16_t ry;
  uint16_t rz;
  int16_t dx;
  int16_t dy;
  int16_t dz;
  uint8_t shading;
  uint8_t shift1;
  uint8_t shift2;
  int8_t shift3;
  uint32_t norm;
} OBJINST;

void transform_meshz_1(POINT3D *p, POINT2DZ *pp, uint32_t pcnt, uint32_t rx, uint32_t ry, uint32_t rz);
void transform_meshz_2(POINT2DZ *pp, uint32_t pcnt, int32_t ox, int32_t oy, int32_t oz, int32_t s);

#endif
