#include "memory.h"

#include <thread>
#include <iostream>

// https://github.com/frk1/hazedumper/blob/master/csgo.hpp
namespace offsets
{
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA964;
	constexpr ::std::ptrdiff_t m_fFlags = 0x104;
	constexpr ::std::ptrdiff_t dwForceJump = 0x52BBC7C;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFEF4;
	constexpr ::std::ptrdiff_t dwGlowObjectManager = 0x535A9C8;
	constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
	constexpr ::std::ptrdiff_t m_iGlowIndex = 0x10488;
}

__declspec(align(16)) struct Color
{
	constexpr Color(const float r, const float g, const float b, const float a = 1.f) noexcept :
		r(r), g(g), b(b), a(a) { }

	float r, g, b, a;
};

int main()
{
	const auto mem = Memory("csgo.exe");

	std::cout << "csgo found!" << std::endl;

	const auto client = mem.GetModuleAddress("client.dll");
	std::cout << "client.dll -> " << "0x" << std::hex << client << std::dec << std::endl;

	constexpr const auto color = Color{ 1.f, 0.f, 1.f };

	while (true)
	{
		// we don't need this running a billion times per second :)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		const auto localPlayer = mem.Read<std::uintptr_t>(client + offsets::dwLocalPlayer);

		if (!localPlayer)
			continue;

		const auto localPlayerTeam = mem.Read<std::uintptr_t>(localPlayer + offsets::m_iTeamNum);
		const auto localPlayerFlags = mem.Read<std::uintptr_t>(localPlayer + offsets::m_fFlags);

		// glow
		const auto glowObjectManager = mem.Read<std::uintptr_t>(client + offsets::dwGlowObjectManager);

		for (auto i = 1; i <= 32; ++i)
		{
			const auto entity = mem.Read<std::uintptr_t>(client + offsets::dwEntityList + i * 0x10);

			if (!entity)
				continue;

			// dont glow if they are on our team
			if (mem.Read<std::uintptr_t>(entity + offsets::m_iTeamNum) == localPlayerTeam)
				continue;

			const auto glowIndex = mem.Read<std::int32_t>(entity + offsets::m_iGlowIndex);

			// do glow by writing each variable
			//mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0x8, 1.f);
			//mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0xC, 0.f);
			//mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0x10, 0.f);
			//mem.Write<float>(glowObjectManager + (glowIndex * 0x38) + 0x14, 1.f);

			//mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
			//mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x29, true);

			// preferred
			// use a color struct to make 1 WPM call
			mem.Write<Color>(glowObjectManager + (glowIndex * 0x38) + 0x8, color);

			mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
			mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x29, false);
		}
	}
}
