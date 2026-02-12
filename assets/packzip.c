/*
    Copyright 2004-2015 Luigi Auriemma

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

/*
compile on Windows:
    make
or
    gcc -s -O2 -Wall -Wunused-function -Wno-pointer-sign -Wno-reorder -Wno-sign-compare -o packzip packzip.c compression\advancecomp.cpp libs\7z_advancecomp\*.cc libs\lzma\LzmaDec.c libs\lzma\Lzma2Dec.c libs\lzma\Bra86.c libs\lzma\LzFind.c libs\lzma\LzFindMt.c libs\lzma\LzmaEnc.c libs\lzma\Lzma2Enc.c libs\lzma\MtCoder.c libs\lzma\Threads.c compression\adler32.c libs\uberflate\uberflate.c libs\zopfli\*.c -lstdc++ -lz -static -D_7Z_TYPES_
*/

//#define NOLFS
#ifndef NOLFS   // 64 bit file support not really needed since the tool uses signed 32 bits at the moment, anyway I leave it enabled
    #define _LARGE_FILES        // if it's not supported the tool will work
    #define __USE_LARGEFILE64   // without support for large files
    #define __USE_FILE_OFFSET64
    #define _LARGEFILE_SOURCE
    #define _LARGEFILE64_SOURCE
    #define _FILE_OFFSET_BITS   64
#endif

#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include <zlib.h>
#include "libs/zopfli/zopfli.h"
//#include "7z/7z.h"  // from AdvanceComp
#include "libs/lzma/LzmaEnc.h"
#include "packzip.h"
#include "libs/uberflate/uberflate.h"
#ifdef WIN32
    #include <windows.h>    // needed to get the number of CPUs
#endif

#if defined(_LARGE_FILES)
    #if defined(__APPLE__)
        #define fseek   fseeko
        #define ftell   ftello
    #elif defined(__FreeBSD__)
    #elif !defined(NOLFS)       // use -DNOLFS if this tool can't be compiled on your OS!
        #define off_t   off64_t
        #define fopen   fopen64
        #define fseek   fseeko64
        #define ftell   ftello64
        #ifndef fstat
            #ifdef WIN32
                #define fstat   _fstati64
                #define stat    _stati64
            #else
                #define fstat   fstat64
                #define stat    stat64
            #endif
        #endif
    #endif
#endif

//typedef uint8_t     u8;
//typedef uint16_t    u16;
//typedef uint32_t    u32;
typedef unsigned char   u8;
typedef unsigned int    u32;



#define VER         "0.3.1"

//#define MAXZIPLEN(n) ((n)+(((n)/1000)+1)+12)
#define MAXZIPLEN(n) ((n)+(((n)/10)+1)+12)  // for lzma



u32 zipit(FILE *fdi, FILE *fdo, int wbits, int flags, int store);
u32 zlib_compress(u8 *in, int insz, u8 *out, int outsz, int wbits, int flags, int store);
int lzma_compress(u8 *in, int insz, u8 *out, int outsz, int flags);
int get_num(char *data);
void myfw(FILE *fd, void *data, unsigned size);
void std_err(void);



int     g_uberflate     = 0,
        g_advancecomp   = 0;



int main(int argc, char *argv[]) {
    FILE    *fdi,
            *fdo;
    u32     offset      = 0,
            len;
    int     i,
            wbits       = 15,
            recreate    = 0,
            flags       = 0,    // LZMA_FLAGS_NONE is the same of Z_DEFAULT_STRATEGY
            store       = 0;
    char    *input,
            *output;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    fputs("\n"
        "PackZip " VER "\n"
        "by Luigi Auriemma\n"
        "e-mail: aluigi@autistici.org\n"
        "web:    aluigi.org\n"
        "\n", stderr);

    if(argc < 3) {
        fprintf(stderr, "\n"
            "Usage: %s [options] <input> <output>\n"
            "\n"
            "Options:\n"
            "-o OFF   offset of the output file where storing the raw zipped data\n"
            "-c       recreate from scratch the output file if already exists\n"
            "         keep this option in mind if you want to re-create a file because the\n"
            "         main usage of this tool is reinjecting files inside archives!\n"
            "-w BITS  windowBits value (usually -15 to -8 and 8 to 15), default is %d\n"
            "         negative for raw deflate and positive for zlib rfc1950 (78...)\n"
            "         if 0 then it will perform the LZMA compression\n"
            "-m MODE  mode:\n"
            "         zlib/deflate            LZMA\n"
            "           %d default strategy     %d normal (default)\n"
            "           %d filtered             %d 86_header\n"
            "           %d huffman only         %d 86_decoder\n"
            "           %d rle                  %d 86_dechead\n"
            "           %d fixed                %d efs\n"
            "-0       store only, no compression\n"
            "-f       use AdvanceComp compression which is faster\n"
            #ifdef WIN32
            "-u       use uberflate for maximum zlib/deflate compression (slower)\n"
            #endif
            "\n"
            "By default this tool \"injects\" the new compressed data in the output file if\n"
            "already exists, useful for modifying archives of unknown formats replacing\n"
            "only the data that has been modified without touching the rest.\n"
            "The tool uses zopfli or AdvanceCOMP for zlib/deflate compression but switches\n"
            "to zlib if the -m option is used.\n"
            "\n", argv[0],
            wbits,
            Z_DEFAULT_STRATEGY,     LZMA_FLAGS_NONE,
            Z_FILTERED,             LZMA_FLAGS_86_HEADER,
            Z_HUFFMAN_ONLY,         LZMA_FLAGS_86_DECODER,
            Z_RLE,                  LZMA_FLAGS_86_DECODER | LZMA_FLAGS_86_HEADER,
            Z_FIXED,                LZMA_FLAGS_EFS
        );
        exit(1);
    }

    argc -= 2;
    for(i = 1; i < argc; i++) {
        if(((argv[i][0] != '-') && (argv[i][0] != '/')) || (strlen(argv[i]) != 2)) {
            fprintf(stderr, "\nError: wrong argument (%s)\n", argv[i]);
            exit(1);
        }
        switch(argv[i][1]) {
            case 'o': offset        = get_num(argv[++i]);   break;
            case 'c': recreate      = 1;                    break;
            case 'z':   // compatibility with offzip!
            case 'w': wbits         = get_num(argv[++i]);   break;
            case 'm': flags         = get_num(argv[++i]);   break;
            case '0': store         = 1;                    break;
            case 'u': g_uberflate   = 1;                    break;
            case 'f': g_advancecomp = 1;                    break;
            default: {
                fprintf(stderr, "\nError: wrong argument (%s)\n", argv[i]);
                exit(1);
            }
        }
    }
    input  = argv[argc];
    output = argv[argc + 1];

    if(!strcmp(input, "-")) {
        fdi = stdin;
    } else {
        fprintf(stderr, "- open input  %s\n", input);
        fdi = fopen(input, "rb");
        if(!fdi) std_err();
    }

    if(!strcmp(output, "-")) {
        fdo = stdout;
    } else {
        fprintf(stderr, "- open output %s\n", output);
        if(recreate) {
            fdo = fopen(output, "wb");
            if(!fdo) std_err();
            fprintf(stderr, "- recreate output file\n");
        } else {
            fdo = fopen(output, "r+b");
            if(!fdo) {
                fdo = fopen(output, "wb");
                if(!fdo) std_err();
            }
        }
    }

    fprintf(stderr, 
        "- offset        0x%08x\n"
        "- windowbits    %d\n",
        offset, wbits);

    if(offset) {
        fprintf(stderr, "- seek offset\n");
        if(fseek(fdo, offset, SEEK_SET)) std_err();
    }

    time_t start_time = time(NULL);

    len = zipit(fdi, fdo, wbits, flags, store);

    if(fdi != stdin)  fclose(fdi);
    if(fdo != stdout) fclose(fdo);

    if(len < 0) {
        fprintf(stderr, "\n- the compression failed\n");
    } else {
        fprintf(stderr, "- output size   0x%08x / %u\n", len, len);
    }
    fprintf(stderr, "- done in %d seconds\n", (int)(time(NULL) - start_time));
    return(0);
}



u8 *incremental_fread(FILE *fd, u32 *ret_size) {
    static const int    STDINSZ = 4096;
    u32     size;
    int     len,
            buffsz  = 0;
    u8      *buff   = NULL;

    if(ret_size) *ret_size = 0;
    size = 0;
    for(;;) {
        if((size + STDINSZ) >= buffsz) {
            buffsz = size + STDINSZ;
            buff = (u8 *)realloc(buff, buffsz + 1);
            if(!buff) std_err();
        }
        len = fread(buff + size, 1, STDINSZ, fd);
        if(len <= 0) break;
        size += len;
    }
    if(buff) buff[size] = 0;
    if(ret_size) *ret_size = size;
    return(buff);
}



u8 *myzopfli(u8 *in, int insz, int *ret_outsz, int type) {
    int     outsz   = 0;
    u8      *out    = NULL;

    // the zopli options are a pain because the results (ratio and time) depends by the input file
    // the following are just the best I found on multiple tests
    ZopfliOptions   opt;
    memset(&opt, 0, sizeof(opt));
    ZopfliInitOptions(&opt);
         if(insz < (10 * 1024 * 1024))  opt.numiterations = 15; // this is
    else if(insz < (50 * 1024 * 1024))  opt.numiterations = 10; // just for
    else                                opt.numiterations = 5;  // speed
    opt.blocksplitting      = 1;
    opt.blocksplittinglast  = 0;
    opt.blocksplittingmax   = 0;
    ZopfliCompress(&opt, type, in, insz, &out, &outsz);

    if(ret_outsz) *ret_outsz = outsz;
    return out;
}



u32 zipit(FILE *fdi, FILE *fdo, int wbits, int flags, int store) {
    struct stat xstat;
    size_t  ret_t;
    int     ret,
            use_zlib    = 0;
    u32     in_size,
            out_size;
    u8      *in_data,
            *out_data;

    if(fdi == stdin) {
        in_data = incremental_fread(fdi, &in_size);
    } else {
        fstat(fileno(fdi), &xstat);
        in_size = xstat.st_size;
        in_data = (u8 *)malloc(in_size);
    }
    in_size = fread(in_data, 1, in_size, fdi);

    if(g_uberflate) out_size = UBERFLATE_MAXZIPLEN(in_size);
    else            out_size = MAXZIPLEN(in_size);
    out_data = (u8 *)malloc(out_size);

    if(!in_data || !out_data) return(0);

    fprintf(stderr, "- input size    0x%08x / %u\n", in_size, in_size);

    if(store || flags) use_zlib = 1;

    fprintf(stderr, "- compression  ");
    if(!wbits) {
        fprintf(stderr, " LZMA\n");
             if(g_uberflate) { fprintf(stderr, "\nError: uberflate doesn't support LZMA\n"); exit(1); }
        ret = lzma_compress(in_data, in_size, out_data, out_size, flags);
        //ret = advancecomp_lzma(in_data, in_size, out_data, out_size, flags, store);
    } else if(wbits > 0) {
        fprintf(stderr, " ZLIB\n");
             if(g_uberflate)    ret = uberflate(in_data, in_size, out_data, out_size, 1);
        else if(use_zlib)       ret = zlib_compress(in_data, in_size, out_data, out_size, wbits, flags, store);
        else if(g_advancecomp)  ret = advancecomp_rfc1950(in_data, in_size, out_data, out_size, store);
        else                    { free(out_data); out_data = myzopfli(in_data, in_size, &ret_t, ZOPFLI_FORMAT_ZLIB);    ret = ret_t; }
    } else {
        fprintf(stderr, " DEFLATE\n");
             if(g_uberflate)    ret = uberflate(in_data, in_size, out_data, out_size, 0);
        else if(use_zlib)       ret = zlib_compress(in_data, in_size, out_data, out_size, wbits, flags, store);
        else if(g_advancecomp)  ret = advancecomp_deflate(in_data, in_size, out_data, out_size, store);
        else                    { free(out_data); out_data = myzopfli(in_data, in_size, &ret_t, ZOPFLI_FORMAT_DEFLATE); ret = ret_t; }
    }
    if(ret <= 0) {
        out_size = 0;
    } else {
        out_size = ret;
    }
    myfw(fdo, out_data, out_size);

    free(in_data);
    free(out_data);
    return(out_size);
}



u32 zlib_compress(u8 *in, int insz, u8 *out, int outsz, int wbits, int flags, int store) {
    z_stream    z;
    u32     ret;
    int     zerr;

    z.zalloc = Z_NULL;
    z.zfree  = Z_NULL;
    z.opaque = Z_NULL;
    if(deflateInit2(&z, store ? Z_NO_COMPRESSION : Z_BEST_COMPRESSION, Z_DEFLATED, wbits, 9, flags)) {
        fprintf(stderr, "\nError: zlib initialization error\n");
        exit(1);
    }

    z.next_in   = in;
    z.avail_in  = insz;
    z.next_out  = out;
    z.avail_out = outsz;
    zerr = deflate(&z, Z_FINISH);
    if(zerr != Z_STREAM_END) {
        fprintf(stderr, "\nError: zlib error, %s\n", z.msg ? z.msg : "");
        deflateEnd(&z);
        exit(1);
    }

    ret = z.total_out;
    deflateEnd(&z);
    return(ret);
}



int get_cpu_number(void) {
    #ifdef WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return info.dwNumberOfProcessors;
    #else
        #ifdef _SC_NPROCESSORS_ONLN
        return sysconf(_SC_NPROCESSORS_ONLN);
        #endif
    #endif
    return(-1);
}



char *lzma_status_code(int code) {
    switch(code) {
        case SZ_OK: return "OK"; break;
        case SZ_ERROR_DATA: return "ERROR_DATA"; break;
        case SZ_ERROR_MEM: return "ERROR_MEM"; break;
        case SZ_ERROR_CRC: return "ERROR_CRC"; break;
        case SZ_ERROR_UNSUPPORTED: return "ERROR_UNSUPPORTED"; break;
        case SZ_ERROR_PARAM: return "ERROR_PARAM"; break;
        case SZ_ERROR_INPUT_EOF: return "ERROR_INPUT_EOF"; break;
        case SZ_ERROR_OUTPUT_EOF: return "ERROR_OUTPUT_EOF"; break;
        case SZ_ERROR_READ: return "ERROR_READ"; break;
        case SZ_ERROR_WRITE: return "ERROR_WRITE"; break;
        case SZ_ERROR_PROGRESS: return "ERROR_PROGRESS"; break;
        case SZ_ERROR_FAIL: return "ERROR_FAIL"; break;
        case SZ_ERROR_THREAD: return "ERROR_THREAD"; break;
        case SZ_ERROR_ARCHIVE: return "ERROR_ARCHIVE"; break;
        case SZ_ERROR_NO_ARCHIVE: return "ERROR_NO_ARCHIVE"; break;
        default: return "unknown error"; break;
    }
    return "";
}



void lzma_set_properties(CLzmaEncProps *props, int dictsz) {
        props->level = 9;            /*  0 <= level <= 9 */
        props->dictSize = 1<<dictsz; /* (1 << 12) <= dictSize <= (1 << 27) for 32-bit version
                                       (1 << 12) <= dictSize <= (1 << 30) for 64-bit version
                                       default = (1 << 24) */
        // xz wants lc+lp <= 4
        //props->lc = 1;               /* 0 <= lc <= 8, default = 3 */
        //props->lp = 3;               /* 0 <= lp <= 4, default = 0 */
        props->lc = 8;               /* 0 <= lc <= 8, default = 3 */
        props->lp = 4;               /* 0 <= lp <= 4, default = 0 */

        props->pb = 0; /* yeah 0!*/  /* 0 <= pb <= 4, default = 2 */
        //props->algo = 1;             /* 0 - fast, 1 - normal, default = 1 */
        props->fb = 273;             /* 5 <= fb <= 273, default = 32 */
        //props->btMode = 1;           /* 0 - hashChain Mode, 1 - binTree mode - normal, default = 1 */
        props->numHashBytes = 4;     /* 2, 3 or 4, default = 4 */
        //props->mc = (1 << 30);       /* 1 <= mc <= (1 << 30), default = 32 */

    props->writeEndMark = 1;
    props->numThreads = get_cpu_number();
    // if(props->numThreads <= 0) LZMA will fix it automatically
}



#define LZMA_COMPRESS_SET_FLAGS \
    if(flags & LZMA_FLAGS_EFS) { \
        if(outsz < 4) return(-2); \
        o[0] = 0; \
        o[1] = 0 >> 8; \
        o[2] = propsz; \
        o[3] = propsz >> 8; \
        o     += 4; \
        outsz -= 4; \
    } \
    if(flags & LZMA_FLAGS_86_DECODER) { \
        if(outsz < 1) return(-3); \
        o[0] = filter; \
        o++; \
        outsz--; \
    } \
    if(flags & LZMA_FLAGS_86_HEADER) { \
        if(outsz < 8) return(-4); \
        o[0] = insz; \
        o[1] = insz >> 8; \
        o[2] = insz >> 16; \
        o[3] = insz >> 24; \
        o[4] = 0; \
        o[5] = 0; \
        o[6] = 0; \
        o[7] = 0; \
        o     += 8; \
        outsz -= 8; \
    } \


    
int lzma_compress(u8 *in, int insz, u8 *out, int outsz, int flags) {
void *SzAlloc(void *p, size_t size) { return(real_calloc(size, 1)); }  // xmalloc doesn't return in case of error
void SzFree(void *p, void *address) { if(address) real_free(address); }
ISzAlloc g_Alloc = { SzAlloc, SzFree };

	CLzmaEncHandle  lzma;
    CLzmaEncProps   props;
    SizeT   t,
            outlen;
    int     err,
            filter  = 0,
            propsz  = 5,
            dictsz  = 27;   // it means: allocate (1 << (dictsz + 2)) bytes
    u8      *o;

    lzma = LzmaEnc_Create(&g_Alloc);
	if(!lzma) return -1;

redo:
    o = out;
    LzmaEncProps_Init(&props);
    LzmaEncProps_Normalize(&props);

    lzma_set_properties(&props, dictsz);

    LzmaEnc_SetProps(lzma, &props);

    if(flags & LZMA_FLAGS_PROP0) {
        propsz = 0;
    } else {
        t = outsz;
        LzmaEnc_WriteProperties(lzma, o, &t);
        propsz = t;
        o     += propsz;
        outsz -= propsz;
    }

        /* flags */

    LZMA_COMPRESS_SET_FLAGS

        /* compression */

    outlen = outsz;
    err = LzmaEnc_MemEncode(lzma, o, &outlen, in, insz, props.writeEndMark, NULL, &g_Alloc, &g_Alloc);
    if((err == SZ_ERROR_PARAM) || (err == SZ_ERROR_MEM)) {
        dictsz--;
        if(dictsz >= 12) goto redo;
    }
    if(err != SZ_OK) {
        LzmaEnc_Destroy(lzma, &g_Alloc, &g_Alloc);
        fprintf(stderr, "\nError: lzma error %s\n", lzma_status_code(err));
        return -5;
    }
    LzmaEnc_Destroy(lzma, &g_Alloc, &g_Alloc);
    return((o - out) + outlen);
}



int get_num(char *data) {
    int     num = 0;

    if((data[0] == '$') || (data[0] == 'h') || ((strlen(data) > 2) && (tolower(data[1]) == 'x'))) {
        sscanf(data, "%x", &num);
    } else {
        sscanf(data, "%u", &num);
    }
    return(num);
}



void myfw(FILE *fd, void *data, unsigned size) {
    if(fwrite(data, 1, size, fd) == size) return;
    fprintf(stderr, "\nError: problems during the writing of the output file, check your free space\n");
    exit(1);
}



void std_err(void) {
    perror("\nError");
    exit(1);
}


