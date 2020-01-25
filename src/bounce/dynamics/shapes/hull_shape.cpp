/*
* Copyright (c) 2016-2019 Irlan Robson https://irlanrobson.github.io
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <bounce/dynamics/shapes/hull_shape.h>
#include <bounce/dynamics/time_step.h>
#include <bounce/common/template/array.h>
#include <bounce/collision/shapes/hull.h>
#include <bounce/collision/gjk/gjk.h>
#include <bounce/collision/gjk/gjk_proxy.h>

b3HullShape::b3HullShape()
{
	m_type = e_hullShape;
	m_radius = B3_HULL_RADIUS;
	m_hull = nullptr;
}

b3HullShape::~b3HullShape()
{
}

void b3HullShape::Swap(const b3HullShape& other)
{
	m_radius = other.m_radius;
	m_hull = other.m_hull;
}

void b3HullShape::ComputeMass(b3MassData* massData, scalar density) const
{
	// M. Kallay - "Computing the Moment of Inertia of a Solid Defined by a Triangle Mesh"
	// https://github.com/erich666/jgt-code/blob/master/Volume_11/Number_2/Kallay2006/Moment_of_Inertia.cpp
	
	// Polyhedron mass, center of mass, and inertia.
	// Let rho be the polyhedron density per unit volume

	// mass = rho * int(1 * dV) 

	// centroid.x = (1 / mass) * rho * int(x * dV)
	// centroid.y = (1 / mass) * rho * int(y * dV)
	// centroid.z = (1 / mass) * rho * int(z * dV)

	// Ixx = rho * int((y^2 + z^2) * dV) 
	// Iyy = rho * int((x^2 + z^2) * dV)
	// Izz = rho * int((x^2 + y^2) * dV)
	
	// Ixy = -rho * int((x * y) * dV)
	// Ixz = -rho * int((x * z) * dV)
	// Iyz = -rho * int((y * z) * dV)

	// Iyx = Ixy
	// Izx = Ixz
	// Izy = Iyz
	B3_ASSERT(m_hull->vertexCount >= 4);

	// Put the hull relative to a point that is inside the hull 
	// to help reducing round-off errors.
	b3Vec3 s; s.SetZero();
	for (u32 i = 0; i < m_hull->vertexCount; ++i)
	{
		s += m_hull->vertices[i];
	}
	s /= scalar(m_hull->vertexCount);

	scalar volume = scalar(0);

	b3Vec3 center; center.SetZero();
	
	scalar xx = scalar(0);
	scalar xy = scalar(0);
	scalar yy = scalar(0);
	scalar xz = scalar(0);
	scalar zz = scalar(0);
	scalar yz = scalar(0);

	for (u32 i = 0; i < m_hull->faceCount; ++i)
	{
		const b3Face* face = m_hull->GetFace(i);
		const b3HalfEdge* begin = m_hull->GetEdge(face->edge);

		const b3HalfEdge* edge = m_hull->GetEdge(begin->next);
		do
		{
			const b3HalfEdge* next = m_hull->GetEdge(edge->next);
			
			u32 i1 = begin->origin;
			u32 i2 = edge->origin;
			u32 i3 = next->origin;

			b3Vec3 v1 = m_hull->GetVertex(i1) - s;
			b3Vec3 v2 = m_hull->GetVertex(i2) - s;
			b3Vec3 v3 = m_hull->GetVertex(i3) - s;

			// Signed tetrahedron volume
			scalar D = b3Det(v1, v2, v3);

			// Contribution to the mass
			volume += D;

			// Contribution to the centroid
			b3Vec3 v4 = v1 + v2 + v3;
			
			center += D * v4;

			// Contribution to moment of inertia monomials
			xx += D * (v1.x * v1.x + v2.x * v2.x + v3.x * v3.x + v4.x * v4.x);
			yy += D * (v1.y * v1.y + v2.y * v2.y + v3.y * v3.y + v4.y * v4.y);
			zz += D * (v1.z * v1.z + v2.z * v2.z + v3.z * v3.z + v4.z * v4.z);
			xy += D * (v1.x * v1.y + v2.x * v2.y + v3.x * v3.y + v4.x * v4.y);
			xz += D * (v1.x * v1.z + v2.x * v2.z + v3.x * v3.z + v4.x * v4.z);
			yz += D * (v1.y * v1.z + v2.y * v2.z + v3.y * v3.z + v4.y * v4.z);

			edge = next;
		} while (m_hull->GetEdge(edge->next) != begin);
	}

	b3Mat33 I;

	I.x.x = yy + zz;
	I.x.y = -xy;
	I.x.z = -xz;

	I.y.x = -xy;
	I.y.y = xx + zz;
	I.y.z = -yz;

	I.z.x = -xz;
	I.z.y = -yz;
	I.z.z = xx + yy;
	
	// Total mass
	massData->mass = density * volume / 6.0f;
	
	// Center of mass
	B3_ASSERT(volume > B3_EPSILON);
	center /= scalar(4) * volume;
	massData->center = center + s;

	// Inertia relative to the local origin (s).
	massData->I = (density / scalar(120)) * I;
	
	// Shift the inertia to center of mass then to the body origin.
	// Ib = Ic - m * c^2 + m * m.c^2
	// Simplification:
	// Ib = Ic + m * (m.c^2 - c^2)
	massData->I += massData->mass * (b3Steiner(massData->center) - b3Steiner(center));
}

void b3HullShape::ComputeAABB(b3AABB* aabb, const b3Transform& xf) const
{
	aabb->Set(m_hull->vertices, m_hull->vertexCount, xf);
	aabb->Extend(m_radius);
}

bool b3HullShape::TestSphere(const b3Sphere& sphere, const b3Transform& xf) const
{
	b3GJKProxy proxy1;
	proxy1.vertexCount = m_hull->vertexCount;
	proxy1.vertices = m_hull->vertices;

	b3GJKProxy proxy2;
	proxy2.vertexBuffer[0] = b3MulT(xf, sphere.vertex);
	proxy2.vertexCount = 1;
	proxy2.vertices = proxy2.vertexBuffer;

	b3GJKOutput gjk = b3GJK(b3Transform_identity, proxy1, b3Transform_identity, proxy2, false);

	if (gjk.distance <= m_radius + sphere.radius)
	{
		return true;
	}

	return false;
}

bool b3HullShape::TestSphere(b3TestSphereOutput* output, const b3Sphere& sphere, const b3Transform& xf) const
{
	scalar radius = m_radius + sphere.radius;
	const b3Hull* hull = m_hull;

	// Sphere center in the frame of the hull.
	b3Vec3 cLocal = b3MulT(xf, sphere.vertex);

	// Find the minimum separation face.	
	u32 faceIndex = 0;
	scalar separation = -B3_MAX_SCALAR;

	for (u32 i = 0; i < hull->faceCount; ++i)
	{
		b3Plane plane = hull->GetPlane(i);
		scalar s = b3Distance(cLocal, plane);

		if (s > radius)
		{
			// Early out.
			return false;
		}

		if (s > separation)
		{
			faceIndex = i;
			separation = s;
		}
	}

	if (separation < scalar(0))
	{
		// The center is inside the hull.
		b3Plane plane = b3Mul(xf, m_hull->GetPlane(faceIndex));

		output->point = b3ClosestPointOnPlane(sphere.vertex, plane);
		output->normal = plane.normal;	
		return true;
	}

	// Vertices that subtend the incident face.
	b3StackArray<b3Vec3, 64> incidentPolygon;

	const b3Face* face = hull->GetFace(faceIndex);
	const b3HalfEdge* begin = hull->GetEdge(face->edge);
	const b3HalfEdge* edge = begin;
	do
	{
		b3Vec3 vertex = hull->GetVertex(edge->origin);
		incidentPolygon.PushBack(vertex);
		edge = hull->GetEdge(edge->next);
	} while (edge != begin);

	b3GJKProxy proxy1;
	proxy1.vertexCount = incidentPolygon.Count();
	proxy1.vertices = incidentPolygon.Begin();

	b3GJKProxy proxy2;
	proxy2.vertexBuffer[0] = sphere.vertex;
	proxy2.vertexCount = 1;
	proxy2.vertices = proxy2.vertexBuffer;

	b3Transform xf1 = xf;
	b3Transform xf2; xf2.SetIdentity();

	b3GJKOutput gjk = b3GJK(xf1, proxy1, xf2, proxy2, false);

	if (gjk.distance > radius)
	{
		return false;
	}

	if (gjk.distance > scalar(0))
	{
		b3Vec3 c1 = gjk.point1;
		b3Vec3 c2 = gjk.point2;
		b3Vec3 normal = (c2 - c1) / gjk.distance;

		output->point = c1;
		output->normal = normal;
		return true;
	}

	return false;
}

bool b3HullShape::RayCast(b3RayCastOutput* output, const b3RayCastInput& input, const b3Transform& xf) const
{
	u32 planeCount = m_hull->faceCount;
	const b3Plane* planes = m_hull->planes;

	// Put the segment into the poly's frame of reference.
	b3Vec3 p1 = b3MulC(xf.rotation, input.p1 - xf.translation);
	b3Vec3 p2 = b3MulC(xf.rotation, input.p2 - xf.translation);
	b3Vec3 d = p2 - p1;

	scalar lower = scalar(0);
	scalar upper = input.maxFraction;

	u32 index = B3_MAX_U32;

	// s(lower) = p1 + lower * d, 0 <= lower <= kupper
	// The segment intersects the plane if a 'lower' exists
	// for which s(lower) is inside all half-spaces.

	// Solve line segment to plane:
	// dot(n, s(lower)) = offset
	// dot(n, p1 + lower * d) = offset
	// dot(n, p1) + dot(n, lower * d) = offset
	// dot(n, p1) + lower * dot(n, d) = offset
	// lower * dot(n, d) = offset - dot(n, p1)
	// lower = (offset - dot(n, p1)) / dot(n, d)

	for (u32 i = 0; i < planeCount; ++i)
	{
		scalar numerator = planes[i].offset - b3Dot(planes[i].normal, p1);
		scalar denominator = b3Dot(planes[i].normal, d);

		if (denominator == scalar(0))
		{
			// s is parallel to this half-space.
			if (numerator < scalar(0))
			{
				// s is outside of this half-space.
				// dot(n, p1) and dot(n, p2) < 0.
				return false;
			}
		}
		else
		{
			// Original predicates:
			// lower < numerator / denominator, for denominator < 0
			// upper < numerator / denominator, for denominator < 0
			// Optimized predicates:
			// lower * denominator > numerator
			// upper * denominator > numerator
			if (denominator < scalar(0))
			{
				// s enters this half-space.
				if (numerator < lower * denominator)
				{
					// Increase lower.
					lower = numerator / denominator;
					index = i;
				}
			}
			else
			{
				// s exits the half-space.	
				if (numerator < upper * denominator)
				{
					// Decrease upper.
					upper = numerator / denominator;
				}
			}

			// Exit if intersection becomes empty.
			if (upper < lower)
			{
				return false;
			}
		}
	}

	B3_ASSERT(lower >= scalar(0) && lower <= input.maxFraction);

	if (index != B3_MAX_U32)
	{
		output->fraction = lower;
		output->normal = b3Mul(xf.rotation, planes[index].normal);
		return true;
	}

	return false;
}
