#ifndef __GENERATOR_MATH__
#define __GENERATOR_MATH__
#include <stdlib.h>

namespace math
{
#pragma region Color
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
#pragma endregion
#pragma region Longitudinal Term

    // Gaussian distribution - http://en.wikipedia.org/wiki/Normal_distribution
    double gaussian_distribution(double sigma, double x_mu)
    {
        return ((1.0 / abs((sigma)*sqrt(2.0 * M_PI))) *
                exp(-(x_mu * x_mu) / (2.0 * sigma * sigma)));
    }

    // Hyperbolic cosecant
    double csch(double x)
    {
        double sinh_x = sinh(x);

        // Handle div by zero
        if (sinh_x == 0.0)
            throw std::overflow_error("Division by zero in csch function");

        return 1.0 / sinh_x;
    }

    unsigned long long factorial(int n)
    {
        if (n == 0)
            return 1;
        unsigned long long result = 1;
        for (int i = 1; i <= n; ++i)
        {
            result *= i;
        }
        return result;
    }

    // First kind bessel
    double I_0(double x)
    {

        double sum = 0.0;
        for (int i = 0; i <= 10; i++)
        {
            double numerator = pow(x, 2 * i);
            double denominator = pow(4, i) * pow(factorial(i), 2);
            sum += numerator / denominator;
        }
        return sum;
    }

    // Spherical gaussian convolution over a dirac circle - https://www.researchgate.net/publication/220506677_An_Energy-Conserving_Hair_Reflectance_Model
    double energy_conservant_gaussian_distribution(double beta, double thI, double thR)
    {
        double sin_minus_I = sin(-thI);
        double sin_R = sin(thR);
        double cos_minus_I = cos(-thI);
        double cos_R = cos(thR);

        double v = beta * beta;

        return (csch(1.0 / v) / (2.0 * v)) * exp((sin_minus_I * sin_R) / v) * I_0((cos_minus_I * cos_R) / v);
    }

#pragma endregion
#pragma region Azimuthal Term

    // Miller-Bravais indices - http://en.wikipedia.org/wiki/Miller_index
    double bravais_index(double theta, double eta)
    {
        double sinTheta = sin(theta);
        return (sqrt(eta * eta - sinTheta * sinTheta) / cos(theta));
    }

    // Fresnel equation parallel component - http://en.wikipedia.org/wiki/Fresnel_equations
    double fresnel_parallel(double n2, double angle)
    {
        double R = 1.0;
        double n1 = 1.0;
        double cos_gammaI = cos(angle);
        double a = ((n1 / n2) * sin(angle));
        double b = a * a;

        if (b > 1.0)
            return R;

        double cos_gammaT = sqrt(1.0 - b);

        R = (n2 * cos_gammaI - n1 * cos_gammaT) /
            (n2 * cos_gammaI + n1 * cos_gammaT);

        return std::min(1.0, R * R);
    }

    // Fresnel equation perpendicular component - http://en.wikipedia.org/wiki/Fresnel_equations
    double fresnel_perpendicular(double n2, double angle)
    {
        double R = 1.0;
        double n1 = 1.0;
        double cos_gammaI = cos(angle);
        double a = ((n1 / n2) * sin(angle));
        double b = a * a;

        if (b > 1.0)
            return R;

        double cos_gammaT = sqrt(1.0 - b);

        R = (n1 * cos_gammaI - n2 * cos_gammaT) /
            (n1 * cos_gammaI + n2 * cos_gammaT);

        return std::min(1.0, R * R);
    }

    // Fresnel equation - http://en.wikipedia.org/wiki/Fresnel_equations
    double fresnel(double etaPerpendicular, double etaParallel, double angle)
    {
        return 0.5 * (fresnel_perpendicular(etaPerpendicular, angle) +
                      fresnel_parallel(etaParallel, angle));
    }


    // Clamp function for double values - common shader function
    double clamp(double x, double min, double max)
    {
        if (x < min)
            return min;
        if (x > max)
            return max;
        return x;
    }
    
    // Approx Schlick's Fresnel - https://en.wikipedia.org/wiki/Schlick%27s_approximation
    double fresnelSchlick(double out_eta, double in_eta, double cosTheta)
    {
        double F0 = ((out_eta- in_eta) * (out_eta- in_eta)) / ((out_eta + in_eta) * (out_eta + in_eta));
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    // Saturate function - common shader function
    double saturate(double x)
    {
        return clamp(x, 0.0, 1.0);
    }

    // Smoothstep function - common shader function
    double smoothstep(double edge0, double edge1, double x)
    {
        // Scale, bias and saturate x to 0..1 range
        double t = saturate((x - edge0) / (edge1 - edge0));
        // Evaluate polynomial
        return t * t * (3.0 - 2.0 * t);
    }

#pragma endregion
#pragma region Solvers

    class Equation
    {

    public:
        static const double EPS;
        // Solve a * x + b = 0 - linear equation
        static std::vector<double>
        linear_solver(double a, double b)
        {
            std::vector<double> roots(4, 0.0);

            if (abs(a) > EPS)
            {
                roots[0] = -b / a;
                roots[3] = 1;
            }

            return roots;
        }

        // Solve a * x ^ 2 + b * x + c = 0 - quadratic equation
        static std::vector<double> quadratic_solver(double a, double b, double c)
        {
            std::vector<double> roots(4, 0.0);

            if (abs(a) < EPS)
                return linear_solver(b, c);
            else
            {
                double D = b * b - 4.0 * a * c;

                if (abs(D) < EPS)
                {
                    roots[0] = -b / (2.0 * a);
                    roots[1] = -b / (2.0 * a);
                    roots[3] = 2.0;
                }
                else if (D > 0)
                {
                    double delta = sqrt(D);
                    roots[0] = (-b + delta) / (2.0 * a);
                    roots[1] = (-b - delta) / (2.0 * a);
                    roots[3] = 2.0;
                }
            }

            return roots;
        }

        // Solve x ^ 3 + A * x ^ 2 + B * x + C = 0 - normalized cubic equation
        //  http://en.wikipedia.org/wiki/Cubic_function
        static std::vector<double> normalized_cubic_solver(double A, double B, double C)
        {
            std::vector<double> roots(4, 0.0);

            if (abs(C) < EPS) //	x = 0 solution
            {
                roots = quadratic_solver(1, A, B);
                roots[(int)roots[3]] = 0;
                roots[3] = roots[3] + 1.0;
            }
            else
            {
                double Q = (3 * B - A * A) / 9;
                double R = (9 * A * B - 27 * C - 2 * A * A * A) / 54;
                double D = Q * Q * Q + R * R;

                auto sign = [](double val)
                {
                    return (double(0) < val) - (val < double(0));
                };

                if (D > 0) // 1 root
                {
                    double sqrtD = sqrt(D);
                    double s = sign(R + sqrtD) *
                               pow(abs(R + sqrtD), 1.0f / 3.0f);
                    double t = sign(R - sqrtD) *
                               pow(abs(R - sqrtD), 1.0f / 3.0f);

                    roots[0] = (-A / 3 + (s + t));
                    roots[3] = 1;
                }
                else // 3 roots
                {
                    double theta = acos(R / sqrt(-(Q * Q * Q)));
                    double sqrtQ = sqrt(-Q);
                    roots[0] = (2 * sqrtQ * cos(theta / 3) - A / 3);
                    roots[1] = (2 * sqrtQ *
                                    cos((theta + 2 * M_PI) / 3) -
                                A / 3);
                    roots[2] = (2 * sqrtQ *
                                    cos((theta + 4 * M_PI) / 3) -
                                A / 3);
                    roots[3] = 3;
                }
            }

            return roots;
        }

        // Test if the roots for a cubic equations are correct

        static void test_roots(double a, double b, double c, double d, std::vector<double> roots)
        {
            double val;

            for (int i = 0; i < roots[3]; i++)
                if ((val = abs((((a * roots[i] + b) *
                                 roots[i]) +
                                c) *
                                   roots[i] +
                               d)) > EPS)
                    std::cout << "Error " << i << " A=" << a << " B=" << b << " C=" << c << " D=" << d << " " << (val - EPS) << std::endl;
        }

        // Solve a * x ^ 3 + b * x ^ 2 + c * x  + d = 0 - cubic equation http://en.wikipedia.org/wiki/Cubic_function
        static std::vector<double> cubic_solver(double a, double b, double c, double d)
        {
            std::vector<double> roots(4, 0.0);

            if (abs(a) < EPS)
                roots = quadratic_solver(b, c, d);
            else
                roots = normalized_cubic_solver(b / a, c / a, d / a);

            test_roots(a, b, c, d, roots);

            return roots;
        }

        // Solve o(p,y) - phi = 0
        static std::vector<double> roots(double p, double etaPerpendicular, double phi)
        {
            double c = asin(1 / etaPerpendicular);

            return cubic_solver(-8 * (p * c / (M_PI * M_PI * M_PI)), 0, (6 * p * c / M_PI - 2), p * M_PI - phi);
        }

        // computes the angle polygon
        static double angle_polynomial(double p, double etaPerpendicular, double h)
        {
            double gammaI = asin(h);
            double c = asin(1 / etaPerpendicular);
            return (6 * p * c / M_PI - 2) * gammaI - 8 * (p * c / (M_PI * M_PI * M_PI)) * gammaI * gammaI * gammaI + p * M_PI;
        }

        // Computes the derivative of the polynomial relative to h.

        static double inverse_first_derivative(double p, double etaPerpendicular, double h)
        {
            double gammaI = asin(h);
            double c = asin(1 / etaPerpendicular);
            double dGamma = (6 * p * c / M_PI - 2) - 3 * 8 * (p * c / (M_PI * M_PI * M_PI)) * gammaI * gammaI;

            return sqrt(1 - h * h) / dGamma;
        }

        // Computes the second derivative of the polynomial relative to h.

        static double inverse_second_derivative_aprox(double p, double etaPerpendicular, double h)
        {
            double gammaI = asin(h);
            double c = asin(1 / etaPerpendicular);
            double dGamma = -2 * 3 * 8 *
                            (p * c / (M_PI * M_PI * M_PI)) * gammaI;

            double denom = std::max(dGamma * h, EPS);
            return pow(1 - h * h, 3.0 / 2.0) / denom;
        }

        // computes the second derivative of the polynomial relative to h.
        static double inverse_second_derivative(double p, double etaPerpendicular, double h)
        {
            double gammaI = asin(h);
            double c = asin(1 / etaPerpendicular);

            // h' = (f'g - fg') / g^2
            double f = (6 * p * c / M_PI - 2) - 3 * 8 *
                                                    (p * c / (M_PI * M_PI * M_PI)) * gammaI * gammaI;
            double df = -2 * 3 * 8 * (p * c / (M_PI * M_PI * M_PI)) * gammaI;
            double g = sqrt(1 - h * h);
            double dg = h / std::max(g, EPS);

            return (g * g) / (std::max(df * g - f * dg, EPS));
        }
    };
    const double Equation::EPS = 0.00001;

#pragma endregion
}

#endif