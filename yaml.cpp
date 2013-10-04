#include <cstring>
#include <cmath>

#include "yaml.h"

// telemetry update
bool YAMLupdate(const TelemInfoV01 &info) {
	return true;
}

// scoring update
bool YAMLupdate(const ScoringInfoV01 &info) {
	// WeekendInfo
	strncpy(weekendinfo.TrackName, info.mTrackName, sizeof(info.mTrackName));
	weekendinfo.WeekendOptions.NumStarters = info.mNumVehicles;

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
	weekendinfo.TrackWindVel = (float)sqrtf((info.mWind.x * info.mWind.x) + (info.mWind.y * info.mWind.y));

	// direction A·B = ||A|| ||B|| cos theta
	weekendinfo.TrackWindDir = (float)acosf((info.mWind.x + info.mWind.y) / (abs(info.mWind.x) * abs(info.mWind.y)));

	// mOnPathWetness not used
	// mOffPathWetness not used

	/*
	// DriverInfo
	for( long i = 0; i < info.mNumVehicles; ++i )
    {

	}
	*/

	return true;
}

