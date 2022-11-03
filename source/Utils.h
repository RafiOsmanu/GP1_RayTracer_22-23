#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//analyitic use of ray and sphere intersection
			float A{ Vector3::Dot(ray.direction, ray.direction) };
			float B{ Vector3::Dot((2 * ray.direction), (ray.origin - sphere.origin)) };
			float C{ (Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin))) - (sphere.radius * sphere.radius) };
			float tMin{ ray.min };
			float tMax{ ray.max };


			float D{ (B * B) - (4 * A * C) };

			//todo W1

			if (D < 0)
			{
				hitRecord.didHit = false;
				return false;
			}
			else
			{

				float SquareD{ std::sqrtf(D) };
				float t{ (-B - (SquareD)) / 2 * A };

				if (t >= tMin && t <= tMax)
				{
					if (ignoreHitRecord) return true;

					hitRecord.didHit = true;
					hitRecord.t = t;
					hitRecord.origin = ray.origin + (hitRecord.t * ray.direction);
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					return true;
				}
			}
			return false;
		}
			//assert(false && "No Implemented Yet!");
		

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction, plane.normal) };
			float tMin{ ray.min };
			float tMax{ ray.max };
			
			if (t > tMin  && t <= tMax)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.normal = plane.normal;
					hitRecord.didHit = true;

					hitRecord.origin = ray.origin + (ray.direction * t);
				}
				return true;
			}
			
			return false; 
			//assert(false && "No Implemented Yet!");
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			hitRecord.didHit = false;
			//todo W5
			// flip cullmode for shadows
			TriangleCullMode cullMode{ triangle.cullMode };
			if (ignoreHitRecord && cullMode != TriangleCullMode::NoCulling)
				cullMode = TriangleCullMode(((int)cullMode + 1) % 2);

			
			if(Vector3::Dot(triangle.normal, ray.direction) == 0) return false;

			//FrontFaceCulling
			if (cullMode == TriangleCullMode::FrontFaceCulling)
			{
				if (Vector3::Dot(triangle.normal, ray.direction) < 0) return false;
			}

			//BackFaceCulling
			if (cullMode == TriangleCullMode::BackFaceCulling)
			{
				if (Vector3::Dot(triangle.normal, ray.direction) > 0) return false;
			}

			Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3.f };
			Vector3 originToTriangleCenter{ center - ray.origin };

			float t{ Vector3::Dot(originToTriangleCenter, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal) };


			if (t < ray.min || t > ray.max) return false;
			Vector3 P = ray.origin + t * ray.direction;

			Vector3 edgeA{ triangle.v1 - triangle.v0 };
			Vector3 edgeB{ triangle.v2 - triangle.v1 };
			Vector3 edgeC{ triangle.v0 - triangle.v2 };


			Vector3 pointToSideA{P - triangle.v0 };
			Vector3 pointToSideB{P - triangle.v1 };
			Vector3 pointToSideC{P - triangle.v2 };

			if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeA, pointToSideA)) < 0) return false;
			if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeB, pointToSideB)) < 0) return false;
			if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeC, pointToSideC)) < 0) return false;

			hitRecord.t = t;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.normal = triangle.normal;
			hitRecord.didHit = true;
			hitRecord.origin = ray.origin + (ray.direction * t);

			return true;
			


		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			
			HitRecord testHit{};
			testHit.t = FLT_MAX;
			int normalIndex{ 0 };
			for (int i{}; i < mesh.indices.size(); i += 3)
			{
				int v0 = mesh.indices[i];
				int v1 = mesh.indices[i + 1];
				int v2 = mesh.indices[i + 2];

				Triangle currentTriangle = Triangle(
					mesh.transformedPositions[v0],
					mesh.transformedPositions[v1],
					mesh.transformedPositions[v2],
					mesh.transformedNormals[normalIndex]);

				currentTriangle.cullMode = mesh.cullMode;
				currentTriangle.materialIndex = mesh.materialIndex;

				
				if (GeometryUtils::HitTest_Triangle(currentTriangle, ray, testHit, ignoreHitRecord))
				{
					if (ignoreHitRecord)
					{
						return true;
					}
					else
					{
						if (testHit.t < hitRecord.t)
						{
							hitRecord = testHit;
						}
					}
					
				}
				++normalIndex;
			}
			return hitRecord.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");

			return {light.origin - origin};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3

			if (light.type == LightType::Directional)
			{
				return light.intensity * light.color;
			}

			Vector3 pointToShade{ light.origin - target };
			
			return { light.color * (light.intensity / pointToShade.SqrMagnitude()) };//.SqrMagnitude())};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}