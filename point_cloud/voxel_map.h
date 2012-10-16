// This file is part of snark, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// snark is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// snark is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with snark. If not, see <http://www.gnu.org/licenses/>.

#ifndef SNARK_POINT_CLOUD_VOXELMAP_H
#define SNARK_POINT_CLOUD_VOXELMAP_H

#include <boost/array.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <Eigen/Core>
#include <comma/base/types.h>

namespace snark {

/// quick and dirty hash for array-like containers (its support is awkward in boost)
/// @todo if we have a second use case, move to ark::containers... i guess...
template < typename Array, std::size_t size_type >
struct array_hash : public std::unary_function< Array, std::size_t >
{
    std::size_t operator()( Array const& array ) const
    {
        std::size_t seed = 0;
        for( std::size_t i = 0; i < size_type; ++i ) { boost::hash_combine( seed, array[i] ); }
        return seed;
        // return boost::hash_range( &array[0], &array[size_type] ); // not so easy...
    }
};

/// unordered voxel map
///
/// it may be a much better choice than voxel grid, whenever
/// operations on a voxel do not depend on its neighbours,
/// and also when the grid extents are not known beforehand
template < typename V, unsigned int D, typename P = Eigen::Matrix< double, D, 1 > >
class voxel_map : public boost::unordered_map< boost::array< int, D >, V, snark::array_hash< boost::array< int, D >, D > >
{
    public:
        /// number of dimensions
        enum { dimensions = D };
        
        /// voxel type
        typedef V voxel_type;
        
        /// point type
        typedef P point_type;
        
        /// index type
        typedef boost::array< comma::int32, D > index_type;
        
        /// base class type
        typedef boost::unordered_map< index_type, voxel_type, snark::array_hash< index_type, D > > base;
        
        /// iterator type (otherwise it does not build on windows...)
        typedef typename base::iterator iterator;
        
        /// const iterator type (otherwise it does not build on windows...)
        typedef typename base::const_iterator const_iterator;

        /// constructor
        voxel_map( const point_type& origin, const point_type& resolution );
        
        /// constructor for default origin of all zeroes
        voxel_map( const point_type& resolution );
        
        /// add voxel at the given point, if it does not exist
        iterator touch_at( const point_type& point );
        
        /// add voxel at the given point, if it does not exist
        std::pair< iterator, bool > insert( const point_type& point, const voxel_type& voxel );
        
        /// return index of the point, always rounds it down (does floor for given resolution)
        index_type index_of( const point_type& point ) const;
        
        /// same as index_of( point ), but static
        static index_type index_of( const point_type& point, const point_type& origin, const point_type& resolution );
        
        /// same as index_of( point ), but static
        static index_type index_of( const point_type& point, const point_type& resolution );
        
        /// find voxel
        iterator find( const point_type& point );
        
        /// find voxel
        const_iterator find( const point_type& point ) const;
        
        /// return origin
        const point_type& origin() const;
        
        /// return resolution
        const point_type& resolution() const;

    private:
        point_type origin_;
        point_type resolution_;
};

template < typename V, unsigned int D, typename P >
inline voxel_map< V, D, P >::voxel_map( const typename voxel_map< V, D, P >::point_type& origin, const typename voxel_map< V, D, P >::point_type& resolution )
    : origin_( origin )
    , resolution_( resolution )
{
}

template < typename V, unsigned int D, typename P >
inline voxel_map< V, D, P >::voxel_map( const typename voxel_map< V, D, P >::point_type& resolution )
    : origin_( point_type::Zero() ) // todo: use traits, if decoupling from eigen required
    , resolution_( resolution )
{
}

template < typename V, unsigned int D, typename P >
inline typename voxel_map< V, D, P >::iterator voxel_map< V, D, P >::touch_at( const typename voxel_map< V, D, P >::point_type& point )
{
    index_type index = index_of( point );
    iterator it = this->base::find( index );
    if( it != this->end() ) { return it; }
    return this->base::insert( std::make_pair( index, voxel_type() ) ).first;
}

template < typename V, unsigned int D, typename P >
inline std::pair< typename voxel_map< V, D, P >::iterator, bool > voxel_map< V, D, P >::insert( const typename voxel_map< V, D, P >::point_type& point, const typename voxel_map< V, D, P >::voxel_type& voxel )
{
    return this->base::insert( std::make_pair( index_of( point ), voxel ) );
}

namespace impl {

static int negative_flooring_ = static_cast< int >( -1.5 ) == -1 ? -1 : static_cast< int >( -1.5 ) == -2 ? 0 : 0;
static int positive_flooring_ = static_cast< int >( 1.5 ) == 1 ? 0 : static_cast< int >( 1.5 ) == 2 ? -1 : -1;

} // namespace impl {

template < typename V, unsigned int D, typename P >
inline typename voxel_map< V, D, P >::index_type voxel_map< V, D, P >::index_of( const typename voxel_map< V, D, P >::point_type& point, const typename voxel_map< V, D, P >::point_type& origin, const typename voxel_map< V, D, P >::point_type& resolution )
{
    point_type diff = ( point - origin ).array() / resolution.array();
    index_type index;
    for( unsigned int i = 0; i < dimensions; ++i )
    {
        int d = diff[i];
        index[i] = d;
        if( diff[i] == d ) { continue; }
        index[i] += diff[i] < 0 ? impl::negative_flooring_ : ( d == 0 ? 0 : impl::positive_flooring_ );
    }
    return index;
}

template < typename V, unsigned int D, typename P >
inline typename voxel_map< V, D, P >::index_type voxel_map< V, D, P >::index_of( const typename voxel_map< V, D, P >::point_type& point, const typename voxel_map< V, D, P >::point_type& resolution )
{
    return index_of( point, point_type::Zero(), resolution );
}

template < typename V, unsigned int D, typename P >
inline typename voxel_map< V, D, P >::index_type voxel_map< V, D, P >::index_of( const typename voxel_map< V, D, P >::point_type& point ) const
{
    return index_of( point, origin_, resolution_ );
}

template < typename V, unsigned int D, typename P >
inline typename voxel_map< V, D, P >::iterator voxel_map< V, D, P >::find( const typename voxel_map< V, D, P >::point_type& point )
{
    index_type i = index_of( point );
    return this->base::find( i );
}

template < typename V, unsigned int D, typename P >
inline typename voxel_map< V, D, P >::const_iterator voxel_map< V, D, P >::find( const typename voxel_map< V, D, P >::point_type& point ) const
{
    index_type i = index_of( point );
    return this->base::find( i );
}

template < typename V, unsigned int D, typename P >
inline const typename voxel_map< V, D, P >::point_type& voxel_map< V, D, P >::origin() const { return origin_; }

template < typename V, unsigned int D, typename P >
inline const typename voxel_map< V, D, P >::point_type& voxel_map< V, D, P >::resolution() const { return resolution_; }

} // namespace snark {

#endif // SNARK_POINT_CLOUD_VOXELMAP_H