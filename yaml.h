#ifndef YAML_H
#define YAML_H

#include "irsdk_server.h"
#include "rf2plugin.hpp"

// YAML structs
struct WeekendOptions {
	int NumStarters;
	char StartingGrid[IRSDK_MAX_STRING];
	char QualifyScoring[IRSDK_MAX_STRING];		// enum
	char CourseCautions[IRSDK_MAX_STRING];		// enum
	int StandingStart;
	char Restarts[IRSDK_MAX_STRING];			// enum
	char WeatherType[IRSDK_MAX_STRING];			// enum
	char Skies[IRSDK_MAX_STRING];				// enum
	char WindDirection[IRSDK_MAX_STRING];		// enum
	float WindSpeed;
	float WeatherTemp;
	int RelativeHumidity;
	int FogLevel;
	int Unofficial;
	char CommercialMode[IRSDK_MAX_STRING];		// enum
	int NightMode;
	int isFixedSetup;
	char StrictLapsChecking[IRSDK_MAX_STRING];	// enum
	int HasOpenRegistration;
	int HardcoreLevel;
};

struct WeekendInfo {
	char TrackName[IRSDK_MAX_STRING];
	int TrackID;
	float TrackLength;
	char TrackDisplayName[IRSDK_MAX_STRING];
	char TrackDisplayShortName[IRSDK_MAX_STRING];
	char TrackCity[IRSDK_MAX_STRING];
	char TrackCountry[IRSDK_MAX_STRING];
	float TrackAltitude;
	double TrackLatitude;
	double TrackLongitude;
	int TrackNumTurns;
	float TrackPitSpeedLimit;
	char TrackType[IRSDK_MAX_STRING];			// enum
	char TrackWeatherType[IRSDK_MAX_STRING];	// enum
	char TrackSkies[IRSDK_MAX_STRING];			// enum
	float TrackSurfaceTemp;
	float TrackAirTemp;
	float TrackAirPressure;
	float TrackWindVel;
	float TrackWindDir;
	int TrackRelativeHumidity;
	int TrackFogLevel;
	int SeriesID;
	int SeasonID;
	int SessionID;
	int SubSessionID;
	int LeagueID;
	int Offcial;
	int RaceWeek;
	char EventType[IRSDK_MAX_STRING];			// enum
	char Category[IRSDK_MAX_STRING];			// enum
	char SimMode[IRSDK_MAX_STRING];				// enum
	WeekendOptions WeekendOptions;
};

// function prototypes
bool YAMLupdate(const ScoringInfoV01 &info);
bool YAMLupdate(const TelemInfoV01 &info);

// static variables
static const int YAMLstring_len = irsdkServer::sessionStrLen;
static char YAMLstring[YAMLstring_len] = "";
static WeekendInfo weekendinfo;

#endif //YAML_H
