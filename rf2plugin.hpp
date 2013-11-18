//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//›                                                                         ﬁ
//› Module: Internals Example Header File                                   ﬁ
//›                                                                         ﬁ
//› Description: Declarations for the Internals Example Plugin              ﬁ
//›                                                                         ﬁ
//›                                                                         ﬁ
//› This source code module, and all information, data, and algorithms      ﬁ
//› associated with it, are part of CUBE technology (tm).                   ﬁ
//›                 PROPRIETARY AND CONFIDENTIAL                            ﬁ
//› Copyright (c) 1996-2008 Image Space Incorporated.  All rights reserved. ﬁ
//›                                                                         ﬁ
//›                                                                         ﬁ
//› Change history:                                                         ﬁ
//›   tag.2005.11.30: created                                               ﬁ
//›                                                                         ﬁ
//ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ

#ifndef _INTERNALS_EXAMPLE_H
#define _INTERNALS_EXAMPLE_H

#include "InternalsPlugin.hpp"
#include "irsdk_server.h"
#include "yaml.h"

enum WheelPosition {
	LF =0,
	RF,
	LR,
	RR
};

struct TelemWheelInfoData
{
  float mSuspensionDeflection;  // meters
  float mRideHeight;            // meters
  float mSuspForce;             // pushrod load in Newtons
  float mBrakeTemp;             // Celsius
  float mBrakePressure;         // currently 0.0-1.0, depending on driver input and brake balance; will convert to true brake pressure (kPa) in future

  float mRotation;              // radians/sec
  float mLateralPatchVel;       // lateral velocity at contact patch
  float mLongitudinalPatchVel;  // longitudinal velocity at contact patch
  float mLateralGroundVel;      // lateral velocity at contact patch
  float mLongitudinalGroundVel; // longitudinal velocity at contact patch
  float mCamber;                // radians (positive is left for left-side wheels, right for right-side wheels)
  float mLateralForce;          // Newtons
  float mLongitudinalForce;     // Newtons
  float mTireLoad;              // Newtons

  float mGripFract;             // an approximation of what fraction of the contact patch is sliding
  float mPressure;              // kPa (tire pressure)
  float mTemperature[3];        // Kelvin (subtract 273.16 to get Celsius), left/center/right (not to be confused with inside/center/outside!)
  float mWear;                  // wear (0.0-1.0, fraction of maximum) ... this is not necessarily proportional with grip loss
  char mTerrainName[16];         // the material prefixes from the TDF file
  unsigned char mSurfaceType;    // 0=dry, 1=wet, 2=grass, 3=dirt, 4=gravel, 5=rumblestrip
  bool mFlat;                    // whether tire is flat
  bool mDetached;                // whether wheel is detached

  float mVerticalTireDeflection; // how much is tire deflected from its (speed-sensitive) radius
  float mWheelYLocation;        // wheel's y location relative to vehicle y location
  float mToe;                   // current toe angle w.r.t. the vehicle
};


struct TelemInfoData
{
  // Time
  long mID;                      // slot ID (note that it can be re-used in multiplayer after someone leaves)
								 // --- currently we only export data for the player car, so this should be safe to use as an identifier

  //double mDeltaTime;             // time since last update (seconds)
  //double mElapsedTime;           // game session time
  //long mLapNumber;               // current lap number
  //double mLapStartET;            // time this lap was started
  //char mVehicleName[64];         // current vehicle name
  //char mTrackName[64];           // current track name

	float mLapDist;
	float mLapDistPct;
	
  // Position and derivatives
  float mPos[3];               // world position in meters (XYZ)
  float mLocalVel[3];          // velocity (meters/sec) in local vehicle coordinates
  float mLocalAccel[3];        // acceleration (meters/sec^2) in local vehicle coordinates

  float mSpeed;
  float mLatAccel;
  float mLongAccel;

  // Orientation and derivatives
  float mLocalRot[3];          // rotation (radians/sec) in local vehicle coordinates
  float mLocalRotAccel[3];     // rotational acceleration (radians/sec^2) in local vehicle coordinates

  // Vehicle status
  int mGear;                    // -1=reverse, 0=neutral, 1+=forward gears
  float mEngineRPM;             // engine RPM
  float mEngineWaterTemp;       // Celsius
  float mEngineOilTemp;         // Celsius
  float mClutchRPM;             // clutch RPM

  // Driver input
  float mUnfilteredThrottle;    // ranges  0.0-1.0
  float mUnfilteredBrake;       // ranges  0.0-1.0
  float mUnfilteredSteering;    // ranges -1.0-1.0 (left to right)
  float mUnfilteredClutch;      // ranges  0.0-1.0

  // Filtered input (various adjustments for rev or speed limiting, TC, ABS?, speed sensitive steering, clutch work for semi-automatic shifting, etc.)
  float mFilteredThrottle;      // ranges  0.0-1.0
  float mFilteredBrake;         // ranges  0.0-1.0
  float mFilteredSteering;      // ranges -1.0-1.0 (left to right)
  float mFilteredClutch;        // ranges  0.0-1.0

  // Misc
  //double mSteeringArmForce;      // force on steering arms
  //double mFront3rdDeflection;    // deflection at front 3rd spring
  //double mRear3rdDeflection;     // deflection at rear 3rd spring

  // Aerodynamics
  float mFrontWingHeight;       // front wing height
  float mFrontRideHeight;       // front ride height
  float mRearRideHeight;        // rear ride height
  float mDrag;                  // drag
  float mFrontDownforce;        // front downforce
  float mRearDownforce;         // rear downforce

  // State/damage info
  float mFuel;                  // amount of fuel (liters)
  float mEngineMaxRPM;          // rev limit
  unsigned char mScheduledStops; // number of scheduled pitstops
  bool  mOverheating;            // whether overheating icon is shown
  bool  mDetached;               // whether any parts (besides wheels) have been detached
  bool  mHeadlights;             // whether headlights are on
  unsigned char mSpeedLimiter;   // whether speed limiter is on
  //unsigned char mDentSeverity[8];// dent severity at 8 locations around the car (0=none, 1=some, 2=more)
  //float mLastImpactET;          // time of last impact
  //float mLastImpactMagnitude;   // magnitude of last impact
  //TelemVect3 mLastImpactPos;     // location of last impact

  // Expanded
  double mEngineTq;              // current engine torque (including additive torque)
  //long mCurrentSector;           // the current sector (zero-based) with the pitlane stored in the sign bit (example: entering pits from third sector gives 0x80000002)
  unsigned char mMaxGears;       // maximum forward gears
  unsigned char mFrontTireCompoundIndex;   // index within brand
  unsigned char mRearTireCompoundIndex;    // index within brand
  float mFuelCapacity;          // capacity in liters
  unsigned char mFrontFlapActivated;       // whether front flap is activated
  unsigned char mRearFlapActivated;        // whether rear flap is activated
  unsigned char mRearFlapLegalStatus;      // 0=disallowed, 1=criteria detected but not allowed quite yet, 2=allowed
  unsigned char mIgnitionStarter;          // 0=off 1=ignition 2=ignition+starter

  // keeping this at the end of the structure to make it easier to replace in future versions
  TelemWheelInfoData mWheel[4];       // wheel info (front left, front right, rear left, rear right)
};


// This is used for the app to use the plugin for its intended purpose
class rf2plugin : public InternalsPluginV05
{

 public:

  // Constructor/destructor
  rf2plugin() {

	  memset(&m_telemetryInfo, 0, sizeof(TelemInfoData));

	  irsdkSessionTime = irsdkVar("SessionTime", &g_sessionTime, irsdk_double, 1, "Seconds since session start", "s", IRSDK_LOG_ALL);
	  irsdkSessionNum = irsdkVar("SessionNum", &g_sessionNum, irsdk_int, 1, "Session number", "", IRSDK_LOG_LIVE);
	  irsdkSessionState = irsdkVar("SessionState", &g_sessionState, irsdk_int, 1, "Session state", "irsdk_SessionState", IRSDK_LOG_ALL);
	  irsdkSessionUniqueID = irsdkVar("SessionUniqueID", &g_sessionUniqueID, irsdk_int, 1, "Session ID", "", IRSDK_LOG_LIVE);
	  irsdkSessionFlags = irsdkVar("SessionFlags", &g_sessionFlags, irsdk_bitField, 1, "Session flags", "irsdk_Flags", IRSDK_LOG_ALL);
      irsdkSessionTimeRemain = irsdkVar("SessionTimeRemain", &g_sessionTimeRemain, irsdk_double, 1, "Seconds left till session ends", "s", IRSDK_LOG_LIVE);
	  irsdkSessionLapsRemain = irsdkVar("SessionLapsRemain", &g_sessionLapsRemain, irsdk_int, 1, "Laps left till session ends", "", IRSDK_LOG_LIVE);
	  irsdkLap = irsdkVar("Lap", &g_lap, irsdk_int, 1, "Lap count", "", IRSDK_LOG_ALL);
	  irsdkCamCarIdx = irsdkVar("CamCarIdx", &g_camcaridx, irsdk_int, 1, "Active camera's focus car index", "", IRSDK_LOG_LIVE);
	  irsdkCamGroupNumber = irsdkVar("CamGroupNumber", &g_camgroupnumber, irsdk_int, 1, "Active camera group number", "", IRSDK_LOG_LIVE);
	  irsdkReplayFrameNum = irsdkVar("ReplayFrameNum", &g_replayFrameNum, irsdk_int, 1, "Integer replay frame number (60 per second)", "", IRSDK_LOG_LIVE);
	  irsdkReplaySessionTime = irsdkVar("ReplaySessionTime", &g_replaySessionTime, irsdk_double, 1, "Seconds since replay session start", "s", IRSDK_LOG_LIVE);
	  irsdkCarIdxLapDistPct = irsdkVar("CarIdxLapDistPct", &g_carIdxLapDistPct, irsdk_float, 64, "Percentage distance around lap by car index", "%", IRSDK_LOG_LIVE);
	  irsdkCarIdxLap = irsdkVar("CarIdxLap", &g_carIdxLap, irsdk_int, 64, "Lap count by car index", "", IRSDK_LOG_LIVE);
	  irsdkCarIdxTrackSurface = irsdkVar("CarIdxTrackSurface", &g_carIdxLap, irsdk_int, 64, "Track surface type by car index", "", IRSDK_LOG_LIVE);


	  irsdkVar("LapDist_m", &m_telemetryInfo.mLapDist, irsdk_float, 1, "Meters traveled from S/F this lap", "m", IRSDK_LOG_ALL);
	  irsdkVar("LapDistPct", &m_telemetryInfo.mLapDistPct, irsdk_float, 1, "Percentage distance around lap", "%", IRSDK_LOG_ALL);

	  // Telemetry Data
	  irsdkVar("Speed", &m_telemetryInfo.mSpeed, irsdk_float, 1, "Car speed", "m/s", IRSDK_LOG_ALL);
	  irsdkVar("LatAccel", &m_telemetryInfo.mLatAccel, irsdk_float, 1, "Lateral acceleration", "m/s^2", IRSDK_LOG_ALL);
	  irsdkVar("LongAccel", &m_telemetryInfo.mLongAccel, irsdk_float, 1, "Longitudinal acceleration", "m/s^2", IRSDK_LOG_ALL);

	  irsdkVar("VelocityX", &m_telemetryInfo.mLocalVel[0], irsdk_float, 1, "X velocity", "m/s", IRSDK_LOG_ALL);
	  irsdkVar("VelocityY", &m_telemetryInfo.mLocalVel[1], irsdk_float, 1, "Y velocity", "m/s", IRSDK_LOG_ALL);
	  irsdkVar("VelocityZ", &m_telemetryInfo.mLocalVel[2], irsdk_float, 1, "Z velocity", "m/s", IRSDK_LOG_ALL);

	  irsdkVar("PositionX", &m_telemetryInfo.mPos[0], irsdk_float, 1, "X Position in world", "m", IRSDK_LOG_DISK);
	  irsdkVar("PositionY", &m_telemetryInfo.mPos[1], irsdk_float, 1, "Y Position in world", "m", IRSDK_LOG_DISK);
	  irsdkVar("PositionZ", &m_telemetryInfo.mPos[2], irsdk_float, 1, "Z Position in world", "m", IRSDK_LOG_DISK);

	  irsdkVar("Gear", &m_telemetryInfo.mGear, irsdk_int, 1, "Gear", "", IRSDK_LOG_ALL, 1, 0);
	  irsdkVar("RPM", &m_telemetryInfo.mEngineRPM, irsdk_float, 1, "Engine RPM", "revs/min", IRSDK_LOG_ALL);
      irsdkVar("WaterTemp", &m_telemetryInfo.mEngineWaterTemp, irsdk_float, 1, "Engine coolant temp", "C", IRSDK_LOG_ALL);
	  irsdkVar("OilTemp", &m_telemetryInfo.mEngineOilTemp, irsdk_float, 1, "Engine oil temperature", "C", IRSDK_LOG_ALL);
	  irsdkVar("ClutchRPM", &m_telemetryInfo.mClutchRPM, irsdk_float, 1, "Clutch RPM", "", IRSDK_LOG_ALL);
	  irsdkVar("RawBrakePos", &m_telemetryInfo.mUnfilteredBrake, irsdk_float, 1, "Unfiltered brake pedal position", "%", IRSDK_LOG_DISK);
	  irsdkVar("RawClutchPos", &m_telemetryInfo.mUnfilteredClutch, irsdk_float, 1, "Unfiltered clutch pedal position", "%", IRSDK_LOG_DISK);
	  irsdkVar("RawSteeringWheelPos", &m_telemetryInfo.mUnfilteredSteering, irsdk_float, 1, "Unfiltered steering wheel position", "%", IRSDK_LOG_DISK);
	  irsdkVar("RawThrottlePos", &m_telemetryInfo.mUnfilteredThrottle, irsdk_float, 1, "Unfiltered throttle pedal position", "%", IRSDK_LOG_DISK);
	  irsdkVar("Brake", &m_telemetryInfo.mFilteredBrake, irsdk_float, 1, "0=brake released to 1=max pedal force", "%", IRSDK_LOG_ALL);
	  irsdkVar("Clutch", &m_telemetryInfo.mFilteredClutch, irsdk_float, 1, "0=disengaged to 1=fully engaged", "%", IRSDK_LOG_ALL);
	  irsdkVar("SteeringWheelPos", &m_telemetryInfo.mFilteredSteering, irsdk_float, 1, "Steering wheel position", "%", IRSDK_LOG_ALL);
	  irsdkVar("Throttle", &m_telemetryInfo.mFilteredThrottle, irsdk_float, 1, "0=off throttle to 1=full throttle", "%", IRSDK_LOG_ALL);

	  irsdkVar("CFSRrideHeight", &m_telemetryInfo.mFrontRideHeight, irsdk_float, 1, "Center front splitter ride height", "m", IRSDK_LOG_ALL);
	  irsdkVar("CRDRideHeight", &m_telemetryInfo.mRearRideHeight, irsdk_float, 1, "Center rear diffusor ride height", "m", IRSDK_LOG_ALL);

	  irsdkVar("FuelLevel", &m_telemetryInfo.mFuel, irsdk_float, 1, "Liters of fuel remaining", "L", IRSDK_LOG_ALL);
	  irsdkVar("FuelCapacity", &m_telemetryInfo.mFuelCapacity, irsdk_float, 1, "Fueltank capacity", "L", IRSDK_LOG_ALL);

	  irsdkVar("EngineTorque", &m_telemetryInfo.mEngineTq, irsdk_float, 1, "Engine torque (including additional torque)", "Nm", IRSDK_LOG_ALL);

	  irsdkVar("LFrideHeight", &m_telemetryInfo.mWheel[LF].mRideHeight, irsdk_float, 1, "LF ride height", "m", IRSDK_LOG_ALL);
	  irsdkVar("RFrideHeight", &m_telemetryInfo.mWheel[RF].mRideHeight, irsdk_float, 1, "RF ride height", "m", IRSDK_LOG_ALL);
	  irsdkVar("LRrideHeight", &m_telemetryInfo.mWheel[LR].mRideHeight, irsdk_float, 1, "LR ride height", "m", IRSDK_LOG_ALL);
	  irsdkVar("RRrideHeight", &m_telemetryInfo.mWheel[RR].mRideHeight, irsdk_float, 1, "RR ride height", "m", IRSDK_LOG_ALL);

	  irsdkVar("LFshockDefl", &m_telemetryInfo.mWheel[LF].mSuspensionDeflection, irsdk_float, 1, "LF shock deflection", "m", IRSDK_LOG_DISK);
	  irsdkVar("RFshockDefl", &m_telemetryInfo.mWheel[RF].mSuspensionDeflection, irsdk_float, 1, "RF shock deflection", "m", IRSDK_LOG_DISK);
	  irsdkVar("LRshockDefl", &m_telemetryInfo.mWheel[LR].mSuspensionDeflection, irsdk_float, 1, "LR shock deflection", "m", IRSDK_LOG_DISK);
	  irsdkVar("RRshockDefl", &m_telemetryInfo.mWheel[RR].mSuspensionDeflection, irsdk_float, 1, "RR shock deflection", "m", IRSDK_LOG_DISK);

	  irsdkVar("LFSuspForce", &m_telemetryInfo.mWheel[LF].mSuspForce, irsdk_float, 1, "LF pushrod load", "N", IRSDK_LOG_DISK);
	  irsdkVar("RFSuspForce", &m_telemetryInfo.mWheel[RF].mSuspForce, irsdk_float, 1, "RF pushrod load", "N", IRSDK_LOG_DISK);
	  irsdkVar("LRSuspForce", &m_telemetryInfo.mWheel[LR].mSuspForce, irsdk_float, 1, "LR pushrod load", "N", IRSDK_LOG_DISK);
	  irsdkVar("RRSuspForce", &m_telemetryInfo.mWheel[RR].mSuspForce, irsdk_float, 1, "RR pushrod load", "N", IRSDK_LOG_DISK);

	  irsdkVar("LFBrakeTemp", &m_telemetryInfo.mWheel[LF].mBrakeTemp, irsdk_float, 1, "LF brake disc temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("RFBrakeTemp", &m_telemetryInfo.mWheel[RF].mBrakeTemp, irsdk_float, 1, "RF brake disc temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("LRBrakeTemp", &m_telemetryInfo.mWheel[LR].mBrakeTemp, irsdk_float, 1, "LR brake disc temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("RRBrakeTemp", &m_telemetryInfo.mWheel[RR].mBrakeTemp, irsdk_float, 1, "RR brake disc temperature", "C", IRSDK_LOG_DISK);

	  irsdkVar("LFBrakePress", &m_telemetryInfo.mWheel[LF].mBrakePressure, irsdk_float, 1, "LF brake pressure", "%", IRSDK_LOG_DISK);
	  irsdkVar("RFBrakePress", &m_telemetryInfo.mWheel[RF].mBrakePressure, irsdk_float, 1, "RF brake pressure", "%", IRSDK_LOG_DISK);
	  irsdkVar("LRBrakePress", &m_telemetryInfo.mWheel[LR].mBrakePressure, irsdk_float, 1, "LR brake pressure", "%", IRSDK_LOG_DISK);
	  irsdkVar("RRBrakePress", &m_telemetryInfo.mWheel[RR].mBrakePressure, irsdk_float, 1, "RR brake pressure", "%", IRSDK_LOG_DISK);

	  irsdkVar("LFtempL", &m_telemetryInfo.mWheel[LF].mTemperature[0], irsdk_float, 1, "LF tire left (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("LFtempM", &m_telemetryInfo.mWheel[LF].mTemperature[1], irsdk_float, 1, "LF tire middle (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("LFtempR", &m_telemetryInfo.mWheel[LF].mTemperature[2], irsdk_float, 1, "LF tire right (surface) temperature", "C", IRSDK_LOG_DISK);

	  irsdkVar("RFtempL", &m_telemetryInfo.mWheel[RF].mTemperature[0], irsdk_float, 1, "RF tire left (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("RFtempM", &m_telemetryInfo.mWheel[RF].mTemperature[1], irsdk_float, 1, "RF tire middle (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("RFtempR", &m_telemetryInfo.mWheel[RF].mTemperature[2], irsdk_float, 1, "RF tire right (surface) temperature", "C", IRSDK_LOG_DISK);

	  irsdkVar("LRtempL", &m_telemetryInfo.mWheel[LR].mTemperature[0], irsdk_float, 1, "LR tire left (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("LRtempM", &m_telemetryInfo.mWheel[LR].mTemperature[1], irsdk_float, 1, "LR tire middle (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("LRtempR", &m_telemetryInfo.mWheel[LR].mTemperature[2], irsdk_float, 1, "LR tire right (surface) temperature", "C", IRSDK_LOG_DISK);

	  irsdkVar("RRtempL", &m_telemetryInfo.mWheel[RR].mTemperature[0], irsdk_float, 1, "RR tire left (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("RRtempM", &m_telemetryInfo.mWheel[RR].mTemperature[1], irsdk_float, 1, "RR tire middle (surface) temperature", "C", IRSDK_LOG_DISK);
	  irsdkVar("RRtempR", &m_telemetryInfo.mWheel[RR].mTemperature[2], irsdk_float, 1, "RR tire right (surface) temperature", "C", IRSDK_LOG_DISK);

	  irsdkVar("LFpressure", &m_telemetryInfo.mWheel[LF].mPressure, irsdk_float, 1, "LF tire pressure", "kPa", IRSDK_LOG_DISK);
	  irsdkVar("RFpressure", &m_telemetryInfo.mWheel[RF].mPressure, irsdk_float, 1, "RF tire pressure", "kPa", IRSDK_LOG_DISK);
	  irsdkVar("LRpressure", &m_telemetryInfo.mWheel[LR].mPressure, irsdk_float, 1, "LR tire pressure", "kPa", IRSDK_LOG_DISK);
	  irsdkVar("RRpressure", &m_telemetryInfo.mWheel[RR].mPressure, irsdk_float, 1, "RR tire pressure", "kPa", IRSDK_LOG_DISK);

  }
  ~rf2plugin() {}

     ULONG_PTR gdiplusToken;

  // These are the functions derived from base class InternalsPlugin
  // that can be implemented.
  void Startup( long version );  // game startup
  void Shutdown();               // game shutdown

  void EnterRealtime();          // entering realtime
  void ExitRealtime();           // exiting realtime

  void StartSession();           // session has started
  void EndSession();             // session has ended

  // GAME OUTPUT
  long WantsTelemetryUpdates() { return( true ); } // CHANGE TO 1 TO ENABLE TELEMETRY EXAMPLE!
  void UpdateTelemetry( const TelemInfoV01 &info );

  bool WantsGraphicsUpdates() { return( true ); } // CHANGE TO TRUE TO ENABLE GRAPHICS EXAMPLE!
  void UpdateGraphics( const GraphicsInfoV02 &info );

  bool WantsToViewVehicle( CameraControlInfoV01 &camControl ); // set ID and camera type and return true
  void RenderScreenAfterOverlays( const ScreenInfoV01 &info ); // after resetting

  // GAME INPUT
  bool HasHardwareInputs() { return( false ); } // CHANGE TO TRUE TO ENABLE HARDWARE EXAMPLE!
  void UpdateHardware( const float fDT ) { } // update the hardware with the time between frames
  void EnableHardware() { mEnabled = false; }             // message from game to enable hardware
  void DisableHardware() { mEnabled = false; }           // message from game to disable hardware

  // See if the plugin wants to take over a hardware control.  If the plugin takes over the
  // control, this method returns true and sets the value of the float pointed to by the
  // second arg.  Otherwise, it returns false and leaves the float unmodified.
  bool CheckHWControl( const char * const controlName, float &fRetVal );

  bool ForceFeedback( float &forceValue );  // SEE FUNCTION BODY TO ENABLE FORCE EXAMPLE

  // SCORING OUTPUT
  bool WantsScoringUpdates() { return( true ); } // CHANGE TO TRUE TO ENABLE SCORING EXAMPLE!
  void UpdateScoring( const ScoringInfoV01 &info );

  // COMMENTARY INPUT
  bool RequestCommentary( CommentaryRequestInfoV01 &info );  // SEE FUNCTION BODY TO ENABLE COMMENTARY EXAMPLE

  // VIDEO EXPORT (sorry, no example code at this time)
  virtual bool WantsVideoOutput() { return( false ); }         // whether we want to export video
  virtual bool VideoOpen( const char * const szFilename, float fQuality, unsigned short usFPS, unsigned long fBPS,
                          unsigned short usWidth, unsigned short usHeight, char *cpCodec = NULL ) { return( false ); } // open video output file
  virtual void VideoClose() {}                                 // close video output file
  virtual void VideoWriteAudio( const short *pAudio, unsigned int uNumFrames ) {} // write some audio info
  virtual void VideoWriteImage( const unsigned char *pImage ) {} // write video image

  void YAMLupdate(const ScoringInfoV01 &info);
  //void YAMLupdate(const TelemInfoV01 &info);
  void YAMLgenerate();

 private:
  bool mEnabled; // needed for the hardware example

  static char YAMLstring[irsdkServer::sessionStrLen];
  static int YAMLstring_maxlen;
  static int YAMLstring_len;
  static unsigned int YAMLchecksum;

  static WeekendInfo weekendinfo;
  static SessionInfo sessioninfo;
  static DriverInfo driverinfo;


  // Real time telemetry variables
  TelemInfoData m_telemetryInfo;


  //read once and write to SessionInfoStr - 
  double m_EngineMaxRPM;          // rev limit

  // session data
  double g_sessionTime;
  irsdkVar irsdkSessionTime;

  int g_sessionNum;
  irsdkVar irsdkSessionNum;

  int g_sessionState;
  irsdkVar irsdkSessionState;

  int g_sessionUniqueID;
  irsdkVar irsdkSessionUniqueID;

  int g_sessionFlags;
  irsdkVar irsdkSessionFlags;

  double g_sessionTimeRemain;
  irsdkVar irsdkSessionTimeRemain;

  int g_sessionLapsRemain;
  irsdkVar irsdkSessionLapsRemain;

  int g_lap;
  irsdkVar irsdkLap;

  // cameras and replay
  int g_camcaridx;
  irsdkVar irsdkCamCarIdx;

  int g_camgroupnumber;
  irsdkVar irsdkCamGroupNumber;

  int g_replayFrameNum;
  irsdkVar irsdkReplayFrameNum;

  double g_replaySessionTime;
  irsdkVar irsdkReplaySessionTime;

  // drivers
  float g_carIdxLapDistPct[64];
  irsdkVar irsdkCarIdxLapDistPct;

  int g_carIdxLap[64];
  irsdkVar irsdkCarIdxLap;

  int g_carIdxTrackSurface[64];
  irsdkVar irsdkCarIdxTrackSurface;

};


#endif // _INTERNALS_EXAMPLE_H

