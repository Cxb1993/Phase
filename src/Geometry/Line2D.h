#ifndef LINE_2D
#define LINE_2D

#include "Vector2D.h"
#include "Point2D.h"

class Line2D
{
public:

    static std::pair<Point2D, bool> intersection(const Line2D& lineA, const Line2D& lineB);

    Line2D();
    Line2D(const Point2D& r0, const Vector2D& n);

    Point2D operator()(Scalar t) const;

    bool isApproximatelyOnLine(const Point2D& pt) const;
    bool isAboveLine(const Point2D& pt) const;
    bool isBelowLine(const Point2D& pt) const;

    const Point2D& r0() const { return r0_; }
    const Vector2D& n() const { return n_; }
    const Vector2D& d() const { return d_; }

    const Line2D adjust(Scalar c) const { return Line2D(r0_ + c*n_, n_); }

private:

    Point2D r0_;
    Vector2D n_, d_;

    friend std::ostream& operator<<(std::ostream& os, const Line2D& line);
};

std::ostream& operator<<(std::ostream& os, const Line2D& line);

#endif
