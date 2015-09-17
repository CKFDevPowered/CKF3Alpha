#include "gl_local.h"

#define LOADTGA_SIG "\x8B\x44\x24\x14\x8B\x4C\x24\x10\x8B\x54\x24\x0C\x6A\x01\x50\x8B\x44\x24\x10\x51\x8B\x4C\x24\x10\x52\x50\x51\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\xC3"
#define LOADTGA_SIG_NEW "\x55\x8B\xEC\x8B\x45\x18\x8B\x4D\x14\x8B\x55\x10\x6A\x00\x50\x8B\x45\x0C\x51\x8B\x4D\x08\x52\x50\x51\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\x5D\xC3"

#define R_CLEAR_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xDC\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0"
#define R_CLEAR_SIG2 "\x8B\x15\x2A\x2A\x2A\x2A\x33\xC0\x83\xFA\x01\x0F\x9F\xC0\x50\xE8\x2A\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xDC\x1D\x2A\x2A\x2A\x2A\x83\xC4\x04\xDF\xE0"
#define R_CLEAR_SIG_NEW "\x8B\x15\x2A\x2A\x2A\x2A\x33\xC0\x83\xFA\x01\x0F\x9F\xC0\x50\xE8\x2A\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xDC\x1D\x2A\x2A\x2A\x2A\x83\xC4\x04"

#define R_RENDERSCENE_SIG "\x51\xDB\x05\x2A\x2A\x2A\x2A\xD9\x5C\x24\x00\xD9\x05\x2A\x2A\x2A\x2A\xD8\x5C\x24\x00\xDF\xE0\xF6\xC4\x2A\x2A\x2A\xA1"
#define R_RENDERSCENE_SIG_NEW "\xE8\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\xE8\x2A\x2A\x2A\x2A\xE8"

#define R_NEWMAP_SIG "\x55\x8B\xEC\x83\xEC\x0C\xC7\x45\xFC\x00\x00\x00\x00\x2A\x2A\x8B\x45\xFC\x83\xC0\x01\x89\x45\xFC"
#define R_NEWMAP_SIG_NEW "\x55\x8B\xEC\x83\xEC\x08\xC7\x45\xFC\x00\x00\x00\x00\x2A\x2A\x8B\x45\xFC\x83\xC0\x01\x89\x45\xFC\x81\x7D\xFC\x00\x01\x00\x00\x2A\x2A\x8B\x4D\xFC"

#define R_DRAWWORLD_SIG "\x81\xEC\xB8\x0B\x00\x00\x68\xB8\x0B\x00\x00\x8D\x44\x24\x04\x6A\x00\x50\xE8"
#define R_DRAWWORLD_SIG_NEW "\x55\x8B\xEC\x81\xEC\xB8\x0B\x00\x00\x68\xB8\x0B\x00\x00\x8D\x85\x48\xF4\xFF\xFF\x6A\x00\x50\xE8\x2A\x2A\x2A\x2A\x8B\x0D"

#define R_MARKLEAVES_SIG "\xB8\x00\x10\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x0D\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A"
#define R_MARKLEAVES_SIG_NEW "\x55\x8B\xEC\xB8\x04\x10\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x0D\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A"

#define R_CULLBOX_SIG "\x53\x8B\x5C\x24\x08\x56\x57\x8B\x7C\x24\x14\xBE\x2A\x2A\x2A\x2A\x56\x57\x53\xE8"
#define R_CULLBOX_SIG_NEW "\x55\x8B\xEC\x53\x8B\x5D\x08\x56\x57\x8B\x7D\x0C\xBE\x2A\x2A\x2A\x2A\x56\x57\x53"

#define R_SETUPFRAME_SIG "\xA1\x2A\x2A\x2A\x2A\x83\xEC\x18\x83\xF8\x01\x0F\x8E\x2A\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x2A\x2A\x2A\x68"
#define R_SETUPFRAME_SIG2 "\x8B\x0D\x2A\x2A\x2A\x2A\x83\xEC\x18\x33\xC0\x83\xF9\x01\x0F\x9F\xC0\x50\xE8\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xA1"
#define R_SETUPFRAME_SIG_NEW "\x55\x8B\xEC\x83\xEC\x18\x8B\x0D\x2A\x2A\x2A\x2A\x33\xC0\x83\xF9\x01\x0F\x9F\xC0\x50\xE8\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xA1"

#define GL_BUILDLIGHTMAPS_SIG "\x53\x56\x57\x68\x00\x80\x00\x00\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x83\xC4\x0C\xBB\x01\x00\x00\x00\x85\xC0\x89\x1D"
#define GL_BUILDLIGHTMAPS_SIG2 "\x51\x68\x00\x80\x00\x00\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x83\xC4\x0C\xB9\x01\x00\x00\x00\x85\xC0\x89\x0D"
#define GL_BUILDLIGHTMAPS_SIG_NEW "\x55\x8B\xEC\x51\x53\x56\x57\x68\x00\x80\x00\x00\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x83\xC4\x0C"

#define GL_BIND_SIG "\x8B\x44\x24\x04\x8B\x0D\x2A\x2A\x2A\x2A\x56\x8B\xF0\xC1\xFE\x10\x25\xFF\xFF\x00\x00\x4E\x3B\xC8"
#define GL_BIND_SIG_NEW "\x55\x8B\xEC\x8B\x45\x08\x8B\x0D\x2A\x2A\x2A\x2A\x56\x8B\xF0\xC1\xFE\x10\x25\xFF\xFF\x00\x00\x4E\x3B\xC8"

#define GL_SELECTTEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x56\x85\xC0\x2A\x2A\x8B\x74\x24\x08\x56\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x3B\xF0"
#define GL_SELECTTEXTURE_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x56\x85\xC0\x2A\x2A\x8B\x75\x08\x56\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x3B\xF0"

#define GL_LOADTEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x8B\x4C\x24\x20\x8B\x54\x24\x1C\x50\x8B\x44\x24\x1C\x51\x8B\x4C\x24\x1C\x52\x8B\x54\x24\x1C\x50\x8B\x44\x24\x1C"
#define GL_LOADTEXTURE_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x8B\x4D\x24\x8B\x55\x20\x50\x8B\x45\x1C\x51\x8B\x4D\x18\x52\x8B\x55\x14\x50\x8B\x45\x10\x51"

#define GL_LOADTEXTURE2_SIG "\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x53\x8B\x9C\x24\x14\x40\x00\x00\x55\x56\x8A\x03\x33\xF6"
#define GL_LOADTEXTURE2_SIG2 "\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x84\x24\x10\x40\x00\x00\x53\x55\x33\xDB\x8A\x08\x56\x84\xC9\x57\x89\x5C\x24\x14"
#define GL_LOADTEXTURE2_SIG_NEW "\x55\x8B\xEC\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x45\x08\x53\x33\xDB\x56\x8A\x08\x57\x84\xC9\x89\x5D\xF4\x74\x2A\x33\xF6"

#define GL_BEGINRENDERING_SIG "\x8B\x44\x24\x08\x8B\x4C\x24\x04\x8B\x54\x24\x0C\xC7\x00\x00\x00\x00\x00\xA1\x2A\x2A\x2A\x2A\xC7\x01\x00\x00\x00\x00\x8B\x0D\x2A\x2A\x2A\x2A\x2B\xC1"
#define GL_BEGINRENDERING_SIG_NEW "\x55\x8B\xEC\x8B\x45\x0C\x8B\x4D\x08\x56\x57\xC7\x00\x00\x00\x00\x00\xC7\x01\x00\x00\x00\x00\xE8"

#define GL_ENDRENDERING_SIG "\xFF\x25\x2A\x2A\x2A\x2A\x90"
#define GL_ENDRENDERING_SIG_NEW "\x55\x8B\xEC\x83\xEC\x34\xA1\x2A\x2A\x2A\x2A\x53\x33\xDB"

#define EMITWATERPOLYS_SIG "\x83\xEC\x1C\x33\xD2\x55\x56\x8B\x74\x24\x28\x57\x8B\x46\x2C\x8B\x48\x24\x8B\x41\x44\x33\xC9\x8A\x50\x0C\x8A\x48\x0B\x52"
#define EMITWATERPOLYS_SIG_NEW "\x55\x8B\xEC\x83\xEC\x20\x56\x8B\x75\x08\x33\xD2\x57\x8B\x46\x2C\x8B\x48\x24\x8B\x41\x44\x33\xC9\x8A\x50\x0C\x8A\x48\x0B\x52\x51"

#define R_DRAWSKYCHAIN_SIG "\x81\xEC\x00\x03\x00\x00\xE8\x2A\x2A\x2A\x2A\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\x8B\x84\x24\x04\x03\x00\x00\x57\x8B\xF8\x85\xC0"
#define R_DRAWSKYCHAIN_SIG_NEW "\x55\x8B\xEC\x81\xEC\x00\x03\x00\x00\xE8\x2A\x2A\x2A\x2A\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\x8B\x45\x08\x57\x8B\xF8\x85\xC0"

#define R_DRAWSKYBOX_SIG "\x83\xEC\x1C\xA1\x2A\x2A\x2A\x2A\x53\x55\x56\x57\xBF\x00\x00\x80\x3F\x85\xC0"
#define R_DRAWSKYBOX_SIG2 "\x83\xEC\x1C\xA1\x2A\x2A\x2A\x2A\x53\x55\x56\x57\x33\xF6\xBF\x00\x00\x80\x3F"
#define R_DRAWSKYBOX_SIG_NEW "\x55\x8B\xEC\x83\xEC\x1C\xA1\x2A\x2A\x2A\x2A\x53\x56\xBB\x00\x00\x80\x3F\x57"

#define BUILDSURFACEDISPLAYLIST_SIG "\x83\xEC\x48\x53\x55\x56\x8B\x74\x24\x58\xA1\x2A\x2A\x2A\x2A\x57\x8B\x6E\x10\x8B\x98\x9C\x00\x00\x00\x8D\x0C\xED\x00\x00\x00\x00"
#define BUILDSURFACEDISPLAYLIST_SIG2 "\x83\xEC\x48\x53\x55\x56\x8B\x74\x24\x58\xA1\x2A\x2A\x2A\x2A\x57\x8B\x7E\x10\x8B\xA8\x9C\x00\x00\x00\x8D\x0C\xFD\x00\x00\x00\x00"
#define BUILDSURFACEDISPLAYLIST_SIG_NEW "\x55\x8B\xEC\x83\xEC\x48\x53\x56\x8B\x75\x08\xA1\x2A\x2A\x2A\x2A\x57\x8B\x5E\x10\x8B\x88\x9C\x00\x00\x00\x89\x4D\xF0"

#define R_DRAWENTITIESONLIST_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x56\xDF\xE0\xF6\xC4\x44\x0F\x2A\x2A\x2A\x00\x00"
#define R_DRAWENTITIESONLIST_SIG2 "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x56\x57\xDF\xE0\xF6\xC4\x40\x0F\x2A\x2A\x2A\x00\x00"
#define R_DRAWENTITIESONLIST_SIG_NEW "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x56\xDF\xE0\xF6\xC4\x44\x0F\x8B\x2A\x2A\x2A\x2A\x8B\x0D"

#define R_DRAWTENTITIESONLIST_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x2A\x0F\x2A\x2A\x2A\x00\x00\x8B\x44\x24\x04"
#define R_DRAWTENTITIESONLIST_SIG_NEW "\x55\x8B\xEC\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x44\x0F\x8B\x2A\x2A\x2A\x2A\x8B\x45\x08"

#define R_SETFRUSTUM_SIG "\x83\xEC\x08\xDB\x05\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x56\x89\x44\x24\x04\xD9\x5C\x24\x08\xDB\x05"
#define R_SETFRUSTUM_SIG_NEW "\x55\x8B\xEC\x83\xEC\x08\xDB\x05\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x56\x89\x45\xFC\xD9\x5D\xF8\xDB\x05"

#define R_SETUPGL_SIG "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x10\x53\x55\x56\x57\x68\x01\x17\x00\x00\xFF\x15\x2A\x2A\x2A\x2A\xFF\x15"
#define R_SETUPGL_SIG_NEW "\x55\x8B\xEC\x83\xEC\x10\x53\x56\x57\x68\x01\x17\x00\x00\xFF\x15\x2A\x2A\x2A\x2A\xFF\x15\x2A\x2A\x2A\x2A\x8B\x1D"

#define GL_DISABLEMULTITEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\x68\xE1\x0D\x00\x00\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8"
#define GL_DISABLEMULTITEXTURE_SIG_NEW "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\x68\xE1\x0D\x00\x00\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8"

#define GL_ENABLEMULTITEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x68\xE1\x0D\x00\x00\xFF\x15"
#define GL_ENABLEMULTITEXTURE_SIG_NEW "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x68\xE1\x0D\x00\x00\xFF\x15"

#define R_DRAWSEQUENTIALPOLY_SIG "\xA1\x2A\x2A\x2A\x2A\x53\x55\x56\x8B\x88\xF8\x02\x00\x00\xBE\x01\x00\x00\x00"
#define R_DRAWSEQUENTIALPOLY_SIG_NEW "\x55\x8B\xEC\x51\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\x83\xB8\xF8\x02\x00\x00\x01\x75\x2A\xE8"

#define R_DRAWBRUSHMODEL_SIG "\x83\xEC\x4C\xC7\x05\x2A\x2A\x2A\x2A\xFF\xFF\xFF\xFF\x53\x55\x56\x57"
#define R_DRAWBRUSHMODEL_SIG_NEW "\x55\x8B\xEC\x83\xEC\x50\x53\x56\x57\x8B\x7D\x08\x89\x3D\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\xFF\xFF\xFF\xFF"

#define R_RECURSIVEWORLDNODE_SIG "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x0C\x53\x56\x57\x8B\x7D\x08\x83\x3F\xFE"
#define R_RECURSIVEWORLDNODE_SIG_NEW "\x55\x8B\xEC\x83\xEC\x08\x53\x56\x57\x8B\x7D\x08\x83\x3F\xFE\x0F\x2A\x2A\x2A\x2A\x2A\x8B\x47\x04"

#define R_TEXTUREANIMATION_SIG "\x53\x55\x8B\x6C\x24\x0C\x56\x57\x8B\x45\x2C\x8B\x78\x24\xA1"
#define R_TEXTUREANIMATION_SIG_NEW "\x55\x8B\xEC\x8B\x45\x08\x53\x56\x57\x8B\x48\x2C\xA1\x2A\x2A\x2A\x2A\x85\xC0\x8B\x79\x24\x75\x2A\xBE"

#define R_RENDERDYNAMICLIGHTMAPS_SIG "\x8B\x0D\x2A\x2A\x2A\x2A\x53\x41\x55\x89\x0D\x2A\x2A\x2A\x2A\x8B\x4C\x24\x0C\x56\x57"
#define R_RENDERDYNAMICLIGHTMAPS_SIG_NEW "\x55\x8B\xEC\x8B\x0D\x2A\x2A\x2A\x2A\x53\x41\x56\x89\x0D\x2A\x2A\x2A\x2A\x8B\x4D\x08\x57"

#define R_BUILDLIGHTMAP_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x83\xEC\x18\xDF\xE0\xF6\xC4"
#define R_BUILDLIGHTMAP_SIG_NEW "\x55\x8B\xEC\x83\xEC\x1C\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0"

#define R_DECALMPOLY_SIG "\xA1\x2A\x2A\x2A\x2A\x57\x50\xE8\x2A\x2A\x2A\x2A\x8B\x4C\x24\x10\x8B\x51\x18"
#define R_DECALMPOLY_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x57\x50\xE8\x2A\x2A\x2A\x2A\x8B\x4D\x0C\x8B\x51\x18\x52\xE8"

#define R_DRAWDECALS_SIG "\xB8\x0C\x00\x00\x00\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x85\xC0"
#define R_DRAWDECALS_SIG_NEW "\x55\x8B\xEC\xB8\x10\x00\x00\x00\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x85\xC0\x0F\x84"

#define R_DRAW_DECALTEXTURE_SIG "\x8B\x44\x24\x04\x56\x85\xC0\x57\x7D\x7B\x83\xC9\xFF\x2B\xC8\x8D\x04\x49"
#define R_DRAW_DECALTEXTURE_SIG2 "\x8B\x4C\x24\x04\x56\x85\xC9\x57\x2A\x2A\x83\xC8\xFF\x2B\xC1"
#define R_DRAW_DECALTEXTURE_SIG_NEW "\x55\x8B\xEC\x8B\x4D\x08\x56\x85\xC9\x57\x7D\x2A\x83\xC8\xFF\x2B\xC1\x8D\x0C\xC0"

#define R_ALLOCOBJECTS_SIG "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x74\x0D\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04"
#define R_ALLOCOBJECTS_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x85\xC0\x74\x0D\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x56\x57"

#define GLOWBLEND_SIG "\x81\xEC\xA0\x00\x00\x00\xD9\x05\x2A\x2A\x2A\x2A\xD8\x25\x2A\x2A\x2A\x2A\x56\x8D\x44\x24"
#define GLOWBLEND_SIG2 "\x83\xEC\x58\xD9\x05\x2A\x2A\x2A\x2A\xD8\x25\x2A\x2A\x2A\x2A\x56\x8D\x44\x24"
#define GLOWBLEND_SIG_NEW "\x55\x8B\xEC\x81\xEC\x9C\x00\x00\x00\xD9\x05\x2A\x2A\x2A\x2A\xD8\x25\x2A\x2A\x2A\x2A\x56\x8D\x45\xEC\x57\x50"

#define R_RENDERVIEW_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x83\xEC\x14\xDF\xE0\xF6\xC4"
#define R_RENDERVIEW_SIG_NEW "\x55\x8B\xEC\x83\xEC\x14\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x44"

#define DRAW_MIPTEXTEXTURE_SIG "\x83\xEC\x28\x53\x55\x56\x8B\x74\x24\x38\x57\x83\x7E\x18\x20"
#define DRAW_MIPTEXTEXTURE_SIG2 "\x83\xEC\x28\x53\x8B\x5C\x24\x30\x55\x56\x8B\x43\x18"
#define DRAW_MIPTEXTEXTURE_SIG_NEW "\x55\x8B\xEC\x83\xEC\x28\x53\x56\x8B\x75\x08\x57\x83\x7E\x18\x20\x74\x2A\x8B\x06\x50"

#define FREEFBOBJECTS_SIG_NEW "\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x3B\xC6\x74\x0D\x68\x2A\x2A\x2A\x2A\x6A\x01\xFF\x15"

#define GL_SWAPBUFFER_SIG "\xFF\x25"
#define GL_SWAPBUFFER_SIG_NEW "\xFF\x15\x2A\x2A\x2A\x2A\x5B\x8B\xE5\x5D\xC3"

#define VID_UPDATEWINDOWVARS_SIG "\x56\x8B\x74\x24\x08\x8B\xC6\x8B\x08\x89\x0D\x2A\x2A\x2A\x2A\x8B\x50\x04\x89\x15"
#define VID_UPDATEWINDOWVARS_SIG_NEW "\x55\x8B\xEC\x51\x56\x8B\x75\x08\x8B\xC6\x8B\x08\x89\x0D\x2A\x2A\x2A\x2A\x8B\x50\x04\x89\x15"

//Studio Funcs
#define R_GLSTUDIODRAWPOINTS_SIG "\x83\xEC\x44\xA1\x2A\x2A\x2A\x2A\x8B\x0D\x2A\x2A\x2A\x2A\x53\x55\x56\x8B\x70\x54\x8B\x40\x60\x57"
#define R_GLSTUDIODRAWPOINTS_SIG2 "\x83\xEC\x48\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x15\x2A\x2A\x2A\x2A\x53\x55\x8B\x41\x54\x8B\x59\x60"
#define R_GLSTUDIODRAWPOINTS_SIG_NEW "\x55\x8B\xEC\x83\xEC\x48\xA1\x2A\x2A\x2A\x2A\x8B\x0D\x2A\x2A\x2A\x2A\x53\x56\x8B\x70\x54\x8B\x40\x60\x03\xC1"

#define R_STUDIORENDERFINAL_SIG "\x51\x6A\x00\x6A\x00\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xE8"
#define R_STUDIORENDERFINAL_SIG_NEW "\x55\x8B\xEC\x51\x6A\x00\x6A\x00\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xE8"

#define R_STUDIOLIGHTING_SIG "\x51\xDB\x05\x2A\x2A\x2A\x2A\x8A\x4C\x24\x10\xB8\x01\x00\x00\x00\x84\xC8\xD9\x5C\x24\x00"
#define R_STUDIOLIGHTING_SIG_NEW "\x55\x8B\xEC\x51\xDB\x05\x2A\x2A\x2A\x2A\x8A\x4D\x10\xB8\x01\x00\x00\x00\x84\xC8\xD9\x5D\xFC"

#define R_LIGHTSTRENGTH_SIG "\x8B\x0D\x2A\x2A\x2A\x2A\x83\xEC\x0C\x55\x8B\x6C\x24\x14\x56\x8B\x04\xAD"
#define R_LIGHTSTRENGTH_SIG2 "\x8B\x4C\x24\x04\x8B\x15\x2A\x2A\x2A\x2A\x83\xEC\x0C\x8B\x04\x8D\x2A\x2A\x2A\x2A\x3B\xC2"
#define R_LIGHTSTRENGTH_SIG_NEW "\x55\x8B\xEC\x83\xEC\x0C\x8B\x4D\x08\x8B\x15\x2A\x2A\x2A\x2A\x56\x8B\x04\x8D\x2A\x2A\x2A\x2A\x3B\xC2"

#define R_LIGHTLAMBERT_SIG "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x2C\x8B\x0D\x2A\x2A\x2A\x2A\x53"
#define R_LIGHTLAMBERT_SIG2 "\x8B\x0D\x2A\x2A\x2A\x2A\x83\xEC\x24\x53\x33\xDB\x56"
#define R_LIGHTLAMBERT_SIG_NEW "\x55\x8B\xEC\x83\xEC\x24\x8B\x0D\x2A\x2A\x2A\x2A\x56\x57\x33\xFF\x3B\xCF\x75"

#define R_STUDIOCHROME_SIG "\x83\xEC\x24\x8B\x0D\x2A\x2A\x2A\x2A\x53\x56\x8B\x74\x24\x34\x57\x8B\x04\xB5"
#define R_STUDIOCHROME_SIG_NEW "\x55\x8B\xEC\x83\xEC\x24\x8B\x0D\x2A\x2A\x2A\x2A\x53\x56\x8B\x75\x0C\x57\x8B\x04\xB5"

#define R_LOADTEXTURES_SIG "\xA1\x2A\x2A\x2A\x2A\x81\xEC\x04\x01\x00\x00\x8B\x88\xB8\x00\x00\x00\x85\xC9"
#define R_LOADTEXTURES_SIG_NEW "\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x8B\x88\xB8\x00\x00\x00\x85\xC9\x0F"

#define BUILDGLOWSHELLVERTS_SIG "\x83\xEC\x2A\xA1\x2A\x2A\x2A\x2A\x53\x8B\x1D\x2A\x2A\x2A\x2A\x2A\xDB\x80"
#define BUILDGLOWSHELLVERTS_SIG_NEW "\x55\x8B\xEC\x83\xEC\x1C\xA1\x2A\x2A\x2A\x2A\x53\x8B\x1D\x2A\x2A\x2A\x2A\x56\xDB\x80"

#define BUILDNORMALINDEXTABLE_SIG "\x8B\x15\x2A\x2A\x2A\x2A\x57\x8B\x4A\x50\x85\xC9\x7E\x2A\x83\xC8\xFF\xBF\x2A\x2A\x2A\x2A"
#define BUILDNORMALINDEXTABLE_SIG_NEW "\x55\x8B\xEC\x51\x8B\x15\x2A\x2A\x2A\x2A\x57\x8B\x4A\x50\x85\xC9\x7E\x0A\x83\xC8\xFF\xBF"

#define R_STUDIOSETUPSKIN_SIG "\xA0\x2A\x2A\x2A\x2A\x81\xEC\x0C\x01\x00\x00\xA8\x02\x53\x56\x57\x0F\x85"
#define R_STUDIOSETUPSKIN_SIG_NEW "\x55\x8B\xEC\x81\xEC\x04\x01\x00\x00\xA0\x2A\x2A\x2A\x2A\x53\x56\x57\xA8\x02\x0F\x85\x2A\x2A\x2A\x2A\x8B\x5D\x0C"

//Sprite Funcs
#define R_GETSPRITEFRAME_SIG "\x56\x8B\x74\x24\x08\x57\x33\xFF\x85\xF6\x75\x2A\x68\x2A\x2A\x2A\x2A\xE8"
#define R_GETSPRITEFRAME_SIG_NEW "\x55\x8B\xEC\x56\x8B\x75\x08\x57\x33\xFF\x85\xF6\x75\x13\x68"

#define R_GETSPRITEAXES_SIG "\x8B\x4C\x24\x04\x83\xEC\x0C\xD9\x81\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x56\x57"
#define R_GETSPRITEAXES_SIG_NEW "\x55\x8B\xEC\x83\xEC\x0C\x8B\x4D\x08\x56\x57\xD9\x81\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0"

#define R_SPRITECOLOR_SIG "\x83\xEC\x08\x55\x56\x57\x8B\x7C\x24\x1C\x8B\x87\x2A\x2A\x2A\x2A\x83\xF8\x05\x74\x2A"
#define R_SPRITECOLOR_SIG_NEW "\x55\x8B\xEC\x83\xEC\x08\x53\x56\x57\x8B\x7D\x0C\x8B\x87\xF8\x02\x00\x00\x83\xF8\x05"

#define R_DRAWSRPITEMODEL_SIG "\x83\xEC\x40\x53\x56\x57\x8B\x7C\x24\x50\x8B\x87\x94\x0B\x00\x00"
#define R_DRAWSRPITEMODEL_SIG_NEW "\x55\x8B\xEC\x83\xEC\x44\x53\x56\x57\x8B\x7D\x08\x8B\x87\x94\x0B\x00\x00\xD9\x87\xE0\x02\x00\x00"

void Sys_ErrorEx(const char *fmt, ...);

void R_FillAddress(void)
{
	DWORD addr;
	if(g_iVideoMode == 2)
	{
		Sys_ErrorEx("D3D mode is not supported.");
	}
	if(g_iVideoMode == 0)
	{
		Sys_ErrorEx("Software mode is not supported.");
	}
	if (g_dwEngineBuildnum >= 5953)
	{
		gRefFuncs.LoadTGA = (int (*)(const char *, byte *, int , int *, int *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADTGA_SIG_NEW, sizeof(LOADTGA_SIG_NEW) - 1);
		if(!gRefFuncs.LoadTGA)
			SIG_NOT_FOUND("LoadTGA");

		gRefFuncs.GL_Bind = (void (*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, GL_BIND_SIG_NEW, sizeof(GL_BIND_SIG_NEW) - 1);
		if(!gRefFuncs.GL_Bind)
			SIG_NOT_FOUND("GL_Bind");

		gRefFuncs.GL_SelectTexture = (void (*)(GLenum))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_Bind, g_dwEngineSize - ((DWORD)gRefFuncs.GL_Bind - g_dwEngineBase), GL_SELECTTEXTURE_SIG_NEW, sizeof(GL_SELECTTEXTURE_SIG_NEW) - 1);
		if(!gRefFuncs.GL_SelectTexture)
			SIG_NOT_FOUND("GL_SelectTexture");

		gRefFuncs.GL_LoadTexture2 = (int (*)(char *, int, int, int, byte *, qboolean, int, byte *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_SelectTexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_SelectTexture - g_dwEngineBase), GL_LOADTEXTURE2_SIG_NEW, sizeof(GL_LOADTEXTURE2_SIG_NEW) - 1);
		if(!gRefFuncs.GL_LoadTexture2)
			SIG_NOT_FOUND("GL_LoadTexture2");

		gRefFuncs.R_CullBox = (qboolean (*)(vec3_t, vec3_t))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, g_dwEngineSize - ((DWORD)gRefFuncs.GL_LoadTexture2 - g_dwEngineBase), R_CULLBOX_SIG_NEW, sizeof(R_CULLBOX_SIG_NEW) - 1);
		if(!gRefFuncs.R_CullBox)
			SIG_NOT_FOUND("R_CullBox");

		gRefFuncs.R_SetupFrame = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_CullBox, g_dwEngineSize - ((DWORD)gRefFuncs.R_CullBox - g_dwEngineBase), R_SETUPFRAME_SIG_NEW, sizeof(R_SETUPFRAME_SIG_NEW) - 1);
		if(!gRefFuncs.R_SetupFrame)
			SIG_NOT_FOUND("R_SetupFrame");

		gRefFuncs.R_Clear = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_SetupFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_SetupFrame - g_dwEngineBase), R_CLEAR_SIG_NEW, sizeof(R_CLEAR_SIG_NEW) - 1);
		if(!gRefFuncs.R_Clear)
			SIG_NOT_FOUND("R_Clear");

		gRefFuncs.R_RenderScene = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_Clear, g_dwEngineSize - ((DWORD)gRefFuncs.R_Clear - g_dwEngineBase), R_RENDERSCENE_SIG_NEW, sizeof(R_RENDERSCENE_SIG_NEW) - 1);
		if(!gRefFuncs.R_RenderScene)
			SIG_NOT_FOUND("R_RenderScene");

		gRefFuncs.R_RenderView = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderScene, g_dwEngineSize - ((DWORD)gRefFuncs.R_RenderScene - g_dwEngineBase), R_RENDERVIEW_SIG_NEW, sizeof(R_RENDERVIEW_SIG_NEW)-1);
		if(!gRefFuncs.R_RenderView)
			SIG_NOT_FOUND("R_RenderView");

		gRefFuncs.R_NewMap = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderView, g_dwEngineSize - ((DWORD)gRefFuncs.R_RenderView - g_dwEngineBase), R_NEWMAP_SIG_NEW, sizeof(R_NEWMAP_SIG_NEW) - 1);
		if(!gRefFuncs.R_NewMap)
			SIG_NOT_FOUND("R_NewMap");

		gRefFuncs.R_BuildLightMap = (void (*)(msurface_t *, byte *, int ))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_NewMap, g_dwEngineSize - ((DWORD)gRefFuncs.R_NewMap - g_dwEngineBase), R_BUILDLIGHTMAP_SIG_NEW, sizeof(R_BUILDLIGHTMAP_SIG_NEW)-1);
		if(!gRefFuncs.R_BuildLightMap)
			SIG_NOT_FOUND("R_BuildLightMap");

		gRefFuncs.GL_DisableMultitexture = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_NewMap, g_dwEngineSize - ((DWORD)gRefFuncs.R_NewMap - g_dwEngineBase), GL_DISABLEMULTITEXTURE_SIG_NEW, sizeof(GL_DISABLEMULTITEXTURE_SIG_NEW) - 1);
		if(!gRefFuncs.GL_DisableMultitexture)
			SIG_NOT_FOUND("GL_DisableMultitexture");

		gRefFuncs.GL_EnableMultitexture = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_DisableMultitexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_DisableMultitexture - g_dwEngineBase), GL_ENABLEMULTITEXTURE_SIG_NEW, sizeof(GL_ENABLEMULTITEXTURE_SIG_NEW) - 1);
		if(!gRefFuncs.GL_EnableMultitexture)
			SIG_NOT_FOUND("GL_EnableMultitexture");

		gRefFuncs.R_RenderDynamicLightmaps = (void (*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EnableMultitexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_EnableMultitexture - g_dwEngineBase), R_RENDERDYNAMICLIGHTMAPS_SIG_NEW, sizeof(R_RENDERDYNAMICLIGHTMAPS_SIG_NEW)-1);
		if(!gRefFuncs.R_RenderDynamicLightmaps)
			SIG_NOT_FOUND("R_RenderDynamicLightmaps");

		gRefFuncs.R_DrawBrushModel = (void (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderDynamicLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.R_RenderDynamicLightmaps - g_dwEngineBase), R_DRAWBRUSHMODEL_SIG_NEW, sizeof(R_DRAWBRUSHMODEL_SIG_NEW)-1);
		if(!gRefFuncs.R_DrawBrushModel)
			SIG_NOT_FOUND("R_DrawBrushModel");

		gRefFuncs.R_RecursiveWorldNode = (void (*)(mnode_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawBrushModel, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawBrushModel - g_dwEngineBase), R_RECURSIVEWORLDNODE_SIG_NEW, sizeof(R_RECURSIVEWORLDNODE_SIG_NEW)-1);
		if(!gRefFuncs.R_RecursiveWorldNode)
			SIG_NOT_FOUND("R_RecursiveWorldNode");

		gRefFuncs.R_DrawWorld = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RecursiveWorldNode, g_dwEngineSize - ((DWORD)gRefFuncs.R_RecursiveWorldNode - g_dwEngineBase), R_DRAWWORLD_SIG_NEW, sizeof(R_DRAWWORLD_SIG_NEW) - 1);
		if(!gRefFuncs.R_DrawWorld)
			SIG_NOT_FOUND("R_DrawWorld");

		gRefFuncs.R_MarkLeaves = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawWorld, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawWorld - g_dwEngineBase), R_MARKLEAVES_SIG_NEW, sizeof(R_MARKLEAVES_SIG_NEW) - 1);
		if(!gRefFuncs.R_MarkLeaves)
			SIG_NOT_FOUND("R_MarkLeaves");

		gRefFuncs.BuildSurfaceDisplayList = (void(*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_MarkLeaves, g_dwEngineSize - ((DWORD)gRefFuncs.R_MarkLeaves - g_dwEngineBase), BUILDSURFACEDISPLAYLIST_SIG_NEW, sizeof(BUILDSURFACEDISPLAYLIST_SIG_NEW) - 1);
		if(!gRefFuncs.BuildSurfaceDisplayList)
			SIG_NOT_FOUND("BuildSurfaceDisplayList");

		gRefFuncs.GL_BuildLightmaps = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.BuildSurfaceDisplayList, g_dwEngineSize - ((DWORD)gRefFuncs.R_RecursiveWorldNode - g_dwEngineBase), GL_BUILDLIGHTMAPS_SIG_NEW, sizeof(GL_BUILDLIGHTMAPS_SIG_NEW) - 1);
		if(!gRefFuncs.GL_BuildLightmaps)
			SIG_NOT_FOUND("GL_BuildLightmaps");

		gRefFuncs.R_DecalMPoly = (void (*)(float *, texture_t *, msurface_t *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BuildLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BuildLightmaps - g_dwEngineBase), R_DECALMPOLY_SIG_NEW, sizeof(R_DECALMPOLY_SIG_NEW)-1);
		if(!gRefFuncs.R_DecalMPoly)
			SIG_NOT_FOUND("R_DecalMPoly");

		gRefFuncs.R_DrawDecals = (void (*)(qboolean))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DecalMPoly, g_dwEngineSize - ((DWORD)gRefFuncs.R_DecalMPoly - g_dwEngineBase), R_DRAWDECALS_SIG_NEW, sizeof(R_DRAWDECALS_SIG_NEW)-1);
		if(!gRefFuncs.R_DrawDecals)
			SIG_NOT_FOUND("R_DrawDecals");

		gRefFuncs.GL_BeginRendering = (void (*)(int *, int *, int *, int *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BuildLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BuildLightmaps - g_dwEngineBase), GL_BEGINRENDERING_SIG_NEW, sizeof(GL_BEGINRENDERING_SIG_NEW) - 1);
		if(!gRefFuncs.GL_BeginRendering)
			SIG_NOT_FOUND("GL_BeginRendering");

		gRefFuncs.GL_EndRendering = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BeginRendering, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BeginRendering - g_dwEngineBase), GL_ENDRENDERING_SIG_NEW, sizeof(GL_ENDRENDERING_SIG_NEW) - 1);
		if(!gRefFuncs.GL_EndRendering)
			SIG_NOT_FOUND("GL_EndRendering");

		gRefFuncs.EmitWaterPolys = (void (*)(msurface_t *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BuildLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BuildLightmaps - g_dwEngineBase), EMITWATERPOLYS_SIG_NEW, sizeof(EMITWATERPOLYS_SIG_NEW) - 1);
		if(!gRefFuncs.EmitWaterPolys)
			SIG_NOT_FOUND("EmitWaterPolys");

		gRefFuncs.R_DrawSkyChain = (void (*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.EmitWaterPolys, g_dwEngineSize - ((DWORD)gRefFuncs.EmitWaterPolys - g_dwEngineBase), R_DRAWSKYCHAIN_SIG_NEW, sizeof(R_DRAWSKYCHAIN_SIG_NEW) - 1);
		if(!gRefFuncs.R_DrawSkyChain)
			SIG_NOT_FOUND("R_DrawSkyChain");

		gRefFuncs.R_DrawSkyBox = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawSkyChain, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawSkyChain - g_dwEngineBase), R_DRAWSKYBOX_SIG_NEW, sizeof(R_DRAWSKYBOX_SIG_NEW) - 1);
		if(!gRefFuncs.R_DrawSkyBox)
			SIG_NOT_FOUND("R_DrawSkyBox");

		gRefFuncs.R_DrawEntitiesOnList = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWENTITIESONLIST_SIG_NEW, sizeof(R_DRAWENTITIESONLIST_SIG_NEW) - 1);
		if(!gRefFuncs.R_DrawEntitiesOnList)
			SIG_NOT_FOUND("R_DrawEntitiesOnList");

		gRefFuncs.R_SetFrustum = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawEntitiesOnList, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawEntitiesOnList - g_dwEngineBase), R_SETFRUSTUM_SIG_NEW, sizeof(R_SETFRUSTUM_SIG_NEW)-1);
		if(!gRefFuncs.R_SetFrustum)
			SIG_NOT_FOUND("R_SetFrustum");

		gRefFuncs.R_SetupGL = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_SetupFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_SetupFrame - g_dwEngineBase), R_SETUPGL_SIG_NEW, sizeof(R_SETUPGL_SIG_NEW)-1);
		if(!gRefFuncs.R_SetupGL)
			SIG_NOT_FOUND("R_SetupGL");

		gRefFuncs.R_DrawSequentialPoly = (void (*)(msurface_t *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWSEQUENTIALPOLY_SIG_NEW, sizeof(R_DRAWSEQUENTIALPOLY_SIG_NEW)-1);
		if(!gRefFuncs.R_DrawSequentialPoly)
			SIG_NOT_FOUND("R_DrawSequentialPoly");

		gRefFuncs.R_TextureAnimation = (texture_t *(*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_TEXTUREANIMATION_SIG_NEW, sizeof(R_TEXTUREANIMATION_SIG_NEW)-1);
		if(!gRefFuncs.R_TextureAnimation)
			SIG_NOT_FOUND("R_TextureAnimation");

		gRefFuncs.Draw_DecalTexture = (texture_t *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAW_DECALTEXTURE_SIG_NEW, sizeof(R_DRAW_DECALTEXTURE_SIG_NEW)-1);
		if(!gRefFuncs.Draw_DecalTexture)
			SIG_NOT_FOUND("Draw_DecalTexture");

		gRefFuncs.R_AllocObjects = (void (*)(int ))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_ALLOCOBJECTS_SIG_NEW, sizeof(R_ALLOCOBJECTS_SIG_NEW)-1);
		if(!gRefFuncs.R_AllocObjects)
			SIG_NOT_FOUND("R_AllocObjects");

		gRefFuncs.GlowBlend = (float (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_AllocObjects, g_dwEngineSize - ((DWORD)gRefFuncs.R_AllocObjects - g_dwEngineBase), GLOWBLEND_SIG_NEW, sizeof(GLOWBLEND_SIG_NEW)-1);
		if(!gRefFuncs.GlowBlend)
			SIG_NOT_FOUND("GlowBlend");

		gRefFuncs.R_DrawTEntitiesOnList = (void (*)(int health))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GlowBlend, g_dwEngineSize - ((DWORD)gRefFuncs.GlowBlend - g_dwEngineBase), R_DRAWTENTITIESONLIST_SIG_NEW, sizeof(R_DRAWTENTITIESONLIST_SIG_NEW) - 1);
		if(!gRefFuncs.R_DrawTEntitiesOnList)
			SIG_NOT_FOUND("R_DrawTEntitiesOnList");

		gRefFuncs.Draw_MiptexTexture = (void (*)(cachewad_t *, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, DRAW_MIPTEXTEXTURE_SIG_NEW, sizeof(DRAW_MIPTEXTEXTURE_SIG_NEW)-1);
		if(!gRefFuncs.Draw_MiptexTexture)
			SIG_NOT_FOUND("Draw_MiptexTexture");

		gRefFuncs.VID_UpdateWindowVars = (void (*)(RECT *prc, int x, int y))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VID_UPDATEWINDOWVARS_SIG_NEW, sizeof(VID_UPDATEWINDOWVARS_SIG_NEW)-1);
		if(!gRefFuncs.VID_UpdateWindowVars)
			SIG_NOT_FOUND("VID_UpdateWindowVars");

		//5953 above only
		gRefFuncs.FreeFBObjects = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EndRendering, g_dwEngineSize - ((DWORD)gRefFuncs.GL_EndRendering-g_dwEngineBase), FREEFBOBJECTS_SIG_NEW, sizeof(FREEFBOBJECTS_SIG_NEW)-1);
		if(!gRefFuncs.FreeFBObjects)
			SIG_NOT_FOUND("FreeFBObjects");

		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EndRendering, g_dwEngineSize - ((DWORD)gRefFuncs.GL_EndRendering-g_dwEngineBase), GL_SWAPBUFFER_SIG_NEW, sizeof(GL_SWAPBUFFER_SIG_NEW)-1);
		if(!addr)
			SIG_NOT_FOUND("GL_SwapBuffer");
		addr = *(DWORD *)(addr+2);
		gRefFuncs.GL_SwapBuffer = (void (**)(void))addr;

		//Studio Funcs

		gStudioFuncs.R_GLStudioDrawPoints = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_GLSTUDIODRAWPOINTS_SIG_NEW, sizeof(R_GLSTUDIODRAWPOINTS_SIG_NEW)-1);
		if(!gStudioFuncs.R_GLStudioDrawPoints)
			SIG_NOT_FOUND("R_GLStudioDrawPoints");

		gStudioFuncs.BuildGlowShellVerts = (void (*)(vec3_t *, auxvert_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, BUILDGLOWSHELLVERTS_SIG_NEW, sizeof(BUILDGLOWSHELLVERTS_SIG_NEW)-1);
		if(!gStudioFuncs.BuildGlowShellVerts)
			SIG_NOT_FOUND("BuildGlowShellVerts");

		gStudioFuncs.BuildNormalIndexTable = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, BUILDNORMALINDEXTABLE_SIG_NEW, sizeof(BUILDNORMALINDEXTABLE_SIG_NEW)-1);
		if(!gStudioFuncs.BuildNormalIndexTable)
			SIG_NOT_FOUND("BuildNormalIndexTable");

		gStudioFuncs.R_StudioRenderFinal = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_STUDIORENDERFINAL_SIG_NEW, sizeof(R_STUDIORENDERFINAL_SIG_NEW)-1);
		if(!gStudioFuncs.R_StudioRenderFinal)
			SIG_NOT_FOUND("R_StudioRenderFinal");

		gStudioFuncs.R_StudioLighting = (void (*)(float *, int, int, vec3_t))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_STUDIOLIGHTING_SIG_NEW, sizeof(R_STUDIOLIGHTING_SIG_NEW)-1);
		if(!gStudioFuncs.R_StudioLighting)
			SIG_NOT_FOUND("R_StudioLighting");

		gStudioFuncs.R_LightStrength = (void (*)(int , float *, float (*)[4]))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioLighting, g_dwEngineSize - ((DWORD)gStudioFuncs.R_StudioLighting-g_dwEngineBase), R_LIGHTSTRENGTH_SIG_NEW, sizeof(R_LIGHTSTRENGTH_SIG_NEW)-1);
		if(!gStudioFuncs.R_LightStrength)
			SIG_NOT_FOUND("R_LightStrength");

		gStudioFuncs.R_LightLambert = (void (*)(float (*)[4], float *, float *, float *))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LightStrength, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LightStrength-g_dwEngineBase), R_LIGHTLAMBERT_SIG_NEW, sizeof(R_LIGHTLAMBERT_SIG_NEW)-1);
		if(!gStudioFuncs.R_LightLambert)
			SIG_NOT_FOUND("R_LightLambert");

		gStudioFuncs.R_StudioChrome = (void (*)(int *, int , vec3_t ))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LightLambert, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LightLambert-g_dwEngineBase), R_STUDIOCHROME_SIG_NEW, sizeof(R_STUDIOCHROME_SIG_NEW)-1);
		if(!gStudioFuncs.R_StudioChrome)
			SIG_NOT_FOUND("R_StudioChrome");

		gStudioFuncs.R_LoadTextures = (studiohdr_t *(*)(model_t *))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioChrome, g_dwEngineSize - ((DWORD)gStudioFuncs.R_StudioChrome-g_dwEngineBase), R_LOADTEXTURES_SIG_NEW, sizeof(R_LOADTEXTURES_SIG_NEW)-1);
		if(!gStudioFuncs.R_LoadTextures)
			SIG_NOT_FOUND("R_LoadTextures");

		gStudioFuncs.R_StudioSetupSkin = (void (*)(studiohdr_t *, int))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LoadTextures, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LoadTextures-g_dwEngineBase), R_STUDIOSETUPSKIN_SIG_NEW, sizeof(R_STUDIOSETUPSKIN_SIG_NEW)-1);
		if(!gStudioFuncs.R_StudioSetupSkin)
			SIG_NOT_FOUND("R_StudioSetupSkin");

		//Sprite
		gRefFuncs.R_GetSpriteFrame = (mspriteframe_t *(*)(msprite_t *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_GETSPRITEFRAME_SIG_NEW, sizeof(R_GETSPRITEFRAME_SIG_NEW)-1);
		if(!gRefFuncs.R_GetSpriteFrame)
			SIG_NOT_FOUND("R_GetSpriteFrame");

		gRefFuncs.R_GetSpriteAxes = (void (*)(cl_entity_t *, int , float *, float *, float *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_GetSpriteFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_GetSpriteFrame - g_dwEngineBase), R_GETSPRITEAXES_SIG_NEW, sizeof(R_GETSPRITEAXES_SIG_NEW)-1);
		if(!gRefFuncs.R_GetSpriteFrame)
			SIG_NOT_FOUND("R_GetSpriteFrame");

		gRefFuncs.R_SpriteColor = (void (*)(mcolor24_t *, cl_entity_t *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_GetSpriteAxes, g_dwEngineSize - ((DWORD)gRefFuncs.R_GetSpriteAxes - g_dwEngineBase), R_SPRITECOLOR_SIG_NEW, sizeof(R_SPRITECOLOR_SIG_NEW)-1);
		if(!gRefFuncs.R_SpriteColor)
			SIG_NOT_FOUND("R_SpriteColor");

		gRefFuncs.R_DrawSpriteModel = (void (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWSRPITEMODEL_SIG_NEW, sizeof(R_DRAWSRPITEMODEL_SIG_NEW)-1);
		if(!gRefFuncs.R_DrawSpriteModel)
			SIG_NOT_FOUND("R_DrawSpriteModel");
	}
	else
	{
		gRefFuncs.LoadTGA = (int (*)(const char *, byte *, int , int *, int *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADTGA_SIG, sizeof(LOADTGA_SIG) - 1);
		if(!gRefFuncs.LoadTGA)
			SIG_NOT_FOUND("LoadTGA");

		gRefFuncs.GL_Bind = (void (*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, GL_BIND_SIG, sizeof(GL_BIND_SIG) - 1);
		if(!gRefFuncs.GL_Bind)
			SIG_NOT_FOUND("GL_Bind");

		gRefFuncs.GL_SelectTexture = (void (*)(GLenum))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_Bind, g_dwEngineSize - ((DWORD)gRefFuncs.GL_Bind - g_dwEngineBase), GL_SELECTTEXTURE_SIG, sizeof(GL_SELECTTEXTURE_SIG) - 1);
		if(!gRefFuncs.GL_SelectTexture)
			SIG_NOT_FOUND("GL_SelectTexture");

		gRefFuncs.GL_LoadTexture2 = (int (*)(char *, int, int, int, byte *, qboolean, int, byte *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_SelectTexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_SelectTexture - g_dwEngineBase), GL_LOADTEXTURE2_SIG, sizeof(GL_LOADTEXTURE2_SIG) - 1);
		if (!gRefFuncs.GL_LoadTexture2)//4554
			gRefFuncs.GL_LoadTexture2 = (int(*)(char *, int, int, int, byte *, qboolean, int, byte *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_SelectTexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_SelectTexture - g_dwEngineBase), GL_LOADTEXTURE2_SIG2, sizeof(GL_LOADTEXTURE2_SIG2) - 1);
		if(!gRefFuncs.GL_LoadTexture2)
			SIG_NOT_FOUND("GL_LoadTexture2");

		gRefFuncs.R_CullBox = (qboolean (*)(vec3_t, vec3_t))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, g_dwEngineSize - ((DWORD)gRefFuncs.GL_LoadTexture2 - g_dwEngineBase), R_CULLBOX_SIG, sizeof(R_CULLBOX_SIG) - 1);
		if(!gRefFuncs.R_CullBox)
			SIG_NOT_FOUND("R_CullBox");

		gRefFuncs.R_SetupFrame = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_CullBox, g_dwEngineSize - ((DWORD)gRefFuncs.R_CullBox - g_dwEngineBase), R_SETUPFRAME_SIG, sizeof(R_SETUPFRAME_SIG) - 1);
		if (!gRefFuncs.R_SetupFrame)//4554
			gRefFuncs.R_SetupFrame = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_CullBox, g_dwEngineSize - ((DWORD)gRefFuncs.R_CullBox - g_dwEngineBase), R_SETUPFRAME_SIG2, sizeof(R_SETUPFRAME_SIG2) - 1);
		if(!gRefFuncs.R_SetupFrame)
			SIG_NOT_FOUND("R_SetupFrame");

		gRefFuncs.R_Clear = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_SetupFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_SetupFrame - g_dwEngineBase), R_CLEAR_SIG, sizeof(R_CLEAR_SIG) - 1);
		if (!gRefFuncs.R_Clear)//4554
			gRefFuncs.R_Clear = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_SetupFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_SetupFrame - g_dwEngineBase), R_CLEAR_SIG2, sizeof(R_CLEAR_SIG2) - 1);
		if(!gRefFuncs.R_Clear)
			SIG_NOT_FOUND("R_Clear");

		gRefFuncs.R_RenderScene = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_Clear, g_dwEngineSize - ((DWORD)gRefFuncs.R_Clear - g_dwEngineBase), R_RENDERSCENE_SIG, sizeof(R_RENDERSCENE_SIG) - 1);
		if(!gRefFuncs.R_RenderScene)
			SIG_NOT_FOUND("R_RenderScene");

		gRefFuncs.R_RenderView = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderScene, g_dwEngineSize - ((DWORD)gRefFuncs.R_RenderScene - g_dwEngineBase), R_RENDERVIEW_SIG, sizeof(R_RENDERVIEW_SIG)-1);
		if(!gRefFuncs.R_RenderView)
			SIG_NOT_FOUND("R_RenderView");

		gRefFuncs.R_NewMap = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderView, g_dwEngineSize - ((DWORD)gRefFuncs.R_RenderView - g_dwEngineBase), R_NEWMAP_SIG, sizeof(R_NEWMAP_SIG) - 1);
		if(!gRefFuncs.R_NewMap)
			SIG_NOT_FOUND("R_NewMap");

		gRefFuncs.R_BuildLightMap = (void (*)(msurface_t *, byte *, int ))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_NewMap, g_dwEngineSize - ((DWORD)gRefFuncs.R_NewMap - g_dwEngineBase), R_BUILDLIGHTMAP_SIG, sizeof(R_BUILDLIGHTMAP_SIG)-1);
		if(!gRefFuncs.R_BuildLightMap)
			SIG_NOT_FOUND("R_BuildLightMap");

		gRefFuncs.GL_DisableMultitexture = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_BuildLightMap, g_dwEngineSize - ((DWORD)gRefFuncs.R_BuildLightMap - g_dwEngineBase), GL_DISABLEMULTITEXTURE_SIG, sizeof(GL_DISABLEMULTITEXTURE_SIG) - 1);
		if(!gRefFuncs.GL_DisableMultitexture)
			SIG_NOT_FOUND("GL_DisableMultitexture");

		gRefFuncs.GL_EnableMultitexture = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_DisableMultitexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_DisableMultitexture - g_dwEngineBase), GL_ENABLEMULTITEXTURE_SIG, sizeof(GL_ENABLEMULTITEXTURE_SIG) - 1);
		if(!gRefFuncs.GL_EnableMultitexture)
			SIG_NOT_FOUND("GL_EnableMultitexture");

		gRefFuncs.R_RenderDynamicLightmaps = (void (*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EnableMultitexture, g_dwEngineSize - ((DWORD)gRefFuncs.GL_EnableMultitexture - g_dwEngineBase), R_RENDERDYNAMICLIGHTMAPS_SIG, sizeof(R_RENDERDYNAMICLIGHTMAPS_SIG)-1);
		if(!gRefFuncs.R_RenderDynamicLightmaps)
			SIG_NOT_FOUND("R_RenderDynamicLightmaps");

		gRefFuncs.R_DrawBrushModel = (void (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderDynamicLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.R_RenderDynamicLightmaps - g_dwEngineBase), R_DRAWBRUSHMODEL_SIG, sizeof(R_DRAWBRUSHMODEL_SIG)-1);
		if(!gRefFuncs.R_DrawBrushModel)
			SIG_NOT_FOUND("R_DrawBrushModel");

		gRefFuncs.R_RecursiveWorldNode = (void (*)(mnode_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawBrushModel, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawBrushModel - g_dwEngineBase), R_RECURSIVEWORLDNODE_SIG, sizeof(R_RECURSIVEWORLDNODE_SIG)-1);
		if(!gRefFuncs.R_RecursiveWorldNode)
			SIG_NOT_FOUND("R_RecursiveWorldNode");

		gRefFuncs.R_DrawWorld = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RecursiveWorldNode, g_dwEngineSize - ((DWORD)gRefFuncs.R_RecursiveWorldNode - g_dwEngineBase), R_DRAWWORLD_SIG, sizeof(R_DRAWWORLD_SIG) - 1);
		if(!gRefFuncs.R_DrawWorld)
			SIG_NOT_FOUND("R_DrawWorld");

		gRefFuncs.R_MarkLeaves = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawWorld, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawWorld - g_dwEngineBase), R_MARKLEAVES_SIG, sizeof(R_MARKLEAVES_SIG) - 1);
		if(!gRefFuncs.R_MarkLeaves)
			SIG_NOT_FOUND("R_MarkLeaves");

		gRefFuncs.BuildSurfaceDisplayList = (void (*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_MarkLeaves, g_dwEngineSize - ((DWORD)gRefFuncs.R_MarkLeaves - g_dwEngineBase), BUILDSURFACEDISPLAYLIST_SIG, sizeof(BUILDSURFACEDISPLAYLIST_SIG) - 1);
		if (!gRefFuncs.BuildSurfaceDisplayList)//4554
			gRefFuncs.BuildSurfaceDisplayList = (void(*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_MarkLeaves, g_dwEngineSize - ((DWORD)gRefFuncs.R_MarkLeaves - g_dwEngineBase), BUILDSURFACEDISPLAYLIST_SIG2, sizeof(BUILDSURFACEDISPLAYLIST_SIG2) - 1);
		if(!gRefFuncs.BuildSurfaceDisplayList)
			SIG_NOT_FOUND("BuildSurfaceDisplayList");

		gRefFuncs.GL_BuildLightmaps = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.BuildSurfaceDisplayList, g_dwEngineSize - ((DWORD)gRefFuncs.R_RecursiveWorldNode - g_dwEngineBase), GL_BUILDLIGHTMAPS_SIG, sizeof(GL_BUILDLIGHTMAPS_SIG) - 1);
		if (!gRefFuncs.GL_BuildLightmaps)//4554
			gRefFuncs.GL_BuildLightmaps = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.BuildSurfaceDisplayList, g_dwEngineSize - ((DWORD)gRefFuncs.R_RecursiveWorldNode - g_dwEngineBase), GL_BUILDLIGHTMAPS_SIG2, sizeof(GL_BUILDLIGHTMAPS_SIG2) - 1);
		if(!gRefFuncs.GL_BuildLightmaps)
			SIG_NOT_FOUND("GL_BuildLightmaps");

		gRefFuncs.R_DecalMPoly = (void (*)(float *, texture_t *, msurface_t *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BuildLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BuildLightmaps - g_dwEngineBase), R_DECALMPOLY_SIG, sizeof(R_DECALMPOLY_SIG)-1);
		if(!gRefFuncs.R_DecalMPoly)
			SIG_NOT_FOUND("R_DecalMPoly");

		gRefFuncs.R_DrawDecals = (void (*)(qboolean))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DecalMPoly, g_dwEngineSize - ((DWORD)gRefFuncs.R_DecalMPoly - g_dwEngineBase), R_DRAWDECALS_SIG, sizeof(R_DRAWDECALS_SIG)-1);
		if(!gRefFuncs.R_DrawDecals)
			SIG_NOT_FOUND("R_DrawDecals");

		gRefFuncs.GL_BeginRendering = (void (*)(int *, int *, int *, int *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BuildLightmaps, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BuildLightmaps - g_dwEngineBase), GL_BEGINRENDERING_SIG, sizeof(GL_BEGINRENDERING_SIG) - 1);
		if(!gRefFuncs.GL_BeginRendering)
			SIG_NOT_FOUND("GL_BeginRendering");

		gRefFuncs.GL_EndRendering = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_BeginRendering, g_dwEngineSize - ((DWORD)gRefFuncs.GL_BeginRendering - g_dwEngineBase), GL_ENDRENDERING_SIG, sizeof(GL_ENDRENDERING_SIG) - 1);
		if(!gRefFuncs.GL_EndRendering)
			SIG_NOT_FOUND("GL_EndRendering");

		gRefFuncs.EmitWaterPolys = (void (*)(msurface_t *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EndRendering, g_dwEngineSize - ((DWORD)gRefFuncs.GL_EndRendering - g_dwEngineBase), EMITWATERPOLYS_SIG, sizeof(EMITWATERPOLYS_SIG) - 1);
		if(!gRefFuncs.EmitWaterPolys)
			SIG_NOT_FOUND("EmitWaterPolys");

		gRefFuncs.R_DrawSkyChain = (void (*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.EmitWaterPolys, g_dwEngineSize - ((DWORD)gRefFuncs.EmitWaterPolys - g_dwEngineBase), R_DRAWSKYCHAIN_SIG, sizeof(R_DRAWSKYCHAIN_SIG) - 1);
		if(!gRefFuncs.R_DrawSkyChain)
			SIG_NOT_FOUND("R_DrawSkyChain");

		gRefFuncs.R_DrawSkyBox = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawSkyChain, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawSkyChain - g_dwEngineBase), R_DRAWSKYBOX_SIG, sizeof(R_DRAWSKYBOX_SIG) - 1);
		if(!gRefFuncs.R_DrawSkyBox)
			gRefFuncs.R_DrawSkyBox = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawSkyChain, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawSkyChain - g_dwEngineBase), R_DRAWSKYBOX_SIG2, sizeof(R_DRAWSKYBOX_SIG2) - 1);
		if(!gRefFuncs.R_DrawSkyBox)
			SIG_NOT_FOUND("R_DrawSkyBox");

		gRefFuncs.R_DrawEntitiesOnList = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWENTITIESONLIST_SIG, sizeof(R_DRAWENTITIESONLIST_SIG) - 1);
		if(!gRefFuncs.R_DrawEntitiesOnList)//4554
			gRefFuncs.R_DrawEntitiesOnList = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWENTITIESONLIST_SIG2, sizeof(R_DRAWENTITIESONLIST_SIG2) - 1);
		if(!gRefFuncs.R_DrawEntitiesOnList)
			SIG_NOT_FOUND("R_DrawEntitiesOnList");

		gRefFuncs.R_SetFrustum = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawEntitiesOnList, g_dwEngineSize - ((DWORD)gRefFuncs.R_DrawEntitiesOnList - g_dwEngineBase), R_SETFRUSTUM_SIG, sizeof(R_SETFRUSTUM_SIG)-1);
		if(!gRefFuncs.R_SetFrustum)
			SIG_NOT_FOUND("R_SetFrustum");

		gRefFuncs.R_SetupGL = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_SetupFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_SetupFrame - g_dwEngineBase), R_SETUPGL_SIG, sizeof(R_SETUPGL_SIG)-1);
		if(!gRefFuncs.R_SetupGL)
			SIG_NOT_FOUND("R_SetupGL");

		gRefFuncs.R_DrawSequentialPoly = (void (*)(msurface_t *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWSEQUENTIALPOLY_SIG, sizeof(R_DRAWSEQUENTIALPOLY_SIG)-1);
		if(!gRefFuncs.R_DrawSequentialPoly)
			SIG_NOT_FOUND("R_DrawSequentialPoly");

		gRefFuncs.R_TextureAnimation = (texture_t *(*)(msurface_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_TEXTUREANIMATION_SIG, sizeof(R_TEXTUREANIMATION_SIG)-1);
		if(!gRefFuncs.R_TextureAnimation)
			SIG_NOT_FOUND("R_TextureAnimation");
		
		gRefFuncs.Draw_DecalTexture = (texture_t *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAW_DECALTEXTURE_SIG, sizeof(R_DRAW_DECALTEXTURE_SIG)-1);
		if(!gRefFuncs.Draw_DecalTexture)//4554
			gRefFuncs.Draw_DecalTexture = (texture_t *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAW_DECALTEXTURE_SIG2, sizeof(R_DRAW_DECALTEXTURE_SIG2)-1);
		if(!gRefFuncs.Draw_DecalTexture)
			SIG_NOT_FOUND("Draw_DecalTexture");

		gRefFuncs.R_AllocObjects = (void (*)(int ))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_ALLOCOBJECTS_SIG, sizeof(R_ALLOCOBJECTS_SIG)-1);
		if(!gRefFuncs.R_AllocObjects)
			SIG_NOT_FOUND("R_AllocObjects");

		gRefFuncs.GlowBlend = (float (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_AllocObjects, g_dwEngineSize - ((DWORD)gRefFuncs.R_AllocObjects - g_dwEngineBase), GLOWBLEND_SIG, sizeof(GLOWBLEND_SIG)-1);
		if(!gRefFuncs.GlowBlend)
			gRefFuncs.GlowBlend = (float (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_AllocObjects, g_dwEngineSize - ((DWORD)gRefFuncs.R_AllocObjects - g_dwEngineBase), GLOWBLEND_SIG2, sizeof(GLOWBLEND_SIG2)-1);
		if(!gRefFuncs.GlowBlend)
			SIG_NOT_FOUND("GlowBlend");

		gRefFuncs.R_DrawTEntitiesOnList = (void (*)(int health))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GlowBlend, g_dwEngineSize - ((DWORD)gRefFuncs.GlowBlend - g_dwEngineBase), R_DRAWTENTITIESONLIST_SIG, sizeof(R_DRAWTENTITIESONLIST_SIG) - 1);
		if(!gRefFuncs.R_DrawTEntitiesOnList)
			SIG_NOT_FOUND("R_DrawTEntitiesOnList");

		gRefFuncs.Draw_MiptexTexture = (void (*)(cachewad_t *, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, DRAW_MIPTEXTEXTURE_SIG, sizeof(DRAW_MIPTEXTEXTURE_SIG)-1);
		if(!gRefFuncs.Draw_MiptexTexture)//4554
			gRefFuncs.Draw_MiptexTexture = (void (*)(cachewad_t *, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, DRAW_MIPTEXTEXTURE_SIG2, sizeof(DRAW_MIPTEXTEXTURE_SIG2)-1);
		if(!gRefFuncs.Draw_MiptexTexture)
			SIG_NOT_FOUND("Draw_MiptexTexture");

		gRefFuncs.VID_UpdateWindowVars = (void (*)(RECT *prc, int x, int y))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VID_UPDATEWINDOWVARS_SIG, sizeof(VID_UPDATEWINDOWVARS_SIG)-1);
		if(!gRefFuncs.VID_UpdateWindowVars)
			SIG_NOT_FOUND("VID_UpdateWindowVars");

		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EndRendering, g_dwEngineSize - ((DWORD)gRefFuncs.GL_EndRendering-g_dwEngineBase), GL_SWAPBUFFER_SIG, sizeof(GL_SWAPBUFFER_SIG)-1);
		if(!addr)
			SIG_NOT_FOUND("GL_SwapBuffer");
		addr = *(DWORD *)(addr+2);
		gRefFuncs.GL_SwapBuffer = (void (**)(void))addr;

		//Studio Funcs
		gStudioFuncs.R_GLStudioDrawPoints = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_GLSTUDIODRAWPOINTS_SIG, sizeof(R_GLSTUDIODRAWPOINTS_SIG)-1);
		if(!gStudioFuncs.R_GLStudioDrawPoints)
			gStudioFuncs.R_GLStudioDrawPoints = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_GLSTUDIODRAWPOINTS_SIG2, sizeof(R_GLSTUDIODRAWPOINTS_SIG2)-1);
		if(!gStudioFuncs.R_GLStudioDrawPoints)
			SIG_NOT_FOUND("R_GLStudioDrawPoints");

		gStudioFuncs.BuildGlowShellVerts = (void (*)(vec3_t *, auxvert_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, BUILDGLOWSHELLVERTS_SIG, sizeof(BUILDGLOWSHELLVERTS_SIG)-1);
		if(!gStudioFuncs.BuildGlowShellVerts)
			SIG_NOT_FOUND("BuildGlowShellVerts");

		gStudioFuncs.BuildNormalIndexTable = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, BUILDNORMALINDEXTABLE_SIG, sizeof(BUILDNORMALINDEXTABLE_SIG)-1);
		if(!gStudioFuncs.BuildNormalIndexTable)
			SIG_NOT_FOUND("BuildNormalIndexTable");

		gStudioFuncs.R_StudioRenderFinal = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_STUDIORENDERFINAL_SIG, sizeof(R_STUDIORENDERFINAL_SIG)-1);
		if(!gStudioFuncs.R_StudioRenderFinal)
			SIG_NOT_FOUND("R_StudioRenderFinal");

		gStudioFuncs.R_StudioLighting = (void (*)(float *, int, int, vec3_t))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_STUDIOLIGHTING_SIG, sizeof(R_STUDIOLIGHTING_SIG)-1);
		if(!gStudioFuncs.R_StudioLighting)
			SIG_NOT_FOUND("R_StudioLighting");

		gStudioFuncs.R_LightStrength = (void (*)(int , float *, float (*)[4]))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioLighting, g_dwEngineSize - ((DWORD)gStudioFuncs.R_StudioLighting-g_dwEngineBase), R_LIGHTSTRENGTH_SIG, sizeof(R_LIGHTSTRENGTH_SIG)-1);
		if(!gStudioFuncs.R_LightStrength)
			gStudioFuncs.R_LightStrength = (void (*)(int , float *, float (*)[4]))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioLighting, g_dwEngineSize - ((DWORD)gStudioFuncs.R_StudioLighting-g_dwEngineBase), R_LIGHTSTRENGTH_SIG2, sizeof(R_LIGHTSTRENGTH_SIG2)-1);
		if(!gStudioFuncs.R_LightStrength)
			SIG_NOT_FOUND("R_LightStrength");

		gStudioFuncs.R_LightLambert = (void (*)(float (*)[4], float *, float *, float *))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LightStrength, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LightStrength-g_dwEngineBase), R_LIGHTLAMBERT_SIG, sizeof(R_LIGHTLAMBERT_SIG)-1);
		if(!gStudioFuncs.R_LightLambert)
			gStudioFuncs.R_LightLambert = (void (*)(float (*)[4], float *, float *, float *))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LightStrength, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LightStrength-g_dwEngineBase), R_LIGHTLAMBERT_SIG2, sizeof(R_LIGHTLAMBERT_SIG2)-1);
		if(!gStudioFuncs.R_LightLambert)
			SIG_NOT_FOUND("R_LightLambert");

		gStudioFuncs.R_StudioChrome = (void (*)(int *, int , vec3_t ))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LightLambert, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LightLambert-g_dwEngineBase), R_STUDIOCHROME_SIG, sizeof(R_STUDIOCHROME_SIG)-1);
		if(!gStudioFuncs.R_StudioChrome)
			SIG_NOT_FOUND("R_StudioChrome");

		gStudioFuncs.R_LoadTextures = (studiohdr_t *(*)(model_t *))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioChrome, g_dwEngineSize - ((DWORD)gStudioFuncs.R_StudioChrome-g_dwEngineBase), R_LOADTEXTURES_SIG, sizeof(R_LOADTEXTURES_SIG)-1);
		if(!gStudioFuncs.R_LoadTextures)
			SIG_NOT_FOUND("R_LoadTextures");

		gStudioFuncs.R_StudioSetupSkin = (void (*)(studiohdr_t *, int))g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_LoadTextures, g_dwEngineSize - ((DWORD)gStudioFuncs.R_LoadTextures-g_dwEngineBase), R_STUDIOSETUPSKIN_SIG, sizeof(R_STUDIOSETUPSKIN_SIG)-1);
		if(!gStudioFuncs.R_StudioSetupSkin)
			SIG_NOT_FOUND("R_StudioSetupSkin");

		//Sprite
		gRefFuncs.R_GetSpriteFrame = (mspriteframe_t *(*)(msprite_t *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_GETSPRITEFRAME_SIG, sizeof(R_GETSPRITEFRAME_SIG)-1);
		if(!gRefFuncs.R_GetSpriteFrame)
			SIG_NOT_FOUND("R_GetSpriteFrame");

		gRefFuncs.R_GetSpriteAxes = (void (*)(cl_entity_t *, int , float *, float *, float *))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_GetSpriteFrame, g_dwEngineSize - ((DWORD)gRefFuncs.R_GetSpriteFrame - g_dwEngineBase), R_GETSPRITEAXES_SIG, sizeof(R_GETSPRITEAXES_SIG)-1);
		if(!gRefFuncs.R_GetSpriteFrame)
			SIG_NOT_FOUND("R_GetSpriteFrame");

		gRefFuncs.R_SpriteColor = (void (*)(mcolor24_t *, cl_entity_t *, int))g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_GetSpriteAxes, g_dwEngineSize - ((DWORD)gRefFuncs.R_GetSpriteAxes - g_dwEngineBase), R_SPRITECOLOR_SIG, sizeof(R_SPRITECOLOR_SIG)-1);
		if(!gRefFuncs.R_SpriteColor)
			SIG_NOT_FOUND("R_SpriteColor");

		gRefFuncs.R_DrawSpriteModel = (void (*)(cl_entity_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWSRPITEMODEL_SIG, sizeof(R_DRAWSRPITEMODEL_SIG)-1);
		if(!gRefFuncs.R_DrawSpriteModel)
			SIG_NOT_FOUND("R_DrawSpriteModel");
	}

	//Common Vars

	//R_DrawSkyBox
	//mov     eax, skytexorder[esi*4]
	//mov     ecx, gSkyTexNumber[eax*4]
	//push    ecx
	//call    GL_Bind
#define GSKYTEXNUMBER_SIG "\x8B\x04\xB5\x2A\x2A\x2A\x2A\x8B\x0C\x85\x2A\x2A\x2A\x2A\x51\xE8"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawSkyBox, 0x300, GSKYTEXNUMBER_SIG, sizeof(GSKYTEXNUMBER_SIG)-1 );
	if(!addr)
		SIG_NOT_FOUND("gSkyTexNumber");
	gSkyTexNumber = *(int **)(addr + 10);

	//GL_Bind
	//mov     ecx, currenttexture
	//push    esi
#define CURRENTTEXTURE_SIG "\x8B\x0D\x2A\x2A\x2A\x2A\x56"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_Bind, 0x50, CURRENTTEXTURE_SIG, sizeof(CURRENTTEXTURE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("currenttexture");
	currenttexture = *(int **)(addr + 2);

	//BuildSurfaceDisplayList
#define HUNK_ALLOC_SIG "\xE8"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.BuildSurfaceDisplayList, g_dwEngineSize - ((DWORD)gRefFuncs.BuildSurfaceDisplayList - g_dwEngineBase), HUNK_ALLOC_SIG, sizeof(HUNK_ALLOC_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("Hunk_Alloc");
	gRefFuncs.Hunk_Alloc = (void *(*)(size_t))GetCallAddress(addr);

	//R_SetupFrame
#define R_ORIGIN_SIG "\x40\x68\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xA1"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_SetupFrame, 0x300, R_ORIGIN_SIG, sizeof(R_ORIGIN_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("r_origin");
	addr += 2;
	vup = (vec_t *)(*(DWORD *)addr);
	addr += 10;
	addr += 5;
	vright = (vec_t *)(*(DWORD *)addr);
	addr += 5;
	vpn = (vec_t *)(*(DWORD *)addr);
	addr += 5;
	r_refdef = (refdef_t *)(*(DWORD *)addr - offsetof(refdef_t, viewangles));
	addr += 5;
	if (*(BYTE *)addr == 0x0D)//5953
		addr += 1;
	r_origin = (vec_t *)(*(DWORD *)addr);

	//R_RenderScene
#define GBUSERFOG_SIG "\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x85\xC0\x74\x2A\xE8"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderScene, 0x200, GBUSERFOG_SIG, sizeof(GBUSERFOG_SIG) - 1);
	if(!addr)
		SIG_NOT_FOUND("g_bUserFogOn");
	g_bUserFogOn = *(int **)(addr+6);

	//R_RecursiveWorldNode
#define MODELORG_SIG "\xD9\x05\x2A\x2A\x2A\x2A\x49\x74\x2A\xD8\x48\x08"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RecursiveWorldNode, 0x200, MODELORG_SIG, sizeof(MODELORG_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("modelorg");
	modelorg = *(float **)(addr+2);

	//mov     eax, [edi+4]
	//mov     ecx, r_visframecount
#define R_VISFRAMECOUNT_SIG "\x8B\x47\x04\x8B\x0D\x2A\x2A\x2A\x2A\x3B\xC1"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RecursiveWorldNode, 0x50, R_VISFRAMECOUNT_SIG, sizeof(R_VISFRAMECOUNT_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("r_visframecount");
	r_visframecount = *(int **)(addr + 5);

	//mov     edx, r_framecount
#define R_FRAMECOUNT_SIG "\x8B\x15\x2A\x2A\x2A\x2A\x8B\x30\x83\xC0\x04\x49\x89\x16"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr, 0x200, R_FRAMECOUNT_SIG, sizeof(R_FRAMECOUNT_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("r_framecount");
	r_framecount = *(int **)(addr + 2);

	//mov     ecx, r_viewleaf
	//mov     eax, r_oldviewleaf
#define R_VIEWLEAF_SIG "\x8B\x0D\x2A\x2A\x2A\x2A\xA1"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_MarkLeaves, 0x50, R_VIEWLEAF_SIG, sizeof(R_VIEWLEAF_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("r_viewleaf");
	r_viewleaf = *(mleaf_t ***)(addr + 2);
	r_oldviewleaf = *(mleaf_t ***)(addr + 7);

	//mov     eax, gDecalSurfCount
#define GDECALSURFS_SIG "\xA1\x2A\x2A\x2A\x2A\x89\x1C\x85\x2A\x2A\x2A\x2A\x40\x3D\xF4\x01\x00\x00"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawSequentialPoly, 0x500, GDECALSURFS_SIG, sizeof(GDECALSURFS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("gDecalSurfs");
	gDecalSurfCount = *(int **)(addr + 1);
	gDecalSurfs = *(msurface_t ***)(addr + 8);

	//R_DrawEntitiesOnList
	//mov     eax, CL_UPDATE_MASK
#define CL_PARSECOUNT_SIG "\xA1\x2A\x2A\x2A\x2A\x8B\x0D\x2A\x2A\x2A\x2A\x23\xC1\x8B\x12"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_DrawEntitiesOnList, 0x200, CL_PARSECOUNT_SIG, sizeof(CL_PARSECOUNT_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("cl_parsecount");
	cl_parsecount = *(int **)(addr + 7);

#define CL_FRAMES_SIG "\x8D\x84\xCA"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr, 0x50, CL_FRAMES_SIG, sizeof(CL_FRAMES_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("cl.frames");
	cl_frames = *(frame_t **)(addr + 3);

	//R_StudioLighting
	//call    __ftol
	//shl     eax, 2
	//mov     ecx, offset lightgammatable
#define LIGHTGAMMATABLE_SIG "\xE8\x2A\x2A\x2A\x2A\xC1\xE0\x02\xB9\x2A\x2A\x2A\x2A\x8B"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioLighting, 0x300, LIGHTGAMMATABLE_SIG, sizeof(LIGHTGAMMATABLE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("lightgammatable");
	lightgammatable = *(int (**)[1024])(addr+9);

	//R_StudioChrome
	//mov     eax, chromeage[esi*4]
	//cmp     eax, ecx
#define CHROMEAGE_SIG "\x8B\x04\xB5\x2A\x2A\x2A\x2A\x3B\xC1"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.R_StudioChrome, 0x50, CHROMEAGE_SIG, sizeof(CHROMEAGE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("chromeage");
	chromeage = *(int (**)[MAXSTUDIOBONES])((DWORD)addr + 3);

	//R_AllocObjects
	//3266
	//add     esp, 10h
	//mov     maxTransObjs, edi
	//6153
	//push    ebp
	//mov     ebp, esp
	//mov     eax, transObjects
#define MAXTRANSOBJS_SIG "\x83\xC4\x10\x89\x3D"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_AllocObjects, 0x80, MAXTRANSOBJS_SIG, sizeof(MAXTRANSOBJS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("maxTransObjs");
	maxTransObjs = *(int **)(addr + 5);

	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_AllocObjects, 0x20, "\xA1", 1);
	if(!addr)
		SIG_NOT_FOUND("transObjects");
	transObjects = *(transObjRef ***)(addr + 1);

#define MEM_MALLOC_SIG "\x56\xE8\x2A\x2A\x2A\x2A\x56\x6A\x00"
	//push    esi
	//call Mem_Malloc
	//push    esi
	//push    0
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_AllocObjects, 0x80, MEM_MALLOC_SIG, sizeof(MEM_MALLOC_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("Mem_Malloc");
	gRefFuncs.Mem_Malloc = (void *(*)(size_t))GetCallAddress(addr+1);

#define R_ENTORIGIN_SIG "\xD9\x05"
	//fld     r_entorigin
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GlowBlend, 0x50, R_ENTORIGIN_SIG, sizeof(R_ENTORIGIN_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("Mem_Malloc");
	r_entorigin = *(float **)(addr + 2);

	//BuildNormalIndexTable
	//mov     edx, psubmodel
	//push    edi
#define PSUBMODEL_SIG "\x8B\x15\x2A\x2A\x2A\x2A\x57"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.BuildNormalIndexTable, 0x50, PSUBMODEL_SIG, sizeof(PSUBMODEL_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("psubmodel");
	psubmodel = *(mstudiomodel_t ***)((DWORD)addr + 2);

	//mov     edi, offset g_NormalIndex
	//rep stosd
#define G_NORMALINDEX_SIG "\xBF\x2A\x2A\x2A\x2A\xF3\xAB"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gStudioFuncs.BuildNormalIndexTable, 0x50, G_NORMALINDEX_SIG, sizeof(G_NORMALINDEX_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("g_NormalIndex");
	g_NormalIndex = *(int (**)[MAXSTUDIOVERTS])(addr + 1);

	//R_RenderDynamicLightmaps
	//mov     ecx, c_brush_polys
	//push    ebx
#define C_BRUSH_POLYS_SIG "\x8B\x0D\x2A\x2A\x2A\x2A\x53"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderDynamicLightmaps, 0x30, C_BRUSH_POLYS_SIG, sizeof(C_BRUSH_POLYS_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("c_brush_polys");
	c_brush_polys = *(int **)(addr + 2);

	//and     eax, 0FFh
	//mov     eax, d_lightstylevalue[eax*4]
#define D_LIGHTSTYLEVALUE_SIG "\x25\xFF\x00\x00\x00\x8B\x04\x85"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.R_RenderDynamicLightmaps, 0x200, D_LIGHTSTYLEVALUE_SIG, sizeof(D_LIGHTSTYLEVALUE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("d_lightstylevalue");
	d_lightstylevalue = *(int **)(addr + 8);

#define WINDOW_RECT_SIG "\x89\x0D"
		//89 0D 20 42 7B 02		mov     window_rect.left, ecx
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.VID_UpdateWindowVars, 0x50, WINDOW_RECT_SIG, sizeof(WINDOW_RECT_SIG)-1);
		if(!addr)
			SIG_NOT_FOUND("window_rect");
		window_rect = *(RECT **)(addr+2);

	//Engine special

	if(g_dwEngineBuildnum >= 5953)
	{
#define GLTEXTURES_SIG_NEW "\x33\xF6\xBF\x2A\x2A\x2A\x2A\x3B\x35\x2A\x2A\x2A\x2A\x7D"
		//Search in GL_LoadTexture2
		//xor     esi, esi
		//mov     edi, offset gltextures
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x100, GLTEXTURES_SIG_NEW, sizeof(GLTEXTURES_SIG_NEW)-1);
		if(!addr)
			SIG_NOT_FOUND("gltextures");
		gltextures = *(gltexture_t **)(addr+3);
		numgltextures = *(int **)(addr+9);

#define GHOSTSPAWNCOUNT_SIG_NEW "\x66\x8B\x15\x2A\x2A\x2A\x2A\x66\x89\x57\x04"
		//mov     dx, word ptr gHostSpawnCount
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x200, GHOSTSPAWNCOUNT_SIG_NEW, sizeof(GHOSTSPAWNCOUNT_SIG_NEW)-1);
		if(!addr)
			SIG_NOT_FOUND("gHostSpawnCount");
		gHostSpawnCount = *(int **)(addr+3);

		//5953 use qglGenTexture instead of currenttexid

		//6153 have to use windowvideoaspect
#define WINDOW_VIDEO_ASPECT_SIG_NEW "\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xA1"
		//A3 F8 4D E4 01		mov     videowindowaspect, eax
		//A3 F4 4D E4 01		mov     windowvideoaspect, eax
		//A1 F0 4D E4 01		mov     eax, bNoStretchAspect
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EndRendering, 0x200, WINDOW_VIDEO_ASPECT_SIG_NEW, sizeof(WINDOW_VIDEO_ASPECT_SIG_NEW)-1);
		if(!addr)
			SIG_NOT_FOUND("windowvideoaspect");
		videowindowaspect = *(float **)(addr+1);
		windowvideoaspect = *(float **)(addr+6);
	}
	else
	{
		//Search in GL_LoadTexture2
		//3266
		//BF 2A 2A 2A 2A    mov edi, offset gltextures
		//3B 2D 2A 2A 2A 2A cmp     ebp, numgltextures
#define GLTEXTURES_SIG "\xBF\x2A\x2A\x2A\x2A\x3B\x2D"
#define GLTEXTURES_SIG2 "\xBF\x2A\x2A\x2A\x2A\xBD\x2A\x2A\x2A\x2A\x3B\x35"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x100, GLTEXTURES_SIG, sizeof(GLTEXTURES_SIG)-1);
		if(addr)
		{
			gltextures = *(gltexture_t **)(addr+1);
			numgltextures = *(int **)(addr+7);
		}
		else
		{
			//4554
			//BF 2A 2A 2A 2A    mov edi, offset gltextures
			//BD 2A 2A 2A 2A    mov ebp, offset gltextures.identifier
			//3B 35 2A 2A 2A 2A cmp esi, numgltextures
			addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x100, GLTEXTURES_SIG2, sizeof(GLTEXTURES_SIG2)-1);
			if(!addr)
				SIG_NOT_FOUND("gltextures");
			gltextures = *(gltexture_t **)(addr+1);
			numgltextures = *(int **)(addr+12);
		}
#define GHOSTSPAWNCOUNT_SIG "\x66\xA1\x2A\x2A\x2A\x2A\x66\x89\x47\x04"
#define GHOSTSPAWNCOUNT_SIG2 "\x66\x8B\x0D\x2A\x2A\x2A\x2A\x66\x89\x4F\x04"
		//3266 1D3F8C7 mov ax, word ptr gHostSpawnCount
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x200, GHOSTSPAWNCOUNT_SIG, sizeof(GHOSTSPAWNCOUNT_SIG)-1);
		if(addr)
			gHostSpawnCount = *(int **)(addr+2);
		else
		{
			//4554 1D4A75A mov cx, word ptr gHostSpawnCount
			addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x200, GHOSTSPAWNCOUNT_SIG2, sizeof(GHOSTSPAWNCOUNT_SIG2)-1);
			if(!addr)
				SIG_NOT_FOUND("gHostSpawnCount");
			gHostSpawnCount = *(int **)(addr+3);
		}
		//Below 5953 only
#define CURRENTTEXID_SIG "\x83\xC4\x04\xA1\x2A\x2A\x2A\x2A\x89\x06\x40\xA3"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x300, CURRENTTEXID_SIG, sizeof(CURRENTTEXID_SIG)-1);
		//add esp, 4
		//mov eax, currenttexid
		if(!addr)
			SIG_NOT_FOUND("currenttexid");
		currenttexid = *(int **)(addr+4);

		//< 5953 don't have videowindowaspect & windowvideoaspect so we create one;
		
		videowindowaspect = &videowindowaspect_old;
		windowvideoaspect = &windowvideoaspect_old;
	}
}

void R_InstallHook(void)
{
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_BeginRendering, GL_BeginRendering, (void *&)gRefFuncs.GL_BeginRendering);
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_EndRendering, GL_EndRendering, (void *&)gRefFuncs.GL_EndRendering);

	g_pMetaHookAPI->InlineHook(gRefFuncs.R_RenderView, R_RenderView, (void *&)gRefFuncs.R_RenderView);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_RenderScene, R_RenderScene, (void *&)gRefFuncs.R_RenderScene);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_NewMap, R_NewMap, (void *&)gRefFuncs.R_NewMap);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_DrawWorld, R_DrawWorld, (void *&)gRefFuncs.R_DrawWorld);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_SetupFrame, R_SetupFrame, (void *&)gRefFuncs.R_SetupFrame);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_DrawSkyChain, R_DrawSkyChain, (void *&)gRefFuncs.R_DrawSkyChain);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_CullBox, R_CullBox, (void *&)gRefFuncs.R_CullBox);
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_Bind, GL_Bind, (void *&)gRefFuncs.GL_Bind);
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_BuildLightmaps, GL_BuildLightmaps, (void *&)gRefFuncs.GL_BuildLightmaps);
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_LoadTexture2, GL_LoadTexture2, (void *&)gRefFuncs.GL_LoadTexture2);
	g_pMetaHookAPI->InlineHook(gRefFuncs.EmitWaterPolys, EmitWaterPolys, (void *&)gRefFuncs.EmitWaterPolys);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_DrawSequentialPoly, R_DrawSequentialPoly, (void *&)gRefFuncs.R_DrawSequentialPoly);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_AllocObjects, R_AllocObjects, (void *&)gRefFuncs.R_AllocObjects);
	g_pMetaHookAPI->InlineHook(gStudioFuncs.R_StudioRenderFinal, R_StudioRenderFinal, (void *&)gStudioFuncs.R_StudioRenderFinal);
	g_pMetaHookAPI->InlineHook(gRefFuncs.Draw_DecalTexture, Draw_DecalTexture, (void *&)gRefFuncs.Draw_DecalTexture);
	g_pMetaHookAPI->InlineHook(gRefFuncs.Draw_MiptexTexture, Draw_MiptexTexture, (void *&)gRefFuncs.Draw_MiptexTexture);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_DecalMPoly, R_DecalMPoly,  (void *&)gRefFuncs.R_DecalMPoly);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_DrawEntitiesOnList, R_DrawEntitiesOnList, (void *&)gRefFuncs.R_DrawEntitiesOnList);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_DrawTEntitiesOnList, R_DrawTEntitiesOnList, (void *&)gRefFuncs.R_DrawTEntitiesOnList);
}

void Lightmaps_Patch(void)
{
	max_lightmaps = 128;
	const char *s_num;
	if(g_pInterface->CommandLine->CheckParm("-lightmaps", &s_num))
	{
		if(s_num && s_num[0] && s_num[0] >= '0' && s_num[0] <= '9')
		{
			int i_num = atoi(s_num);
			max_lightmaps = min(max(i_num, 64), 512);
		}
	}

	lightmap_textures = new int[max_lightmaps];
	memset(lightmap_textures, 0, sizeof(int) * max_lightmaps);

	lightmap_rectchange = new glRect_t[max_lightmaps];
	memset(lightmap_rectchange, 0, sizeof(glRect_t) * max_lightmaps);

	lightmap_modified = new int[max_lightmaps];
	memset(lightmap_modified, 0, sizeof(int) * max_lightmaps);

	lightmap_alloc = new int[max_lightmaps * BLOCK_WIDTH];
	memset(lightmap_alloc, 0, sizeof(int) * max_lightmaps * BLOCK_WIDTH);

	lightmaps_new = new byte[max_lightmaps * BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES];
	memset(lightmaps_new, 0, sizeof(byte) * max_lightmaps * BLOCK_WIDTH * BLOCK_HEIGHT * LIGHTMAP_BYTES);
}

void CL_VisEdicts_Patch(void)
{
	cl_maxvisedicts = 512;
	const char *s_num;
	if(g_pInterface->CommandLine->CheckParm("-visedicts", &s_num))
	{
		if(s_num && s_num[0] && s_num[0] >= '0' && s_num[0] <= '9')
		{
			int i_num = atoi(s_num);
			cl_maxvisedicts = min(max(i_num, 512), 4096);
		}
	}

	//search in CL_CreateVisibleEntity
	DWORD addr_cl_numvisedicts = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_pMetaSave->pEngineFuncs->CL_CreateVisibleEntity, 0x50, "\x8B\x0D\x2A\x2A\x2A\x2A\x81\xF9\x00\x02\x00\x00", sizeof("\x8B\x0D\x2A\x2A\x2A\x2A\x81\xF9\x00\x02\x00\x00")-1);
	if(!addr_cl_numvisedicts)
		SIG_NOT_FOUND("cl_numvisedicts");
	cl_numvisedicts = *(int **)(addr_cl_numvisedicts+2);

	//search cl_visedicts
	DWORD addr_cl_visedicts = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_pMetaSave->pEngineFuncs->CL_CreateVisibleEntity, 0x100, "\xB8\x01\x00\x00\x00\x89\x14\x8D\x2A\x2A\x2A\x2A\x41", sizeof("\xB8\x01\x00\x00\x00\x89\x14\x8D\x2A\x2A\x2A\x2A\x41")-1);
	if(!addr_cl_visedicts)
		SIG_NOT_FOUND("cl_visedicts");
	cl_visedicts_old = *(cl_entity_t ***)(addr_cl_visedicts + 8);

	//no need to patch
	if(cl_maxvisedicts <= 512)
	{
		cl_visedicts_new = cl_visedicts_old;
		return;
	}

	//alloc here
	cl_visedicts_new = (cl_entity_t **)malloc(cl_maxvisedicts*sizeof(cl_entity_t *));
	if(!cl_visedicts_new)
	{
		Sys_ErrorEx("CL_VisEdicts_Patch: out of memory");
		return;
	}

	//replace this first
	g_pMetaHookAPI->WriteDWORD((void *)(cl_visedicts_old+8), (DWORD)cl_visedicts_new);
	g_pMetaSave->pEngineFuncs->Con_Printf("CL_VisEdicts_Patch: CL_CreateVisibleEntity patched.\n");

	//replace all "cmp cl_numvisedicts, 200h"
	char sig[32];
	int count;
	DWORD addr, size, end;

	end = g_dwEngineBase+g_dwEngineSize;

	count = 0;
	memcpy(sig, "\x81\x3D\x2A\x2A\x2A\x2A\x00\x02\x00\x00", 10);
	*(DWORD *)(&sig[2]) = (DWORD)cl_numvisedicts;
	addr = g_dwEngineBase;
	while(1)
	{
		size = end-addr;
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr, size, sig, 10);
		if(addr == 0)
			break;
		g_pMetaHookAPI->WriteDWORD((void *)(addr+6), (DWORD)cl_maxvisedicts);
		count ++;
		addr += 10;
	}
	g_pMetaSave->pEngineFuncs->Con_DPrintf("CL_VisEdicts_Patch: %d of \"cmp cl_numvisedicts, 200h\" patched.\n", count);

	//patch ClientDLL_AddEntity
	memcpy(sig, "\x89\x34\x85\x2A\x2A\x2A\x2A\x40\xA3\x2A\x2A\x2A\x2A", 13);
	*(DWORD *)(&sig[3]) = (DWORD)cl_visedicts_old;
	*(DWORD *)(&sig[9]) = (DWORD)cl_numvisedicts;
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, sig, 13);
	if(!addr)
		SIG_NOT_FOUND("ClientDLL_AddEntity->cl_visedicts");
	g_pMetaHookAPI->WriteDWORD((void *)(addr+3), (DWORD)cl_visedicts_new);
	g_pMetaSave->pEngineFuncs->Con_DPrintf("CL_VisEdicts_Patch: ClientDLL_AddEntity patched.\n");

	//patch CL_MoveAiments
	memcpy(sig, "\x8B\x04\x95\x2A\x2A\x2A\x2A\x8B\x88\x44\x03\x00\x00", 13);
	*(DWORD *)(&sig[3]) = (DWORD)cl_visedicts_old;
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, sig, 13);
	if(!addr)
		SIG_NOT_FOUND("CL_MoveAiments->cl_visedicts");
	g_pMetaHookAPI->WriteDWORD((void *)(addr+3), (DWORD)cl_visedicts_new);
	g_pMetaSave->pEngineFuncs->Con_DPrintf("CL_VisEdicts_Patch: CL_MoveAiments patched.\n");

	//we have already rewrite the R_DrawEntitiesOnList code
	//no need to patch R_DrawEntitiesOnList

	/*count = 0;
	memcpy(sig, "\x8B\x2A\x2A\x2A\x2A\x2A\x2A", 7);
	*(DWORD *)(&sig[3]) = (DWORD)cl_visedicts_old;
	addr = (DWORD)gRefFuncs.R_DrawEntitiesOnList;
	size = 0x350;
	end = size + addr;
	do
	{
		size = end - addr;
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)addr, size, sig, 7);
		if(!addr)
			break;
		g_pMetaHookAPI->WriteDWORD((void *)(addr+3), (DWORD)cl_visedicts_new);
		count ++;
		addr += 7;
	}while(1);*/
	
	//g_pMetaSave->pEngineFuncs->Con_DPrintf("CL_VisEdicts_Patch: %d replaced in R_DrawEntitiesOnList\n", count);
}
