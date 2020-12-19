/*
    Copyright 2004-2011 Luigi Auriemma

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
#include <zlib.h>
#include "sign_ext.h"

#ifdef WIN32
    #include <direct.h>
#else
    #include <dirent.h>
#endif

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;



#define VER             "0.3.5"
#define INSZ            0x800   // the amount of bytes we want to decompress each time
#define OUTSZ           0x10000 // the buffer used for decompressing the data
#define FBUFFSZ         0x40000 // this buffer is used for reading, faster
#define SHOWX           0x7ff   // AND to show the current scanned offset each SHOWX offsets
#define FCLOSE(X)       { if(X) fclose(X); X = NULL; }

#define ZIPDOSCAN1      0
#define ZIPDOSCAN       1
#define ZIPDOWRITE      2
#define ZIPDODUMP       3
#define ZIPDOFILE       4

#define Z_INIT_ERROR    -1000
#define Z_END_ERROR     -1001
#define Z_RESET_ERROR   -1002



int buffread(FILE *fd, u8 *buff, int size);
void buffseek(FILE *fd, int len, int mode);
void buffinc(int increase);
int zip_search(FILE *fd);
int unzip_all(FILE *fd, int zipdo);
int unzip(FILE *fd, FILE **fdo, u32 *inlen, u32 *outlen, int zipdo, u8 *dumpname);
u32 get_num(u8 *str);
void zlib_err(int err);
FILE *save_file(u8 *fname);
void myfw(u8 *buff, int size, FILE *fd);
void std_err(void);



z_stream    z;
FILE    *fdl        = NULL;
u32     offset      = 0,
        filebuffoff = 0,
        filebuffsz  = 0;
int     zipwbits    = 15,
        minzip      = 32,
        quiet       = 0,
        reminval    = 1,
        only_one    = 0,
        hexview     = 0;
u8      *in,
        *out,
        *filebuff,
        *listfile   = NULL;



int main(int argc, char *argv[]) {
    FILE    *fd,
            *fdo  = NULL;
    u32     inlen,
            outlen;
    int     i,
            zipdo = ZIPDOFILE,
            files;
    u8      *file_input,
            *file_output,
            *file_offset;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    fputs("\n"
        "Offset file unzipper "VER"\n"
        "by Luigi Auriemma\n"
        "e-mail: aluigi@autistici.org\n"
        "web:    aluigi.org\n"
        "\n", stdout);

    if(argc < 4) {
        printf("\n"
            "Usage: %s [options] <input> <output/dir> <offset>\n"
            "\n"
            "Options:\n"
            "-s       search for possible zip/gzip data in the input file, the scan starts\n"
            "         from the specified offset and finishs when something is found\n"
            "         the output field is ignored so you can use any name you want\n"
            "-S       as above but continues the scan (just like -a but without extraction)\n"
            "-a       unzip all the possible zip data found in the file. the output\n"
            "         directory where are unzipped the files is identified by <output>\n"
            "         all the output filenames contain the offset where they have been found\n"
            "-A       as above but without unzipping data, the output files will contain the\n"
            "         same original zipped data, just like a simple data dumper\n"
            "-1       related to -a/-A, generates one unique output file instead of many\n"
            "-m SIZE  lets you to decide the length of the zip block to check if it is a\n"
            "         valid zip data. default is %d. use a higher value to reduce the number\n"
            "         of false positive or a smaller one (eg 16) to see small zip data too\n"
            "-z NUM   this option is needed to specify a windowBits value. If you don't find\n"
            "         zip data in a file (like a classical zip file) try to set it to -15\n"
            "         valid values go from -8 to -15 and from 8 to 15. Default is 15\n"
            "-q       quiet, all the verbose error messages will be suppressed (-Q for more)\n"
            "-r       don't remove the invalid uncompressed files generated with -a and -A\n"
            "-x       visualize hexadecimal numbers\n"
            "-L FILE  dump the list of \"0xoffset zsize size\" in the specified FILE\n"
            "\n"
            "Note: offset is a decimal number or a hex number if you add a 0x before it\n"
            "      examples: 1234 or 0x4d2\n"
            "\n", argv[0], minzip);
        exit(1);
    }

    argc -= 3;
    for(i = 1; i < argc; i++) {
        if(((argv[i][0] != '-') && (argv[i][0] != '/')) || (strlen(argv[i]) != 2)) {
            printf("\nError: recheck your options, %s is not valid\n", argv[i]);
            exit(1);
        }
        switch(argv[i][1]) {
            case 's': zipdo     = ZIPDOSCAN1;           break;
            case 'S': zipdo     = ZIPDOSCAN;            break;
            case 'a': zipdo     = ZIPDOWRITE;           break;
            case 'A': zipdo     = ZIPDODUMP;            break;
            case '1': only_one  = 1;                    break;
            case 'm': minzip    = get_num(argv[++i]);   break;
            case 'z': zipwbits  = atoi(argv[++i]);      break;
            case 'q': quiet     = 1;                    break;
            case 'Q': quiet     = -1;                   break;
            case 'r': reminval  = 0;                    break;
            case 'x': hexview   = 1;                    break;
            case 'L': listfile  = argv[++i];            break;
            default: {
                printf("\nError: wrong command-line argument (%s)\n\n", argv[i]);
                exit(1);
                break;
            }
        }
    }

    file_input  = argv[argc];
    file_output = argv[argc + 1];
    file_offset = argv[argc + 2];

    printf("- open input file:    %s\n", file_input);
    fd = fopen(file_input, "rb");
    if(!fd) std_err();

    if(minzip > INSZ) minzip = INSZ;
    if(minzip < 1)    minzip = 1;

    if((zipdo == ZIPDOWRITE) || (zipdo == ZIPDODUMP)) {
        printf("- enter in directory: %s\n", file_output);
        if(chdir(file_output) < 0) std_err();
    }

    printf(
        "- zip data to check:  %d bytes\n"
        "- zip windowBits:     %d\n",
        minzip, zipwbits);

    in       = malloc(INSZ);
    out      = malloc(OUTSZ);
    filebuff = malloc(FBUFFSZ);
    if(!in || !out || !filebuff) std_err();

    offset = get_num(file_offset);  // do not skip, needed for buffseek
    printf("- seek offset:        0x%08x  (%u)\n", offset, offset);
    buffseek(fd, offset, SEEK_SET);

    z.zalloc = (alloc_func)0;
    z.zfree  = (free_func)0;
    z.opaque = (voidpf)0;
    if(inflateInit2(&z, zipwbits) != Z_OK) zlib_err(Z_INIT_ERROR);

    if(zipdo == ZIPDOFILE) {
        printf("- open output file:   %s\n", file_output);
        fdo = save_file(file_output);
        unzip(fd, &fdo, &inlen, &outlen, zipdo, NULL);
        FCLOSE(fdo)

        printf("\n"
            "- %u bytes read (zipped)\n"
            "- %u bytes unzipped\n",
            inlen, outlen);

    } else {
        printf("\n"
            "+------------+-------------+-------------------------+\n"
            "| hex_offset | blocks_dots | zip_size --> unzip_size |\n"
            "+------------+-------------+-------------------------+\n");

        files = unzip_all(fd, zipdo);
        if(files) {
            printf("\n\n- %u valid zip blocks found\n", files);
        } else {
            printf("\n\n- no valid full zip data found\n");
        }
    }

    FCLOSE(fdo)
    FCLOSE(fdl)
    FCLOSE(fd)
    inflateEnd(&z);
    free(in);
    free(out);
    free(filebuff);
    return(0);
}



int buffread(FILE *fd, u8 *buff, int size) {
    int     len,
            rest,
            ret;

    rest = filebuffsz - filebuffoff;

    ret = size;
    if(rest < size) {
        ret = size - rest;
        memmove(filebuff, filebuff + filebuffoff, rest);
        len = fread(filebuff + rest, 1, FBUFFSZ - rest, fd);
        filebuffoff = 0;
        filebuffsz  = rest + len;
        if(len < ret) {
            ret = rest + len;
        } else {
            ret = size;
        }
    }

    memcpy(buff, filebuff + filebuffoff, ret);
    return(ret);
}



void buffseek(FILE *fd, int off, int mode) {
    if(fseek(fd, off, mode) < 0) std_err();
    filebuffoff = 0;
    filebuffsz  = 0;
    offset      = ftell(fd);
}



void buffinc(int increase) {
    filebuffoff += increase;
    offset      += increase;
}



int zip_search(FILE *fd) {
    int     len,
            zerr,
            ret;

    for(ret = - 1; (len = buffread(fd, in, minzip)) >= minzip; buffinc(1)) {
        z.next_in   = in;
        z.avail_in  = len;
        z.next_out  = out;
        z.avail_out = OUTSZ;

        inflateReset(&z);
        zerr = inflate(&z, Z_SYNC_FLUSH);

        if(zerr == Z_OK) {  // do not use Z_STREAM_END here! gives only troubles!!!
            if(!quiet) fprintf(stderr, "\r  0x%08x\r", offset);
            if(listfile) {
                if(!fdl) fdl = save_file(listfile);
                fprintf(fdl, "0x%08x\n", offset);
            }
            ret = 0;
            break;
        }

        if(!quiet && !(offset & SHOWX)) fprintf(stderr, "\r  0x%08x\r", offset);
    }
    return(ret);
}



int unzip_all(FILE *fd, int zipdo) {
    FILE    *fdo    = NULL;
    u32     inlen,
            outlen;
    int     zipres,
            extracted;
    char    filename[64]    = "";

    extracted = 0;
    zipres    = -1;

    while(!zip_search(fd)) {
        printf("  0x%08x ", offset);

        switch(zipdo) {
            case ZIPDOSCAN1: {
                return(1);
                break;
            }
            case ZIPDOSCAN: {
                zipres = unzip(fd, &fdo, &inlen, &outlen, zipdo, NULL);
                break;
            }
            case ZIPDOWRITE:
            case ZIPDODUMP: {
                filename[0] = 0;
                if(!only_one || (only_one == 1)) {
                    sprintf(filename, "%08x", offset);
                    //sprintf(filename, "%08x.dat", offset);
                    //fdo = save_file(filename);
                    if(only_one == 1) only_one = 2;
                }
                zipres = unzip(fd, &fdo, &inlen, &outlen, zipdo, filename);
                if(!only_one) FCLOSE(fdo)
                if(reminval && (zipres < 0) && filename[0]) unlink(filename);
                break;
            }
            default: break;
        }

        if(!zipres) {
            if(hexview) {
                printf(" %08x --> %08x", inlen, outlen);
            } else {
                printf(" %u --> %u", inlen, outlen);
            }
            if(listfile) {
                if(!fdl) fdl = save_file(listfile);
                fprintf(fdl, "0x%08x %u %u\n", offset, inlen, outlen);
            }
            extracted++;
        } else {
            if(quiet > 0) printf(" error");
        }

        printf("\n");
    }

    //if(only_one == 2) FCLOSE(fdo)
    FCLOSE(fdo)
    return(extracted);
}



int unzip(FILE *fd, FILE **fdo, u32 *inlen, u32 *outlen, int zipdo, u8 *dumpname) {
    u32     oldsz = 0,
            oldoff,
            len;
    int     ret   = -1,
            zerr  = Z_OK;

    if(dumpname && !dumpname[0]) dumpname = NULL;
    oldoff = offset;
    inflateReset(&z);
    for(; (len = buffread(fd, in, INSZ)); buffinc(len)) {
        if(quiet >= 0) fputc('.', stderr);

        z.next_in   = in;
        z.avail_in  = len;
        do {
            z.next_out  = out;
            z.avail_out = OUTSZ;
            zerr = inflate(&z, Z_SYNC_FLUSH);

            switch(zipdo) {
                case ZIPDOWRITE:
                case ZIPDOFILE: {
                    if(dumpname) {
                        sprintf(dumpname + strlen(dumpname), ".%s", sign_ext(out, z.total_out - oldsz));
                        *fdo = save_file(dumpname);
                        dumpname = NULL;
                    }
                    myfw(out, z.total_out - oldsz, *fdo);
                    oldsz = z.total_out;
                    break;
                }
                case ZIPDODUMP: {
                    if(!z.avail_in) {
                        if(dumpname) {
                            sprintf(dumpname + strlen(dumpname), ".%s", sign_ext(out, z.total_out - oldsz));
                            *fdo = save_file(dumpname);
                            dumpname = NULL;
                        }
                        myfw(in, len, *fdo);
                    }
                    break;
                }
                default: break;
            }

            if(zerr != Z_OK) {      // inflate() return value MUST be handled now
                if(zerr == Z_STREAM_END) {
                    ret = 0;
                } else {
                    if(!quiet) zlib_err(zerr);
                }
                break;
            }
            ret = 0;    // it's better to return 0 even if the z stream is incomplete... or not?
        } while(z.avail_in);

        if(zerr != Z_OK) break;     // Z_STREAM_END included, for avoiding "goto"
    }

    *inlen  = z.total_in;
    *outlen = z.total_out;
    if(!ret) {
        oldoff += z.total_in;
    } else {
        oldoff++;
    }
    buffseek(fd, oldoff, SEEK_SET);
    return(ret);
}



u32 get_num(u8 *str) {
    u32     offsetx;

    if((str[0] == '0') && (tolower(str[1]) == 'x')) {
        sscanf(str + 2, "%x", &offsetx);
    } else {
        sscanf(str, "%u", &offsetx);
    }
    return(offsetx);
}



void zlib_err(int zerr) {
    switch(zerr) {
        case Z_DATA_ERROR:
            fprintf(stderr, "\n"
                "- zlib Z_DATA_ERROR, the data in the file is not in zip format\n"
                "  or uses a different windowBits value (-z). Try to use -z %d\n",
                -zipwbits);
            break;
        case Z_NEED_DICT:
            fprintf(stderr, "\n"
                "- zlib Z_NEED_DICT, you need to set a dictionary (option not available)\n");
            break;
        case Z_MEM_ERROR:
            fprintf(stderr, "\n"
                "- zlib Z_MEM_ERROR, insufficient memory\n");
            break;
        case Z_BUF_ERROR:
            fprintf(stderr, "\n"
                "- zlib Z_BUF_ERROR, the output buffer for zlib decompression is not enough\n");
            break;
        case Z_INIT_ERROR: {
            fprintf(stderr, "\nError: zlib initialization error (inflateInit2)\n");
            exit(1);
            break;
        }
        case Z_END_ERROR: {
            fprintf(stderr, "\nError: zlib free error (inflateEnd)\n");
            exit(1);
            break;
        }
        case Z_RESET_ERROR: {
            fprintf(stderr, "\nError: zlib reset error (inflateReset)\n");
            exit(1);
            break;
        }
        default: {
            fprintf(stderr, "\nError: zlib unknown error %d\n", zerr);
            exit(1);
            break;
        }
    }
}



FILE *save_file(u8 *fname) {
    static int  all = 0;
    FILE    *fd;
    u8      ans[10];

    if(!all) {
        fd = fopen(fname, "rb");
        if(fd) {
            fclose(fd);
            fprintf(stderr, "\n- the file \"%s\" already exists\n  do you want to overwrite it? (y/N/all)\n  ", fname);
            fgets(ans, sizeof(ans), stdin);
            if(tolower(ans[0]) == 'a') {
                all = 1;
            } else if(tolower(ans[0]) != 'y') {
                exit(1);
            }
        }
    }
    fd = fopen(fname, "wb");
    if(!fd) std_err();
    return(fd);
}



void myfw(u8 *buff, int size, FILE *fd) {
    if(!fd) return;
    if(size <= 0) return;
    if(fwrite(buff, 1, size, fd) != size) {
        fprintf(stderr, "\nError: problems during files writing, check permissions and disk space\n");
        exit(1);
    }
}


void std_err(void) {
    perror("\nError");
    exit(1);
}


