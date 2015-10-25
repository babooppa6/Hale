#ifndef HALE_MATH_H
#define HALE_MATH_H

#include <cmath>

namespace hale {

#define hale_PI      3.14159265358979323846
#define hale_PI2     1.57079632679489661923
#define hale_PI4     0.785398163397448309616

inline b32
equal(r32 a, r32 b, r32 epsilon) {
    return fabs(b - a) <= epsilon;
}

template<typename T>
inline T
minimum(T A, T B) { return A < B ? A : B; }

template<typename T>
inline T
maximum(T A, T B) { return A > B ? A : B; }

template<typename T>
struct V2
{
    union
    {
        struct
        {
            T x, y;
        };
        struct
        {
            T u, v;
        };
        T E[2];
    };
};

template<typename T>
struct V3
{
    union
    {
        struct
        {
            T x, y, z;
        };
        struct
        {
            T u, v, w;
        };
        struct
        {
            T r, g, b;
        };
        struct
        {
            V2<T> xy;
            T ignored0_;
        };
        struct
        {
            T ignored1_;
            V2<T> yz;
        };
        struct
        {
            V2<T> uv;
            T ignored2_;
        };
        struct
        {
            T ignored3_;
            V2<T> vw;
        };
        T E[3];
    };
};

template<typename T>
struct V4
{
    union
    {
        struct
        {
            union
            {
                V3<T> xyz;
                struct
                {
                    T x, y, z;
                };
            };

            T w;
        };
        struct
        {
            union
            {
                V3<T> rgb;
                struct
                {
                    T r, g, b;
                };
            };

            T a;
        };
        struct
        {
            V2<T> xy;
            T ignored0_;
            T ignored1_;
        };
        struct
        {
            T ignored2_;
            V2<T> yz;
            T ignored3_;
        };
        struct
        {
            T ignored4_;
            T ignored5_;
            V2<T> zw;
        };
        T E[4];
    };
};

typedef V2<s32> V2i;
typedef V2<u32> V2u;
typedef V2<r32> V2r;

typedef V4<s32> V4i;
typedef V4<r32> V4r;

typedef V4<u8> Color32;

static_assert(sizeof(Color32) == 4, "Pixel32 should be aligned to 4 bytes, you moron.");

template<typename T>
// TODO: Optimize.
inline bool operator ==(const V4<T> &lhs, const V4<T> &rhs) {
    return (lhs.E[0] == rhs.E[0] &&
            lhs.E[1] == rhs.E[1] &&
            lhs.E[2] == rhs.E[2] &&
            lhs.E[3] == rhs.E[3]);
}

template<typename T>
struct Margin
{
    union {
        struct {
            T left;
            T top;
            T right;
            T bottom;
        };
        struct {
            V2<T> left_top;
            V2<T> right_bottom;
        };
        T E[4];
    };
};

template<typename T>
struct Rect
{
    union {
        struct {
            T min_x;
            T min_y;
            T max_x;
            T max_y;
        };
        struct {
            V2<T> min;
            V2<T> max;
        };
        T E[4];
    };
};

template<typename T>
inline b32
rect_is_valid(Rect<T> *rect)
{
    return (rect->min_x < rect->max_x) && (rect->min_y < rect->max_y);
}

template<typename T>
inline b32
rect_is_valid(Rect<T> &rect)
{
    return (rect.min_x < rect.max_x) && (rect.min_y < rect.max_y);
}

template<typename T>
inline Rect<T>
rect_intersect(Rect<T> rect, T min_x, T min_y, T max_x, T max_y)
{
    Rect<T> result;

    result.min_x = maximum(rect.min_x, min_x);
    result.max_x = minimum(rect.max_x, max_x);
    result.min_y = maximum(rect.min_y, min_y);
    result.max_y = minimum(rect.max_y, max_y);

    return result;
}

template<typename T>
inline V2<T>
rect_size(Rect<T> rect) {
    // TODO: Decide whether max_x/max_y are inclusive.
    return V2<T> {rect.max_x - rect.min_x + 1, rect.max_y - rect.min_y + 1};
}

//
//
//
//

template<typename T>
void
set(V4<T> *v, T e1, T e2, T e3, T e4)
{
    v->E[0] = e1;
    v->E[1] = e2;
    v->E[2] = e3;
    v->E[3] = e4;
}

} // namespace hale

#endif // HALE_MATH_H

