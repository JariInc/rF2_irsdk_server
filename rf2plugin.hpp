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


// This is used for the app to use the plugin for its intended purpose
class rf2plugin : public InternalsPluginV03
{

 public:

  // Constructor/destructor
  rf2plugin() {
	  irsdkSessionTime = irsdkVar("SessionTime", &g_sessionTime, irsdk_double, 1, "Seconds since session start", "s", IRSDK_LOG_ALL, 1, 0);
	  irsdkSessionNum = irsdkVar("SessionNum", &g_sessionNum, irsdk_int, 1, "Session number", "", IRSDK_LOG_ALL);
	  irsdkSessionState = irsdkVar("SessionState", &g_sessionState, irsdk_int, 1, "Session state", "irsdk_SessionState", IRSDK_LOG_ALL);
	  irsdkSessionUniqueID = irsdkVar("SessionUniqueID", &g_sessionUniqueID, irsdk_int, 1, "Session ID", "", IRSDK_LOG_ALL);
	  irsdkSessionFlags = irsdkVar("SessionFlags", &g_sessionFlags, irsdk_bitField, 1, "Session flags", "irsdk_Flags", IRSDK_LOG_ALL);
      irsdkSessionTimeRemain = irsdkVar("SessionTimeRemain", &g_sessionTimeRemain, irsdk_double, 1, "Seconds left till session ends", "s", IRSDK_LOG_ALL);
	  irsdkSessionLapsRemain = irsdkVar("SessionLapsRemain", &g_sessionLapsRemain, irsdk_int, 1, "Laps left till session ends", "", IRSDK_LOG_ALL);
	  irsdkLap = irsdkVar("Lap", &g_lap, irsdk_int, 1, "Lap count", "", IRSDK_LOG_ALL);
	  irsdkCamCarIdx = irsdkVar("CamCarIdx", &g_camcaridx, irsdk_int, 1, "Active camera's focus car index", "", IRSDK_LOG_ALL);
	  irsdkCamGroupNumber = irsdkVar("CamGroupNumber", &g_camgroupnumber, irsdk_int, 1, "Active camera group number", "", IRSDK_LOG_ALL);
	  irsdkReplayFrameNum = irsdkVar("ReplayFrameNum", &g_replayFrameNum, irsdk_int, 1, "Integer replay frame number (60 per second)", "", IRSDK_LOG_ALL);
	  irsdkReplaySessionTime = irsdkVar("ReplaySessionTime", &g_replaySessionTime, irsdk_double, 1, "Seconds since replay session start", "s", IRSDK_LOG_ALL);
	  irsdkCarIdxLapDistPct = irsdkVar("CarIdxLapDistPct", &g_carIdxLapDistPct, irsdk_float, 64, "Percentage distance around lap by car index", "%", IRSDK_LOG_ALL);
	  irsdkCarIdxLap = irsdkVar("CarIdxLap", &g_carIdxLap, irsdk_int, 64, "Lap count by car index", "", IRSDK_LOG_ALL);
	  irsdkCarIdxTrackSurface = irsdkVar("CarIdxTrackSurface", &g_carIdxLap, irsdk_int, 64, "Track surface type by car index", "", IRSDK_LOG_ALL);

  }
  ~rf2plugin() {}

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

