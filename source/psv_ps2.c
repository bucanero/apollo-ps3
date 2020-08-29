#include <stdio.h>

#include "util.h"
#include "lzari.h"
#include "ps2mc.h"

#define  MAX_HEADER_MAGIC   "Ps2PowerSave"

int psv_resign(const char *src_psv);
void get_psv_filename(char* psvName, const char* path, const char* dirName);

static void printMAXHeader(const maxHeader_t *header)
{
    if(!header)
        return;

    LOG("Magic            : %.*s", (int)sizeof(header->magic), header->magic);
    LOG("CRC              : %08X", ES32(header->crc));
    LOG("dirName          : %.*s", (int)sizeof(header->dirName), header->dirName);
    LOG("iconSysName      : %.*s", (int)sizeof(header->iconSysName), header->iconSysName);
    LOG("compressedSize   : %u", ES32(header->compressedSize));
    LOG("numFiles         : %u", ES32(header->numFiles));
    LOG("decompressedSize : %u", ES32(header->decompressedSize));
}

static int roundUp(int i, int j)
{
    return (i + j - 1) / j * j;
}

int isMAXFile(const char *path)
{
    if(!path)
        return 0;

    FILE *f = fopen(path, "rb");
    if(!f)
        return 0;

    // Verify file size
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(len < sizeof(maxHeader_t))
    {
        fclose(f);
        return 0;
    }

    // Verify header
    maxHeader_t header;
    fread(&header, 1, sizeof(maxHeader_t), f);
    fclose(f);

    printMAXHeader(&header);

    return (ES32(header.compressedSize) > 0) &&
           (ES32(header.decompressedSize) > 0) &&
           (ES32(header.numFiles) > 0) &&
           strncmp(header.magic, MAX_HEADER_MAGIC, sizeof(header.magic)) == 0 &&
           strlen(header.dirName) > 0 &&
           strlen(header.iconSysName) > 0;
}

void setMcDateTime(sceMcStDateTime* mc, struct tm *ftm)
{
    mc->Resv2 = 0;
    mc->Sec = ftm->tm_sec;
    mc->Min = ftm->tm_min;
    mc->Hour = ftm->tm_hour;
    mc->Day = ftm->tm_mday;
    mc->Month = ftm->tm_mon + 1;
    mc->Year = ES16(ftm->tm_year + 1900);
}

int ps2_max2psv(const char *save, const char* psv_path)
{
    if (!isMAXFile(save))
        return 0;
    
    FILE *f = fopen(save, "rb");
    if(!f)
        return 0;

    struct stat st;
    struct tm *ftm;
    sceMcStDateTime fctime;
    sceMcStDateTime fmtime;

    fstat(fileno(f), &st);

    ftm = gmtime(&st.st_ctime);
    setMcDateTime(&fctime, ftm);

    ftm = gmtime(&st.st_mtime);
    setMcDateTime(&fmtime, ftm);

    maxHeader_t header;
    fread(&header, 1, sizeof(maxHeader_t), f);
    header.compressedSize = ES32(header.compressedSize);
    header.decompressedSize = ES32(header.decompressedSize);
    header.numFiles = ES32(header.numFiles);

    char dirName[sizeof(header.dirName) + 1];
    char psvName[256];

    memcpy(dirName, header.dirName, sizeof(header.dirName));
    dirName[32] = '\0';

    get_psv_filename(psvName, psv_path, dirName);
    FILE* psv = fopen(psvName, "wb");

    if (!psv)
        return 0;

    // Get compressed file entries
    u8 *compressed = malloc(header.compressedSize);

    fseek(f, sizeof(maxHeader_t) - 4, SEEK_SET); // Seek to beginning of LZARI stream.
    u32 ret = fread(compressed, 1, header.compressedSize, f);
    if(ret != header.compressedSize)
    {
        LOG("Compressed size: actual=%d, expected=%d\n", ret, header.compressedSize);
        free(compressed);
        return 0;
    }

    fclose(f);
    u8 *decompressed = malloc(header.decompressedSize);

    ret = unlzari(compressed, header.compressedSize, decompressed, header.decompressedSize);
    // As with other save formats, decompressedSize isn't acccurate.
    if(ret == 0)
    {
        LOG("Decompression failed.\n");
        free(decompressed);
        free(compressed);
        return 0;
    }

    free(compressed);

    int i;
    u32 offset = 0;
    u32 dataPos = 0;
    maxEntry_t *entry;
    
    psv_header_t ph;
    ps2_header_t ps2h;
    ps2_IconSys_t *ps2sys = NULL;
    ps2_MainDirInfo_t ps2md;
    
    memset(&ph, 0, sizeof(psv_header_t));
    memset(&ps2h, 0, sizeof(ps2_header_t));
    memset(&ps2md, 0, sizeof(ps2_MainDirInfo_t));
    
    ps2h.numberOfFiles = ES32(header.numFiles);

    ps2md.attribute = ES32(0x00008427);
    ps2md.numberOfFilesInDir = ES32(header.numFiles+2);
    memcpy(&ps2md.created, &fctime, sizeof(sceMcStDateTime));
    memcpy(&ps2md.modified, &fmtime, sizeof(sceMcStDateTime));
    memcpy(&ps2md.filename, &dirName, sizeof(ps2md.filename));
    
    ph.headerSize = ES32(0x0000002C);
    ph.saveType = ES32(0x00000002);
    memcpy(&ph.magic, "\0VSP", 4);
    memcpy(&ph.salt, "www.bucanero.com.ar", 20);

    fwrite(&ph, sizeof(psv_header_t), 1, psv);

    LOG("\nSave contents:\n");

    // Find the icon.sys (need to know the icons names)
    for(i = 0, offset = 0; i < header.numFiles; i++)
    {
        entry = (maxEntry_t*) &decompressed[offset];
        entry->length = ES32(entry->length);
        offset += sizeof(maxEntry_t);

        if(strcmp(entry->name, "icon.sys") == 0)
            ps2sys = (ps2_IconSys_t*) &decompressed[offset];

        offset = roundUp(offset + entry->length + 8, 16) - 8;
        ps2h.displaySize += entry->length;

        LOG(" %8d bytes  : %s", entry->length, entry->name);
    }

    LOG(" %8d Total bytes", ps2h.displaySize);
    ps2h.displaySize = ES32(ps2h.displaySize);

    if (!ps2sys)
        return 0;

    // Calculate the start offset for the file's data
    dataPos = sizeof(psv_header_t) + sizeof(ps2_header_t) + sizeof(ps2_MainDirInfo_t) + sizeof(ps2_FileInfo_t)*header.numFiles;

    ps2_FileInfo_t *ps2fi = malloc(sizeof(ps2_FileInfo_t)*header.numFiles);

    // Build the PS2 FileInfo entries
    for(i = 0, offset = 0; i < header.numFiles; i++)
    {
        entry = (maxEntry_t*) &decompressed[offset];
        offset += sizeof(maxEntry_t);

        ps2fi[i].attribute = ES32(0x00008497);
        ps2fi[i].positionInFile = ES32(dataPos);
        ps2fi[i].filesize = ES32(entry->length);
        memcpy(&ps2fi[i].created, &fctime, sizeof(sceMcStDateTime));
        memcpy(&ps2fi[i].modified, &fmtime, sizeof(sceMcStDateTime));
        memcpy(&ps2fi[i].filename, &entry->name, sizeof(ps2fi[i].filename));

        dataPos += entry->length;

        if (strcmp(ps2fi[i].filename, ps2sys->IconName) == 0)
        {
            ps2h.icon1Size = ps2fi[i].filesize;
            ps2h.icon1Pos = ps2fi[i].positionInFile;
        }

        if (strcmp(ps2fi[i].filename, ps2sys->copyIconName) == 0)
        {
            ps2h.icon2Size = ps2fi[i].filesize;
            ps2h.icon2Pos = ps2fi[i].positionInFile;
        }

        if (strcmp(ps2fi[i].filename, ps2sys->deleteIconName) == 0)
        {
            ps2h.icon3Size = ps2fi[i].filesize;
            ps2h.icon3Pos = ps2fi[i].positionInFile;
        }

        if(strcmp(ps2fi[i].filename, "icon.sys") == 0)
        {
            ps2h.sysSize = ps2fi[i].filesize;
            ps2h.sysPos = ps2fi[i].positionInFile;
        }

        offset = roundUp(offset + entry->length + 8, 16) - 8;
    }

    fwrite(&ps2h, sizeof(ps2_header_t), 1, psv);
    fwrite(&ps2md, sizeof(ps2_MainDirInfo_t), 1, psv);
    fwrite(ps2fi, sizeof(ps2_FileInfo_t), header.numFiles, psv);

    free(ps2fi);
    
    // Write the file's data
    for(i = 0, offset = 0; i < header.numFiles; i++)
    {
        entry = (maxEntry_t*) &decompressed[offset];
        offset += sizeof(maxEntry_t);

        fwrite(&decompressed[offset], 1, entry->length, psv);
 
        offset = roundUp(offset + entry->length + 8, 16) - 8;
    }

    fclose(psv);
    free(decompressed);

    return psv_resign(psvName);
}

int ps2_psu2psv(const char *save, const char* psv_path)
{
    u32 dataPos = 0;
    FILE *psuFile, *psvFile;
    int numFiles, next, i;
    char dstName[256];
    u8 *data;
    ps2_McFsEntry entry;
    
    psuFile = fopen(save, "rb");
    if(!psuFile)
        return 0;
    
    // Read main directory entry
    fread(&entry, 1, sizeof(ps2_McFsEntry), psuFile);
    numFiles = ES32(entry.length) - 2;

    get_psv_filename(dstName, psv_path, entry.name);
    psvFile = fopen(dstName, "wb");
    
    if(!psvFile)
    {
        fclose(psuFile);
        return 0;
    }

    psv_header_t ph;
    ps2_header_t ps2h;
    ps2_IconSys_t ps2sys;
    ps2_MainDirInfo_t ps2md;
    
    memset(&ph, 0, sizeof(psv_header_t));
    memset(&ps2h, 0, sizeof(ps2_header_t));
    memset(&ps2md, 0, sizeof(ps2_MainDirInfo_t));
    
    ps2h.numberOfFiles = ES32(numFiles);

    ps2md.attribute = entry.mode;
    ps2md.numberOfFilesInDir = entry.length;
    memcpy(&ps2md.created, &entry.created, sizeof(sceMcStDateTime));
    memcpy(&ps2md.modified, &entry.modified, sizeof(sceMcStDateTime));
    memcpy(&ps2md.filename, &entry.name, sizeof(ps2md.filename));
    
    ph.headerSize = ES32(0x0000002C);
    ph.saveType = ES32(0x00000002);
    memcpy(&ph.magic, "\0VSP", 4);
    memcpy(&ph.salt, "www.bucanero.com.ar", 20);

    fwrite(&ph, sizeof(psv_header_t), 1, psvFile);

    // Skip "." and ".."
    fseek(psuFile, sizeof(ps2_McFsEntry)*2, SEEK_CUR);

    // Find the icon.sys (need to know the icons names)
    for(i = 0; i < numFiles; i++)
    {
        fread(&entry, 1, sizeof(ps2_McFsEntry), psuFile);
        entry.length = ES32(entry.length);

        if(strcmp(entry.name, "icon.sys") == 0)
            fread(&ps2sys, 1, sizeof(ps2_IconSys_t), psuFile);
        else
            fseek(psuFile, entry.length, SEEK_CUR);

        ps2h.displaySize += entry.length;

        LOG(" %8d bytes  : %s", entry.length, entry.name);

        next = 1024 - (entry.length % 1024);
        if(next < 1024)
            fseek(psuFile, next, SEEK_CUR);
    }

    LOG(" %8d Total bytes", ps2h.displaySize);
    ps2h.displaySize = ES32(ps2h.displaySize);

    // Skip "." and ".."
    fseek(psuFile, sizeof(ps2_McFsEntry)*3, SEEK_SET);

    // Calculate the start offset for the file's data
    dataPos = sizeof(psv_header_t) + sizeof(ps2_header_t) + sizeof(ps2_MainDirInfo_t) + sizeof(ps2_FileInfo_t)*numFiles;

    ps2_FileInfo_t *ps2fi = malloc(sizeof(ps2_FileInfo_t)*numFiles);

    // Build the PS2 FileInfo entries
    for(i = 0; i < numFiles; i++)
    {
        fread(&entry, 1, sizeof(ps2_McFsEntry), psuFile);

        ps2fi[i].attribute = entry.mode;
        ps2fi[i].positionInFile = ES32(dataPos);
        ps2fi[i].filesize = entry.length;
        memcpy(&ps2fi[i].created, &entry.created, sizeof(sceMcStDateTime));
        memcpy(&ps2fi[i].modified, &entry.modified, sizeof(sceMcStDateTime));
        memcpy(&ps2fi[i].filename, &entry.name, sizeof(ps2fi[i].filename));
        
        entry.length = ES32(entry.length);
        dataPos += entry.length;
        fseek(psuFile, entry.length, SEEK_CUR);
        
        if (strcmp(ps2fi[i].filename, ps2sys.IconName) == 0)
        {
            ps2h.icon1Size = ps2fi[i].filesize;
            ps2h.icon1Pos = ps2fi[i].positionInFile;
        }

        if (strcmp(ps2fi[i].filename, ps2sys.copyIconName) == 0)
        {
            ps2h.icon2Size = ps2fi[i].filesize;
            ps2h.icon2Pos = ps2fi[i].positionInFile;
        }

        if (strcmp(ps2fi[i].filename, ps2sys.deleteIconName) == 0)
        {
            ps2h.icon3Size = ps2fi[i].filesize;
            ps2h.icon3Pos = ps2fi[i].positionInFile;
        }

        if(strcmp(ps2fi[i].filename, "icon.sys") == 0)
        {
            ps2h.sysSize = ps2fi[i].filesize;
            ps2h.sysPos = ps2fi[i].positionInFile;
        }

        next = 1024 - (entry.length % 1024);
        if(next < 1024)
            fseek(psuFile, next, SEEK_CUR);
    }

    fwrite(&ps2h, sizeof(ps2_header_t), 1, psvFile);
    fwrite(&ps2md, sizeof(ps2_MainDirInfo_t), 1, psvFile);
    fwrite(ps2fi, sizeof(ps2_FileInfo_t), numFiles, psvFile);

    free(ps2fi);

    // Skip "." and ".."
    fseek(psuFile, sizeof(ps2_McFsEntry)*3, SEEK_SET);
    
    // Copy each file entry
    for(i = 0; i < numFiles; i++)
    {
        fread(&entry, 1, sizeof(ps2_McFsEntry), psuFile);
        entry.length = ES32(entry.length);

        data = malloc(entry.length);
        fread(data, 1, entry.length, psuFile);
        fwrite(data, 1, entry.length, psvFile);

        free(data);
        
        next = 1024 - (entry.length % 1024);
        if(next < 1024)
            fseek(psuFile, next, SEEK_CUR);
    }

    fclose(psvFile);
    fclose(psuFile);
    
    return psv_resign(dstName);
}
