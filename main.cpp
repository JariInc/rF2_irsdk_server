/*
Copyright (c) 2013, iRacing.com Motorsport Simulations, LLC.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of iRacing.com Motorsport Simulations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//A sample app that demonstrates how to integrate the iRacing telemetry server side api in your own product
// by default data is transmitted to a memory mapped file, and written to c:\temp\*.ibt


// don't warn about stdlib
#pragma warning(disable: 4996) 

#include <windows.h>
//#include <gdiplus.h>
//#include <D3D9.h>
//#include <D3dx9tex.h>

#include <stdio.h>
#include <conio.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>


#include "irsdk_defines.h"
#include "yaml.h"

#include "irsdk_server.h"
#include "rf2plugin.hpp"
#include "MurmurHash3.h"

// plugin information
extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()                   { return( "irsdk_server" ); }
extern "C" __declspec( dllexport )
PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }
extern "C" __declspec( dllexport )
int __cdecl GetPluginVersion()                         { return( 5 ); } // InternalsPluginV03 functionality
extern "C" __declspec( dllexport )
PluginObject * __cdecl CreatePluginObject()            { return( (PluginObject *) new rf2plugin ); }
extern "C" __declspec( dllexport )
void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( (rf2plugin *) obj ); }

WeekendInfo rf2plugin::weekendinfo = {0};
SessionInfo rf2plugin::sessioninfo = {0};
DriverInfo rf2plugin::driverinfo = {0};

int rf2plugin::YAMLstring_maxlen = irsdkServer::sessionStrLen;
int rf2plugin::YAMLstring_len = 0;
char rf2plugin::YAMLstring[irsdkServer::sessionStrLen] = "";
unsigned int rf2plugin::YAMLchecksum = 0;

// used to create the filename in irsdk_diskserver
const char* g_vehicleName;
const char* g_trackName;

HHOOK rf2plugin::g_wndProcHook =NULL;
long rf2plugin::g_newCam =0;
bool rf2plugin::g_switchCam =false;

//using namespace Gdiplus;


LRESULT CALLBACK rf2plugin::WndProcHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	static unsigned int broadcastMsgId = RegisterWindowMessageA(IRSDK_BROADCASTMSGNAME);
	CWPSTRUCT* cwp =(CWPSTRUCT*)lParam;

	if (cwp !=NULL)
	{
		UINT message =cwp->message;
		if(broadcastMsgId && message == broadcastMsgId) {

			irsdk_BroadcastMsg msg =(irsdk_BroadcastMsg)LOWORD(cwp->wParam);
			int var1 =HIWORD(cwp->wParam);
			int var2 =LOWORD(cwp->lParam);
			int var3 =HIWORD(cwp->wParam);
			int var2DWord =(cwp->lParam & 0xFFFFFFFF);

			switch(msg) {
				
				case irsdk_BroadcastCamSwitchPos:
					g_newCam =(long)var1;
					g_switchCam =true;
					break;

				default:
					break;
			}
		}
	}

	return CallNextHookEx(g_wndProcHook, nCode, wParam, lParam);
}


void rf2plugin::Startup( long version )
{
   //GdiplusStartupInput gdiplusStartupInput;
   //GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);  
}


void rf2plugin::Shutdown()
{
	//GdiplusShutdown(gdiplusToken);
}


void rf2plugin::StartSession()
{
	HINSTANCE hInst =GetModuleHandle("irsdk_server.dll");
	g_wndProcHook =SetWindowsHookEx(WH_CALLWNDPROC, &rf2plugin::WndProcHook, hInst, GetCurrentThreadId());

	// Generate random session id
	srand(time(NULL));
	g_sessionUniqueID = rand();

	g_sessionTime =0;
	m_EngineMaxRPM =-1;
}


void rf2plugin::EndSession()
{
	UnhookWindowsHookEx(g_wndProcHook);

	g_sessionTime =0;
	m_EngineMaxRPM =-1;
}


void rf2plugin::EnterRealtime()
{
	// use these to turn disk based logging on and off
	if(!irsdkServer::instance()->isDiskLoggingEnabled()) // disk logging is turned on
		irsdkServer::instance()->toggleDiskLogging(); // turn logging on or off
	// --- logging to disk enabled, so the next call to pullSampleVars opens a new file

	// needed here, because this value can change in garage for some vehicles
	m_EngineMaxRPM =-1;

	g_isInGarage =false;
	g_isOnTrack =true;
}


void rf2plugin::ExitRealtime()
{
	g_isInGarage =true;
	g_isOnTrack =false;

	// write latest values to file
	irsdkServer::instance()->pollSampleVars();

	if(irsdkServer::instance()->isDiskLoggingEnabled()) // disk logging is turned on
		irsdkServer::instance()->toggleDiskLogging(); // turn logging on or off

	// logging to disk is diabled; call pollSampleVars once again to finalize and close the file
	irsdkServer::instance()->pollSampleVars();

}


void rf2plugin::UpdateTelemetry( const TelemInfoV01 &info )
{
	g_vehicleName =&info.mVehicleName[0];
	g_trackName =&info.mTrackName[0];

	if(info.mElapsedTime > g_sessionTime) {
		// before going into your main data output loop, you must mark the headers as final
		// it is possible for finalizeHeader() to fail, so try every tick, just in case
		if(!irsdkServer::instance()->isHeaderFinalized())
			irsdkServer::instance()->finalizeHeader();

		// reset the data for this pass, if something is not updated
		// we will output zero for that tick ... and clear the garbage out of the vars if this is the first call
		if(irsdkServer::instance()->isInitialized())
			irsdkServer::instance()->resetSampleVars();

		if(irsdkServer::instance()->isHeaderFinalized())
		{
			// read out variables that have registered with pointers back to them
			irsdkServer::instance()->pollSampleVars();

			// update our data...
			g_sessionTime = info.mElapsedTime;
			g_replaySessionTime = info.mElapsedTime;
			g_replayFrameNum = (int)floor(g_sessionTime * TICKS_PER_SEC);

			if (g_lap < info.mLapNumber) {
				m_telemetryInfo.mLapDistPct =0.0f;
			}
			g_lap = info.mLapNumber;
			m_telemetryInfo.mDeltaTime =info.mDeltaTime;

			if (m_EngineMaxRPM < 0) {
				driverinfo.DriverCarRedLine  =m_EngineMaxRPM =info.mEngineMaxRPM;
			}
			for (int i =0; i < 3; i++)
			{
				// TODO: need to rotate coordinate system?
				m_telemetryInfo.mPos[i] =info.mPos[i];
				m_telemetryInfo.mLocalAccel[i] =info.mLocalAccel[i];
				m_telemetryInfo.mLocalVel[i] =info.mLocalVel[i];
			}

			float x =(float)info.mLocalVel.x, y =(float)info.mLocalVel.y, z =(float)info.mLocalVel.z;
			m_telemetryInfo.mSpeed =(float)sqrt((x*x)+(y*y)+(z*z));
			
			// TODO: check if vert accel needs to be inverted
			m_telemetryInfo.mLatAccel =(float)info.mLocalAccel.x; // * -1.0f;
			m_telemetryInfo.mLongAccel =(float)info.mLocalAccel.z * -1.0f;
			m_telemetryInfo.mVertAccel =(float)info.mLocalAccel.y;
			
			if (info.mOverheating) {
				m_telemetryInfo.m_engineWarnings |=irsdk_waterTempWarning;
			}
			else {
				m_telemetryInfo.m_engineWarnings &= ~irsdk_waterTempWarning;
			}

			if (info.mEngineRPM >= (info.mEngineMaxRPM - 10.0f)) {
				m_telemetryInfo.m_engineWarnings |=irsdk_revLimiterActive;
			}
			else {
				m_telemetryInfo.m_engineWarnings &= ~irsdk_revLimiterActive;
			}

			if (info.mSpeedLimiter) {
				m_telemetryInfo.m_engineWarnings |=irsdk_pitSpeedLimiter;
			}
			else {
				m_telemetryInfo.m_engineWarnings &= ~irsdk_pitSpeedLimiter;
			}
			// TODO: check if engine stalled 

			// engine info
			m_telemetryInfo.mGear =(int)info.mGear;
			m_telemetryInfo.mEngineRPM =(float)info.mEngineRPM;
			m_telemetryInfo.mEngineWaterTemp =(float)info.mEngineWaterTemp;
			m_telemetryInfo.mEngineOilTemp =(float)info.mEngineOilTemp;
			m_telemetryInfo.mClutchRPM =(float)info.mClutchRPM;
			m_telemetryInfo.mEngineTq =(float)info.mEngineTq;
			m_telemetryInfo.mIgnitionStarter =info.mIgnitionStarter;

			// driver inputs
			m_telemetryInfo.mUnfilteredBrake =(float)info.mUnfilteredBrake;
			m_telemetryInfo.mUnfilteredClutch =(float)info.mUnfilteredClutch;
			m_telemetryInfo.mUnfilteredSteering =(float)info.mUnfilteredSteering;
			m_telemetryInfo.mUnfilteredThrottle =(float)info.mUnfilteredThrottle;

			m_telemetryInfo.mFilteredBrake =(float)info.mFilteredBrake;
			m_telemetryInfo.mFilteredClutch =(float)info.mFilteredClutch;
			m_telemetryInfo.mFilteredSteering =(float)info.mFilteredSteering;
			m_telemetryInfo.mFilteredThrottle =(float)info.mFilteredThrottle;

			// global suspension info
			m_telemetryInfo.mFrontRideHeight =(float)info.mFrontRideHeight;
			m_telemetryInfo.mRearRideHeight =(float)info.mRearRideHeight;

			m_telemetryInfo.mFuel =(float)info.mFuel;
			m_telemetryInfo.mFuelCapacity =(float)info.mFuelCapacity;
			m_telemetryInfo.mFuelLevelPct =(float)(info.mFuel/info.mFuelCapacity);

			m_telemetryInfo.mSteeringArmForce =(float)info.mSteeringArmForce;

			// wheel info
			for (int i = 0; i < 4; i++)
			{
				m_telemetryInfo.mWheel[i].mRideHeight =(float)info.mWheel[i].mRideHeight;
				m_telemetryInfo.mWheel[i].mSuspensionDeflection =(float)info.mWheel[i].mSuspensionDeflection;
				m_telemetryInfo.mWheel[i].mSuspForce =(float)info.mWheel[i].mSuspForce;
				m_telemetryInfo.mWheel[i].mBrakeTemp =(float)info.mWheel[i].mBrakeTemp;
				m_telemetryInfo.mWheel[i].mBrakePressure =(float)info.mWheel[i].mBrakePressure;
				m_telemetryInfo.mWheel[i].mTemperature[0] =(float)info.mWheel[i].mTemperature[0];
				m_telemetryInfo.mWheel[i].mTemperature[1] =(float)info.mWheel[i].mTemperature[1];
				m_telemetryInfo.mWheel[i].mTemperature[2] =(float)info.mWheel[i].mTemperature[2];
				m_telemetryInfo.mWheel[i].mPressure =(float)info.mWheel[i].mPressure;
				m_telemetryInfo.mWheel[i].mCamber =(float)info.mWheel[i].mCamber;
				m_telemetryInfo.mWheel[i].mLateralForce =(float)info.mWheel[i].mLateralForce;
				m_telemetryInfo.mWheel[i].mLateralGroundVel =(float)info.mWheel[i].mLateralGroundVel;
				m_telemetryInfo.mWheel[i].mLateralPatchVel =(float)info.mWheel[i].mLateralPatchVel;
				m_telemetryInfo.mWheel[i].mLongitudinalForce =(float)info.mWheel[i].mLongitudinalForce;
				m_telemetryInfo.mWheel[i].mLongitudinalGroundVel =(float)info.mWheel[i].mLongitudinalGroundVel;
				m_telemetryInfo.mWheel[i].mLongitudinalPatchVel =(float)info.mWheel[i].mLongitudinalPatchVel;
				m_telemetryInfo.mWheel[i].mRotation =(float)info.mWheel[i].mRotation;
				m_telemetryInfo.mWheel[i].mTireLoad =(float)info.mWheel[i].mTireLoad;
				m_telemetryInfo.mWheel[i].mToe =(float)info.mWheel[i].mToe;
			}
		}
	}

	/*
  // Use the incoming data, for now I'll just write some of it to a file to a) make sure it
  // is working, and b) explain the coordinate system a little bit (see header for more info)
  FILE *fo = fopen( "ExampleInternalsTelemetryOutput.txt", "a" );
  if( fo != NULL )
  {
    // Delta time is variable, as we send out the info once per frame
    fprintf( fo, "DT=%.4f  ET=%.4f\n", info.mDeltaTime, info.mElapsedTime );
    fprintf( fo, "Lap=%d StartET=%.3f\n", info.mLapNumber, info.mLapStartET );
    fprintf( fo, "Vehicle=%s\n", info.mVehicleName );
    fprintf( fo, "Track=%s\n", info.mTrackName );
    fprintf( fo, "Pos=(%.3f,%.3f,%.3f)\n", info.mPos.x, info.mPos.y, info.mPos.z );

    // Forward is roughly in the -z direction (although current pitch of car may cause some y-direction velocity)
    fprintf( fo, "LocalVel=(%.2f,%.2f,%.2f)\n", info.mLocalVel.x, info.mLocalVel.y, info.mLocalVel.z );
    fprintf( fo, "LocalAccel=(%.1f,%.1f,%.1f)\n", info.mLocalAccel.x, info.mLocalAccel.y, info.mLocalAccel.z );

    // Orientation matrix is left-handed
    fprintf( fo, "[%6.3f,%6.3f,%6.3f]\n", info.mOri[0].x, info.mOri[0].y, info.mOri[0].z );
    fprintf( fo, "[%6.3f,%6.3f,%6.3f]\n", info.mOri[1].x, info.mOri[1].y, info.mOri[1].z );
    fprintf( fo, "[%6.3f,%6.3f,%6.3f]\n", info.mOri[2].x, info.mOri[2].y, info.mOri[2].z );
    fprintf( fo, "LocalRot=(%.3f,%.3f,%.3f)\n", info.mLocalRot.x, info.mLocalRot.y, info.mLocalRot.z );
    fprintf( fo, "LocalRotAccel=(%.2f,%.2f,%.2f)\n", info.mLocalRotAccel.x, info.mLocalRotAccel.y, info.mLocalRotAccel.z );

    // Vehicle status
    fprintf( fo, "Gear=%d RPM=%.1f RevLimit=%.1f\n", info.mGear, info.mEngineRPM, info.mEngineMaxRPM );
    fprintf( fo, "Water=%.1f Oil=%.1f\n", info.mEngineWaterTemp, info.mEngineOilTemp );
    fprintf( fo, "ClutchRPM=%.1f\n", info.mClutchRPM );

    // Driver input
    fprintf( fo, "UnfilteredThrottle=%.1f%%\n", 100.0f * info.mUnfilteredThrottle );
    fprintf( fo, "UnfilteredBrake=%.1f%%\n", 100.0f * info.mUnfilteredBrake );
    fprintf( fo, "UnfilteredSteering=%.1f%%\n", 100.0f * info.mUnfilteredSteering );
    fprintf( fo, "UnfilteredClutch=%.1f%%\n", 100.0f * info.mUnfilteredClutch );

    // Filtered input
    fprintf( fo, "FilteredThrottle=%.1f%%\n", 100.0f * info.mFilteredThrottle );
    fprintf( fo, "FilteredBrake=%.1f%%\n", 100.0f * info.mFilteredBrake );
    fprintf( fo, "FilteredSteering=%.1f%%\n", 100.0f * info.mFilteredSteering );
    fprintf( fo, "FilteredClutch=%.1f%%\n", 100.0f * info.mFilteredClutch );

    // Misc
    fprintf( fo, "SteeringArmForce=%.1f\n", info.mSteeringArmForce );
    fprintf( fo, "Front3rdDeflection=%.3f Rear3rdDeflection=%.3f\n", info.mFront3rdDeflection, info.mRear3rdDeflection );

    // Aerodynamics
    fprintf( fo, "FrontWingHeight=%.3f FrontRideHeight=%.3f RearRideHeight=%.3f\n", info.mFrontWingHeight, info.mFrontRideHeight, info.mRearRideHeight );
    fprintf( fo, "Drag=%.1f FrontDownforce=%.1f RearDownforce=%.1f\n", info.mDrag, info.mFrontDownforce, info.mRearDownforce );

    // Other
    fprintf( fo, "Fuel=%.1f ScheduledStops=%d Overheating=%d Detached=%d\n", info.mFuel, info.mScheduledStops, info.mOverheating, info.mDetached );
    fprintf( fo, "Dents=(%d,%d,%d,%d,%d,%d,%d,%d)\n", info.mDentSeverity[0], info.mDentSeverity[1], info.mDentSeverity[2], info.mDentSeverity[3],
                                                      info.mDentSeverity[4], info.mDentSeverity[5], info.mDentSeverity[6], info.mDentSeverity[7] );
    fprintf( fo, "LastImpactET=%.1f Mag=%.1f, Pos=(%.1f,%.1f,%.1f)\n", info.mLastImpactET, info.mLastImpactMagnitude,
             info.mLastImpactPos.x, info.mLastImpactPos.y, info.mLastImpactPos.z );

    // Wheels
    for( long i = 0; i < 4; ++i )
    {
      const TelemWheelV01 &wheel = info.mWheel[i];
      fprintf( fo, "Wheel=%s\n", (i==0)?"FrontLeft":(i==1)?"FrontRight":(i==2)?"RearLeft":"RearRight" );
      fprintf( fo, " SuspensionDeflection=%.3f RideHeight=%.3f\n", wheel.mSuspensionDeflection, wheel.mRideHeight );
      fprintf( fo, " SuspForce=%.1f BrakeTemp=%.1f BrakePressure=%.3f\n", wheel.mSuspForce, wheel.mBrakeTemp, wheel.mBrakePressure );
      fprintf( fo, " ForwardRotation=%.1f Camber=%.3f\n", -wheel.mRotation, wheel.mCamber );
      fprintf( fo, " LateralPatchVel=%.2f LongitudinalPatchVel=%.2f\n", wheel.mLateralPatchVel, wheel.mLongitudinalPatchVel );
      fprintf( fo, " LateralGroundVel=%.2f LongitudinalGroundVel=%.2f\n", wheel.mLateralGroundVel, wheel.mLongitudinalGroundVel );
      fprintf( fo, " LateralForce=%.1f LongitudinalForce=%.1f\n", wheel.mLateralForce, wheel.mLongitudinalForce );
      fprintf( fo, " TireLoad=%.1f GripFract=%.3f TirePressure=%.1f\n", wheel.mTireLoad, wheel.mGripFract, wheel.mPressure );
      fprintf( fo, " TireTemp(l/c/r)=%.1f/%.1f/%.1f\n", wheel.mTemperature[0], wheel.mTemperature[1], wheel.mTemperature[2] );
      fprintf( fo, " Wear=%.3f TerrainName=%s SurfaceType=%d\n", wheel.mWear, wheel.mTerrainName, wheel.mSurfaceType );
      fprintf( fo, " Flat=%d Detached=%d\n", wheel.mFlat, wheel.mDetached );
    }

    // Compute some auxiliary info based on the above
    TelemVect3 forwardVector = { -info.mOri[0].z, -info.mOri[1].z, -info.mOri[2].z };
    TelemVect3    leftVector = {  info.mOri[0].x,  info.mOri[1].x,  info.mOri[2].x };

    // These are normalized vectors, and remember that our world Y coordinate is up.  So you can
    // determine the current pitch and roll (w.r.t. the world x-z plane) as follows:
    const double pitch = atan2( forwardVector.y, sqrt( ( forwardVector.x * forwardVector.x ) + ( forwardVector.z * forwardVector.z ) ) );
    const double  roll = atan2(    leftVector.y, sqrt( (    leftVector.x *    leftVector.x ) + (    leftVector.z *    leftVector.z ) ) );
    const double radsToDeg = 57.296;
    fprintf( fo, "Pitch = %.1f deg, Roll = %.1f deg\n", pitch * radsToDeg, roll * radsToDeg );

    const double metersPerSec = sqrt( ( info.mLocalVel.x * info.mLocalVel.x ) +
                                      ( info.mLocalVel.y * info.mLocalVel.y ) +
                                      ( info.mLocalVel.z * info.mLocalVel.z ) );
    fprintf( fo, "Speed = %.1f KPH, %.1f MPH\n\n", metersPerSec * 3.6f, metersPerSec * 2.237f );

    // Close file
    fclose( fo );
  }
  */
}


void rf2plugin::UpdateGraphics( const GraphicsInfoV02 &info )
{
	g_camcaridx = info.mID + 1; 
	g_camgroupnumber = info.mCameraType;


	/*
  // Use the incoming data, for now I'll just write some of it to a file to a) make sure it
  // is working, and b) explain the coordinate system a little bit (see header for more info)
  FILE *fo = fopen( "ExampleInternalsGraphicsOutput.txt", "a" );
  if( fo != NULL )
  {
    // Print stuff
    fprintf( fo, "CamPos=(%.1f,%.1f,%.1f)\n", info.mCamPos.x, info.mCamPos.y, info.mCamPos.z );
    fprintf( fo, "CamOri[0]=(%.1f,%.1f,%.1f)\n", info.mCamOri[0].x, info.mCamOri[0].y, info.mCamOri[0].z );
    fprintf( fo, "CamOri[1]=(%.1f,%.1f,%.1f)\n", info.mCamOri[1].x, info.mCamOri[1].y, info.mCamOri[1].z );
    fprintf( fo, "CamOri[2]=(%.1f,%.1f,%.1f)\n", info.mCamOri[2].x, info.mCamOri[2].y, info.mCamOri[2].z );
    fprintf( fo, "HWND=%d\n", info.mHWND );
    fprintf( fo, "Ambient Color=(%.1f,%.1f,%.1f)\n\n", info.mAmbientRed, info.mAmbientGreen, info.mAmbientBlue );

    // Close file
    fclose( fo );
  }
  */
}


bool rf2plugin::CheckHWControl( const char * const controlName, float &fRetVal )
{
/*
  // only if enabled, of course
  if( !mEnabled )
    return( false );

  // Note that incoming value is the game's computation, in case you're interested.

  // Sorry, no control allowed over actual vehicle inputs ... would be too easy to cheat!
  // However, you can still look at the values.

  // Note: since the game calls this function every frame for every available control, you might consider
  // doing a binary search if you are checking more than 7 or 8 strings, just to keep the speed up.
  if( _stricmp( controlName, "LookLeft" ) == 0 )
  {
    const float headSwitcheroo = fmodf( mET, 2.0f );
    if( headSwitcheroo < 0.5 )
      fRetVal = 1.0f;
    else
      fRetVal = 0.0f;
    return( true );
  }
  else if( _stricmp( controlName, "LookRight" ) == 0 )
  {
    const float headSwitcheroo = fmodf( mET, 2.0f );
    if( ( headSwitcheroo > 1.0f ) && ( headSwitcheroo < 1.5f ) )
      fRetVal = 1.0f;
    else
      fRetVal = 0.0f;
    return( true );
  }
  */
  return( false );
}


bool rf2plugin::ForceFeedback( float &forceValue )
{
  // Note that incoming value is the game's computation, in case you're interested.

  // CHANGE COMMENTS TO ENABLE FORCE EXAMPLE

  return( false );

  // I think the bounds are -11500 to 11500 ...
//  forceValue = 11500.0f * sinf( mET );
//  return( true );
}

void rf2plugin::UpdateScoring( const ScoringInfoV01 &info )
{
	// update data realtime
	g_sessionNum = info.mSession;
	switch (info.mGamePhase)
	{
		case 0: // Before session has begun
			g_sessionState = irsdk_SessionState::irsdk_StateInvalid;
			g_sessionFlags = 0;
			break;
		case 1: // Reconnaissance laps
			g_sessionState = irsdk_SessionState::irsdk_StateWarmup;
			g_sessionFlags = irsdk_Flags::irsdk_green;
			break;
		case 2: // Grid walk-through
			g_sessionState = irsdk_SessionState::irsdk_StateGetInCar;
			g_sessionFlags = irsdk_Flags::irsdk_yellow;
			break;
		case 3: // Formation lap
			g_sessionState = irsdk_SessionState::irsdk_StateParadeLaps;
			g_sessionFlags = irsdk_Flags::irsdk_yellow;
			break;
		case 4: // Starting-light countdown has begun
			g_sessionState = irsdk_SessionState::irsdk_StateParadeLaps;
			g_sessionFlags = 0;
			break;
		case 5: // Green flag
			g_sessionState = irsdk_SessionState::irsdk_StateRacing;
			g_sessionFlags = irsdk_Flags::irsdk_green;
			break;
		case 6: // Full course yellow / safety car
			g_sessionState = irsdk_SessionState::irsdk_StateRacing;
			g_sessionFlags = irsdk_Flags::irsdk_yellowWaving;
			break;
		case 7: // Session stopped
			g_sessionFlags = irsdk_Flags::irsdk_red;
			break;
		case 8: // Session over
			g_sessionState = irsdk_SessionState::irsdk_StateCoolDown;
			g_sessionFlags = irsdk_Flags::irsdk_checkered;
			break;
		default:
			break;
	}

	// Last lap
	if (info.mYellowFlagState == 5)
		g_sessionFlags = irsdk_Flags::irsdk_white;

	g_sessionTimeRemain = info.mEndET - info.mCurrentET;
	VehicleScoringInfoV01 &leader = info.mVehicle[1];
	g_sessionLapsRemain = info.mMaxLaps - leader.mTotalLaps;

	// ---
	m_telemetryInfo.mLapDist =(float)info.mVehicle[m_telemetryInfo.mID].mLapDist;
	m_telemetryInfo.mLapDistPct =(float)(m_telemetryInfo.mLapDist/info.mLapDist);
	// ---

	for( long i = 0; i < info.mNumVehicles; ++i )
    {
		VehicleScoringInfoV01 &vinfo = info.mVehicle[i];
		int caridx = vinfo.mID+1;
		g_carIdxLapDistPct[caridx] = vinfo.mLapDist/info.mLapDist;
		g_carIdxLap[caridx] = vinfo.mTotalLaps;
		if(vinfo.mPitState == 3)
			g_carIdxTrackSurface[caridx] = irsdk_TrkLoc::irsdk_InPitStall;
		else if(vinfo.mInPits && (vinfo.mLocalVel.x + vinfo.mLocalVel.x) < 0.1)
			g_carIdxTrackSurface[caridx] = irsdk_TrkLoc::irsdk_NotInWorld;
		else if(vinfo.mInPits)
			g_carIdxTrackSurface[caridx] = irsdk_TrkLoc::irsdk_AproachingPits;
		else
			g_carIdxTrackSurface[caridx] = irsdk_TrkLoc::irsdk_OnTrack;
	}

	// update YAML
	YAMLupdate(info);
	unsigned int prevchecksum = YAMLchecksum;
	MurmurHash3_x86_32(YAMLstring, YAMLstring_len, 0, &YAMLchecksum);
	
	if(prevchecksum != YAMLchecksum)
		irsdkServer::instance()->regSessionInfo(YAMLstring);

	FILE *fo = fopen("ExampleInternalsYAML.txt", "w");
	if(fo != NULL)
		fprintf(fo, "%s", YAMLstring);
	fclose(fo);

	/*
  // Note: function is called twice per second now (instead of once per second in previous versions)
  FILE *fo = fopen( "c:\\temp\\ExampleInternalsScoringOutput.txt", "a" );
  if( fo != NULL )
  {
    // Print general scoring info
    fprintf( fo, "TrackName=%s\n", info.mTrackName );
    fprintf( fo, "Session=%d NumVehicles=%d CurET=%.3f\n", info.mSession, info.mNumVehicles, info.mCurrentET );
    fprintf( fo, "EndET=%.3f MaxLaps=%d LapDist=%.1f\n", info.mEndET, info.mMaxLaps, info.mLapDist );

    // Note that only one plugin can use the stream (by enabling scoring updates) ... sorry if any clashes result
    fprintf( fo, "START STREAM\n" );
    const char *ptr = info.mResultsStream;
    while( *ptr != NULL )
      fputc( *ptr++, fo );
    fprintf( fo, "END STREAM\n" );

    // New version 2 stuff
    fprintf( fo, "GamePhase=%d YellowFlagState=%d SectorFlags=(%d,%d,%d)\n", info.mGamePhase, info.mYellowFlagState,
             info.mSectorFlag[0], info.mSectorFlag[1], info.mSectorFlag[2] );
    fprintf( fo, "InRealtime=%d StartLight=%d NumRedLights=%d\n", info.mInRealtime, info.mStartLight, info.mNumRedLights );
    fprintf( fo, "PlayerName=%s PlrFileName=%s\n", info.mPlayerName, info.mPlrFileName );
    fprintf( fo, "DarkCloud=%.2f Raining=%.2f AmbientTemp=%.1f TrackTemp=%.1f\n", info.mDarkCloud, info.mRaining, info.mAmbientTemp, info.mTrackTemp );
    fprintf( fo, "Wind=(%.1f,%.1f,%.1f) OnPathWetness=%.2f OffPathWetness=%.2f\n", info.mWind.x, info.mWind.y, info.mWind.z, info.mOnPathWetness, info.mOffPathWetness );

    // Print vehicle info
    for( long i = 0; i < info.mNumVehicles; ++i )
    {
      VehicleScoringInfoV01 &vinfo = info.mVehicle[ i ];
      fprintf( fo, "Driver %d: %s\n", i, vinfo.mDriverName );
      fprintf( fo, " ID=%d Vehicle=%s\n", vinfo.mID, vinfo.mVehicleName );
      fprintf( fo, " Laps=%d Sector=%d FinishStatus=%d\n", vinfo.mTotalLaps, vinfo.mSector, vinfo.mFinishStatus );
      fprintf( fo, " LapDist=%.1f PathLat=%.2f RelevantTrackEdge=%.2f\n", vinfo.mLapDist, vinfo.mPathLateral, vinfo.mTrackEdge );
      fprintf( fo, " Best=(%.3f, %.3f, %.3f)\n", vinfo.mBestSector1, vinfo.mBestSector2, vinfo.mBestLapTime );
      fprintf( fo, " Last=(%.3f, %.3f, %.3f)\n", vinfo.mLastSector1, vinfo.mLastSector2, vinfo.mLastLapTime );
      fprintf( fo, " Current Sector 1 = %.3f, Current Sector 2 = %.3f\n", vinfo.mCurSector1, vinfo.mCurSector2 );
      fprintf( fo, " Pitstops=%d, Penalties=%d\n", vinfo.mNumPitstops, vinfo.mNumPenalties );

      // New version 2 stuff
      fprintf( fo, " IsPlayer=%d Control=%d InPits=%d LapStartET=%.3f\n", vinfo.mIsPlayer, vinfo.mControl, vinfo.mInPits, vinfo.mLapStartET );
      fprintf( fo, " Place=%d VehicleClass=%s\n", vinfo.mPlace, vinfo.mVehicleClass );
      fprintf( fo, " TimeBehindNext=%.3f LapsBehindNext=%d\n", vinfo.mTimeBehindNext, vinfo.mLapsBehindNext );
      fprintf( fo, " TimeBehindLeader=%.3f LapsBehindLeader=%d\n", vinfo.mTimeBehindLeader, vinfo.mLapsBehindLeader );
      fprintf( fo, " Pos=(%.3f,%.3f,%.3f)\n", vinfo.mPos.x, vinfo.mPos.y, vinfo.mPos.z );

      // Forward is roughly in the -z direction (although current pitch of car may cause some y-direction velocity)
      fprintf( fo, " LocalVel=(%.2f,%.2f,%.2f)\n", vinfo.mLocalVel.x, vinfo.mLocalVel.y, vinfo.mLocalVel.z );
      fprintf( fo, " LocalAccel=(%.1f,%.1f,%.1f)\n", vinfo.mLocalAccel.x, vinfo.mLocalAccel.y, vinfo.mLocalAccel.z );

      // Orientation matrix is left-handed
      fprintf( fo, " [%6.3f,%6.3f,%6.3f]\n", vinfo.mOri[0].x, vinfo.mOri[0].y, vinfo.mOri[0].z );
      fprintf( fo, " [%6.3f,%6.3f,%6.3f]\n", vinfo.mOri[1].x, vinfo.mOri[1].y, vinfo.mOri[1].z );
      fprintf( fo, " [%6.3f,%6.3f,%6.3f]\n", vinfo.mOri[2].x, vinfo.mOri[2].y, vinfo.mOri[2].z );
      fprintf( fo, " LocalRot=(%.3f,%.3f,%.3f)\n", vinfo.mLocalRot.x, vinfo.mLocalRot.y, vinfo.mLocalRot.z );
      fprintf( fo, " LocalRotAccel=(%.2f,%.2f,%.2f)\n", vinfo.mLocalRotAccel.x, vinfo.mLocalRotAccel.y, vinfo.mLocalRotAccel.z );
    }
      
    // Delimit sections
    fprintf( fo, "\n" );

    // Close file
    fclose( fo );
  }
  */
}

bool rf2plugin::WantsToViewVehicle(CameraControlInfoV01 &camControl)
{
	if (g_switchCam) {
		camControl.mCameraType =g_newCam;
		camControl.mID =0;

		g_switchCam =false;
		return true;
	}

	return false;
}

void rf2plugin::RenderScreenAfterOverlays(const ScreenInfoV01 &info)
{
	// TODO: add logo to screen to indicate logging to disk is active or not

	//LPDIRECT3DDEVICE9 dev =(LPDIRECT3DDEVICE9)info.mDevice;
	//if (dev !=NULL) {
	//	LPDIRECT3DSURFACE9 surf =NULL;
	//	HRESULT hr =dev->GetRenderTarget(0, &surf);
	//	if (SUCCEEDED(hr)) {
	//		//HDC dc =NULL;
	//		//hr =surf->GetDC(&dc);
	//		D3DSURFACE_DESC desc;
	//		D3DLOCKED_RECT lockRc ={0};
	//		surf->GetDesc(&desc);
	//		if (desc.Usage ==D3DUSAGE_RENDERTARGET)
	//			int x =0xffff;

	//		//hr =surf->LockRect(&lockRc, NULL, D3DLOCK_DONOTWAIT );
	//		void* container =NULL;
	//		LPDIRECT3DTEXTURE9 tex =NULL;

	//		hr =surf->GetContainer(IID_IDirect3DTexture9, &container);
	//		if (SUCCEEDED(hr)  && container) {

	//			tex =(LPDIRECT3DTEXTURE9)container;
	//			hr =tex->GetLevelDesc(0, &desc);
	//			if (SUCCEEDED(hr)) {
	//				hr =tex->LockRect(0, &lockRc, NULL, 0);
	//				if (SUCCEEDED(hr)) {

	//					tex->UnlockRect(0);
	//				}
	//			}
	//			tex->Release();
	//			tex =NULL;

	//			//Graphics* gfx =Graphics::FromHDC(dc);
	//			//if (gfx !=NULL) {
	//			//	SolidBrush br(Color::Yellow);
	//			//	gfx->FillRectangle(&br, 100,100,200,200);
	//			//	delete gfx;
	//			//	gfx =NULL;
	//			//}
	//			//surf->ReleaseDC(dc);
	//			//surf->UnlockRect();
	//		}
	//		surf->Release();
	//		surf =NULL;
	//	}
	//}

	//Bitmap* bmp =new Bitmap(256,256, PixelFormat32bppARGB);
	//if (bmp !=NULL) {
	//	Graphics* gfx =Graphics::FromImage(bmp);
	//	SolidBrush br(Color::Beige);

	//	if (gfx !=NULL) {
	//		gfx->FillRectangle(&br, 20,20,bmp->GetWidth(),bmp->GetHeight());
	//		delete gfx;
	//	}
	//	
	//	LPDIRECT3DTEXTURE9 tex =NULL;
	//	HRESULT hr =D3DXCreateTexture((LPDIRECT3DDEVICE9)info.mDevice, bmp->GetWidth(), bmp->GetHeight(), 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &tex);
	//	if (D3D_OK ==hr) {
	//		D3DLOCKED_RECT lr ={0};
	//		hr =tex->LockRect(0, &lr, NULL, 0);
	//		if (D3D_OK ==hr) {
	//			tex->UnlockRect(0);
	//			tex->Release();
	//			tex =NULL;
	//		}
	//	}

	//	delete bmp;
	//}

}

bool rf2plugin::RequestCommentary( CommentaryRequestInfoV01 &info )
{
  /*
  // COMMENT OUT TO ENABLE EXAMPLE
  return( false );

  // only if enabled, of course
  if( !mEnabled )
    return( false );

  // Note: function is called twice per second

  // Say green flag event for no particular reason every 20 seconds ...
  const float timeMod20 = fmodf( mET, 20.0f );
  if( timeMod20 > 19.0f )
  {
    strcpy( info.mName, "GreenFlag" );
    info.mInput1 = 0.0;
    info.mInput2 = 0.0;
    info.mInput3 = 0.0;
    info.mSkipChecks = true;
    return( true );
  }
  */
  return( false );
  
}

/*
void main()
{
	printf("irsdk_server 1.0\n");
	printf(" demo program to show how to setup your own telemetry server\n\n");
	printf("press enter to exit:\n\n");

	// hook a variable up dynamically at runtime.  
	// This is only needed if you don't know what the variable name is at compile time
	// or you want to selectively output the variable
	// In either case you still must register the variable before calling irsdkServer::instance()->finalizeHeader()
	int dynamicVar = 1;
	int dynamicVarIndex = irsdkServer::instance()->regVar("dynamicVar", &dynamicVar, irsdk_int, 1, "hook a variable up dynamically at runtime", "m", IRSDK_LOG_ALL);
	// same as above, but don't add a pointer to the variable, instead we will update it on the fly
	int dynamicVar2Index = irsdkServer::instance()->regVar("dynamicVar2", NULL, irsdk_int, 1, "hook a variable up dynamically at runtime", "m", IRSDK_LOG_ALL);

	// do your game initialization...

	while(!_kbhit())
	{
		// before going into your main data output loop, you must mark the headers as final
		// it is possible for finalizeHeader() to fail, so try every tick, just in case
		if(!irsdkServer::instance()->isHeaderFinalized())
			irsdkServer::instance()->finalizeHeader();

		// reset the data for this pass, if something is not updated
		// we will output zero for that tick
		if(irsdkServer::instance()->isInitialized())
			irsdkServer::instance()->resetSampleVars();

		if(irsdkServer::instance()->isHeaderFinalized())
		{
			//update the session string, but only once a second
			static int tickCount = 0;
			if(0 == (tickCount++ % TICKS_PER_SEC))
			{
				irsdkUpdateSessionInfo();
			}

			// read out variables that have registered with pointers back to them
			irsdkServer::instance()->pollSampleVars();

			// update our data...
			g_staticVariable[0] = 1.0f;
			irsdkManualSet.setVar(true);

			dynamicVar = 1;
			int data = 1;
			irsdkServer::instance()->setVar(dynamicVar2Index, &data);

			g_sessionTime += 1.0/TICKS_PER_SEC;

			// use these to turn disk based logging on and off
			if(!irsdkServer::instance()->isDiskLoggingEnabled()) // disk logging is turned on
				irsdkServer::instance()->toggleDiskLogging(); // turn logging on or off
			//irsdkServer::instance()->isDiskLoggingActive(); // disk logging is actively writting to disk
		}
	}

	// if you passed a pointer to the dynamic variable, then release the pointer before your variable disapears
	irsdkServer::instance()->reRegVar(dynamicVarIndex, NULL);

	// otherwise there is no need to deconstruct anything

	printf("Shutting down.\n\n");
}		

// our YAML string of 'slow' changing data and hirarchical data
void irsdkUpdateSessionInfo()
{
	static const int m_len = irsdkServer::sessionStrLen;
	int len = 0;
	char tstr[m_len] = "";

	// Start of YAML file
	len += _snprintf(tstr+len, m_len-len,"---\n");

	// YAML uses spaces at the front to mark nested values
	len += _snprintf(tstr+len, m_len-len,"Section1:\n");
	len += _snprintf(tstr+len, m_len-len," SubVal1: %s\n", "value");
	len += _snprintf(tstr+len, m_len-len," SubVal2: %d\n", 1);
	len += _snprintf(tstr+len, m_len-len," SubSection1:\n");
	len += _snprintf(tstr+len, m_len-len,"  SubVal3: %d\n", 1);

	len += _snprintf(tstr+len, m_len-len,"Section2:\n");
	len += _snprintf(tstr+len, m_len-len," SubSectionArray:\n");

	// arrays of data use '-' to mark the start of a new element
	for(int i=0; i< 10; i++)
	{
		len += _snprintf(tstr+len, m_len-len," - Element1: %d\n", i);
		len += _snprintf(tstr+len, m_len-len,"   Element2: value\n");
		len += _snprintf(tstr+len, m_len-len,"   Element3: %d\n", i*2);
	}
	len += _snprintf(tstr+len, m_len-len,"\n");

	// End of YAML file
	len += _snprintf(tstr+len, m_len-len,"...\n");

	tstr[m_len-1] = '\0';

	// make sure we are not close to running out of room
	assert(len < (m_len-256)); 

	// and push it out
	irsdkServer::instance()->regSessionInfo(tstr);
}

*/