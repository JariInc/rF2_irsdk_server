#ifndef APIVARS_DEFINES_H
#define APIVARS_DEFINES_H

// All the variables API provides are defined here

/* Examples of static and dynamic variables
// add an output based on a static var, 
// nothing more is needed here the irsdk server 
// will automatically poll the variable every tick
static float g_staticVariable[10] = { 0.0f };
static irsdkVar irsdkStaticVariable("StaticVariable", &g_staticVariable, irsdk_float, 10, "Static variable array that is auto polled", "m/s", IRSDK_LOG_ALL);

// add an output that is manually updated every tick.
// You can use the call iradkDynamicVar->setValue(x) to update it.
static irsdkVar irsdkManualSet("ManualSet", NULL, irsdk_bool, 1, "Manually set a value to telemetry every tick", "", IRSDK_LOG_LIVE);
*/

// Session parameters
static double g_sessionTime = 0;
static irsdkVar irsdkSessionTime("SessionTime", &g_sessionTime, irsdk_double, 1, "Seconds since session start", "s", IRSDK_LOG_ALL);

static int g_sessionNum = 0;
static irsdkVar irsdkSessionNum("SessionNum", &g_sessionNum, irsdk_int, 1, "Session number", "", IRSDK_LOG_ALL);

static int g_sessionState = 0;
static irsdkVar irsdkSessionState("SessionState", &g_sessionState, irsdk_int, 1, "Session state", "irsdk_SessionState", IRSDK_LOG_ALL);

static int g_sessionUniqueID = 0;
static irsdkVar irsdkSessionUniqueID("SessionUniqueID", &g_sessionUniqueID, irsdk_int, 1, "Session ID", "", IRSDK_LOG_ALL);

static int g_sessionFlags = 0;
static irsdkVar irsdkSessionFlags("SessionFlags", &g_sessionFlags, irsdk_bitField, 1, "Session flags", "irsdk_Flags", IRSDK_LOG_ALL);

static double g_sessionTimeRemain = 0;
static irsdkVar irsdkSessionTimeRemain("SessionTimeRemain", &g_sessionTimeRemain, irsdk_double, 1, "Seconds left till session ends", "s", IRSDK_LOG_ALL);

static int g_sessionLapsRemain = 0;
static irsdkVar irsdkSessionLapsRemain("SessionLapsRemain", &g_sessionLapsRemain, irsdk_int, 1, "Laps left till session ends", "", IRSDK_LOG_ALL);

static int g_lap = 0;
static irsdkVar irsdkLap("Lap", &g_lap, irsdk_int, 1, "Lap count", "", IRSDK_LOG_ALL);

static int g_camcaridx = 0;
static irsdkVar irsdkCamCarIdx("CamCarIdx", &g_camcaridx, irsdk_int, 1, "Active camera's focus car index", "", IRSDK_LOG_ALL);

static int g_replayFrameNum = 0;
static irsdkVar irsdkReplayFrameNum("ReplayFrameNum", &g_replayFrameNum, irsdk_int, 1, "Integer replay frame number (60 per second)", "", IRSDK_LOG_ALL);

static double g_replaySessionTime = 0;
static irsdkVar irsdkReplaySessionTime("ReplaySessionTime", &g_replaySessionTime, irsdk_double, 1, "Seconds since replay session start", "s", IRSDK_LOG_ALL);

static float g_carIdxLapDistPct[64] = { -1.0f };
static irsdkVar irsdkCarIdxLapDistPct("CarIdxLapDistPct", &g_carIdxLapDistPct, irsdk_float, 64, "Percentage distance around lap by car index", "%", IRSDK_LOG_ALL);

static int g_carIdxLap[64] = { -1.0f };
static irsdkVar irsdkCarIdxLap("CarIdxLap", &g_carIdxLap, irsdk_int, 64, "Lap count by car index", "", IRSDK_LOG_ALL);


#endif //APIVARS_DEFINES_H
