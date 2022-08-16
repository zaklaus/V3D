#pragma once
#include <cassert>
#include <glm/glm.hpp>

class I3D_frame;
class I3D_sector;
class I3D_driver;

enum I3D_FRAME_TYPE {
    FRAME_NULL,
    FRAME_VISUAL,
    FRAME_LIGHT,
    FRAME_CAMERA,
    FRAME_SOUND,
    FRAME_SECTOR,
    FRAME_DUMMY,
    FRAME_reserved,
    FRAME_USER,
    FRAME_MODEL,
    FRAME_JOINT,
    FRAME_VOLUME,
    FRAME_OCCLUDER,
    FRAME_LAST,
};

//----------------------------
// bounding box defined by 2 boundary points
struct I3D_bbox {
    I3D_bbox() {}
    I3D_bbox(const glm::vec3 &n, const glm::vec3 &x) {
        min = n;
        max = x;
    }

    inline const glm::vec3 &operator[](int i) const { return (&min)[i]; }

    //----------------------------
    // Make bounding-box 'invalid' - expand min to positive infinity and max to negative infinity.
    // In this state the bounding-box is prepared for extens expansion.
    inline void Invalidate() {
        min = glm::vec3(1e+16f, 1e+16f, 1e+16f);
        max = glm::vec3(-1e+16f, -1e+16f, -1e+16f);
    }

    //----------------------------
    // Check if bounding-box is valid.
    inline bool IsValid() const { return (min.x <= max.x); }

    //----------------------------
    // Expand AA bounding-box, defined by 2 extreme points, into a bounding-box defined by
    // 8 corner points.
    void Expand(glm::vec3 bbox_full[8]) const {
        // expand bound-box (2 corner points) to full bbox (8 points)
        bbox_full[0].x = min.x;
        bbox_full[0].y = min.y;
        bbox_full[0].z = min.z;
        bbox_full[1].x = max.x;
        bbox_full[1].y = min.y;
        bbox_full[1].z = min.z;
        bbox_full[2].x = min.x;
        bbox_full[2].y = max.y;
        bbox_full[2].z = min.z;
        bbox_full[3].x = max.x;
        bbox_full[3].y = max.y;
        bbox_full[3].z = min.z;
        bbox_full[4].x = min.x;
        bbox_full[4].y = min.y;
        bbox_full[4].z = max.z;
        bbox_full[5].x = max.x;
        bbox_full[5].y = min.y;
        bbox_full[5].z = max.z;
        bbox_full[6].x = min.x;
        bbox_full[6].y = max.y;
        bbox_full[6].z = max.z;
        bbox_full[7].x = max.x;
        bbox_full[7].y = max.y;
        bbox_full[7].z = max.z;
    }

    glm::vec3 min;
    glm::vec3 max;
};

//----------------------------
// boundnig sphere
struct I3D_bsphere {
    glm::vec3 pos;
    float radius;
    I3D_bsphere() {}
    I3D_bsphere(const glm::vec3 &p, float r) : pos(p),
                                               radius(r) {}
};