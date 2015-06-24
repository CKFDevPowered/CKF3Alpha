#ifndef IDEMOPLAYER_H
#define IDEMOPLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "DirectorCmd.h"

class IBaseSystem;
class ISystemModule;
class IObjectContainer;
class IWorld;
class IProxy;

typedef struct ref_params_s ref_params_t;

class IDemoPlayer : public IBaseInterface
{
	virtual bool Init(IBaseSystem *system, int serial, char *name);
	virtual void RunFrame(double time);
	virtual void ReceiveSignal(ISystemModule *module, unsigned int signal, void *data);
	virtual void ExecuteCommand(int commandID, char *commandLine);
	virtual void RegisterListener(ISystemModule *module);
	virtual void RemoveListener(ISystemModule *module);
	virtual IBaseSystem *GetSystem(void);
	virtual int GetSerial(void);
	virtual char *GetStatusLine(void);
	virtual char *GetType(void);
	virtual char *GetName(void);
	virtual int GetState(void);
	virtual int GetVersion(void);
	virtual void ShutDown(void);
	virtual void NewGame(IWorld *world, IProxy *proxy);
	virtual char *GetModName(void);
	virtual void WriteCommands(BitBuffer *stream, float startTime, float endTime);
	virtual int AddCommand(DirectorCmd *cmd);
	virtual bool RemoveCommand(int index);
	virtual DirectorCmd *GetLastCommand(void);
	virtual IObjectContainer *GetCommands(void);
	virtual void SetWorldTime(double time, bool relative);
	virtual void SetTimeScale(float scale);
	virtual void SetPaused(bool state);
	virtual void SetEditMode(bool state);
	virtual void SetMasterMode(bool state);
	virtual bool IsPaused(void);
	virtual bool IsLoading(void);
	virtual bool IsActive(void);
	virtual bool IsEditMode(void);
	virtual bool IsMasterMode(void);
	virtual void RemoveFrames(double starttime, double endtime);
	virtual void ExecuteDirectorCmd(DirectorCmd *cmd);
	virtual double GetWorldTime(void);
	virtual double GetStartTime(void);
	virtual double GetEndTime(void);
	virtual float GetTimeScale(void);
	virtual IWorld *GetWorld(void);
	virtual char *GetFileName(void);
	virtual bool SaveGame(char *filename);
	virtual bool LoadGame(char *filename);
	virtual void Stop(void);
	virtual void ForceHLTV(bool state);
	virtual void GetDemoViewInfo(ref_params_t *rp, float *view, int *viewmodel);
	virtual int ReadDemoMessage(unsigned char *buffer, int size);
	virtual void ReadNetchanState(int *incoming_sequence, int *incoming_acknowledged, int *incoming_reliable_acknowledged, int *incoming_reliable_sequence, int *outgoing_sequence, int *reliable_sequence, int *last_reliable_sequence);
};

#define INTERFACE_DEMOPLAYER_NAME "demoplayer001"

#endif