#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB pho{ cd * kd };
			
			return { pho/float(M_PI)};
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			
			ColorRGB pho{ cd * kd };

			return { pho / float(M_PI) };
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3
			const Vector3 reflect{Vector3::Reflect(l, n)};
			float cosAngle{ Vector3::Dot(reflect, v)};
			if (cosAngle < 0.f) cosAngle = 0.f;

			const float specReflection{ks * powf(cosAngle, exp)};
			
			return { specReflection, specReflection, specReflection };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3
			

			ColorRGB Schlick = f0 + (ColorRGB(1, 1, 1) - f0) * powf((1 - (Vector3::Dot(h, v))), 5);
			
			
			return Schlick;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			//normal distrb function (how many microfacets point in the right direction);
			float roughnessSquared{ roughness * roughness };

			float nhDotSquared{ Vector3::Dot(n, h) * Vector3::Dot(n, h) };

			float denominatorSquared{ (nhDotSquared * (roughnessSquared - 1) + 1) * (nhDotSquared * (roughnessSquared - 1) + 1) };

			float D = (roughnessSquared) / ((float(M_PI) * denominatorSquared));

			return D;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			float dp{ Vector3::Dot(n, v) };
			if (dp < 0) dp = 0;
			float k = ((roughness + 1) * (roughness + 1)) / 8.f;

			float schlickGGX = dp / (dp * (1 - k) + k);
			
			return schlickGGX;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//todo: W3
			
			return { GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness) };
		}

	}
}