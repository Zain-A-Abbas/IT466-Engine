#include "Collision.h"
#include "gfc_types.h"
#include "Entity.h"
#include "simple_logger.h"




void setCapsuleFinalBase(GFC_Capsule* c, Entity* ent) {
	if (ent) {
		c->finalBase = entityGlobalPosition(ent);
	}
	else {
		slog("No entity provided to capsule final base");
	}
}

void setCapsuleFinalTip(GFC_Capsule* c, Entity* ent) {
	GFC_Vector3D finalTip = { 0, 0, c->height };
	if (ent) {
		finalTip = gfc_vector3d_multiply(finalTip, entityGlobalScale(ent));
		GFC_Vector3D entityRotation = entityGlobalRotation(ent);
		gfc_vector3d_rotate_about_x(&finalTip, c->rotation_x);
		gfc_vector3d_rotate_about_x(&finalTip, entityRotation.x);
		gfc_vector3d_rotate_about_z(&finalTip, c->rotation_z);
		gfc_vector3d_rotate_about_z(&finalTip, entityRotation.z);
	}
	else {
		gfc_vector3d_rotate_about_x(&finalTip, c->rotation_x);
		gfc_vector3d_rotate_about_z(&finalTip, c->rotation_z);
	}
	finalTip = gfc_vector3d_added(finalTip, c->finalBase);
	c->finalTip.x = finalTip.x;
	c->finalTip.y = finalTip.y;
	c->finalTip.z = finalTip.z;
}
//SOURCE: https://wickedengine.net/2020/04/capsule-collision-detection
//ACCESSED: 11/10/2024

short capsuleToCapsuleTest(GFC_Capsule a, GFC_Capsule b, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth) {
	GFC_Vector3D baseA = a.finalBase;
	GFC_Vector3D baseB = b.finalBase;
	GFC_Vector3D tipA = a.finalTip;
	GFC_Vector3D tipB = b.finalTip;

	// Gets the spheres at each end of the capsules
	GFC_Vector3D aNormal = gfc_vector3d_subbed(tipA, baseA);
	gfc_vector3d_normalize(&aNormal);
	GFC_Vector3D aLineOffset = gfc_vector3d_multiply(aNormal, gfc_vector3d(a.radius, a.radius, a.radius));
	GFC_Vector3D aA = gfc_vector3d_added(baseA, aLineOffset);
	GFC_Vector3D aB = gfc_vector3d_subbed(tipA, aLineOffset);

	GFC_Vector3D bNormal = gfc_vector3d_subbed(tipB, baseB);
	gfc_vector3d_normalize(&bNormal);
	GFC_Vector3D bLineOffset = gfc_vector3d_multiply(bNormal, gfc_vector3d(b.radius, b.radius, b.radius));
	GFC_Vector3D bA = gfc_vector3d_added(baseB, bLineOffset);
	GFC_Vector3D bB = gfc_vector3d_subbed(tipB, bLineOffset);

	// Get the differences between all four calculated endpoints
	GFC_Vector3D v0 = gfc_vector3d_subbed(bA, aA);
	GFC_Vector3D v1 = gfc_vector3d_subbed(bB, aA);
	GFC_Vector3D v2 = gfc_vector3d_subbed(bA, aB);
	GFC_Vector3D v3 = gfc_vector3d_subbed(bB, aB);

	// Square the lenghts
	float d0 = gfc_vector3d_dot_product(v0, v0);
	float d1 = gfc_vector3d_dot_product(v1, v1);
	float d2 = gfc_vector3d_dot_product(v2, v2);
	float d3 = gfc_vector3d_dot_product(v3, v3);

	GFC_Vector3D bestA;
	if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1) {
		bestA = aB;
	}
	else {
		bestA = aA;
	}

	GFC_Vector3D bestB = closestPointOnLineSegment(bA, bB, bestA);
	bestA = closestPointOnLineSegment(aA, aB, bestB);

	//printf("\nBest A: %f, %f, %f", bestA.x, bestA.y, bestA.z);
	//printf("\nBest B: %f, %f, %f", bestB.x, bestB.y, bestB.z);
	GFC_Vector3D _penetrationNormal = gfc_vector3d_subbed(bestA, bestB);
	float len = sqrt(gfc_vector3d_dot_product(_penetrationNormal, _penetrationNormal));
	_penetrationNormal.x /= len;
	_penetrationNormal.y /= len;
	_penetrationNormal.z /= len;
	*penetrationNormal = _penetrationNormal;
	*penetrationDepth = a.radius + b.radius - len;


	return *penetrationDepth > 0;
}

short capsuleToTriangleTest(GFC_Capsule c, GFC_Triangle3D t, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth) {
	// Draw a raycast from the base of the capsule to the tip and multiply by radius
	GFC_Vector3D capsuleNormal = gfc_vector3d_subbed(c.finalTip, c.finalBase);
	gfc_vector3d_normalize(&capsuleNormal);
	GFC_Vector3D lineEndOffset = gfc_vector3d_multiply(capsuleNormal, gfc_vector3d(c.radius, c.radius, c.radius));

	GFC_Vector3D a = gfc_vector3d_added(c.finalBase, lineEndOffset);
	GFC_Vector3D b = gfc_vector3d_subbed(c.finalTip, lineEndOffset);

	// We then get the closest point on the triangle to this raycast
	// If none exists (dot product = 0) then we just use an arbitrary point on the triangle
	GFC_Vector3D normalized = gfc_trigfc_angle_get_normal(t);

	GFC_Vector3D referencePoint;
	if (gfc_vector3d_dot_product(capsuleNormal, normalized) != 0) {
		GFC_Vector3D pointBaseDifference = gfc_vector3d_subbed(t.a, c.finalBase);
		float m = 1 / fabsf(gfc_vector3d_dot_product(normalized, capsuleNormal));
		pointBaseDifference = gfc_vector3d_multiply(pointBaseDifference, gfc_vector3d(m, m, m));

		float intersct = gfc_vector3d_dot_product(normalized, pointBaseDifference);
		GFC_Vector3D linePlaneIntersection = gfc_vector3d_added(c.finalBase, gfc_vector3d_multiply(capsuleNormal, gfc_vector3d(intersct, intersct, intersct)));

		// Determine whether line_plane_intersection is inside all triangle edges: 
		GFC_Vector3D c0 = { 0 };
		GFC_Vector3D c1 = { 0 };
		GFC_Vector3D c2 = { 0 };
		gfc_vector3d_cross_product(&c0, gfc_vector3d_subbed(linePlaneIntersection, t.a), gfc_vector3d_subbed(t.b, t.a));
		gfc_vector3d_cross_product(&c1, gfc_vector3d_subbed(linePlaneIntersection, t.b), gfc_vector3d_subbed(t.c, t.b));
		gfc_vector3d_cross_product(&c2, gfc_vector3d_subbed(linePlaneIntersection, t.c), gfc_vector3d_subbed(t.a, t.c));
		Bool inside = (
			fabsf(gfc_vector3d_dot_product(c0, normalized)) <= GFC_EPSILON &&
			fabsf(gfc_vector3d_dot_product(c1, normalized)) <= GFC_EPSILON &&
			fabsf(gfc_vector3d_dot_product(c2, normalized)) <= GFC_EPSILON);

		if (inside) {
			printf("\nInside");
			referencePoint = linePlaneIntersection;
		}
		else {
			GFC_Vector3D point1 = closestPointOnLineSegment(t.a, t.b, linePlaneIntersection);
			GFC_Vector3D v1 = gfc_vector3d_subbed(linePlaneIntersection, point1);
			float distSq = gfc_vector3d_dot_product(v1, v1);
			float bestDist = distSq;
			referencePoint = point1;

			GFC_Vector3D point2 = closestPointOnLineSegment(t.b, t.c, linePlaneIntersection);
			GFC_Vector3D v2 = gfc_vector3d_subbed(linePlaneIntersection, point2);
			distSq = gfc_vector3d_dot_product(v2, v2);
			if (distSq < bestDist) {
				bestDist = distSq;
				referencePoint = point2;
			}

			GFC_Vector3D point3 = closestPointOnLineSegment(t.c, t.a, linePlaneIntersection);
			GFC_Vector3D v3 = gfc_vector3d_subbed(linePlaneIntersection, point3);
			distSq = gfc_vector3d_dot_product(v3, v3);
			if (distSq < bestDist) {
				bestDist = distSq;
				referencePoint = point3;
			}
		}
	}
	else {
		referencePoint = t.a;
	}

	GFC_Vector3D center = closestPointOnLineSegment(a, b, referencePoint);

	GFC_Sphere collisionSphere = { 0 };
	collisionSphere.r = c.radius;
	collisionSphere.x = center.x;
	collisionSphere.y = center.y;
	collisionSphere.z = center.z;
	if (sphereTriangleTest(collisionSphere, t, intersectionPoint, penetrationNormal, penetrationDepth)) {
		return true;
	}
	return false;

}

short sphereTriangleTest(GFC_Sphere sphere, GFC_Triangle3D triangle, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth) {
	GFC_Vector3D normalized = { 0 };
	GFC_Vector3D sphereCenter = { 0 };

	sphereCenter = gfc_vector3d(sphere.x, sphere.y, sphere.z);

	normalized = gfc_trigfc_angle_get_normal(triangle);

	float dist = gfc_vector3d_dot_product(gfc_vector3d_subbed(sphereCenter, triangle.a), normalized);
	if (dist > sphere.r || dist < -sphere.r) {
		return false;
	}


	GFC_Vector3D point0 = gfc_vector3d_subbed(sphereCenter, gfc_vector3d_multiply(normalized, gfc_vector3d(dist, dist, dist)));

	GFC_Vector3D cross0, cross1, cross2;
	gfc_vector3d_cross_product(&cross0, gfc_vector3d_subbed(point0, triangle.a), gfc_vector3d_subbed(triangle.b, triangle.a));
	gfc_vector3d_cross_product(&cross1, gfc_vector3d_subbed(point0, triangle.b), gfc_vector3d_subbed(triangle.c, triangle.b));
	gfc_vector3d_cross_product(&cross2, gfc_vector3d_subbed(point0, triangle.c), gfc_vector3d_subbed(triangle.a, triangle.c));

	Uint8 inside = gfc_vector3d_dot_product(cross0, normalized) <= 0 && gfc_vector3d_dot_product(cross1, normalized) <= 0 && gfc_vector3d_dot_product(cross2, normalized) <= 0;


	// If inside is not true, we have to find the closest edge
	float radiussq = sphere.r * sphere.r;

	// Grab each of the closest edges, and then check if the distance is less than the radius
	// Edge 1
	GFC_Vector3D point1 = closestPointOnLineSegment(triangle.a, triangle.b, sphereCenter);
	GFC_Vector3D v1 = gfc_vector3d_subbed(sphereCenter, point1);
	float distsq1 = gfc_vector3d_dot_product(v1, v1);
	Uint8 intersects = distsq1 < radiussq;

	// Edge 2
	GFC_Vector3D point2 = closestPointOnLineSegment(triangle.b, triangle.c, sphereCenter);
	GFC_Vector3D v2 = gfc_vector3d_subbed(sphereCenter, point2);
	float distsq2 = gfc_vector3d_dot_product(v2, v2);
	intersects |= distsq2 < radiussq;

	// Edge 3
	GFC_Vector3D point3 = closestPointOnLineSegment(triangle.c, triangle.a, sphereCenter);
	GFC_Vector3D v3 = gfc_vector3d_subbed(sphereCenter, point3);
	float distsq3 = gfc_vector3d_dot_product(v3, v3);
	intersects |= distsq3 < radiussq;
	
	if (inside || intersects) {
		GFC_Vector3D bestPoint = point0;
		GFC_Vector3D intersectionVector;
		if (inside) {
			// If triangle is wholly inside sphere, then set the intersection point to where they meet
			intersectionVector = gfc_vector3d_subbed(sphereCenter, point0);
			printf("\nINside");
		}
		else {
			printf("\nOutside");
			GFC_Vector3D d = gfc_vector3d_subbed(sphereCenter, point1);

			float bestDistSq = gfc_vector3d_dot_product(d, d);
			bestPoint = point1;
			intersectionVector = d;

			d = gfc_vector3d_subbed(sphereCenter, point2);
			float distSq = gfc_vector3d_dot_product(d, d);
			if (distSq < bestDistSq) {
				distSq = bestDistSq;
				bestPoint = point2;
				intersectionVector = d;
			}

			d = gfc_vector3d_subbed(sphereCenter, point3);
			distSq = gfc_vector3d_dot_product(d, d);
			if (distSq < bestDistSq) {
				distSq = bestDistSq;
				bestPoint = point3;
				intersectionVector = d;
			}

		}

		intersectionVector = gfc_vector3d_subbed(sphereCenter, bestPoint);
		printf("\nintersectionVector: %f, %f, %f", intersectionVector.x, intersectionVector.y, intersectionVector.z);
		float lenSquare = gfc_vector3d_dot_product(intersectionVector, intersectionVector);
		if (lenSquare == 0) {
			*intersectionPoint = sphereCenter;
			*penetrationNormal = normalized;
			*penetrationDepth = 0;
		}
		else {
			float len = sqrt(lenSquare);
			GFC_Vector3D length = { len, len, len };
			*intersectionPoint = intersectionVector;
			*penetrationNormal = gfc_vector3d_multiply(intersectionVector, gfc_vector3d(1 / length.x, 1 / length.y, 1 / length.z));

			*penetrationDepth = sphere.r - len;

		}
		return true;
	}
	return false;

}

GFC_Vector3D closestPointOnLineSegment(GFC_Vector3D a, GFC_Vector3D b, GFC_Vector3D point) {
	GFC_Vector3D ab = gfc_vector3d_subbed(b, a);
	float t = gfc_vector3d_dot_product(gfc_vector3d_subbed(point, a), ab) / gfc_vector3d_dot_product(ab, ab);
	float saturated = MIN(MAX(t, 0), 1);
	return gfc_vector3d_added(a, gfc_vector3d_multiply(ab, gfc_vector3d(saturated, saturated, saturated)));
}