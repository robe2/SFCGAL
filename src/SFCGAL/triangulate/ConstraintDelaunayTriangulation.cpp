/**
 *   SFCGAL
 *
 *   Copyright (C) 2012-2013 Oslandia <contact@oslandia.com>
 *   Copyright (C) 2012-2013 IGN (http://www.ign.fr)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <SFCGAL/triangulate/ConstraintDelaunayTriangulation.h>

#include <SFCGAL/Exception.h>
#include <SFCGAL/TriangulatedSurface.h>

namespace SFCGAL {
namespace triangulate {

///
///
///
ConstraintDelaunayTriangulation::ConstraintDelaunayTriangulation():
	_cdt()
{

}


///
///
///
ConstraintDelaunayTriangulation::Vertex_handle ConstraintDelaunayTriangulation::addVertex( const Coordinate & position )
{
	if ( position.isEmpty() ){
		BOOST_THROW_EXCEPTION(Exception(
			"try to add empty position to ConstraintDelaunayTriangulation"
		));
	}

	if ( _projectionPlane ){
		Vertex_handle vertex = _cdt.insert( _projectionPlane->to_2d( position.toPoint_3() ) );
		vertex->info().original = position ;
		return vertex ;
	}else{
		Vertex_handle vertex = _cdt.insert( position.toPoint_2() );
		vertex->info().original = position ;
		return vertex ;
	}
}

///
///
///
void  ConstraintDelaunayTriangulation::addConstraint( Vertex_handle source, Vertex_handle target )
{
	if ( source == target ){
		return ;
	}
	_cdt.insert_constraint( source, target );
}

///
///
///
void ConstraintDelaunayTriangulation::clear()
{
	_cdt.clear();
}

///
///
///
size_t	ConstraintDelaunayTriangulation::numVertices() const
{
	return _cdt.number_of_vertices() ;
}

///
///
///
size_t	ConstraintDelaunayTriangulation::numTriangles() const
{
	return _cdt.number_of_faces() ;
}

///
///
///
void ConstraintDelaunayTriangulation::setProjectionPlane( const Kernel::Plane_3 & projectionPlane )
{
	BOOST_ASSERT( ! projectionPlane.is_degenerate() ) ;
	_projectionPlane = projectionPlane ;
}

///
///
///
Kernel::Plane_3 ConstraintDelaunayTriangulation::projectionPlane() const
{
	if ( _projectionPlane ){
		return *_projectionPlane ;
	}else{
		return Kernel::Plane_3( Kernel::RT(0), Kernel::RT(0), Kernel::RT(1), Kernel::RT(0) );
	}
}


///
/// adapted from CGAL example
///
void ConstraintDelaunayTriangulation::markDomains()
{
	for ( CDT::All_faces_iterator it = _cdt.all_faces_begin(); it != _cdt.all_faces_end(); ++it ) {
		it->info().nestingLevel = -1;
	}

	int index = 0;
	std::list< CDT::Edge > border;
	_markDomains( _cdt.infinite_face(), index++, border);
	while ( ! border.empty() ) {
		CDT::Edge e = border.front();
		border.pop_front();
		CDT::Face_handle n = e.first->neighbor(e.second);
		if ( n->info().nestingLevel == -1 ) {
			_markDomains( n, e.first->info().nestingLevel + 1, border );
		}
	}
}

///
/// adapted from CGAL example
///
///explore set of facets connected with non constrained edges,
///and attribute to each such set a nesting level.
///We start from facets incident to the infinite vertex, with a nesting
///level of 0. Then we recursively consider the non-explored facets incident
///to constrained edges bounding the former set and increase the nesting level by 1.
///Facets in the domain are those with an odd nesting level.
void ConstraintDelaunayTriangulation::_markDomains(
	CDT::Face_handle start,
	int index,
	std::list< CDT::Edge >& border
)
{
	if (start->info().nestingLevel != -1) {
		return;
	}
	std::list< CDT::Face_handle > queue;
	queue.push_back(start);

	while (!queue.empty()) {
		CDT::Face_handle fh = queue.front();
		queue.pop_front();
		if (fh->info().nestingLevel == -1) {
			fh->info().nestingLevel = index;
			for (int i = 0; i < 3; i++) {
				CDT::Edge e(fh, i);
				CDT::Face_handle n = fh->neighbor(i);
				if (n->info().nestingLevel == -1) {
					if ( _cdt.is_constrained(e) )
						border.push_back(e);
					else
						queue.push_back(n);
				}
			}
		}
	}
}


///
///
///
void ConstraintDelaunayTriangulation::getTriangles( TriangulatedSurface & triangulatedSurface, bool filterExteriorParts ) const
{
	triangulatedSurface.reserve( triangulatedSurface.numTriangles() + numTriangles() );
	for ( Finite_faces_iterator it = finite_faces_begin(); it != finite_faces_end(); ++it )
	{
		if ( filterExteriorParts && ( it->info().nestingLevel % 2 == 0 ) ){
			continue ;
		}
		const Coordinate & a = it->vertex(0)->info().original ;
		const Coordinate & b = it->vertex(1)->info().original ;
		const Coordinate & c = it->vertex(2)->info().original ;

		// check that vertex has an original vertex
		if ( a.isEmpty() || b.isEmpty() || c.isEmpty() ){
			BOOST_THROW_EXCEPTION( Exception(
				( boost::format("Can't convert Triangulation to TriangulatedSurface (constraint intersection found)") ).str()
			) ) ;
		}

		triangulatedSurface.addTriangle( new Triangle( Point(a), Point(b), Point(c) ) );
	}
}

///
///
///
std::auto_ptr< TriangulatedSurface > ConstraintDelaunayTriangulation::getTriangulatedSurface( bool filterExteriorParts ) const
{
	std::auto_ptr< TriangulatedSurface > result( new TriangulatedSurface );
	getTriangles(*result, false);
	return result ;
}


} // namespace triangulate
} // namespace SFCGAL

