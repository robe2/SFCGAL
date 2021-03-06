/**
 *   SFCGAL
 *
 *   Copyright (C) 2012-2013 Oslandia <infos@oslandia.com>
 *   Copyright (C) 2012-2013 IGN (http://www.ign.fr)
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.

 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <SFCGAL/io/asc.h>

#include <boost/format.hpp>

#include <SFCGAL/Exception.h>
#include <SFCGAL/detail/tools/InputStreamReader.h>

using namespace SFCGAL::detail;

namespace SFCGAL {
namespace io {

///
///
///
std::auto_ptr< Grid > readASC( std::istream& s )
{
    tools::InputStreamReader reader( s );

    //ncols         4
    size_t ncols ;

    if ( ! reader.match( "ncols" ) || ! reader.read( ncols ) ) {
        BOOST_THROW_EXCEPTION( Exception( "expecting ncols <ncols>" ) );
    }

    //nrows         6
    size_t nrows ;

    if ( ! reader.match( "nrows" ) || ! reader.read( nrows ) ) {
        BOOST_THROW_EXCEPTION( Exception( "expecting nrows <nrows>" ) );
    }

    //xllcorner     0.0
    double xllcorner ;

    if ( ! reader.match( "xllcorner" ) || ! reader.read( xllcorner ) ) {
        BOOST_THROW_EXCEPTION( Exception( "expecting xllcorner <xllcorner>" ) );
    }

    //yllcorner     0.0
    double yllcorner ;

    if ( ! reader.match( "yllcorner" ) || ! reader.read( yllcorner ) ) {
        BOOST_THROW_EXCEPTION( Exception( "expecting yllcorner <yllcorner>" ) );
    }

    //cellsize      50.0
    double cellsize ;

    if ( ! reader.match( "cellsize" ) || ! reader.read( cellsize ) ) {
        BOOST_THROW_EXCEPTION( Exception( "expecting cellsize <cellsize>" ) );
    }

    //NODATA_value  -9999
    double NODATA_value ;

    if ( ! reader.match( "NODATA_value" ) || ! reader.read( NODATA_value ) ) {
        BOOST_THROW_EXCEPTION( Exception( "expecting NODATA_value <NODATA_value>" ) );
    }

    /*
     * a matrix and read data
     */
    ublas::matrix< double > data( nrows, ncols );

    //-9999 -9999 5 2
    //-9999 20 100 36
    //3 8 35 10
    //32 42 50 6
    //88 75 27 9
    //13 5 1 -9999
    for ( size_t i = 0; i < nrows; i++ ) {
        for ( size_t j = 0; j < ncols; j++ ) {
            double value ;

            if ( ! reader.read( value ) ) {
                BOOST_THROW_EXCEPTION( Exception(
                                           ( boost::format( "fail to read data(%s,%s) value" ) % i % j ).str()
                                       ) );
            }

            if ( value == NODATA_value ) {
                data( i,j ) = NaN() ;
            }
            else {
                data( i,j ) = value ;
            }
        }
    }

    /*
     * produce the resulting grid
     */
    Envelope limits(
        xllcorner, xllcorner + cellsize * ncols,
        yllcorner, yllcorner + cellsize * nrows
    );
    return std::auto_ptr< Grid >( new Grid( data, limits, PIXEL_IS_AREA ) ) ;
}


} // namespace io
} // namespace SFCGAL

