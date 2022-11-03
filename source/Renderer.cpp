//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <thread>
#include <iostream>

#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <future>
#include <ppl.h>

using namespace dae;
//#define ASYNC
#define PARALLEL_FOR


Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	camera.CalculateCameraToWorld();

	float aspectRatio{ float(m_Width) / float(m_Height) };

	float fov{  tan(TO_RADIANS * camera.fovAngle / 2.f) };

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t numPixels = m_Width * m_Height;

#if defined(ASYNC)
	//async logic
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelPerTask = numPixel / numCores;
	uint32_t numUnassignedPixels = numPixel % numCores;
	uint32_t currentPixelIndex{ 0 };

	for (uint32_t coreId{}; coreId < numCores; ++coreId)
	{
		uint32_t taskSize{ numPixelPerTask };
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=,this]
				{

					const uint32_t pixelIndexEnd = currentPixelIndex + taskSize;
					for (uint32_t pixelIndex{ currentPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
					{
						RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
					}
				})
		);

		currentPixelIndex += taskSize;
	}

	//wait
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}
	
#elif defined(PARALLEL_FOR)

	Concurrency::parallel_for(0u, numPixels,
		[=, this](int i)
		{
			RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
		});

#else

	for (uint32_t i{}; i < numPixel; ++i)
	{
		RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
	}

#endif

	

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	Vector3 rayDirection{};

	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;


	float pxc{ float(px) + 0.5f };
	rayDirection.x = (((2 * pxc) / float(m_Width)) - 1) * aspectRatio * fov;

	float pyc{ float(py) + 0.5f };
	rayDirection.y = ((1 - ((2 * pyc) / float(m_Height))) * fov);


	rayDirection.z = 1;


	rayDirection.Normalize();


	rayDirection = camera.cameraToWorld.TransformVector(rayDirection.Normalized());
	
	

	Ray viewRay{ camera.origin, rayDirection };
	ColorRGB finalColor{};
	HitRecord closestHit{};



	pScene->GetClosestHit(viewRay, closestHit);



	if (closestHit.didHit)
	{
		//finalColor = materials[closestHit.materialIndex]->Shade();

		for (int i{}; i < pScene->GetLights().size(); ++i)
		{
			Ray originToLight{};

			Vector3 lightDirection = LightUtils::GetDirectionToLight(pScene->GetLights()[i], closestHit.origin + closestHit.normal * 0.001f);

			originToLight.origin = closestHit.origin + closestHit.normal * 0.001f;
			originToLight.direction = lightDirection;
			originToLight.min = 0.001f;
			originToLight.max = originToLight.direction.Magnitude();
			originToLight.direction.Normalize();


			if (pScene->DoesHit(originToLight) && m_ShadowsEnabled) continue;


			const float observedArea{ Vector3::Dot(closestHit.normal,originToLight.direction) };

			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				if (observedArea < 0) continue;
				finalColor += ColorRGB{ observedArea, observedArea, observedArea };
				break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(pScene->GetLights()[i], originToLight.origin);
				break;
			case LightingMode::BRDF:
				//if (observedArea < 0) continue;
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, originToLight.direction, -viewRay.direction);
				break;
			case LightingMode::Combined:
				if (observedArea < 0) continue;
				finalColor += LightUtils::GetRadiance(pScene->GetLights()[i], originToLight.origin)
					* materials[closestHit.materialIndex]->Shade(closestHit, originToLight.direction, -viewRay.direction)
					* observedArea;
				break;
			default:
				break;
			}
		}

		/*const float scaled_t{ closestHit.t  / 500.f };
		finalColor = { scaled_t, scaled_t, scaled_t };*/
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));

}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	m_CurrentLightingMode = LightingMode((int(m_CurrentLightingMode) + 1) % 4);
}
