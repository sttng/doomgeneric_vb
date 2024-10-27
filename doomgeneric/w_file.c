//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include <stdio.h>

#include "config.h"

#include "doomtype.h"
#include "m_argv.h"

#include "w_file.h"

#include "z_zone.h"
#include "doom1_wad.h"
#include <string.h>

extern wad_file_class_t stdc_wad_file;

/*
#ifdef _WIN32
extern wad_file_class_t win32_wad_file;
#endif
*/

#ifdef HAVE_MMAP
extern wad_file_class_t posix_wad_file;
#endif 


extern wad_file_class_t rom_wad_file;
wad_file_t *OpenRomFile(char *path)
{
    wad_file_t *result;
    result = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);
    result->file_class = &rom_wad_file;
    result->mapped = DOOM1_WAD;
    result->length = sizeof(DOOM1_WAD);
    return result;
};

void CloseRomFile(wad_file_t *file) {}

size_t ReadRomFile(wad_file_t *file, unsigned int offset,
                   void *buffer, size_t buffer_len) {
  memcpy(buffer, &DOOM1_WAD[offset], buffer_len);
  return buffer_len;
}

wad_file_class_t rom_wad_file = {
  .OpenFile = OpenRomFile,
  .CloseFile = CloseRomFile,
  .Read = ReadRomFile,
};




wad_file_t *W_OpenFile(char *path)
{
    wad_file_t *result = OpenRomFile(path);
#if 0
    int i;

    //!
    // Use the OS's virtual memory subsystem to map WAD files
    // directly into memory.
    //

    if (!M_CheckParm("-mmap"))
    {
        return stdc_wad_file.OpenFile(path);
    }

    // Try all classes in order until we find one that works

    result = NULL;

    for (i = 0; i < arrlen(wad_file_classes); ++i)
    {
        result = wad_file_classes[i]->OpenFile(path);

        if (result != NULL)
        {
            break;
        }
    }
#endif

    return result;
}

void W_CloseFile(wad_file_t *wad)
{
    wad->file_class->CloseFile(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset,
              void *buffer, size_t buffer_len)
{
    return wad->file_class->Read(wad, offset, buffer, buffer_len);
}

