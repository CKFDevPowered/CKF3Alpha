#define svc_nop 1
#define svc_disconnect 2
#define svc_event 3
#define svc_version 4
#define svc_setview 5
#define svc_sound 6
#define svc_time 7
#define svc_print 8
#define svc_stufftext 9
#define svc_setangle 10
#define svc_serverinfo 11
#define svc_lightstyle 12
#define svc_updateuserinfo 13
#define svc_deltadescription 14
#define svc_clientdata 15
#define svc_stopsound 16
#define svc_pings 17
#define svc_particle 18
#define svc_damage 19
#define svc_spawnstatic 20
#define svc_event_reliable 21
#define svc_spawnbaseline 22
#define svc_tempentity 23
#define svc_setpause 24
#define svc_signonnum 25
#define svc_centerprint 26
#define svc_killedmonster 27
#define svc_foundsecret 28
#define svc_spawnstaticsound 29
#define svc_intermission 30
#define svc_finale 31
#define svc_cdtrack 32
#define svc_restore 33
#define svc_cutscene 34
#define svc_weaponanim 35
#define svc_decalname 36
#define svc_roomtype 37
#define svc_addangle 38
#define svc_newusermsg 39
#define svc_packetentities 40
#define svc_deltapacketentities 41
#define svc_choke 42
#define svc_resourcelist 43
#define svc_newmovevars 44
#define svc_resourcerequest 45
#define svc_customization 46
#define svc_crosshairangle 47
#define svc_soundfade 48
#define svc_filetxferfailed 49
#define svc_hltv 50
#define svc_director 51
#define svc_voiceinit 52
#define svc_voicedata 53
#define svc_sendextrainfo 54
#define svc_timescale 55
#define svc_resourcelocation 56
#define svc_sendcvarvalue 57
#define svc_sendcvarvalue2 58

typedef void (*pfnSVC_Parse)(void);

void SVC_Init(void);
pfnSVC_Parse SVC_HookFunc(int opcode, pfnSVC_Parse pfnParse);
void *SVC_GetBuffer(void);
int SVC_GetBufferSize(void);