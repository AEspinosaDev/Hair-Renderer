#ifndef __MATH__
#define __MATH__
#include <stdlib.h>

namespace math
{
    struct Color
    {
        double r{0.0};
        double g{0.0};
        double b{0.0};

        Color(double n) : r(n), g(n), b(n){};
        Color(double _r, double _g, double _b) : r(_r), g(_g), b(_b){};

        inline void operator*=(double s)
        {
            r *= s;
            g *= s;
            b *= s;
        }
        inline Color operator*(double s)
        {
            return Color(
                r *= s,
                g *= s,
                b += s);
        }
        inline void operator/=(double s)
        {
            r /= s;
            g /= s;
            b /= s;
        }
        inline Color operator/(double s)
        {
            return Color(
                r /= s,
                g /= s,
                b /= s);
        }
        inline Color operator/(Color c)
        {
            return Color(
                r /= c.r,
                g /= c.g,
                b /= c.b);
        }
    };

    // Gaussian distribution - http://en.wikipedia.org/wiki/Normal_distribution
    double gaussian_distribution(double sigma, double x_mu)
    {
        return ((1.0 / abs((sigma)*sqrt(2.0 * M_PI))) *
                exp(-(x_mu * x_mu) / (2.0 * sigma * sigma)));
    }


    

}

#endif