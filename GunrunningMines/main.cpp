#include "main.h"
#include "enums.h"
#include "natives.h"
#include "util.h"
#include <chrono>
#include <cstdarg>
#include <unordered_set>

namespace CUSTOM {
namespace GAMEPLAY {
void _0xBFE5756E7407064A(const Vector3 &pos1, const Vector3 &pos2, int damage,
						 BOOL p7, Hash weaponHash, Ped ownerPed, BOOL isAudible,
						 BOOL isInvisible, float speed, Entity entity, BOOL p14,
						 BOOL p15, BOOL p16, BOOL p17, BOOL p18, BOOL p19) {
	invoke<Void>(0xBFE5756E7407064A, pos1.x, pos1.y, pos1.z, pos2.x, pos2.y,
				 pos2.z, damage, p7, weaponHash, ownerPed, isAudible,
				 isInvisible, speed, entity, p14, p15, p16, p17, p18, p19);
}
BOOL _HAS_CHEAT_STRING_JUST_BEEN_ENTERED(Hash hash) {
	return invoke<BOOL>(0x557E43C447E700A8, hash);
}
};
};

#ifdef DEBUGOUT
void Output(const char *fmt, ...) {
	char buff[256];
	sprintf_s(buff, "[GR_MINES] %s", fmt);
	char buff2[512];
	va_list _ArgList;
	__crt_va_start(_ArgList, fmt);
	vsprintf_s(buff2, buff, _ArgList);
	__crt_va_end(_ArgList);
	OutputDebugStringA(buff2);
}
#else
#define Output(...)
#endif

template <size_t N>
bool PathRenameExtension(char (&path)[N], const char *newExt) {
	auto begin  = std::begin(path);
	auto end	= std::end(path);
	auto period = end;
	while (begin != end) {
		if (*begin == '.' || *begin == '\0') {
			period = begin;
			break;
		}
		begin++;
	}
	if (period == end) {
		return false;
	}
	if (!newExt || *newExt == '\0') {
		*period = '\0';
		return true;
	}
	if (*newExt == '.') {
		newExt++;
	}
	auto len   = strlen(newExt);
	auto bSize = len + 1;
	if (std::distance(period, end) > bSize) {
		*period++ = '.';
		do {
			*period++ = *newExt++;
		} while (*newExt);
		return true;
	}
	else {
		return false;
	}
}

enum class GameVersion {
	Unknown = -1,
	v1_0_335_2_Steam,
	v1_0_335_2_NoSteam,
	v1_0_350_1_Steam,
	v1_0_350_2_NoSteam,
	v1_0_372_2_Steam,
	v1_0_372_2_NoSteam,
	v1_0_393_2_Steam,
	v1_0_393_2_NoSteam,
	v1_0_393_4_Steam,
	v1_0_393_4_NoSteam,
	v1_0_463_1_Steam,
	v1_0_463_1_NoSteam,
	v1_0_505_2_Steam,
	v1_0_505_2_NoSteam,
	v1_0_573_1_Steam,
	v1_0_573_1_NoSteam,
	v1_0_617_1_Steam,
	v1_0_617_1_NoSteam,
	v1_0_678_1_Steam,
	v1_0_678_1_NoSteam,
	v1_0_757_2_Steam,
	v1_0_757_2_NoSteam,
	v1_0_757_3_Steam,
	v1_0_757_4_NoSteam,
	v1_0_791_2_Steam,
	v1_0_791_2_NoSteam,
	v1_0_877_1_Steam,
	v1_0_877_1_NoSteam,
	v1_0_944_2_Steam,
	v1_0_944_2_NoSteam,
	v1_0_1011_1_Steam,
	v1_0_1011_1_NoSteam,
	v1_0_1032_1_Steam,
	v1_0_1032_1_NoSteam,
	v1_0_1103_2_Steam,
	v1_0_1103_2_NoSteam
};

auto getGameVersionSafe() { return static_cast<GameVersion>(getGameVersion()); }

const std::unordered_set<uint32_t> MineVehicles = {
	joaat("apc"),	joaat("dune3"),	  joaat("halftrack"),
	joaat("tampa3"), joaat("insurgent3"), joaat("technical3"),
};

bool checkMod = false;

char iniPath[MAX_PATH] = {};

void writeModSetting(bool check) {
	if (*iniPath) {
		Output("Writing value to ini file");
		WritePrivateProfileStringA("Base", "RequireMod",
								   check ? "true" : "false", iniPath);
	}
	else {
		Output("Error writing ini file, invalid path");
	}
}

void setupIni(HMODULE module) {
	GetModuleFileNameA(module, iniPath, sizeof(iniPath));
	if (PathRenameExtension(iniPath, "ini")) {
		Output("Path = %s", iniPath);
		if (WIN32_FIND_DATAA data;
			FindFirstFileA(iniPath, &data) == INVALID_HANDLE_VALUE) {
			Output("File doesn't exist, Writing");
			writeModSetting(false);
		}
		else {
			char outBuffer[256];
			if (GetPrivateProfileStringA("Base", "RequireMod", "false",
										 outBuffer, sizeof(outBuffer),
										 iniPath)) {
				if (_stricmp(outBuffer, "true") == 0 ||
					_stricmp(outBuffer, "on") == 0 ||
					_stricmp(outBuffer, "1") == 0) {
					checkMod = true;
					Output("Requires Mod = true");
				}
				else {
					Output("Requires Mod = false (%s)", outBuffer);
				}
			}
		}
	}
	else {
		Output("Error changing path extension");
		memset(iniPath, 0, sizeof(iniPath));
	}
}

bool isVehicleOk(Vehicle vehicle, Ped expectedDriver) {
	if (!VEHICLE::IS_VEHICLE_DRIVEABLE(vehicle, false)) {
		Output("Failed on Driveable");
		return false;
	}
	if (MineVehicles.find(ENTITY::GET_ENTITY_MODEL(vehicle)) ==
		MineVehicles.end()) {
		return false;
	}
	else if (checkMod && VEHICLE::GET_VEHICLE_MOD(vehicle, 9) != 0) {
		return false;
	}
	if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(vehicle, -1) != expectedDriver) {
		return false;
	}
	if (!ENTITY::IS_ENTITY_UPRIGHT(vehicle, 90)) {
		return false;
	}
	if (!VEHICLE::IS_VEHICLE_ON_ALL_WHEELS(vehicle)) {
		return false;
	}
	if (ENTITY::IS_ENTITY_UPSIDEDOWN(vehicle)) {
		return false;
	}
	return true;
}

auto getTime() { return std::chrono::milliseconds(GAMEPLAY::GET_GAME_TIMER()); }

constexpr auto mineModel  = joaat("w_ex_vehiclemine");
constexpr auto mineWeapon = joaat("vehicle_weapon_mine");

void modelOffsetStuff(Entity entity, Vector3 &uParam2, Vector3 &uParam3,
					  Vector3 &uParam4, Vector3 &uParam5) {
	Vector3 min;
	Vector3 max;

	GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(entity), &min,
								   &max);
	uParam2 = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, min.x,
															 max.y, min.z);
	uParam3 = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, max.x,
															 max.y, min.z);
	uParam4 = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, min.x,
															 min.y, min.z);
	uParam5 = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, max.x,
															 min.y, min.z);
}

float func_4394(float fParam0, float fParam1, float fParam2, float fParam3,
				float fParam4) {
	return (fParam1 - fParam0) / (fParam3 - fParam2) * (fParam4 - fParam2) +
		   fParam0;
}

auto func_4393(const Vector3 &vParam0, const Vector3 &vParam3, float fParam6,
			   float fParam7, float fParam8) {
	Vector3 result;
	result.x = func_4394(vParam0.x, vParam3.x, fParam6, fParam7, fParam8);
	result.y = func_4394(vParam0.y, vParam3.y, fParam6, fParam7, fParam8);
	result.z = func_4394(vParam0.z, vParam3.z, fParam6, fParam7, fParam8);
	return result;
}

void process() {
	using namespace std::chrono_literals;
	static bool requested = false;
	bool shouldRelease	= false;

	static auto nextRun  = 0ms;
	constexpr auto delay = 750ms;

	auto pSelf = PLAYER::PLAYER_ID();
	auto pPed  = PLAYER::PLAYER_PED_ID();

	if (PLAYER::IS_PLAYER_CONTROL_ON(pSelf)) {
		if (PED::IS_PED_IN_ANY_VEHICLE(pPed, false)) {
			if (auto vehicle = PED::GET_VEHICLE_PED_IS_USING(pPed);
				isVehicleOk(vehicle, pPed)) {
				CONTROLS::DISABLE_CONTROL_ACTION(0, ControlVehicleHorn, 1);
				if (!requested) {
					STREAMING::REQUEST_MODEL(mineModel);
					requested = true;
				}
				else if (STREAMING::HAS_MODEL_LOADED(mineModel)) {
					if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(
							0, ControlVehicleHorn)) {
						if (nextRun <= getTime()) {
							nextRun = getTime() + delay;
							Vector3 v1, v2, v3, v4;
							modelOffsetStuff(vehicle, v1, v2, v3, v4);
							auto vVar14 = func_4393(v1, v2, 0.f, 1.f, 0.5f);
							auto vVar17 = func_4393(v3, v4, 0.f, 1.f, 0.5f);
							vVar14.z += 0.2f;
							vVar17.z += 0.2f;
							auto vVar21 =
								func_4393(vVar14, vVar17, 0.f, 1.f, 0.7f);
							vVar14.z -= 0.2f;
							vVar17.z -= 0.2f;
							auto vVar24 =
								func_4393(vVar14, vVar17, 0.f, 1.f, 0.8f);
							CUSTOM::GAMEPLAY::_0xBFE5756E7407064A(
								vVar21, vVar24, 0, 1, mineWeapon, pPed, 1, 1,
								-1.f, 0, 0, 0, 0, 1, 1, 0);
#ifdef DEBUGOUT
							auto pedPos = ENTITY::GET_ENTITY_COORDS(pPed, 0);
							Output("Dropped Mine, Self pos = %.2f, %.2f, %.2f",
								   pedPos.x, pedPos.y, pedPos.z);
							Output("Target coord = %.2f, %.2f, %.2f", vVar24.x,
								   vVar24.y, vVar24.z);
#endif
						}
						else {
							Output("Horn Pressed, Time not ready");
						}
					}
				}
			}
			else {
				shouldRelease = false;
			}
		}
		else {
			shouldRelease = true;
		}
	}
	else {
		shouldRelease = true;
	}

	if (requested && shouldRelease) {
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(mineModel);
		requested = false;
	}

	if (CUSTOM::GAMEPLAY::_HAS_CHEAT_STRING_JUST_BEEN_ENTERED(
			joaat("minemod"))) {
		checkMod = !checkMod;
		UI::_SET_TEXT_ENTRY_2((char *)"STRING");
		UI::_ADD_TEXT_COMPONENT_STRING(
			(char *)(checkMod
						 ? "Mines now require vehicle proxy mine mod"
						 : "Mines no longer require vehicle proxy mines mod"));
		UI::_DRAW_SUBTITLE_TIMED(5000, 1);
		writeModSetting(checkMod);
	}
}

void ScriptMain() {
	if (getGameVersionSafe() >= GameVersion::v1_0_1103_2_Steam &&
		STREAMING::IS_MODEL_VALID(mineModel)) {
		while (true) {
			process();
			WAIT(0);
		}
	}
	else {
		Output("Error initialising");
		while (true) {
			WAIT(100'000'000);
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			scriptRegister(hInstance, ScriptMain);
			setupIni(hInstance);
			break;
		case DLL_PROCESS_DETACH: scriptUnregister(hInstance); break;
	}
	return TRUE;
}