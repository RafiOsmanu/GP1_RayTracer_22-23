//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;


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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float fov{  tan(TO_RADIANS * camera.fovAngle / 2.f) };
	float AspectRatio{ float(m_Width) / float(m_Height) };


	Vector3 rayDirection{};

	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };
	for (int px{}; px < m_Width; ++px)
	{

		for (int py{}; py < m_Height; ++py)
		{


			float pxc{ float(px) + 0.5f };
			rayDirection.x = (((2 * pxc) / float(m_Width)) - 1) * AspectRatio * fov;

			float pyc{ float(py) + 0.5f };
			rayDirection.y = ((1 - ((2 * pyc) / float(m_Height))) * fov);
			
			
			rayDirection.z = 1;
			

			rayDirection.Normalize();
			

			rayDirection = cameraToWorld.TransformVector(rayDirection);

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

	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	m_CurrentLightingMode = LightingMode((int(m_CurrentLightingMode) + 1) % 4);
}
