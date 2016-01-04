#include <cstring>
#include <cmath>
#include <stdio.h>
#include <assert.h>

#include "djb2.h"
#include "yaml.h"
#include "rf2plugin.hpp"

//#include "api_variables.h"

/*
// telemetry update
void rf2plugin::YAMLupdate(const TelemInfoV01 &info) {

}
*/

// scoring update
void rf2plugin::YAMLupdate(const ScoringInfoV01 &info) {
	// WeekendInfo
	strncpy(weekendinfo.TrackName, info.mTrackName, sizeof(info.mTrackName));
	weekendinfo.WeekendOptions.NumStarters = info.mNumVehicles;
	weekendinfo.TrackLength = (float)info.mLapDist/1000;
	weekendinfo.TrackID = djb2((unsigned char *)&info.mTrackName) & 0xFFFFFF;

	if(info.mDarkCloud < 0.25)
		strcpy (weekendinfo.TrackSkies, "Clear");
	else if(info.mDarkCloud < 0.5)
		strcpy (weekendinfo.TrackSkies, "Partly Cloudy");
	else if(info.mDarkCloud < 0.5)
		strcpy (weekendinfo.TrackSkies, "Mostly Cloudy");
	else 
		strcpy (weekendinfo.TrackSkies, "Overcast");

	// mRaining not used
	weekendinfo.TrackAirTemp = (float)info.mAmbientTemp;
	weekendinfo.TrackSurfaceTemp = (float)info.mTrackTemp;

	// Wind isn't actually enabled, meh...
	weekendinfo.TrackWindVel = sqrtf((info.mWind.x * info.mWind.x) + (info.mWind.y * info.mWind.y));
	// direction A·B = ||A|| ||B|| cos theta
	if(weekendinfo.TrackWindVel > 0.0)
		weekendinfo.TrackWindDir = acosf((info.mWind.x + info.mWind.y) / (sqrtf((info.mWind.x * info.mWind.x) + (info.mWind.y * info.mWind.y)) * sqrtf(1)));
	else {
		weekendinfo.TrackWindDir = 0.0;
	}

	// mOnPathWetness not used
	// mOffPathWetness not used

	// SessionInfo
	sessioninfo.Sessions[info.mSession].SessionNum = info.mSession;
	sessioninfo.Sessions[info.mSession].SessionTime = (float)info.mEndET;

	switch (info.mSession) // (0=testday 1-4=practice 5-8=qual 9=warmup 10-13=race)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			strcpy (sessioninfo.Sessions[info.mSession].SessionType, "Practice");
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			strcpy (sessioninfo.Sessions[info.mSession].SessionType, "Qualify");
			break;
		case 9:
			strcpy (sessioninfo.Sessions[info.mSession].SessionType, "Warmup");
			break;
		case 10:
		case 11:
		case 12:
		case 13:
			strcpy (sessioninfo.Sessions[info.mSession].SessionType, "Race");
			break;
		default:
			break;
	}

	if(info.mMaxLaps >= 2147483647)
		strcpy(sessioninfo.Sessions[info.mSession].SessionLaps, "unlimited");
	else
		sprintf(sessioninfo.Sessions[info.mSession].SessionLaps, "%i", info.mMaxLaps);
	
	sessioninfo.Sessions[info.mSession].ResultsFastestLap.FastestTime = 999999.9f;

	// DriverInfo
	for( long i = 0; i < info.mNumVehicles; ++i )
    {
		VehicleScoringInfoV01 &vinfo = info.mVehicle[i];
		if((float)vinfo.mBestLapTime < sessioninfo.Sessions[info.mSession].ResultsFastestLap.FastestTime) {
			sessioninfo.Sessions[info.mSession].ResultsFastestLap.CarIdx = vinfo.mID;
			sessioninfo.Sessions[info.mSession].ResultsFastestLap.FastestTime = (float)vinfo.mBestLapTime;
			//sessioninfo.Sessions[info.mSession].ResultsFastestLap.FastestLap = ; dunno
		}

		sessioninfo.Sessions[info.mSession].ResultsPositions[i].CarIdx = vinfo.mID;
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].Position = vinfo.mPlace;
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].ClassPosition = vinfo.mPlace; // TODO
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].FastestTime = (float)vinfo.mBestLapTime;
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].Lap = vinfo.mTotalLaps;
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].LastTime = (float)vinfo.mLastLapTime;
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].LapsComplete = vinfo.mTotalLaps;
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].LapsDriven = vinfo.mTotalLaps; //(float)((double)vinfo.mTotalLaps + (vinfo.mLapDist/info.mLapDist));
		sessioninfo.Sessions[info.mSession].ResultsPositions[i].ReasonOutId = (int)vinfo.mFinishStatus;

		switch (vinfo.mFinishStatus) // 0=none, 1=finished, 2=dnf, 3=dq
		{
			case 0:
				strcpy(sessioninfo.Sessions[info.mSession].ResultsPositions[i].ReasonOutStr, "Running");
				break;
			case 1:
				strcpy(sessioninfo.Sessions[info.mSession].ResultsPositions[i].ReasonOutStr, "Finished");
				break;
			case 2:
				strcpy(sessioninfo.Sessions[info.mSession].ResultsPositions[i].ReasonOutStr, "DNF");
				break;
			case 3:
				strcpy(sessioninfo.Sessions[info.mSession].ResultsPositions[i].ReasonOutStr, "Disqualified");
				break;
			default:
				break;
		}

		driverinfo.Drivers[i].CarIdx = vinfo.mID;
		driverinfo.Drivers[i].UserID = djb2((unsigned char *)&vinfo.mDriverName) & 0xFFFFFF;
		driverinfo.Drivers[i].CarID = djb2((unsigned char *)&vinfo.mVehicleName) & 0xFFFFFF;
		driverinfo.Drivers[i].CarClassID = djb2((unsigned char *)&vinfo.mVehicleClass) & 0xFFFFFF;
		strcpy(driverinfo.Drivers[i].UserName, vinfo.mDriverName);
		strcpy(driverinfo.Drivers[i].CarPath, vinfo.mVehicleName);
		strcpy(driverinfo.Drivers[i].CarClassShortName, vinfo.mVehicleClass);

		if(vinfo.mIsPlayer) {
			driverinfo.DriverCarIdx = vinfo.mID;
		}
	}
	
	YAMLgenerate();
}

void rf2plugin::YAMLgenerate() {
	int len = 0;

	YPRINT("---\n");

	// WeekendInfo (current/latest data)
	YPRINT("WeekendInfo:\n");
	YPRINT(" TrackName: %s\n", weekendinfo.TrackName);
	YPRINT(" TrackID: %u\n", weekendinfo.TrackID);
	YPRINT(" TrackLength: %3.2f km\n", weekendinfo.TrackLength);
	YPRINT(" TrackDisplayName: %s\n", weekendinfo.TrackName);
	YPRINT(" TrackDisplayShortName: %s\n", weekendinfo.TrackName);
	YPRINT(" TrackCity: %s\n", weekendinfo.TrackCity);
	YPRINT(" TrackCountry: %s\n", weekendinfo.TrackCountry);
	YPRINT(" TrackAltitude: %3.2f\n", weekendinfo.TrackAltitude);
	YPRINT(" TrackLatitude: %f\n", weekendinfo.TrackLatitude);
	YPRINT(" TrackLongitude: %f\n", weekendinfo.TrackLongitude);
	YPRINT(" TrackNumTurns: %i\n", weekendinfo.TrackNumTurns);
	YPRINT(" TrackPitSpeedLimit: %f\n", weekendinfo.TrackPitSpeedLimit);
	YPRINT(" TrackType: %s\n", weekendinfo.TrackType);
	YPRINT(" TrackWeatherType: %s\n", weekendinfo.TrackWeatherType);
	YPRINT(" TrackSkies: %s\n", weekendinfo.TrackSkies);
	YPRINT(" TrackSurfaceTemp: %4.2f C\n", weekendinfo.TrackSurfaceTemp);
	YPRINT(" TrackAirTemp: %4.2f C\n", weekendinfo.TrackAirTemp);
	YPRINT(" TrackAirPressure: %4.2f Hg\n", weekendinfo.TrackAirPressure);
	YPRINT(" TrackWindVel: %4.2f m/s\n", weekendinfo.TrackWindVel);
	YPRINT(" TrackWindDir: %4.2f rad\n", weekendinfo.TrackWindDir);
	YPRINT(" TrackRelativeHumidity: %i %%\n", weekendinfo.TrackRelativeHumidity);
	YPRINT(" TrackFogLevel: %i %%\n", weekendinfo.TrackFogLevel);
	YPRINT(" SeriesID: %u\n", weekendinfo.SeriesID);
	YPRINT(" SeasonID: %u\n", weekendinfo.SeasonID);
	YPRINT(" SessionID: %u\n", weekendinfo.SessionID);
	YPRINT(" SubSessionID: %u\n", weekendinfo.SubSessionID);
	YPRINT(" LeagueID: %u\n", weekendinfo.LeagueID);
	YPRINT(" Official: %u\n", weekendinfo.Offcial);
	YPRINT(" RaceWeek: %u\n", weekendinfo.RaceWeek);
	YPRINT(" EventType: %s\n", weekendinfo.EventType);
	YPRINT(" Category: %s\n", weekendinfo.Category);
	YPRINT(" SimMode: %s\n", weekendinfo.SimMode);

	// WeekendOptions (server settings when created)
	YPRINT(" WeekendOptions:\n");
	YPRINT("  NumStarters: %u\n", weekendinfo.WeekendOptions.NumStarters);
	YPRINT("  StartingGrid: %s\n", weekendinfo.WeekendOptions.StandingStart);
	YPRINT("  QualifyScoring: %s\n", weekendinfo.WeekendOptions.QualifyScoring);
	YPRINT("  CourseCautions: %s\n", weekendinfo.WeekendOptions.CourseCautions);
	YPRINT("  StandingStart: %u\n", weekendinfo.WeekendOptions.StandingStart);
	YPRINT("  Restarts: %s\n", weekendinfo.WeekendOptions.Restarts);
	YPRINT("  WeatherType: %s\n", weekendinfo.WeekendOptions.WeatherType);
	YPRINT("  Skies: %s\n", weekendinfo.WeekendOptions.Skies);
	YPRINT("  WindDirection: %s\n", weekendinfo.WeekendOptions.WindDirection);
	YPRINT("  WindSpeed: %f m/s\n", weekendinfo.WeekendOptions.WindSpeed);
	YPRINT("  WeatherTemp: %f rad\n", weekendinfo.WeekendOptions.WeatherTemp);
	YPRINT("  RelativeHumidity: %i %%\n", weekendinfo.WeekendOptions.RelativeHumidity);
	YPRINT("  FogLevel: %i %%\n", weekendinfo.WeekendOptions.FogLevel);
	YPRINT("  Unofficial: %u\n", weekendinfo.WeekendOptions.Unofficial);
	YPRINT("  CommercialMode: %s\n", weekendinfo.WeekendOptions.CommercialMode);
	YPRINT("  NightMode: %i\n", weekendinfo.WeekendOptions.NightMode);
	YPRINT("  IsFixedSetup: %i\n", weekendinfo.WeekendOptions.isFixedSetup);
	YPRINT("  StrictLapsChecking: %s\n", weekendinfo.WeekendOptions.StrictLapsChecking);
	YPRINT("  HasOpenRegistration: %i\n", weekendinfo.WeekendOptions.HasOpenRegistration);
	YPRINT("  HardcoreLevel: %i\n", weekendinfo.WeekendOptions.HardcoreLevel);
	YPRINT("\n");

	// SessionInfo
	YPRINT("SessionInfo:\n");
	YPRINT(" Sessions:\n");
	for(int i = 0; i < MAX_SESSIONS; i++) {
		if(sessioninfo.Sessions[i].SessionTime > 0.0f) {
			YPRINT(" - SessionNum: %i\n", sessioninfo.Sessions[i].SessionNum);
			YPRINT("   SessionLaps: %s\n", sessioninfo.Sessions[i].SessionLaps);
			YPRINT("   SessionTime: %3.2f\n", sessioninfo.Sessions[i].SessionTime);
			YPRINT("   SessionNumLapsToAvg: %i\n", sessioninfo.Sessions[i].SessionNumLapsToAvg);
			YPRINT("   SessionType: %s\n", sessioninfo.Sessions[i].SessionType);
			YPRINT("   ResultsAverageLapTime: %4.3f\n", sessioninfo.Sessions[i].ResultsAverageLapTime);
			YPRINT("   ResultsNumCautionFlags: %i\n", sessioninfo.Sessions[i].ResultsNumCautionFlags);
			YPRINT("   ResultsNumCautionLaps: %i\n", sessioninfo.Sessions[i].ResultsNumCautionLaps);
			YPRINT("   ResultsNumLeadChanges: %i\n", sessioninfo.Sessions[i].ResultsNumLeadChanges);
			YPRINT("   ResultsLapsComplete: %i\n", sessioninfo.Sessions[i].ResultsLapsComplete);
			YPRINT("   ResultsOfficial: %i\n", sessioninfo.Sessions[i].ResultsOfficial);
			YPRINT("   ResultsPositions:\n");
			for(int j = 0; j < MAX_CARS; j++) {
				if(sessioninfo.Sessions[i].ResultsPositions[j].CarIdx > 0) {
					YPRINT("   - Position: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].Position);
					YPRINT("     ClassPosition: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].ClassPosition);
					YPRINT("     CarIdx: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].CarIdx);
					YPRINT("     Lap: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].Lap);
					YPRINT("     Time: %4.3f\n", sessioninfo.Sessions[i].ResultsPositions[j].Time);
					YPRINT("     FastestLap: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].FastestLap);
					YPRINT("     FastestTime: %4.3f\n", sessioninfo.Sessions[i].ResultsPositions[j].FastestTime);
					YPRINT("     LastTime: %4.3f\n", sessioninfo.Sessions[i].ResultsPositions[j].LastTime);
					YPRINT("     LapsLed: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].LapsLed);
					YPRINT("     LapsComplete: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].LapsComplete);
					YPRINT("     LapsDriven: %4.3f\n", sessioninfo.Sessions[i].ResultsPositions[j].LapsDriven);
					YPRINT("     Incidents: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].Incidents);
					YPRINT("     ReasonOutId: %i\n", sessioninfo.Sessions[i].ResultsPositions[j].ReasonOutId);
					YPRINT("     ReasonOutStr: %s\n", sessioninfo.Sessions[i].ResultsPositions[j].ReasonOutStr);
				}
			}
			YPRINT("   ResultsFastestLap:\n");
			YPRINT("   - CarIdx: %i\n", sessioninfo.Sessions[i].ResultsFastestLap.CarIdx);
			YPRINT("     FastestLap: %i\n", sessioninfo.Sessions[i].ResultsFastestLap.FastestLap);
			YPRINT("     FastestTime: %4.3f\n", sessioninfo.Sessions[i].ResultsFastestLap.FastestTime);
		}
	}
	YPRINT("\n");

	// Camera info (static, bare minimum definition)
	YPRINT("CameraInfo:\n");
	YPRINT(" Groups:\n");
	YPRINT(" - GroupNum: 1\n");
	YPRINT("   GroupName: Rollbar\n");
	YPRINT(" - GroupNum: 2\n");
	YPRINT("   GroupName: Cockpit\n");
	YPRINT(" - GroupNum: 3\n");
	YPRINT("   GroupName: Nose\n");
	YPRINT(" - GroupNum: 4\n");
	YPRINT("   GroupName: Chase\n");
	YPRINT(" - GroupNum: 5\n");
	YPRINT("   GroupName: TV\n");
	YPRINT(" - GroupNum: 6\n");
	YPRINT("   GroupName: unknown\n");
	YPRINT(" - GroupNum: 7\n");
	YPRINT("   GroupName: FrontBumper\n");
	YPRINT(" - GroupNum: 8\n");
	YPRINT("   GroupName: RearBumper\n");
	YPRINT(" - GroupNum: 9\n");
	YPRINT("   GroupName: Side\n");
	YPRINT(" - GroupNum: 10\n");
	YPRINT("   GroupName: Rear\n");
	YPRINT("\n");

	// SessionInfo
	YPRINT("DriverInfo:\n");
	YPRINT(" DriverCarIdx: %i\n", driverinfo.DriverCarIdx);
	YPRINT(" DriverHeadPosX: %4.3f\n", driverinfo.DriverHeadPosX);
	YPRINT(" DriverHeadPosY: %4.3f\n", driverinfo.DriverHeadPosY);
	YPRINT(" DriverHeadPosZ: %4.3f\n", driverinfo.DriverHeadPosZ);
	YPRINT(" DriverCarRedLine: %4.3f\n", driverinfo.DriverCarRedLine);
	YPRINT(" DriverCarFuelKgPerLtr: %4.3f\n", driverinfo.DriverCarFuelKgPerLtr);
	YPRINT(" DriverCarSLFirstRPM: %4.3f\n", driverinfo.DriverCarSLFirstRPM);
	YPRINT(" DriverCarSLShiftRPM: %4.3f\n", driverinfo.DriverCarSLShiftRPM);
	YPRINT(" DriverCarSLLastRPM: %4.3f\n", driverinfo.DriverCarSLLastRPM);
	YPRINT(" DriverCarSLBlinkRPM: %4.3f\n", driverinfo.DriverCarSLBlinkRPM);
	YPRINT(" DriverPitTrkPct: %4.3f\n", driverinfo.DriverPitTrkPct);
	YPRINT(" Drivers:\n");
	for(int i = 0; i < MAX_CARS; i++) {
		if(driverinfo.Drivers[i].CarIdx > 0) {
			YPRINT(" - CarIdx: %i\n", driverinfo.Drivers[i].CarIdx);
			YPRINT("   UserName: %s\n", driverinfo.Drivers[i].UserName);
			YPRINT("   AbbrevName: %s\n", driverinfo.Drivers[i].AbbrevName);
			YPRINT("   Initials: %s\n", driverinfo.Drivers[i].Initials);
			YPRINT("   UserID: %u\n", driverinfo.Drivers[i].UserID);
			YPRINT("   CarNumber: %i\n", driverinfo.Drivers[i].CarNumber);
			YPRINT("   CarPath: %s\n", driverinfo.Drivers[i].CarPath);
			YPRINT("   CarClassID: %u\n", driverinfo.Drivers[i].CarClassID);
			YPRINT("   CarID: %u\n", driverinfo.Drivers[i].CarID);
			YPRINT("   CarClassShortName: %s\n", driverinfo.Drivers[i].CarClassShortName);
			YPRINT("   CarClassRelSpeed: %i\n", driverinfo.Drivers[i].CarClassRelSpeed);
			YPRINT("   CarClassLicenseLevel: %i\n", driverinfo.Drivers[i].CarClassLicenseLevel);
			YPRINT("   CarClassMaxFuel: %f %%\n", driverinfo.Drivers[i].CarClassMaxFuel);
			YPRINT("   CarClassWeightPenalty: %f kg\n", driverinfo.Drivers[i].CarClassWeightPenalty);
			YPRINT("   IRating: %i\n", driverinfo.Drivers[i].IRating);
			YPRINT("   LicLevel: %i\n", driverinfo.Drivers[i].LicLevel);
			YPRINT("   LicSubLevel: %i\n", driverinfo.Drivers[i].LicSubLevel);
			YPRINT("   LicColor: %s\n", driverinfo.Drivers[i].LicColor);
			YPRINT("   ClubName: %s\n", driverinfo.Drivers[i].ClubName);
			YPRINT("   DivisionName: %s\n", driverinfo.Drivers[i].DivisionName);
		}
	}
	YPRINT("\n");

	// Splits
	YPRINT("SplitTimeInfo:\n");
	YPRINT(" Sectors:\n");
	YPRINT(" - SectorNum: 0\n");
	YPRINT("   SectorStartPct: %1.3f\n", m_sectorPos[1]);
	YPRINT(" - SectorNum: 1\n");
	YPRINT("   SectorStartPct: %1.3f\n", m_sectorPos[2]);
	YPRINT(" - SectorNum: 2\n");
	YPRINT("   SectorStartPct: %1.3f\n", m_sectorPos[0]);
	YPRINT("\n");

	// end YAML
	YPRINT("...\n");

	YAMLstring[YAMLstring_maxlen-1] = '\0';
	assert(len < (YAMLstring_maxlen-256));

	YAMLstring_len = len;

}
