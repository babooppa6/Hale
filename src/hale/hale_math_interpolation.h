#ifndef HALE_MATH_INTERPOLATION_H
#define HALE_MATH_INTERPOLATION_H

// TODO: Quickly ported, code review / actual C implementation needed.
// Take a look at: https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c

// Adopted from this:
// https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c

namespace hale {

// TODO: Move to hale_math.h and check if it cannot be done better.
template<typename T>
inline T
clamp01(T value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 1) {
        return 1;
    }
    return value;
}

// Modeled after the parabola y = -x^2 + 2x
template<typename T>
inline T
quadratic_ease_out(T p)
{
    return -(p * (p - 2));
}

// Modeled after the cubic y = (x - 1)^3 + 1
template<typename T>
inline T
cubic_ease_out(T p)
{
    T f = (p - 1);
    return f * f * f + 1;
}

// Modeled after the quintic y = (x - 1)^5 + 1
template<typename T>
inline T
quintic_ease_out(T p)
{
    T f = (p - 1);
    return f * f * f * f * f + 1;
}

// Modeled after quarter-cycle of sine wave (different phase)
template<typename T>
inline T
sine_ease_out(T p)
{
    return sin(p * hale_PI2);
}


#if 0
template<typename T>
inline T
interpolate_linear(T v0, T v1, T t)
{
    return (1 - t)*v0 + t*v1;
}

template<typename T>
inline T
interpolate_spring(T start, T end, T value)
{
    value = clamp01(value);
    value = (sin(value * PI * (0.2f + 2.5f * value * value * value)) * pow(1.0f - value, 2.2f) + value) * (1.0f + (1.2f * (1.0f - value)));
    return start + (end - start) * value;
}

template<typename T>
inline T
interpolate_in_quad(T start, T end, T value)
{
    end -= start;
    return end * value * value + start;
}
template<typename T>
inline T
interpolate_out_quad(T start, T end, T value)
{
    end -= start;
    return -end * value * (value - 2) + start;
}

template<typename T>
inline T
interpolate_in_out_quad(T start, T end, T value)
{
    value /= 0.5f;
    end -= start;
    if (value < 1) return end * 0.5f * value * value + start;
    value--;
    return -end * 0.5f * (value * (value - 2) - 1) + start;
}

template<typename T>
inline T
interpolate_in_cubic(T start, T end, T value)
{
    end -= start;
    return end * value * value * value + start;
}

template<typename T>
inline T
interpolate_out_cubic(T start, T end, T value)
{
    value--;
    end -= start;
    return end * (value * value * value + 1) + start;
}

template<typename T>
inline T
interpolate_in_out_cubic(T start, T end, T value)
{
    value /= .5f;
    end -= start;
    if (value < 1) return end * 0.5f * value * value * value + start;
    value -= 2;
    return end * 0.5f * (value * value * value + 2) + start;
}

template<typename T>
inline T
interpolate_in_quart(T start, T end, T value)
{
    end -= start;
    return end * value * value * value * value + start;
}

template<typename T>
inline T
interpolate_out_quart(T start, T end, T value)
{
    value--;
    end -= start;
    return -end * (value * value * value * value - 1) + start;
}

template<typename T>
inline T
interpolate_in_out_quart(T start, T end, T value){
    value /= .5f;
    end -= start;
    if (value < 1) return end * 0.5f * value * value * value * value + start;
    value -= 2;
    return -end * 0.5f * (value * value * value * value - 2) + start;
}

template<typename T>
inline T
interpolate_in_quint(T start, T end, T value)
{
    end -= start;
    return end * value * value * value * value * value + start;
}

template<typename T>
inline T
interpolate_out_quint(T start, T end, T value)
{
    value--;
    end -= start;
    return end * (value * value * value * value * value + 1) + start;
}


template<typename T>
inline T
// Modeled after the quintic y = x^5.
interpolate_out_quintic(T t)
{
    T f = (t - 1);
    return f * f * f * f * f + 1;
}


template<typename T>
inline T
interpolate_in_out_quint(T start, T end, T value)
{
    value /= .5f;
    end -= start;
    if (value < 1) return end * 0.5f * value * value * value * value * value + start;
    value -= 2;
    return end * 0.5f * (value * value * value * value * value + 2) + start;
}

template<typename T>
inline T
interpolate_in_out_elastic(T start, T end, T value)
{
    end -= start;

    T d = 1.0f;
    T p = d * .3f;
    T s = 0;
    T a = 0;

    if (value == 0) return start;

    if ((value /= d*0.5f) == 2) return start + end;

    if (a == 0.0f || a < std::abs(end)){
        a = end;
        s = p / 4;
    }
    else{
        s = p / (2 * PI) * asin(end / a);
    }

    value -= 1.f;
    if (value < 1.f) {
        return -0.5f * (a * pow(2.f, 10.f * value) * sin((value * d - s) * (2.f * PI) / p)) + start;
    }
    return a * pow(2.f, -10.f * value) * sin((value * d - s) * (2.f * PI) / p) * 0.5f + end + start;
}

template<typename T>
inline T
interpolate_in_elastic(T start, T end, T value){
    end -= start;

    T d = 1.0f;
    T p = d * 0.3f;
    T s = 0;
    T a = 0;

    if (value == 0) return start;

    if ((value /= d) == 1) return start + end;

    if (a == 0.0f || a < std::abs(end)){
        a = end;
        s = p / 4;
    }
    else{
        s = p / (2 * PI) * asin(end / a);
    }

    value -= 1.f;
    return -(a * pow(2.f, 10.f * value) * sin((value * d - s) * (2.f * PI) / p)) + start;
}

template<typename T>
inline T
interpolate_out_elastic(T start, T end, T value){
    end -= start;

    T d = 1.0f;
    T p = d * .3f;
    T s = 0;
    T a = 0;

    if (value == 0) return start;

    if ((value /= d) == 1) return start + end;

    if (a == 0.0f || a < std::abs(end)){
        a = end;
        s = p * 0.25f;
    }
    else{
        s = p / (2 * PI) * asin(end / a);
    }

    return (a * pow(2.f, -10.f * value) * sin((value * d - s) * (2.f * PI) / p) + end + start);
}

template<typename T>
inline T
interpolate_in_back(T start, T end, T value){
    end -= start;
    value /= 1;
    T s = 1.70158f;
    return end * (value)* value * ((s + 1) * value - s) + start;
}

template<typename T>
inline T
interpolate_out_back(T start, T end, T value){
    T s = 1.70158f;
    end -= start;
    value = (value)-1;
    return end * ((value)* value * ((s + 1) * value + s) + 1) + start;
}

template<typename T>
inline T
interpolate_in_out_back(T start, T end, T value){
    T s = 1.70158f;
    end -= start;
    value /= .5f;
    if ((value) < 1){
        s *= (1.525f);
        return end * 0.5f * (value * value * (((s)+1) * value - s)) + start;
    }
    value -= 2;
    s *= (1.525f);
    return end * 0.5f * ((value)* value * (((s)+1) * value + s) + 2) + start;
}

template<typename T>
inline T
interpolate_accelerate_decelerate(T start, T end, T value)
{
    return start + (end - start) * (cos((value + 1) * PI) / 2.0f + 0.5f);
}
#endif

}

#endif // HALE_MATH_INTERPOLATION_H

