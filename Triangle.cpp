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
   vec3 o = vertices[0].position;
   vec3 e0 = vertices[1].position - o;
   vec3 e1 = vertices[2].position - o;
   vec3 intersectionMat[3] = { -givenRay.direction, e0, e1 };
   vec3 c01 = vec3::cross(intersectionMat[0], intersectionMat[1]);
   vec3 c12 = vec3::cross(intersectionMat[1], intersectionMat[2]);
   vec3 c20 = vec3::cross(intersectionMat[2], intersectionMat[0]);
   float det = vec3::dot(intersectionMat[0], c12);
   float inverseDet = 1.0f / det;
   
   vec3 inverseTransposedMat[3] = { c12*inverseDet, c20*inverseDet, c01*inverseDet };

   vec3 dir = (givenRay.origin - o);
   vec3 tuv = vec3(vec3::dot(inverseTransposedMat[0], dir), vec3::dot(inverseTransposedMat[1], dir), vec3::dot(inverseTransposedMat[2], dir));
   if (minT < tuv.x() && tuv.x() < maxT && 0.0f < tuv.y() && 0.0f < tuv.z() && tuv.y() + tuv.z() < 1.0f)
   {
       record.point = givenRay.pointAt(tuv.x());
       record.distance = tuv.x();
       record.u = tuv.y() * vertices[1].u + tuv.z() * vertices[2].u + (1.0f - tuv.y() - tuv.z()) * vertices[0].u;
       record.v = tuv.y() * vertices[1].v + tuv.z() * vertices[2].v + (1.0f - tuv.y() - tuv.z()) * vertices[0].v;
       record.normal = vec3::cross(e0, e1).normalized();
       record.material = material;
       return true;
   }
   return false;

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
