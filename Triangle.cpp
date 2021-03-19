#include "Triangle.h"

Triangle::Triangle(Vertex *givenVertices)
{
    vertices[0] = givenVertices[0];
    vertices[1] = givenVertices[1];
    vertices[2] = givenVertices[2];
}

bool Triangle::hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const
{
   const vec3 o = vertices[0].position;
   const vec3 e0 = vertices[1].position - o;
   const vec3 e1 = vertices[2].position - o;
   const vec3 intersectionMat[3] = { -givenRay.direction, e0, e1 };
   const vec3 c01 = vec3::cross(intersectionMat[0], intersectionMat[1]);
   const vec3 c12 = vec3::cross(intersectionMat[1], intersectionMat[2]);
   const vec3 c20 = vec3::cross(intersectionMat[2], intersectionMat[0]);
   const float det = vec3::dot(intersectionMat[0], c12);
   const float inverseDet = 1.0f / det;
   
   const vec3 inverseTransposedMat[3] = { c12*inverseDet, c20*inverseDet, c01*inverseDet };

   const vec3 dir = (givenRay.origin - o);
   const vec3 tuv = vec3(vec3::dot(inverseTransposedMat[0], dir), vec3::dot(inverseTransposedMat[1], dir), vec3::dot(inverseTransposedMat[2], dir));
   if (minT < tuv.x() && tuv.x() < maxT && 0.0f < tuv.y() && 0.0f < tuv.z() && tuv.y() + tuv.z() < 1.0f)
   {
       record.point = givenRay.pointAt(tuv.x());
       record.distance = tuv.x();
       const float firstUVWeight = 1.0f - tuv.y() - tuv.z();
       record.u = firstUVWeight * vertices[0].u + tuv.y() * vertices[1].u + tuv.z() * vertices[2].u;
       record.v = firstUVWeight * vertices[0].v + tuv.y() * vertices[1].v + tuv.z() * vertices[2].v;
       record.normal = vec3::cross(e0, e1).normalized();

       //tangent and bitangent
       {
           const vec2 deltaUV1 = vec2(vertices[1].u - vertices[0].u, vertices[1].v - vertices[0].v);
           const vec2 deltaUV2 = vec2(vertices[2].u - vertices[0].u, vertices[2].v - vertices[0].v);
           const float f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());

           record.tangent = vec3
           {
               f *(deltaUV2.y() * e0.x() - deltaUV1.y() * e1.x()),
               f *(deltaUV2.y() * e0.y() - deltaUV1.y() * e1.y()),
               f *(deltaUV2.y() * e0.z() - deltaUV1.y() * e1.z())
           } * f;

           record.bitangent = vec3
           {
               (-deltaUV2.y() * e0.x() + deltaUV1.x() * e1.x()),
               (-deltaUV2.y() * e0.y() + deltaUV1.x() * e1.y()),
               (-deltaUV2.y() * e0.z() + deltaUV1.x() * e1.z())
           } * f;
       }

       return true;
   }
   return false;

}

AABB3 Triangle::boundingBox() const
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
    return AABB(min, max);
}
