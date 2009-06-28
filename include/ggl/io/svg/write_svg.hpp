// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_IO_SVG_WRITE_SVG_HPP
#define GGL_IO_SVG_WRITE_SVG_HPP

#include <iostream>
#include <string>

#include <boost/concept/assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/ring_type.hpp>

/*!
\defgroup svg svg: stream SVG (Virtual Earth shapes for in VE Ajax Control)
\note VE assumes points in LatLong, Lat first
*/

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace svg {


template <typename Point>
struct svg_point
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Point const& p, std::string const& style, int size)
    {
        os << "<circle cx=\"" << p.x()
            << "\" cy=\"" << p.y()
            << "\" r=\"" << (size < 0 ? 5 : size)
            << "\" style=\"" << style << "\"/>";
    }

    private:
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<Point>) );
};


template <typename Box>
struct svg_box
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Box const& box, std::string const& style, int size)
    {
        int x = ggl::get<ggl::min_corner, 0>(box);
        int y = ggl::get<ggl::min_corner, 1>(box);
        int width = std::abs(ggl::get<ggl::max_corner, 0>(box) - x);
        int height = std::abs(ggl::get<ggl::max_corner, 1>(box) - y);

        os << "<rect x=\"" << x << "\" y=\"" << y
            << "\" width=\"" << width << "\" height=\"" << height
            << "\" style=\"" << style << "\"/>";
    }

    private:
        typedef typename ggl::point_type<Box>::type point_type;
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<point_type>) );
};


/*!
\brief Stream ranges as SVG
\note policy is used to select type (polyline/polygon)
*/
template <typename Range, typename Policy>
struct svg_range
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        Range const& range, std::string const& style, int size)
    {
        typedef typename boost::range_const_iterator<Range>::type iterator;

        bool first = true;

        os << "<" << Policy::prefix() << " points=\"";

        for (iterator it = boost::begin(range);
            it != boost::end(range);
            ++it, first = false)
        {
            os << (first ? "" : " " ) << it->x() << "," << it->y();
        }
        os << "\" style=\"" << style << Policy::style() << "\"/>";
    }

    private:
        typedef typename boost::range_value<Range>::type point;
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<point>) );
};



template <typename Polygon>
struct svg_poly
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        Polygon const& polygon, std::string const& style, int size)
    {
        typedef typename ggl::ring_type<Polygon>::type ring_type;
        typedef typename boost::range_const_iterator<ring_type>::type iterator_type;

        bool first = true;
        os << "<g fill-rule=\"evenodd\"><path d=\"";

        ring_type const& ring = ggl::exterior_ring(polygon);
        for (iterator_type it = boost::begin(ring);
            it != boost::end(ring);
            ++it, first = false)
        {
            os << (first ? "M" : " L") << " " << it->x() << "," << it->y();
        }

        // Inner rings:
        {
            typedef typename boost::range_const_iterator
                <
                    typename ggl::interior_type<Polygon>::type
                >::type ring_iterator_type;
            for (ring_iterator_type rit = boost::begin(interior_rings(polygon));
                 rit != boost::end(interior_rings(polygon));
                 ++rit)
            {
                first = true;
                for (iterator_type it = boost::begin(*rit);
                    it != boost::end(*rit);
                    ++it, first = false)
                {
                    os << (first ? "M" : " L") << " " << it->x() << "," << it->y();
                }
            }
        }
        os << " z \" style=\"" << style << "\"/></g>";

    }

    private:
        BOOST_CONCEPT_ASSERT
            (
                (concept::ConstPoint<typename point_type<Polygon>::type>)
            );
};



struct prefix_linestring
{
    static inline const char* prefix() { return "polyline"; }
    static inline const char* style() { return ";fill:none"; }
};


struct prefix_ring
{
    static inline const char* prefix() { return "polygon"; }
    static inline const char* style() { return ""; }
};



}} // namespace detail::svg
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

/*!
\brief Dispatching base struct for SVG streaming, specialized below per geometry type
\details Specializations should implement a static method "stream" to stream a geometry
The static method should have the signature:

template <typename Char, typename Traits>
static inline void apply(std::basic_ostream<Char, Traits>& os, const G& geometry)
*/
template <typename GeometryTag, typename Geometry>
struct svg {};

template <typename Point>
struct svg<point_tag, Point> : detail::svg::svg_point<Point> {};

template <typename Box>
struct svg<box_tag, Box> : detail::svg::svg_box<Box> {};

template <typename Linestring>
struct svg<linestring_tag, Linestring>
    : detail::svg::svg_range<Linestring, detail::svg::prefix_linestring> {};

template <typename Ring>
struct svg<ring_tag, Ring>
    : detail::svg::svg_range<Ring, detail::svg::prefix_ring> {};

template <typename Polygon>
struct svg<polygon_tag, Polygon>
    : detail::svg::svg_poly<Polygon> {};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
\brief Generic geometry template manipulator class, takes corresponding output class from traits class
\ingroup svg
\details Stream manipulator, streams geometry classes as Virtual Earth shape
*/
template <typename G>
class svg_manipulator
{
public:

    inline svg_manipulator(G const& g, std::string const& style, int size)
        : m_geometry(g)
        , m_style(style)
        , m_size(size)
    {}

    template <typename Char, typename Traits>
    inline friend std::basic_ostream<Char, Traits>& operator<<(
                    std::basic_ostream<Char, Traits>& os, svg_manipulator const& m)
    {
        dispatch::svg
            <
                typename tag<G>::type, G
            >::apply(os, m.m_geometry, m.m_style, m.m_size);
        os.flush();
        return os;
    }

private:
    G const& m_geometry;
    std::string const& m_style;
    int m_size;
};

/*!
\brief Object generator to conveniently stream objects without including streamsvg
\ingroup svg
\par Example:
Small example showing how to use the make_svg helper function
\dontinclude doxygen_examples.cpp
\skip example_as_svg_vector
\line {
\until }
*/
template <typename Geometry>
inline svg_manipulator<Geometry> svg(Geometry const& t, std::string const& style, int size = -1)
{
    return svg_manipulator<Geometry>(t, style, size);
}

} // namespace ggl

#endif // GGL_IO_SVG_WRITE_SVG_HPP
