/*
    Copyright 2004-2019 Luigi Auriemma

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl-2.0.txt
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <zlib.h>
#include <dirent.h>


#ifdef APOLLO_ENABLE_LOGGING
#include <dbglogger.h>
#define LOG dbglogger_log
#else
#define LOG(...)
#endif


typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;


// use FREE instead of free
#define FREE(X)         if(X) { \
                            free(X); \
                            X = NULL; \
                        }
#define PATH_DELIMITERS     "\\/"
#define PATHSLASH   '/'


#define VER             "0.4.1"
#define INSZ            0x800   // the amount of bytes we want to decompress each time
#define OUTSZ           0x10000 // the buffer used for decompressing the data
#define FBUFFSZ         0x40000 // this buffer is used for reading, faster
#define SHOWX           0x7ff   // AND to show the current scanned offset each SHOWX offsets

enum {
    ZIPDOSCAN2,
    ZIPDOSCAN,
    ZIPDODUMP,
    ZIPDODUMP2,
    ZIPDOFILE,
    ZIPDOERROR
};

#define Z_INIT_ERROR    -1000
#define Z_END_ERROR     -1001
#define Z_RESET_ERROR   -1002

#define MAXZIPLEN(n) ((n)+(((n)/1000)+1)+12)

#define PRId            PRId64
#define PRIu            PRIu64
#define PRIx            "08"PRIx64 //"016"PRIx64

#define g_minzip        32



int offzip(char *file_input, char *file_output, u64 file_offset, int zipdo, FILE **fdo);
char *mystrrchrs(char *str, char *chrs);
char *get_filename(char *fname);
int make_dir(char *folder);
int check_is_dir(char *fname);
int buffread(FILE *fd, u8 *buff, int size);
void buffseek(FILE *fd, u64 off, int mode);
void buffinc(int increase);
int zip_search(FILE *fd);
int unzip_all(FILE *fd, FILE **fdo, const char* fpath, int zipdo);
int unzip(FILE *fd, FILE **fdo, u64 *inlen, u64 *outlen, int zipdo, char *dumpname);
int zlib_err(int err);
FILE *save_file(char *fname);
int myfwrite(u8 *buff, int size, FILE *fd);
void FCLOSE(FILE **fd);


z_stream    z;
u64     g_total_zsize   = 0,
        g_total_size    = 0,
        g_offset        = 0,
        g_filebuffoff   = 0,
        g_filebuffsz    = 0,
        g_last_offset   = 0;
int     g_zipwbits      = 15;
char    *g_basename     = NULL;
u8      *g_in           = NULL,
        *g_out          = NULL,
        *g_filebuff     = NULL;


int offzip_util(const char *f_input, const char *output_dir, const char *basename, u64 file_offset, int wbits) {
    FILE    *fdo            = NULL;

    LOG("Offzip "VER"\n"
        "by Luigi Auriemma\n"
        "e-mail: aluigi@autistici.org\n"
        "web:    aluigi.org");

/*
        LOG("\n"
            "Usage: %s [options] <input/dir> [output/dir] [offset]\n"
            "\n"
            "Options:\n"
            "-s       scan for one compressed data in the input from the specified offset\n"
            "-S       as above but continues the scan, just like -a without extraction\n"
            "-a       extracts all the compressed data found in the input file into the\n"
            "         specified output folder, each output file is identified by the offset\n"
            "         of where the data was located\n"
            "-A       as above but without decompressing the data, just dumped \"as-is\"\n"
            "-1       related to -a/-A, generates one unique output file instead of many\n"
            "-m SIZE  minimum compressed size to check for valid data. default is %d, use a\n"
            "         higher value to reduce the false positives or a smaller one (eg 16) to\n"
            "         see very small compressed data too\n"
            "-z NUM   this option sets the windowBits value:\n"
            "         -z  15 = zlib data (default) zlib is header+deflate+crc\n"
            "         -z -15 = deflate data (many false positives, used in ZIP archives)\n"
            "-q       quiet, all the verbose error messages will be suppressed (-Q for more)\n"
            "-R       do not remove the invalid uncompressed files generated with -a and -A\n"
            "-x       visualization of hexadecimal numbers\n"
            "-L FILE  dump the list of \"0xoffset zsize size\" into the specified FILE\n"
            "-D FD    use a dictionary from file FD\n"
            "-d N     hex dump of N bytes before and after the compressed stream\n"
            "-c N     experimental guessing of files splitted in chunks where N is the max\n"
            "         uncompressed size of the chunks, note that the non-compressed chunks\n"
            "         can be recognized only if the chunks are sequential, requires -a\n"
            "-o       overwrite existent files without asking\n"
            "-r       reimport mode that works EXACTLY like in QuickBMS\n"
            "\n"
            "Note: Offset is a decimal number or a hex number if you use the 0x prefix, for\n"
            "      example: 1234 or 0x4d2\n"
            "      The displayed zlib info are CM, CINFO, FCHECK, FDICT, FLEVEL, ADLER32.\n"
            "\n"
            "Quick examples:\n"
            "  scan zlib        offzip -S input.dat\n"
            "  scan offset      offzip -S input.dat 0 0x12345678\n"
            "  cool scan zlib   offzip -S -x -Q input.dat\n"
            "  extract zlib     offzip -a input.dat c:\\output\n"
            "  extract deflate  offzip -a -z -15 -Q input.dat c:\\output\n"
            "  reimport zlib    offzip -a -r file.dat c:\\input\n"
            "\n", argv[0], g_minzip);
*/

    g_zipwbits = wbits;
    g_basename = (char*)basename;

    if(!g_in)       g_in       = calloc(INSZ,    1);
    if(!g_out)      g_out      = calloc(OUTSZ,   1);
    if(!g_filebuff) g_filebuff = calloc(FBUFFSZ, 1);
    if(!g_in || !g_out || !g_filebuff)
    {
        LOG("Error: unable to create buffers");
        return 0;
    }

    char* file_input  = strdup(f_input);
    char* file_output = strdup(output_dir);

    int ret = offzip(file_input, file_output, file_offset, ZIPDODUMP, &fdo);

    FCLOSE(&fdo);
    FREE(g_in);
    FREE(g_out);
    FREE(g_filebuff);
    FREE(file_input);
    FREE(file_output);
    return (ret > 0);
}


int offzip(char *file_input, char *file_output, u64 file_offset, int zipdo, FILE **fdo) {
    FILE    *fdo_dummy = NULL; // totally useless, just for testing
    if(!fdo) fdo = &fdo_dummy;
    FILE    *fd     = NULL;
    u64     inlen,
            outlen;
    int     files, ret = 0;

    g_total_zsize   = 0;
    g_total_size    = 0;
    g_offset        = 0;
    g_filebuffoff   = 0;
    g_filebuffsz    = 0;
    g_last_offset   = 0;

    LOG("- open input file:    %s", file_input);
    fd = fopen(file_input, "rb");
    if(!fd)
    {
        LOG("Error: can't open file");
        return 0;
    }

//    if(g_minzip > INSZ) g_minzip = INSZ;
//    if(g_minzip < 1)    g_minzip = 1;

    if((zipdo == ZIPDODUMP) || (zipdo == ZIPDODUMP2)) {
        if(file_output && file_output[0]) {
            LOG("- dump to directory:  %s", file_output);
//            if(chdir(file_output) < 0) {
//                return std_err("Error: can't open folder");
//            }
        }
    }

    LOG("- zip data to check:  %d bytes", g_minzip);
    LOG("- zip windowBits:     %d", g_zipwbits);

    g_offset = file_offset;  // do not skip, needed for buffseek
    LOG("- seek offset:        0x%"PRIx"  (%"PRIu")", g_offset, g_offset);
    buffseek(fd, g_offset, SEEK_SET);

    memset(&z, 0, sizeof(z));
    if(inflateInit2(&z, g_zipwbits) != Z_OK) 
        return zlib_err(Z_INIT_ERROR);

    if(zipdo == ZIPDOFILE) {
        char *add_folder = NULL;
        if(file_output && check_is_dir(file_output)) {
            if(file_output[0]) {
                chdir(file_output); // ... ignore the check
            }
            file_output = NULL;
            add_folder = NULL;
        }
        if(!file_output || !file_output[0]) {
            char *p, *ext;
            file_output = malloc(strlen(file_input) + (add_folder ? (strlen(add_folder)+1) : 0) + 64 + 1);
            p = strrchr(file_input, '\\');
            if(!p) p = strrchr(file_input, '/');
            if(!p) p = file_input;
            else   p++;
            ext = strrchr(p, '.');
            if(!ext) ext = p + strlen(p);
            file_output[0] = 0;
            if(add_folder) {
                sprintf(file_output + strlen(file_output), "%s", add_folder);
                make_dir(file_output);
                sprintf(file_output + strlen(file_output), "%c", PATHSLASH);
            }
            sprintf(file_output + strlen(file_output), "%.*s_%s", (int)(ext - p), p, "unpack");
            if(ext[0]) strcat(file_output, ext);
        }
        LOG("- open output file:   %s", file_output);
        if(!*fdo) *fdo = save_file(file_output);
        unzip(fd, fdo, &inlen, &outlen, zipdo, NULL);

        LOG("- %"PRIu" bytes read (zipped)", inlen);
        LOG("- %"PRIu" bytes unzipped", outlen);

        ret = (outlen > 0);
    } else {
        LOG("+------------+-----+----------------------------+----------------------+");
        LOG("| hex_offset | ... | zip -> unzip size / offset | spaces before | info |");
        LOG("+------------+-----+----------------------------+----------------------+");

        files = unzip_all(fd, fdo, file_output, zipdo);
        if(files) {
            //if(g_offset - g_last_offset) LOG("  0x%08x spaces from the last compressed stream\n", g_offset - g_last_offset);
            LOG("- %u valid compressed streams found", files);
            LOG("- 0x%"PRIx" -> 0x%"PRIx" bytes covering the %"PRId"%% of the file", g_total_zsize, g_total_size,
                ((u64)((u64)g_total_zsize * (u64)100) / (u64)ftell(fd)));
            
            ret = files;
        } else {
            LOG("- no valid full zip data found");
            ret = 0;
        }
    }

    FCLOSE(fdo);
    FCLOSE(&fd);
    inflateEnd(&z);
    if(*fdo && (fdo == &fdo_dummy)) FCLOSE(fdo);
    return ret;
}


char *mystrrchrs(char *str, char *chrs) {
    char    *p,
            *ret = NULL;

    if(str && chrs) {
        for(p = str + strlen(str) - 1; p >= str; p--) {
            if(strchr(chrs, *p)) return(p);
        }
    }
    return ret;
}


char *get_filename(char *fname) {
    char    *p;

    if(fname) {
        p = mystrrchrs(fname, PATH_DELIMITERS);
        if(p) return(p + 1);
    }
    return(fname);
}


int make_dir(char *folder) {
    return mkdir(folder, 0755);
}


int check_is_dir(char *fname) {
    struct stat xstat;
    if(!fname || !fname[0]) return 1;
    if(!strcmp(fname, ".")) return 1;
    if(!strcmp(fname, "..")) return 1;
    if(stat(fname, &xstat) < 0) return 0;
    if(!S_ISDIR(xstat.st_mode)) return 0;
    return 1;
}


// these buffering functions are just specific for this usage

int buffread(FILE *fd, u8 *buff, int size) {
    int     len,
            rest,
            ret;

    if(size > FBUFFSZ)
        return(0); // ???

    rest = g_filebuffsz - g_filebuffoff;

    ret = size;
    if(rest < size) {
        ret = size - rest;
        memmove(g_filebuff, g_filebuff + g_filebuffoff, rest);
        len = fread(g_filebuff + rest, 1, FBUFFSZ - rest, fd);
        g_filebuffoff = 0;
        g_filebuffsz  = rest + len;
        if(len < ret) {
            ret = rest + len;
        } else {
            ret = size;
        }
    }

    memcpy(buff, g_filebuff + g_filebuffoff, ret);
    return ret;
}


void buffseek(FILE *fd, u64 off, int mode) {
    if(fseek(fd, off, mode) < 0)
    {
        LOG("Error: buffseek");
        return;
    }
    g_filebuffoff = 0;
    g_filebuffsz  = 0;
    g_offset      = ftell(fd);
}


void buffinc(int increase) {
    g_filebuffoff += increase;
    g_offset      += increase;
}


int zip_search(FILE *fd) {
    int     len,
            zerr,
            ret;

    for(ret = - 1; (len = buffread(fd, g_in, g_minzip)) >= g_minzip; buffinc(1)) {
        z.next_in   = g_in;
        z.avail_in  = len;
        z.next_out  = g_out;
        z.avail_out = OUTSZ;

        inflateReset(&z);
        zerr = inflate(&z, Z_SYNC_FLUSH);

        if(zerr == Z_OK) {  // do not use Z_STREAM_END here! gives only troubles!!!
            LOG("Zip found at 0x%"PRIx" offset", g_offset);

            ret = 0;
            break;
        }

        if(!(g_offset & SHOWX))
            LOG("Scanned 0x%"PRIx" offset", g_offset);
    }
    return ret;
}


int unzip_all(FILE *fd, FILE **fdo, const char* fpath, int zipdo) {
    FILE    *fdo_dummy = NULL; // totally useless, just for testing
    if(!fdo) fdo = &fdo_dummy;
    u64     backup_offset,
            start_offset;
    u64     inlen,
            outlen;
    u32     crc;
    int     zipres,
            extracted;
    char    filename[256]   = "";
    u8      zlib_header[2],
            *tmp_buff       = NULL;

    extracted = 0;
    zipres    = -1;

    while(!zip_search(fd)) {
        LOG("  0x%"PRIx" ", g_offset);
        start_offset = g_offset;

        switch(zipdo) {
            case ZIPDOSCAN2: {
                return 1;
                break;
            }
            case ZIPDOSCAN: {
                zipres = unzip(fd, fdo, &inlen, &outlen, zipdo, NULL);
                break;
            }
            case ZIPDODUMP:
            case ZIPDODUMP2: {
                filename[0] = 0;    // it means that the file will be not created

                sprintf(filename, "%s[%s]%"PRIx, fpath, g_basename, g_offset);    // create the file
                //sprintf(filename, "%"PRIx".dat", g_offset);
                //*fdo = save_file(filename, 1);

                zipres = unzip(fd, fdo, &inlen, &outlen, zipdo, filename);

                FCLOSE(fdo);

                if((zipres < 0) && filename[0]) unlink(filename);
                break;
            }
            default: break;
        }

        if(!zipres) {
            LOG(" %"PRIu" -> %"PRIu" / 0x%"PRIx" _ %"PRIu, inlen, outlen, g_offset, (start_offset - g_last_offset));

            extracted++;
            g_last_offset = g_offset;   // g_offset points to start_offset + inlen, so it's ok

            if(g_zipwbits > 0) {
                backup_offset = ftell(fd);
                fseek(fd, start_offset, SEEK_SET);
                if(fread(zlib_header, 1, 2, fd) != 2) memset(zlib_header, 0, 2);
                fseek(fd, g_offset - 4, SEEK_SET);
                crc = (fgetc(fd) << 24) | (fgetc(fd) << 16) | (fgetc(fd) << 8) | fgetc(fd);
                LOG(" CM=%d CINFO=%d FCHECK=%d FDICT=%d FLEVEL=%d ADLER32=%08x",
                    (zlib_header[0] & 0xf),         // CM
                    (zlib_header[0] >> 4) & 0xf,    // CINFO
                    (zlib_header[1] & 0x1f),        // FCHECK
                    (zlib_header[1] >> 5) & 1,      // FDICT
                    (zlib_header[0] >> 6) & 3,      // FLEVEL
                    crc);                           // ADLER32
                fseek(fd, backup_offset, SEEK_SET);
            }

        } else {
            LOG(" error");
        }

    }

    if(*fdo && (fdo == &fdo_dummy)) FCLOSE(fdo);
    if(tmp_buff) FREE(tmp_buff);
    return extracted;
}


int unzip(FILE *fd, FILE **fdo, u64 *inlen, u64 *outlen, int zipdo, char *dumpname) {
    FILE    *fdo_dummy = NULL; // totally useless, just for testing
    if(!fdo) fdo = &fdo_dummy;
    u64     oldsz   = 0,
            oldoff;
    int     len;
    int     ret     = -1,
            zerr    = Z_OK;
    char    *freeme = NULL;

    if(dumpname && !dumpname[0]) dumpname = NULL;
    oldoff = g_offset;
    inflateReset(&z);

    for(; (len = buffread(fd, g_in, INSZ)); buffinc(len)) {
        //if(g_quiet >= 0) fputc('.', stderr);

        z.next_in   = g_in;
        z.avail_in  = len;
        do {
            z.next_out  = g_out;
            z.avail_out = OUTSZ;
            zerr = inflate(&z, Z_SYNC_FLUSH);

            switch(zipdo) {
                case ZIPDODUMP:
                case ZIPDOFILE:
                case ZIPDODUMP2: {
                    if(!dumpname) break;

                    sprintf(dumpname + strlen(dumpname), ".dat");
                    //, sign_ext(g_out, z.total_out - oldsz));

                    if(!*fdo) *fdo = save_file(dumpname);
                    dumpname = NULL;
                }
                default: break;
            }

            switch(zipdo) {
                case ZIPDODUMP:
                case ZIPDOFILE: {
                    myfwrite(g_out, z.total_out - oldsz, *fdo);
                    oldsz = z.total_out;

                    break;
                }
                case ZIPDODUMP2: {
                    myfwrite(g_in, z.total_in - oldsz, *fdo);
                    oldsz = z.total_in;

                    break;
                }
                default: break;
            }

            if(zerr != Z_OK) {      // inflate() return value MUST be handled now
                if(zerr == Z_STREAM_END) {
                    ret = 0;
                } else {
                    zlib_err(zerr);
                }
                break;
            }
            ret = 0;    // it's better to return 0 even if the z stream is incomplete... or not?
        } while(z.avail_in);

        if(zerr != Z_OK) break;     // Z_STREAM_END included, for avoiding "goto"
    }

    if(inlen)  *inlen  = z.total_in;
    if(outlen) *outlen = z.total_out;
    if(!ret) {
        oldoff        += z.total_in;
        g_total_zsize += z.total_in;
        g_total_size  += z.total_out;
    } else {
        oldoff++;
    }
    buffseek(fd, oldoff, SEEK_SET);
    FREE(freeme);
    if(*fdo && (fdo == &fdo_dummy)) FCLOSE(fdo);
    return ret;
}


int zlib_err(int zerr) {
    switch(zerr) {
        case Z_DATA_ERROR: {
            LOG("- zlib Z_DATA_ERROR, the data in the file is not in zip format\n"
                "  or uses a different windowBits value (-z). Try to use -z %d\n",
                -g_zipwbits);
            break;
        }
        case Z_NEED_DICT: {
            LOG("- zlib Z_NEED_DICT, you need to set a dictionary (option not available)");
            break;
        }
        case Z_MEM_ERROR: {
            LOG("- zlib Z_MEM_ERROR, insufficient memory");
            break;
        }
        case Z_BUF_ERROR: {
            LOG("- zlib Z_BUF_ERROR, the output buffer for zlib decompression is not enough");
            break;
        }
        case Z_INIT_ERROR: {
            LOG("Error: zlib initialization error (inflateInit2)");
            break;
        }
        case Z_END_ERROR: {
            LOG("Error: zlib free error (inflateEnd)");
            break;
        }
        case Z_RESET_ERROR: {
            LOG("Error: zlib reset error (inflateReset)");
            break;
        }
        default: {
            LOG("Error: zlib unknown error %d", zerr);
            break;
        }
    }
    return 0;
}


FILE *save_file(char *fname) {
    FILE    *fd;

    fd = fopen(fname, "wb");
    if(!fd)
    {
        LOG("Error: can't open file");
        return NULL;
    }
    return fd;
}


int myfwrite(u8 *buff, int size, FILE *fd) {
    if(!fd) {
        LOG("Error: myfw fd is NULL, contact me.");
        return(0);
    }
    if(size <= 0) return 1;
    if(fwrite(buff, 1, size, fd) != size) {
        LOG("Error: problems during files writing, check permissions and disk space");
        return(0);
    }
    return 1;
}


void FCLOSE(FILE **fd) {
    if(fd && *fd) {
        fclose(*fd);
        *fd = NULL;
    }
}
