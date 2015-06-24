#if !defined (PROTO_OOB_H)
#define PROTO_OOB_H

#ifdef _WIN32
#pragma once
#endif

#define PROTOCOL_VERSION 48

#define PORT_RCON 27015
#define PORT_MASTER 27011
#define PORT_CLIENT 27005
#define PORT_SERVER 27015
#define PORT_HLTV 27020
#define PORT_MATCHMAKING 27025
#define PORT_SYSTEMLINK 27030
#define PORT_RPT 27035
#define PORT_RPT_LISTEN 27036

#define A2M_GET_SERVERS 'c'
#define M2A_SERVERS 'd'
#define A2M_GET_SERVERS_BATCH 'e'
#define M2A_SERVER_BATCH 'f'
#define A2M_GET_MOTD 'g'
#define M2A_MOTD 'h'
#define A2A_PING 'i'
#define A2S_INFO 'T'
#define A2S_PLAYER 'U'
#define S2A_PLAYER 'D'
#define A2A_ACK 'j'
#define C2S_CONNECT 'k'
#define A2A_PRINT 'l'
#define S2A_INFO_DETAILED 'm'
#define A2S_GETCHALLENGE 'q'
#define A2S_RCON 'r'
#define A2A_CUSTOM 't'
#define A2M_GETMASTERSERVERS 'v'
#define M2A_MASTERSERVERS 'w'
#define A2M_GETACTIVEMODS 'x'
#define M2A_ACTIVEMODS 'y'
#define M2M_MSG 'z'
#define S2C_CHALLENGE 'A'
#define S2C_CONNECTION 'B'
#define S2A_INFO_GOLDSRC 'm'
#define S2M_GETFILE 'J'
#define M2S_SENDFILE 'K'
#define S2C_REDIRECT 'L'
#define C2M_CHECKMD5 'M'
#define M2C_ISVALIDMD5 'N'
#define M2A_ACTIVEMODS3 'P'
#define A2M_GETACTIVEMODS3 'Q'
#define S2A_LOGSTRING 'R'
#define S2A_LOGKEY 'S'
#define A2S_SERVERQUERY_GETCHALLENGE 'W'
#define A2S_KEY_STRING "Source Engine Query"
#define A2M_GET_SERVERS_BATCH2 '1'
#define A2M_GETACTIVEMODS2 '2'
#define C2S_AUTHREQUEST1 '3'
#define S2C_AUTHCHALLENGE1 '4'
#define C2S_AUTHCHALLENGE2 '5'
#define S2C_AUTHCOMPLETE '6'
#define C2S_AUTHCONNECT '7'
#define S2C_CONNREJECT '9'

#endif