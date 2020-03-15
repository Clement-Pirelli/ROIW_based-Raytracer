#include "Triangle.h"

Triangle::Triangle(Vertex *givenVertices, Material *mat)
{
    vertices[0] = givenVertices[0];
    vertices[1] = givenVertices[1];
    vertices[2] = givenVertices[2];
    material = mat;
}

bool Triangle::hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const
{
   //normal->normalize(cross(e0, e1))
   vec3 o = vertices[0].position;
   vec3 e0 = vertices[1].position - o;
   vec3 e1 = vertices[2].position - o;
   vec3 intersectionMat[3] = { -givenRay.direction, e0, e1 };

   //mat<T, 3, 3> inverseTransposed(const mat<T, 3, 3>& arg)
   //{
   //    vec<T, 3> c01 = cross(arg[0], arg[1]);
   //    vec<T, 3> c12 = cross(arg[1], arg[2]);
   //    vec<T, 3> c20 = cross(arg[2], arg[0]);
   //    T det = dot(arg[0], c12);
   //    return mat<T, 3, 3>(c12, c20, c01) / det;
   //}

   vec3 c01 = vec3::cross(intersectionMat[0], intersectionMat[1]);
   vec3 c12 = vec3::cross(intersectionMat[1], intersectionMat[2]);
   vec3 c20 = vec3::cross(intersectionMat[2], intersectionMat[0]);
   float det = vec3::dot(intersectionMat[0], c12);
   float inverseDet = 1.0f / det;
   
   vec3 inverseTransposedMat[3] = { c12*inverseDet, c20*inverseDet, c01*inverseDet };

   vec3 dir = (givenRay.origin - o);
   vec3 tuv = vec3(vec3::dot(inverseTransposedMat[0], dir), vec3::dot(inverseTransposedMat[1], dir), vec3::dot(inverseTransposedMat[2], dir));
   if (minT < tuv.x && tuv.x < maxT && 0.0f < tuv.y && 0.0f < tuv.z && tuv.y + tuv.z < 1.0f)
   {
       record.point = givenRay.pointAt(tuv.x);
       record.distance = tuv.x;
       record.u = tuv.y;
       record.v = tuv.z;
       record.normal = vec3::cross(e0, e1).normalized();
       record.material = material;
   }
   else
   {
       return false;
   }




#if 0
    vec3 v0v1 = vertices[1].position - vertices[0].position;
    vec3 v0v2 = vertices[2].position - vertices[0].position;
    vec3 pvec = vec3::cross(givenRay.direction, v0v2);
    float det = vec3::dot(v0v1, pvec);
    // ray and triangle are parallel if det is close to 0
    if (fabs(det) < .0001f) return false;
    float invDet = 1.0f / det;

    vec3 tvec = givenRay.origin - vertices[0].position;
    float u = vec3::dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f) return false;

    vec3 qvec = vec3::cross(tvec, v0v1);
    float v = vec3::dot(givenRay.direction, qvec) * invDet;
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = vec3::dot(v0v2, qvec) * invDet;


    record.normal = vec3::cross(vertices[1].position-vertices[0].position, vertices[2].position -vertices[0].position);
    record.distance = t;
    record.material = material;
    record.point = givenRay.pointAt(t);
    record.u = u;
    record.v = v;

    return true;



    vec3 v0v1 = vertices[1].position - vertices[0].position;
    vec3 v0v2 = vertices[2].position - vertices[0].position;
    // no need to normalize
    vec3 N = vec3::cross(v0v1, v0v2);
    float area2 = N.length();

    // Step 1: finding P

    // check if ray and plane are parallel ?
    float NdotRayDirection = vec3::dot(N, givenRay.direction);
    if (fabs(NdotRayDirection) < .0001f) // almost 0 
        return false; // they are parallel so they don't intersect ! 

    // compute d parameter using equation 2
    float d = vec3::dot(N, vertices[0].position);

    // compute t (equation 3)
    float t = (vec3::dot(N, givenRay.origin) + d) / NdotRayDirection;
    // check if the triangle is in behind the ray
    if (t < 0) return false; // the triangle is behind 

    // compute the intersection point using equation 1
    vec3 P = givenRay.origin + givenRay.direction * t;

    // Step 2: inside-outside test
    vec3 C; // vector perpendicular to triangle's plane 

    // edge 0
    vec3 edge0 = v0v1;
    vec3 vp0 = P - vertices[0].position;
    C = vec3::cross(edge0, vp0);
    if (vec3::dot(N, C) < 0) return false; // P is on the right side 

    // edge 1
    vec3 edge1 = vertices[2].position - vertices[1].position;
    vec3 vp1 = P - vertices[1].position;
    C = vec3::cross(edge1, vp1);
    if (vec3::dot(N, C) < 0)  return false; // P is on the right side 

    // edge 2
    vec3 edge2 = vertices[0].position -vertices[2].position;
    vec3 vp2 = P - vertices[2].position;
    C = vec3::cross(edge2, vp2);
    if (vec3::dot(N,C) < 0) return false; // P is on the right side; 

    record.distance = t;
    record.point = P;
    record.material = material;
    record.normal = N.normalized();
    record.exitPoint = record.normal * -.001f + record.point;

    return true; // this ray hits the triangle 
#endif
}

bool Triangle::boundingBox(AABB &aabb) const
{
    vec3 min = vec3( 10000000.0f, 1000000.0f, 1000000.0f);
    vec3 max = vec3(-10000000.0f,-1000000.0f,-1000000.0f);

    for(int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++) 
        {
            min[j] = __min(vertices[i].position[j], min[j]);
            max[j] = __max(vertices[i].position[j], max[j]);
        }
    }
    aabb = AABB(min, max);
	return true;
}
