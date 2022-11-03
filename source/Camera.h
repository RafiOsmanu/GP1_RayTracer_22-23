#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			cameraToWorld = Matrix({ right }, { up }, { forward }, { origin });
			
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float CameraMovementSpeed{10.f};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * CameraMovementSpeed * deltaTime;
			}
			else if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * CameraMovementSpeed * deltaTime;
			}
			else if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * CameraMovementSpeed * deltaTime;
			}
			else if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * CameraMovementSpeed * deltaTime;
			}
			


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState == SDL_BUTTON_LEFT)
			{
				totalPitch -= mouseY * deltaTime;
				totalYaw += mouseX * deltaTime;
			}

			//todo: W2
			Matrix finalRotation{Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };

			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			right = finalRotation.TransformVector(Vector3::UnitX);
			right.Normalize();

			up = finalRotation.TransformVector(Vector3::UnitY);
			up.Normalize();
			
			//assert(false && "Not Implemented Yet");
		}
	};
}
