#pragma once

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#undef min
#undef max

namespace delaunator {

    //@see https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op/33333636#33333636
    inline size_t fast_mod( const size_t i, const size_t c ) {
        return i >= c ? i % c : i;
    }

    // Kahan and Babuska summation, Neumaier variant; accumulates less FP error
    inline double sum( const std::vector<double> &x ) {
	    auto sum = x[ 0 ];
	    auto err = 0.0;

        for ( size_t i = 1; i < x.size( ); i++ ) {
            const auto k = x[ i ];
            const auto m = sum + k;
            err += std::fabs( sum ) >= std::fabs( k ) ? sum - m + k : k - m + sum;
            sum = m;
        }
        return sum + err;
    }

    inline double dist(
        const double ax,
        const double ay,
        const double bx,
        const double by ) {
        const auto dx = ax - bx;
        const auto dy = ay - by;
        return dx * dx + dy * dy;
    }

    inline double circumradius(
        const double ax,
        const double ay,
        const double bx,
        const double by,
        const double cx,
        const double cy ) {
        const auto dx = bx - ax;
        const auto dy = by - ay;
        const auto ex = cx - ax;
        const auto ey = cy - ay;

        const auto bl = dx * dx + dy * dy;
        const auto cl = ex * ex + ey * ey;
        const auto d = dx * ey - dy * ex;

        const auto x = ( ey * bl - dy * cl ) * 0.5 / d;
        const auto y = ( dx * cl - ex * bl ) * 0.5 / d;

        if ( ( bl > 0.0 || bl < 0.0 ) && ( cl > 0.0 || cl < 0.0 ) && ( d > 0.0 || d < 0.0 ) ) {
            return x * x + y * y;
        } else {
            return std::numeric_limits<double>::max( );
        }
    }

    inline bool orient(
        const double px,
        const double py,
        const double qx,
        const double qy,
        const double rx,
        const double ry ) {
        return ( qy - py ) * ( rx - qx ) - ( qx - px ) * ( ry - qy ) < 0.0;
    }

    inline std::pair<double, double> circumcenter(
        const double ax,
        const double ay,
        const double bx,
        const double by,
        const double cx,
        const double cy ) {
        const auto dx = bx - ax;
        const auto dy = by - ay;
        const auto ex = cx - ax;
        const auto ey = cy - ay;

        const auto bl = dx * dx + dy * dy;
        const auto cl = ex * ex + ey * ey;
        const auto d = dx * ey - dy * ex;

        const auto x = ax + ( ey * bl - dy * cl ) * 0.5 / d;
        const auto y = ay + ( dx * cl - ex * bl ) * 0.5 / d;

        return std::make_pair( x, y );
    }

    struct compare {

        std::vector<double> const &coords;
        double cx;
        double cy;

        bool operator()( std::size_t i, std::size_t j ) const {
            const auto d1 = dist( coords[ 3 * i ], coords[ 3 * i + 1 ], cx, cy );
            const auto d2 = dist( coords[ 3 * j ], coords[ 3 * j + 1 ], cx, cy );
            const auto diff1 = d1 - d2;
            const auto diff2 = coords[ 3 * i ] - coords[ 3 * j ];
            const auto diff3 = coords[ 3 * i + 1 ] - coords[ 3 * j + 1 ];

            if ( diff1 > 0.0 || diff1 < 0.0 ) {
                return diff1 < 0;
            } else if ( diff2 > 0.0 || diff2 < 0.0 ) {
                return diff2 < 0;
            } else {
                return diff3 < 0;
            }
        }
    };

    inline bool in_circle(
        const double ax,
        const double ay,
        const double bx,
        const double by,
        const double cx,
        const double cy,
        const double px,
        const double py ) {
        const auto dx = ax - px;
        const auto dy = ay - py;
        const auto ex = bx - px;
        const auto ey = by - py;
        const auto fx = cx - px;
        const auto fy = cy - py;

        const auto ap = dx * dx + dy * dy;
        const auto bp = ex * ex + ey * ey;
        const auto cp = fx * fx + fy * fy;

        return ( dx * ( ey * cp - bp * fy ) -
                 dy * ( ex * cp - bp * fx ) +
                 ap * ( ex * fy - ey * fx ) ) < 0.0;
    }

    constexpr double EPSILON = std::numeric_limits<double>::epsilon( );
    constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max( );

    inline bool check_pts_equal( double x1, double y1, double x2, double y2 ) {
        return std::fabs( x1 - x2 ) <= EPSILON &&
            std::fabs( y1 - y2 ) <= EPSILON;
    }

    // monotonically increases with real angle, but doesn't need expensive trigonometry
    inline double pseudo_angle( const double dx, const double dy ) {
        const auto p = dx / ( std::abs( dx ) + std::abs( dy ) );
        return ( dy > 0.0 ? 3.0 - p : 1.0 + p ) / 4.0; // [0..1)
    }

    struct DelaunatorPoint {
        std::size_t i;
        double x;
        double y;
        std::size_t t;
        std::size_t prev;
        std::size_t next;
        bool removed;
    };

    class Delaunator {

    public:
        std::vector<double> const &coords;
        std::vector<std::size_t> triangles;
        std::vector<std::size_t> halfedges;
        std::vector<std::size_t> hull_prev;
        std::vector<std::size_t> hull_next;
        std::vector<std::size_t> hull_tri;
        std::size_t hull_start;

        Delaunator( std::vector<double> const &in_coords );

        double get_hull_area( );

    private:
        std::vector<std::size_t> m_hash;
        double m_center_x;
        double m_center_y;
        std::size_t m_hash_size;
        std::vector<std::size_t> m_edge_stack;

        std::size_t legalize( std::size_t a );
        std::size_t hash_key( double x, double y ) const;
        std::size_t add_triangle(
            std::size_t i0,
            std::size_t i1,
            std::size_t i2,
            std::size_t a,
            std::size_t b,
            std::size_t c );
        void link( std::size_t a, std::size_t b );
    };

    Delaunator::Delaunator( std::vector<double> const &in_coords )
        : coords( in_coords ),
        triangles( ),
        halfedges( ),
        hull_prev( ),
        hull_next( ),
        hull_tri( ),
        hull_start( ),
        m_hash( ),
        m_center_x( ),
        m_center_y( ),
        m_hash_size( ),
        m_edge_stack( ) {
        auto n = ((coords.size( )/3) * 2) >> 1;

        auto max_x = std::numeric_limits<double>::min( );
        auto max_y = std::numeric_limits<double>::min( );
        auto min_x = std::numeric_limits<double>::max( );
        auto min_y = std::numeric_limits<double>::max( );
        std::vector<std::size_t> ids;
        ids.reserve( n );

        for ( std::size_t i = 0; i < n; i++ ) {
            const auto x = coords[ 3 * i ];
            const auto y = coords[ 3 * i + 1 ];

            if ( x < min_x ) min_x = x;
            if ( y < min_y ) min_y = y;
            if ( x > max_x ) max_x = x;
            if ( y > max_y ) max_y = y;

            ids.push_back( i );
        }
        const auto cx = ( min_x + max_x ) / 2;
        const auto cy = ( min_y + max_y ) / 2;
        auto min_dist = std::numeric_limits<double>::max( );

        auto i0 = INVALID_INDEX;
        auto i1 = INVALID_INDEX;
        auto i2 = INVALID_INDEX;

        // pick a seed point close to the centroid
        for ( std::size_t i = 0; i < n; i++ ) {
            const auto d = dist( cx, cy, coords[ 3 * i ], coords[ 3 * i + 1 ] );
            if ( d < min_dist ) {
                i0 = i;
                min_dist = d;
            }
        }

        const auto i0x = coords[ 3 * i0 ];
        const auto i0y = coords[ 3 * i0 + 1 ];

        min_dist = std::numeric_limits<double>::max( );

        // find the point closest to the seed
        for ( std::size_t i = 0; i < n; i++ ) {
            if ( i == i0 ) continue;
            const auto d = dist( i0x, i0y, coords[ 3 * i ], coords[ 3 * i + 1 ] );
            if ( d < min_dist && d > 0.0 ) {
                i1 = i;
                min_dist = d;
            }
        }

        auto i1x = coords[ 3 * i1 ];
        auto i1y = coords[ 3 * i1 + 1 ];

        auto min_radius = std::numeric_limits<double>::max( );

        // find the third point which forms the smallest circumcircle with the first two
        for ( std::size_t i = 0; i < n; i++ ) {
            if ( i == i0 || i == i1 ) continue;

            const auto r = circumradius(
                i0x, i0y, i1x, i1y, coords[ 3 * i ], coords[ 3 * i + 1 ] );

            if ( r < min_radius ) {
                i2 = i;
                min_radius = r;
            }
        }

        if ( !( min_radius < std::numeric_limits<double>::max( ) ) ) {
            throw std::runtime_error( "not triangulation" );
        }

        auto i2x = coords[ 3 * i2 ];
        auto i2y = coords[ 3 * i2 + 1 ];

        if ( orient( i0x, i0y, i1x, i1y, i2x, i2y ) ) {
            std::swap( i1, i2 );
            std::swap( i1x, i2x );
            std::swap( i1y, i2y );
        }

        std::tie( m_center_x, m_center_y ) = circumcenter( i0x, i0y, i1x, i1y, i2x, i2y );

        // sort the points by distance from the seed triangle circumcenter
        std::sort( ids.begin( ), ids.end( ), compare{ coords, m_center_x, m_center_y } );

        // initialize a hash table for storing edges of the advancing convex hull
        m_hash_size = static_cast< std::size_t >( std::llround( std::ceil( std::sqrt( n ) ) ) );
        m_hash.resize( m_hash_size );
        std::fill( m_hash.begin( ), m_hash.end( ), INVALID_INDEX );

        // initialize arrays for tracking the edges of the advancing convex hull
        hull_prev.resize( n );
        hull_next.resize( n );
        hull_tri.resize( n );

        hull_start = i0;

        size_t hull_size = 3;

        hull_next[ i0 ] = hull_prev[ i2 ] = i1;
        hull_next[ i1 ] = hull_prev[ i0 ] = i2;
        hull_next[ i2 ] = hull_prev[ i1 ] = i0;

        hull_tri[ i0 ] = 0;
        hull_tri[ i1 ] = 1;
        hull_tri[ i2 ] = 2;

        m_hash[ hash_key( i0x, i0y ) ] = i0;
        m_hash[ hash_key( i1x, i1y ) ] = i1;
        m_hash[ hash_key( i2x, i2y ) ] = i2;

        auto max_triangles = n < 3 ? 1 : 2 * n - 5;
        triangles.reserve( max_triangles * 3 );
        halfedges.reserve( max_triangles * 3 );
        add_triangle( i0, i1, i2, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX );
        auto xp = std::numeric_limits<double>::quiet_NaN( );
        auto yp = std::numeric_limits<double>::quiet_NaN( );
        for ( std::size_t k = 0; k < n; k++ ) {
            const auto i = ids[ k ];
            const auto x = coords[ 3 * i ];
            const auto y = coords[ 3 * i + 1 ];

            // skip near-duplicate points
            if ( k > 0 && check_pts_equal( x, y, xp, yp ) ) continue;
            xp = x;
            yp = y;

            // skip seed triangle points
            if (
                check_pts_equal( x, y, i0x, i0y ) ||
                check_pts_equal( x, y, i1x, i1y ) ||
                check_pts_equal( x, y, i2x, i2y ) ) continue;

            // find a visible edge on the convex hull using edge hash
            std::size_t start = 0;

            auto key = hash_key( x, y );
            for ( size_t j = 0; j < m_hash_size; j++ ) {
                start = m_hash[ fast_mod( key + j, m_hash_size ) ];
                if ( start != INVALID_INDEX && start != hull_next[ start ] ) break;
            }

            start = hull_prev[ start ];
            auto e = start;
            size_t q;

            while ( q = hull_next[ e ], !orient( x, y, coords[ 3 * e ], coords[ 3 * e + 1 ], coords[ 3 * q ], coords[ 3 * q + 1 ] ) ) { //TODO: does it works in a same way as in JS
                e = q;
                if ( e == start ) {
                    e = INVALID_INDEX;
                    break;
                }
            }

            if ( e == INVALID_INDEX ) continue; // likely a near-duplicate point; skip it

            // add the first triangle from the point
            auto t = add_triangle(
                e,
                i,
                hull_next[ e ],
                INVALID_INDEX,
                INVALID_INDEX,
                hull_tri[ e ] );

            hull_tri[ i ] = legalize( t + 2 );
            hull_tri[ e ] = t;
            hull_size++;

            // walk forward through the hull, adding more triangles and flipping recursively
            auto next = hull_next[ e ];
            while (
                q = hull_next[ next ],
                orient( x, y, coords[ 3 * next ], coords[ 3 * next + 1 ], coords[ 3 * q ], coords[ 3 * q + 1 ] ) ) {
                t = add_triangle( next, i, q, hull_tri[ i ], INVALID_INDEX, hull_tri[ next ] );
                hull_tri[ i ] = legalize( t + 2 );
                hull_next[ next ] = next; // mark as removed
                hull_size--;
                next = q;
            }

            // walk backward from the other side, adding more triangles and flipping
            if ( e == start ) {
                while (
                    q = hull_prev[ e ],
                    orient( x, y, coords[ 3 * q ], coords[ 3 * q + 1 ], coords[ 3 * e ], coords[ 3 * e + 1 ] ) ) {
                    t = add_triangle( q, i, e, INVALID_INDEX, hull_tri[ e ], hull_tri[ q ] );
                    legalize( t + 2 );
                    hull_tri[ q ] = t;
                    hull_next[ e ] = e; // mark as removed
                    hull_size--;
                    e = q;
                }
            }

            // update the hull indices
            hull_prev[ i ] = e;
            hull_start = e;
            hull_prev[ next ] = i;
            hull_next[ e ] = i;
            hull_next[ i ] = next;

            m_hash[ hash_key( x, y ) ] = i;
            m_hash[ hash_key( coords[ 3 * e ], coords[ 3 * e + 1 ] ) ] = e;
        }
    }

    double Delaunator::get_hull_area( ) {
        std::vector<double> hull_area;
        auto e = hull_start;
        do {
            hull_area.push_back( ( coords[ 3 * e ] - coords[ 3 * hull_prev[ e ] ] ) * ( coords[ 3 * e + 1 ] + coords[ 3 * hull_prev[ e ] + 1 ] ) );
            e = hull_next[ e ];
        } while ( e != hull_start );
        return sum( hull_area );
    }

    std::size_t Delaunator::legalize( std::size_t a ) {
        std::size_t i = 0;
        std::size_t ar = 0;
        m_edge_stack.clear( );

        // recursion eliminated with a fixed-size stack
        while ( true ) {
            const auto b = halfedges[ a ];

            /* if the pair of triangles doesn't satisfy the Delaunay condition
            * (p1 is inside the circumcircle of [p0, pl, pr]), flip them,
            * then do the same check/flip recursively for the new pair of triangles
            *
            *           pl                    pl
            *          /||\                  /  \
            *       al/ || \bl            al/    \a
            *        /  ||  \              /      \
            *       /  a||b  \    flip    /___ar___\
            *     p0\   ||   /p1   =>   p0\---bl---/p1
            *        \  ||  /              \      /
            *       ar\ || /br             b\    /br
            *          \||/                  \  /
            *           pr                    pr
            */
            const auto a0 = 3 * ( a / 3 );
            ar = a0 + ( a + 2 ) % 3;

            if ( b == INVALID_INDEX ) {
                if ( i > 0 ) {
                    i--;
                    a = m_edge_stack[ i ];
                    continue;
                } else {
                    //i = INVALID_INDEX;
                    break;
                }
            }

            const auto b0 = 3 * ( b / 3 );
            const auto al = a0 + ( a + 1 ) % 3;
            const auto bl = b0 + ( b + 2 ) % 3;

            const auto p0 = triangles[ ar ];
            const auto pr = triangles[ a ];
            const auto pl = triangles[ al ];
            const auto p1 = triangles[ bl ];

            const auto illegal = in_circle(
                coords[ 3 * p0 ],
                coords[ 3 * p0 + 1 ],
                coords[ 3 * pr ],
                coords[ 3 * pr + 1 ],
                coords[ 3 * pl ],
                coords[ 3 * pl + 1 ],
                coords[ 3 * p1 ],
                coords[ 3 * p1 + 1 ] );

            if ( illegal ) {
                triangles[ a ] = p1;
                triangles[ b ] = p0;

                const auto hbl = halfedges[ bl ];

                // edge swapped on the other side of the hull (rare); fix the halfedge reference
                if ( hbl == INVALID_INDEX ) {
	                auto e = hull_start;
                    do {
                        if ( hull_tri[ e ] == bl ) {
                            hull_tri[ e ] = a;
                            break;
                        }
                        e = hull_next[ e ];
                    } while ( e != hull_start );
                }
                link( a, hbl );
                link( b, halfedges[ ar ] );
                link( ar, bl );
                auto br = b0 + ( b + 1 ) % 3;

                if ( i < m_edge_stack.size( ) ) {
                    m_edge_stack[ i ] = br;
                } else {
                    m_edge_stack.push_back( br );
                }
                i++;

            } else {
                if ( i > 0 ) {
                    i--;
                    a = m_edge_stack[ i ];
                    continue;
                } else {
                    break;
                }
            }
        }
        return ar;
    }

    inline std::size_t Delaunator::hash_key( const double x, const double y ) const {
        const auto dx = x - m_center_x;
        const auto dy = y - m_center_y;
        return fast_mod(
            static_cast< std::size_t >( std::llround( std::floor( pseudo_angle( dx, dy ) * static_cast< double >( m_hash_size ) ) ) ),
            m_hash_size );
    }

    std::size_t Delaunator::add_triangle(
        std::size_t i0,
        std::size_t i1,
        std::size_t i2,
        std::size_t a,
        std::size_t b,
        std::size_t c ) {
	    const auto t = triangles.size( );
        triangles.push_back( i0 );
        triangles.push_back( i1 );
        triangles.push_back( i2 );
        link( t, a );
        link( t + 1, b );
        link( t + 2, c );
        return t;
    }

    void Delaunator::link( const std::size_t a, const std::size_t b ) {
	    const auto s = halfedges.size( );
        if ( a == s ) {
            halfedges.push_back( b );
        } else if ( a < s ) {
            halfedges[ a ] = b;
        } else {
            throw std::runtime_error( "Cannot link edge" );
        }
        if ( b != INVALID_INDEX ) {
	        const auto s2 = halfedges.size( );
            if ( b == s2 ) {
                halfedges.push_back( a );
            } else if ( b < s2 ) {
                halfedges[ b ] = a;
            } else {
                throw std::runtime_error( "Cannot link edge" );
            }
        }
    }

} //namespace delaunator