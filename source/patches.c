#include <polarssl/md.h>
#include <polarssl/md5.h>
#include <polarssl/sha1.h>
#include <polarssl/sha2.h>
#include <polarssl/sha4.h>
#include <zlib.h>

#include "saves.h"
#include "util.h"
#include "crc_util.h"
#include "list.h"

#define skip_spaces(str)        while (*str == ' ') str++;
#define TLOU_HMAC_KEY			"xM;6X%/p^L/:}-5QoA+K8:F*M!~sb(WK<E%6sW_un0a[7Gm6,()kHoXY+yI/s;Ba"

typedef enum
{
    BSD_VAR_NULL = 0,
    BSD_VAR_INT8 = 1,
    BSD_VAR_INT16 = 2,
    BSD_VAR_INT32 = 4,
    BSD_VAR_INT64 = 8,
    BSD_VAR_MD5 = 16,
    BSD_VAR_SHA1 = 20,
    BSD_VAR_SHA256 = 32,
    BSD_VAR_SHA512 = 64,
} bsd_var_types;

typedef struct
{
    char* name;
    uint8_t len;
    uint8_t* data;
} bsd_variable_t;


void remove_char(char * str, int len, char seek)
{
	int x;
	for (x = 0; x < len; x++)
		if (str[x] == seek)
			str[x] = '\n';
}

long search_data(const char* data, size_t size, const char* search, int len, int count)
{
	long i;
	int k = 1;

	for (i = 0; i < (size-len); i++)
		if ((memcmp(data + i, search, len) == 0) && (k++ == count))
			return i;

    return -1;
}

bsd_variable_t* _get_bsd_variable(list_t* list, const char* vname)
{
	list_node_t *node = list_head(list);
	bsd_variable_t *var;

	while (node) {
		var = list_get(node);
		if (strcmp(var->name, vname) == 0)
			return var;

		node = node->next;
	}

	return NULL;
}

char* _decode_variable_data(const char* line, int *data_len, list_t* vlist)
{
    int i, len = 0;
    char* output = NULL;

    skip_spaces(line);
	if (wildcard_match(line, "\"*\"*"))
	{
	    char* c = strchr(line, '"')+1;
	    len = strrchr(line, '"') - c;
		output = malloc(len);
	    
	    for (i = 0; i < len; i++)
	        output[i] = c[i];
	}
	else if (wildcard_match(line, "[*]*"))
	{
	    line++;
	    
	    char* tmp = strchr(line, ']');
	    *tmp = 0;
	    
	    bsd_variable_t* var = _get_bsd_variable(vlist, line);
	    
	    line = tmp+1;
	    *tmp = ']';
	    
	    if (var)
	    {
	        len = var->len;
    		output = malloc(len);
	        memcpy(output, var->data, len);
	    }	        
	}
	else
	{
	    if (line[0] == '0' && line[1] == 'x')
	        line += 2;

	    len = strlen(line) / 2;
	    output = (char*) x_to_u8_buffer(line);
	}

	*data_len = len;
	return output;
}

int _parse_int_value(const char* line, const int ptrval, const int size, list_t* vlist)
{
    int ret = 0;

    skip_spaces(line);
    if (strlen(line) == 0)
    {
        return 0;
    }
	else if (wildcard_match_icase(line, "pointer*"))
	{
	    line += strlen("pointer");
	    skip_spaces(line);

	    ret = ptrval + _parse_int_value(line, ptrval, size, vlist);
	}
	else if (wildcard_match_icase(line, "eof*"))
	{
		line += strlen("eof");
	    skip_spaces(line);

//        sscanf(line, "%x", &ret);
        ret += size - 1 + _parse_int_value(line, ptrval, size, vlist);
	}
	else if (wildcard_match(line, "[*]*"))
	{
	    line++;
	    
	    char* tmp = strchr(line, ']');
	    *tmp = 0;
	    
	    bsd_variable_t* var = _get_bsd_variable(vlist, line);
	    
	    line = tmp+1;
	    *tmp = ']';
	    
	    if (var)
	    {
	        switch (var->len)
	        {
	            case BSD_VAR_INT8:
        	        ret = *((uint8_t*)var->data) + _parse_int_value(line, ptrval, size, vlist);
        	        break;
	            case BSD_VAR_INT16:
        	        ret = *((uint16_t*)var->data) + _parse_int_value(line, ptrval, size, vlist);
        	        break;
	            case BSD_VAR_INT32:
        	        ret = *((uint32_t*)var->data) + _parse_int_value(line, ptrval, size, vlist);
        	        break;
	        }
	    }
	}
	else
	{
	    sscanf(line, "%x", &ret);
	}
	
	return ret;
}

void _cleanup_var_list(list_t* list)
{
	list_node_t *node;
	node = list_head(list);
	while (node) {
		if (node->value)
		{
			bsd_variable_t* bv = (bsd_variable_t*) node->value;
			if (bv->data)
				free(bv->data);

			free(node->value);
		}
		node = node->next;
	}
    list_free(list);
}

int apply_bsd_patch_code(const char* filepath, code_entry_t* code)
{
    char *data;
	size_t dsize;
	long pointer = 0;
	long range_start = 0, range_end = 0;
	uint8_t carry = 0;
	uint32_t old_val;
	custom_crc_t custom_crc = {0,0,0,0,0,0};
	int codelen = strlen(code->codes);
    char *line = strtok(code->codes, "\n");
    list_t* var_list = list_alloc();
	
	LOG("Applying [%s] to '%s'...", code->name, filepath);
	read_buffer(filepath, (uint8_t**) &data, &dsize);

//	write_buffer(APOLLO_PATH "PAYLOAD.src", (uint8_t*) data, dsize);

    while (line)
    {
        // carry(*)
		if (wildcard_match_icase(line, "carry(*)"))
		{
			int tmpi;
			// carry setting for add() / sub()
			line += strlen("carry");
		    sscanf(line, "(%d)", &tmpi);
			carry = tmpi;
		    
		    LOG("Set carry bytes = %d\n", carry);
		}

        // set *:*
		else if (wildcard_match_icase(line, "set *:*"))
		{
			// set pointer: 43
			// ;Sets the file pointer to a specific address.
			//
			// write next 0: 446966666963756C7479
			// ;Overwrites 0 bytes after the file pointer address.
			//
			// set [hash]:md5
			// ;[hash] is the variable. it can be any name. you can have many variables.
			// ;md5 is a function. the md5 hash is calculated for the current file, and stored in the variable [hash]
			//
			// set [myvariable2]:"hello"
			// ;sets the text "hello" into the variable [myvariable2]
			//
			// set [anyname1]:read(0x100, (10)) 
			// ;read(offset, length) is a function that reads n bytes from current file
			// ;0x100 is the offset in hex
			// ;(10) is the length in decimal
			// ;the example 4 reads 10 bytes starting from offset 0x100 and store them in the variable [anyname1]

			// set *:param = "lastbyte*"
			// set *:param = "eof*"
			// set *:param = "xor:*", "not *", "md5*", "crc*", "eachecksum", "adler16", "adler32", "sha1*", "sha256", 
			// set *:param = "sha384", "sha512", "hmac*", "md4*", "md2*", "read(*,*)*", "xor(*,*,*)*", "add(*,*)", 
			// set *:param = "wadd(*,*)", "[dq]wadd(*,*)", "sub(*,*)", "wsub(*,*)", "[dq]wsub(*,*)", "repeat(*,*)*", 
			// set *:param = "mid(*,*)", "left(*,*)", "right(*,*)"
			// UNUSED: "userid*", "titleid*", "psid*", "account*", "profile*",
			// UNUSED: crc16, adler16, md4, sha384, sha512, left, not
			//
			// "set range:*,*", "set crc_*:*", "set md5:*", "set md2:*", "set sha1:*", "set crc32:*", "set crc32big:*", 
			// "set crc32little:*", "set crc16:*", "set adler16:*", "set adler32:*",  
			// "set ""*"":*", "set pointer:*"
			// UNUSED: "set psid:*", "set userid:*", "set titleid:*", "set *account*:*", "set *profile*:*",
			// UNUSED: crc16, adler16, md4, sha384, sha512,

			int ptr_off, len;
			char* tmp = NULL;

			line += strlen("set");
		    skip_spaces(line);

		    // set pointer:*
			if (wildcard_match_icase(line, "pointer:*"))
			{
    			line += strlen("pointer:");

    			// set pointer:eof*
    			if (wildcard_match_icase(line, "eof*"))
    			{
        			line += strlen("eof");
        		    skip_spaces(line);

                    sscanf(line, "%x", &ptr_off);
                    pointer = dsize + ptr_off - 1;
    			}
    			// set pointer:lastbyte*
    			else if (wildcard_match_icase(line, "lastbyte*"))
    			{
        			line += strlen("lastbyte");
        		    skip_spaces(line);

                    sscanf(line, "%x", &ptr_off);
                    pointer = dsize + ptr_off - 1;
    			}
    			// set pointer:pointer*
    			else if (wildcard_match_icase(line, "pointer*"))
    			{
        			line += strlen("pointer");
        		    skip_spaces(line);

                    sscanf(line, "%x", &ptr_off);
                    pointer += ptr_off;
    			}
    			// set pointer:read(*,*)*
    			else if (wildcard_match_icase(line, "read(*,*)*"))
    			{
        			line += strlen("read");
        			
        			int raddr, rlen;
        			sscanf(line, "(%x,%x)", &raddr, &rlen);

                    int *rval = (int*) &data[raddr];
            	    LOG("address = %d len %d ", raddr, rlen);
            	    
            	    pointer = *rval;
    			}
    			// set pointer:[*]*
    			else if (wildcard_match_icase(line, "[*]*"))
    			{
    			    LOG("Getting value for %s\n", line);
    			    pointer = _parse_int_value(line, pointer, dsize, var_list);
    			}
    			// set pointer:* (e.g. 0x00000000)
    			else
    			{
                    sscanf(line, "%x", &ptr_off);
                    pointer = ptr_off;
    			}

                LOG("POINTER = %ld (0x%lX)\n", pointer, pointer);
			}

			// set range:*,*
			else if (wildcard_match_icase(line, "range:*,*"))
			{
    			line += strlen("range:");

			    tmp = strchr(line, ',');
			    *tmp = 0;
			    
			    range_start = _parse_int_value(line, pointer, dsize, var_list);
				if (range_start < 0)
					range_start = 0;

			    line = tmp+1;
			    *tmp = ',';

			    range_end = _parse_int_value(line, pointer, dsize, var_list) + 1;
				if (range_end > dsize)
					range_end = dsize;

                LOG("RANGE = %ld (0x%lX) - %ld (0x%lX)\n", range_start, range_start, range_end-1, range_end-1);
			}

			// set crc_*:*
			else if (wildcard_match_icase(line, "crc_*:*"))
			{
				int tmpi;
			    line += strlen("crc_");

			    if (wildcard_match_icase(line, "bandwidth:*"))
			    {
    			    line += strlen("bandwidth:");
    			    sscanf(line, "%d", &tmpi);
					custom_crc.bandwidth = tmpi;
			    }

			    else if (wildcard_match_icase(line, "polynomial:*"))
			    {
    			    line += strlen("polynomial:");
    			    sscanf(line, "%x", &custom_crc.polynomial);
			    }

			    else if (wildcard_match_icase(line, "initial_value:*"))
			    {
    			    line += strlen("initial_value:");
    			    sscanf(line, "%x", &custom_crc.initial_value);
			    }

			    else if (wildcard_match_icase(line, "output_xor:*"))
			    {
    			    line += strlen("output_xor:");
    			    sscanf(line, "%x", &custom_crc.output_xor);
			    }

			    else if (wildcard_match_icase(line, "reflection_input:*"))
			    {
    			    line += strlen("reflection_input:");
    			    sscanf(line, "%d", &tmpi);
					custom_crc.reflection_input = tmpi;
			    }

			    else if (wildcard_match_icase(line, "reflection_output:*"))
			    {
    			    line += strlen("reflection_output:");
    			    sscanf(line, "%d", &tmpi);
					custom_crc.reflection_output = tmpi;
			    }
			}

			// set [*]:*
			else if (wildcard_match(line, "[*]:*"))
			{
			    line++;
			    
			    tmp = strchr(line, ']');
			    *tmp = 0;
			    
        	    bsd_variable_t* var = _get_bsd_variable(var_list, line);

			    if (!var)
			    {
			        var = malloc(sizeof(bsd_variable_t));
    			    var->name = strdup(line);
    			    var->data = NULL;
    			    var->len = BSD_VAR_NULL;
    			    list_append(var_list, var);
    			}
    			else
    			{
    			    // for now we don't update variable values, we only overwrite
			        switch (var->len)
			        {
			            case BSD_VAR_INT8:
		        	        old_val = *((uint8_t*)var->data);
		        	        break;
			            case BSD_VAR_INT16:
		        	        old_val = *((uint16_t*)var->data);
		        	        break;
			            case BSD_VAR_INT32:
		        	        old_val = *((uint32_t*)var->data);
		        	        break;
		        	    default:
		        	    	old_val = 0;
		        	    	break;
			        }

    			    if (var->data)
    			    {
    			        free(var->data);
    			        var->data = NULL;
    			    }

    			    LOG("Old value 0x%X\n", old_val);
    			}

        	    LOG("Var name = %s\n", var->name);

			    line = tmp+2;
			    *tmp = ']';

    		    // set [*]:xor:*
    			if (wildcard_match_icase(line, "xor:*"))
    			{
    			    line += strlen("xor:");
        		    skip_spaces(line);
    
    			    int wlen;
    			    char* xor_val = _decode_variable_data(line, &wlen, var_list);

    			    if (var->len != wlen)
    			    {
    			        // variable has different length
    			        LOG("[%s]:XOR: error! var length doesn't match", var->name);
    			        return 0;
    			    }
    			    
    			    for (int i=0; i < wlen; i++)
    			        ((u8*)&old_val)[i] ^= xor_val[i];

                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &old_val, var->len);

    			    LOG("Var [%s]:XOR:%s = %X\n", var->name, line, old_val);
    			}

			    // set [*]:crc32*
			    else if (wildcard_match_icase(line, "crc32*"))
			    {
			        uint32_t hash;
			        custom_crc.initial_value = CRC_32_INIT_VALUE;
					custom_crc.polynomial = CRC_32_POLYNOMIAL;
					custom_crc.output_xor = CRC_32_XOR_VALUE;

    			    tmp = strchr(line, ':');
    			    if (tmp)
    			    {
    			        sscanf(tmp+1, "%x", &custom_crc.initial_value);
    			    }
    			    
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

    			    if (wildcard_match_icase(line, "crc32big*"))
    			    {
    			        // CRC-32/BZIP
						custom_crc.reflection_input = 0;
						custom_crc.reflection_output = 0;

        			    hash = crc32_hash(start, len, &custom_crc);
        			    LOG("len %d CRC32Big HASH = %X\n", len, hash);
    			    }
    			    else
    			    {
    			        // CRC-32
						custom_crc.reflection_input = 1;
						custom_crc.reflection_output = 1;

        			    hash = crc32_hash(start, len, &custom_crc);
        			    LOG("len %d CRC32 HASH = %X\n", len, hash);    			    
    			    }

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &hash, var->len);
			    }

			    // set [*]:crc16*
			    // CRC-16/XMODEM
			    else if (wildcard_match_icase(line, "crc16*"))
			    {
			        uint16_t hash;
			        custom_crc.initial_value = CRC_16_INIT_VALUE;
					custom_crc.polynomial = CRC_16_POLYNOMIAL;
					custom_crc.output_xor = CRC_16_XOR_VALUE;
					custom_crc.reflection_input = 0;
					custom_crc.reflection_output = 0;

    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

    			    hash = crc16_hash(start, len, &custom_crc);

                    var->len = BSD_VAR_INT16;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &hash, var->len);

    			    LOG("len %d CRC16 HASH = %X\n", len, hash);
			    }

			    // set [*]:crc*
			    // Custom CRC
			    else if (wildcard_match_icase(line, "crc*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    LOG("ref.in %d ref.out %d\n", custom_crc.reflection_input, custom_crc.reflection_output);

			        if (custom_crc.bandwidth == 16)
			        {
    			        // Custom CRC-16
    			        uint16_t hash;

        			    hash = crc16_hash(start, len, &custom_crc);

                        var->len = BSD_VAR_INT16;
                        var->data = malloc(var->len);
                        memcpy(var->data, (u8*) &hash, var->len);

        			    LOG("len %d Custom CRC16 HASH = %X\n", len, hash);
			        }
			        else
			        {
    			        // Custom CRC-32
    			        uint32_t hash;

        			    hash = crc32_hash(start, len, &custom_crc);

                        var->len = BSD_VAR_INT32;
                        var->data = malloc(var->len);
                        memcpy(var->data, (u8*) &hash, var->len);

        			    LOG("len %d Custom CRC32 HASH = %X\n", len, hash);
			        }
			    }

			    // set [*]:crc64*
			    else if (wildcard_match_icase(line, "crc64*"))
			    {
					//low priority
					//crc64_ecma / crc64_iso
					LOG("Error: command not implemented");
					return 0;
			    }

			    // set [*]:md5_xor*
			    else if (wildcard_match_icase(line, "md5_xor*"))
			    {
			        uint8_t hash[BSD_VAR_MD5];
			        int j;
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

			        md5(start, len, hash);
			        
			        for (j = 4; j < BSD_VAR_MD5; j += 4)
			        {
			        	hash[0] ^= hash[j];
			        	hash[1] ^= hash[j+1];
			        	hash[2] ^= hash[j+2];
			        	hash[3] ^= hash[j+3];
					}

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) hash, var->len);

    			    LOG("len %d MD5_XOR HASH = %X\n", len, *(uint32_t*)hash);
			    }

			    // set [*]:md5*
			    else if (wildcard_match_icase(line, "md5*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_MD5;
                    var->data = malloc(var->len);
			        md5(start, len, var->data);

    			    LOG("len %d MD5 HASH = %llx%llx\n", len, ((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1]);
			    }

			    // set [*]:md2*
			    else if (wildcard_match_icase(line, "md2*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_MD5;
                    var->data = malloc(var->len);
                    md(md_info_from_type(POLARSSL_MD_MD2), start, len, var->data);

    			    LOG("len %d MD2 HASH = %llx%llx\n", len, ((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1]);
			    }

			    // set [*]:md4*
			    else if (wildcard_match_icase(line, "md4*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_MD5;
                    var->data = malloc(var->len);
                    md(md_info_from_type(POLARSSL_MD_MD4), start, len, var->data);

    			    LOG("len %d MD4 HASH = %llx%llx\n", len, ((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1]);
				}

			    // set [*]:sha1*
			    else if (wildcard_match_icase(line, "sha1*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_SHA1;
                    var->data = malloc(var->len);
			        sha1(start, len, var->data);

    			    LOG("len %d SHA1 HASH = %llx%llx%x\n", len, ((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1], ((uint32_t*)var->data)[4]);
			    }

			    // set [*]:sha256*
			    else if (wildcard_match_icase(line, "sha256*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_SHA256;
                    var->data = malloc(var->len);
					sha2(start, len, var->data, 0);

    			    LOG("len %d SHA256 HASH = %llx%llx%llx%llx\n", len, ((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1], ((uint64_t*)var->data)[2], ((uint64_t*)var->data)[3]);
			    }

			    // set [*]:sha384*
			    else if (wildcard_match_icase(line, "sha384*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_SHA512;
                    var->data = malloc(var->len);
					sha4(start, len, var->data, 1);

    			    LOG("len %d SHA384 HASH = %llx%llx%llx%llx %llx%llx%llx%llx\n", len,
						((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1], ((uint64_t*)var->data)[2], ((uint64_t*)var->data)[3],
						((uint64_t*)var->data)[4], ((uint64_t*)var->data)[5], ((uint64_t*)var->data)[6], ((uint64_t*)var->data)[7]);
			    }

			    // set [*]:sha512*
			    else if (wildcard_match_icase(line, "sha512*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_SHA512;
                    var->data = malloc(var->len);
					sha4(start, len, var->data, 0);

    			    LOG("len %d SHA512 HASH = %llx%llx%llx%llx %llx%llx%llx%llx\n", len,
						((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1], ((uint64_t*)var->data)[2], ((uint64_t*)var->data)[3],
						((uint64_t*)var->data)[4], ((uint64_t*)var->data)[5], ((uint64_t*)var->data)[6], ((uint64_t*)var->data)[7]);
			    }

			    // set [*]:adler32*
			    else if (wildcard_match_icase(line, "adler32*"))
			    {
    			    uint32_t hash = adler32(0L, Z_NULL, 0);
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

    			    hash = adler32(hash, start, len);

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &hash, var->len);

    			    LOG("len %d Adler32 HASH = %X\n", len, hash);
			    }

			    // set [*]:adler16*
			    else if (wildcard_match_icase(line, "adler16*"))
			    {
			        //low priority - UNUSED
					LOG("Error: command not implemented");
					return 0;
			    }

			    // set [*]:hmac_sha1*
			    else if (wildcard_match_icase(line, "hmac_sha1*"))
			    {
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

                    var->len = BSD_VAR_SHA1;
                    var->data = malloc(var->len);
					sha1_hmac((u8*) TLOU_HMAC_KEY, strlen(TLOU_HMAC_KEY), start, len, var->data);

    			    LOG("len %d SHA1/HMAC HASH = %llx%llx%x\n", len, ((uint64_t*)var->data)[0], ((uint64_t*)var->data)[1], ((uint32_t*)var->data)[4]);
			    }

			    // set [*]:eachecksum*
			    else if (wildcard_match_icase(line, "eachecksum*"))
			    {
			        uint32_t hash;
    			    u8* start = (u8*)data + range_start;
    			    len = range_end - range_start;

    			    hash = MC02_hash(start, len);

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &hash, var->len);

    			    LOG("len %d EA/MC02 HASH = %X\n", len, hash);
			    }

			    // set [*]:qwadd(*,*)*
			    else if (wildcard_match_icase(line, "qwadd(*,*)*"))
			    {
			        //low priority
			        // qwadd(<start>,<endrange>)
			        // 64-bit	0xFFFFFFFFFFFFFFFF
					LOG("Error: command not implemented");
					return 0;
			    }

			    // set [*]:dwadd(*,*)*
			    else if (wildcard_match_icase(line, "dwadd(*,*)*"))
			    {
			        // dwadd(<start>,<endrange>)
			        // 32-bit	0xFFFFFFFF
			        int add_s, add_e;
			        uint32_t add = 0;

			        line += strlen("dwadd(");
    			    tmp = strchr(line, ',');
    			    *tmp = 0;
    			    
    			    add_s = _parse_int_value(line, pointer, dsize, var_list);

			        line = tmp+1;
    			    *tmp = ',';
    			    tmp = strchr(line, ')');
    			    *tmp = 0;

    			    add_e = _parse_int_value(line, pointer, dsize, var_list);

    			    *tmp = ')';
    			    
    			    char* read = data + add_s;
    			    
    			    while (read < data + add_e)
    			    {
    			    	add += (*(uint32_t*)read);
    			    	read += BSD_VAR_INT32;
    			    }

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &add, var->len);
    			    
    			    LOG("[%s]:dwadd(0x%X , 0x%X) = %X\n", var->name, add_s, add_e, add);
			    }

			    // set [*]:wadd(*,*)*
			    else if (wildcard_match_icase(line, "wadd(*,*)*"))
			    {
			        // wadd(<start>,<endrange>)
			        // 16-bit	0xFFFF
			        int add_s, add_e;
			        uint32_t add = 0;

			        line += strlen("wadd(");
    			    tmp = strchr(line, ',');
    			    *tmp = 0;
    			    
    			    add_s = _parse_int_value(line, pointer, dsize, var_list);

			        line = tmp+1;
    			    *tmp = ',';
    			    tmp = strchr(line, ')');
    			    *tmp = 0;

    			    add_e = _parse_int_value(line, pointer, dsize, var_list);

    			    *tmp = ')';
    			    
    			    char* read = data + add_s;
    			    
    			    while (read < data + add_e)
    			    {
    			    	add += (*(uint16_t*)read);
    			    	read += BSD_VAR_INT16;
    			    }
    			    
    			    if ((carry > 0) && (add > 0xFFFF))
    			    {
    			    	add = (add & 0x0000FFFF) + ((add & 0xFFFF0000) >> 8*carry);
    			    }

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &add, var->len);
    			    
    			    LOG("[%s]:wadd(0x%X , 0x%X) = %X\n", var->name, add_s, add_e, add);
			    }

			    // set [*]:add(*,*)*
			    else if (wildcard_match_icase(line, "add(*,*)*"))
			    {
			        // add(<start>,<endrange>)
			        // 8-bit	0xFF
			        int add_s, add_e;
			        uint32_t add = 0;

			        line += strlen("add(");
    			    tmp = strchr(line, ',');
    			    *tmp = 0;
    			    
    			    add_s = _parse_int_value(line, pointer, dsize, var_list);

			        line = tmp+1;
    			    *tmp = ',';
    			    tmp = strchr(line, ')');
    			    *tmp = 0;

    			    add_e = _parse_int_value(line, pointer, dsize, var_list);

    			    *tmp = ')';
    			    
    			    char* read = data + add_s;
    			    
    			    while (read <= data + add_e)
    			    {
    			    	add += (*(uint8_t*)read);
    			    	read += BSD_VAR_INT8;
    			    }

    			    if ((carry > 0) && (add > 0xFFFF))
    			    {
    			    	add = (add & 0x0000FFFF) + ((add & 0xFFFF0000) >> 8*carry);
    			    }

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &add, var->len);
    			    
    			    LOG("[%s]:add(0x%X , 0x%X) = %X\n", var->name, add_s, add_e, add);
			    }

			    // set [*]:wsub(*,*)*
			    else if (wildcard_match_icase(line, "wsub(*,*)*"))
			    {
			        // wsub(<start>,<endrange>)
			        // 16-bit	0xFFFF
					// sub()			byte		8-bit	0xFF
					// dwsub()			dbl word	32-bit	0xFFFFFFFF
					// qwsub()			quad word	64-bit	0xFFFFFFFFFFFFFFFF
			        int sub_s, sub_e;
			        uint32_t sub = 0;

			        line += strlen("wsub(");
    			    tmp = strchr(line, ',');
    			    *tmp = 0;
    			    
    			    sub_s = _parse_int_value(line, pointer, dsize, var_list);

			        line = tmp+1;
    			    *tmp = ',';
    			    tmp = strchr(line, ')');
    			    *tmp = 0;

    			    sub_e = _parse_int_value(line, pointer, dsize, var_list);

    			    *tmp = ')';
    			    
    			    char* read = data + sub_s;
    			    
    			    while (read < data + sub_e)
    			    {
    			    	sub += (*(uint16_t*)read);
    			    	read += BSD_VAR_INT16;
    			    }

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &sub, var->len);
    			    
    			    LOG("[%s]:wsub(0x%X , 0x%X) = %X\n", var->name, sub_s, sub_e, sub);
			    }

			    // set [*]:xor(*,*,*)*
			    else if (wildcard_match_icase(line, "xor(*,*,*)*"))
			    {
			        // xor(<start>,<endrange>,<incr>)
			        int xor_s, xor_e, xor_i;
			        uint8_t j, xor[4] = {0,0,0,0};

			        line += strlen("xor(");
    			    tmp = strchr(line, ',');
    			    *tmp = 0;
    			    
    			    xor_s = _parse_int_value(line, pointer, dsize, var_list);

			        line = tmp+1;
    			    *tmp = ',';
    			    tmp = strchr(line, ',');
    			    *tmp = 0;

    			    xor_e = _parse_int_value(line, pointer, dsize, var_list);

			        line = tmp+1;
    			    *tmp = ',';
    			    tmp = strchr(line, ')');
    			    *tmp = 0;

    			    xor_i = _parse_int_value(line, pointer, dsize, var_list);

    			    *tmp = ')';
    			    char* read = data + xor_s;
    			    
    			    while (read < data + xor_e)
    			    {
    			        for (j = 0; j < xor_i; j++)
    			            xor[j] ^= read[j];

    			        read += xor_i;
    			    }

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) xor, var->len);

    			    LOG("[%s]:XOR(0x%X , 0x%X, %d) = %X\n", var->name, xor_s, xor_e, xor_i, ((uint32_t*)var->data)[0]);
			    }

			    // set [*]:right(*,*)*
			    else if (wildcard_match_icase(line, "right(*,*)*"))
			    {
			        //low priority
					LOG("Error: command not implemented");
					return 0;
			    }

			    // set [*]:left(*)*
			    else if (wildcard_match_icase(line, "left(*)*"))
			    {
			        //low priority - UNUSED
					LOG("Error: command not implemented");
					return 0;
			    }

			    // set [*]:mid(*)*
			    else if (wildcard_match_icase(line, "mid(*)*"))
			    {
			        //low priority
					LOG("Error: command not implemented");
					return 0;
			    }

			    // set [*]:* (e.g. 0x00000000)
			    else
			    {
			        uint32_t tval;
                    sscanf(line, "%x", &tval);

                    var->len = BSD_VAR_INT32;
                    var->data = malloc(var->len);
                    memcpy(var->data, (u8*) &tval, var->len);

    			    LOG("[%s]:%s = %X\n", var->name, line, tval);
			    }
			        
			}

		}

        // write *:*
		else if (wildcard_match_icase(line, "write *:*"))
		{
			// write next / write at
			//
			// write at (67): 446966666963756C7479
			//
			// ; (67) = 0x43
			// ;Addresses enclosed in parenthesis are treated as decimal.
			//
			// The following 3 lines are equivalent:
			// write at (67): 446966666963756C7479
			// write at 43: "Difficulty"
			// write at 0x43: "Difficulty"
			//
			// write next 0: 446966666963756C7479
			// ;Overwrites 0 bytes after the file pointer address.
			//
			// write next 0: "Difficulty"
			// ;Overwrites with the text at 0 bytes after the found offset in this case "next 0" is at the found offset.
			// ;"next (10)" or "next A" would write 10 bytes after the found offset.
			//
			// write at 0x100:[anyname1]
			// ;Overwrites the content of the variable [anyname1] starting at offset 0x100.
   			int off, wlen;
			u8 from_pointer = 0;
			char* tmp = NULL;
			char* write_val = NULL;
			
			line += strlen("write");
		    skip_spaces(line);

			if (wildcard_match_icase(line, "at*"))
			{
			    from_pointer = 0;
			    line += strlen("at");
			}
			else if (wildcard_match_icase(line, "next*"))
			{
			    from_pointer = 1;
			    line += strlen("next");
			}
			else
			{
			    // invalid command
			    LOG("ERROR: Invalid write command");
			    return 0;
			}

		    skip_spaces(line);

		    tmp = strchr(line, ':');
		    *tmp = 0;

//		    sscanf(line, wildcard_match(line, "(*)") ? "(%d)" : "%x", &off);
			if (wildcard_match(line, "(*)"))
			    sscanf(line, "(%d)", &off);
			else
			    sscanf(line, "%x", &off);

			off += (from_pointer ? pointer : 0);

			line = tmp+1;
			*tmp = ':';

		    skip_spaces(line);

		    // write at/next *:xor:*
			if (wildcard_match_icase(line, "xor:*"))
			{
			    line += strlen("xor:");
    		    skip_spaces(line);

			    write_val = _decode_variable_data(line, &wlen, var_list);
			    
			    for (int i=0; i < wlen; i++)
			        write_val[i] ^= data[off + i];

				LOG(":xor:%s\n", line);
			}

			// write at/next *:repeat(*,*)*
			else if (wildcard_match_icase(line, "repeat(*,*)*"))
			{
				// repeat(<count>,<value>)
				int r_cnt, j;
				char* r_val;

				line += strlen("repeat(");
				tmp = strchr(line, ',');
				*tmp = 0;
				
				r_cnt = _parse_int_value(line, pointer, dsize, var_list);

				line = tmp+1;
				*tmp = ',';
				tmp = strchr(line, ')');
				*tmp = 0;

				r_val = _decode_variable_data(line, &wlen, var_list);

				*tmp = ')';
				
				write_val = malloc(r_cnt * wlen);
				
				for(j = 0; j < r_cnt; j++)
					memcpy(write_val + (j * wlen), r_val, wlen);

				free(r_val);
				wlen = r_cnt * wlen;

				LOG(":repeat(0x%X , %s)\n", r_cnt, line);
			}

		    // write at/next *:[*]
			else if (wildcard_match(line, "[*]"))
			{
			    LOG("Getting value for %s\n", line);
			    write_val = _decode_variable_data(line, &wlen, var_list);
			}

		    // write at/next *:* (e.g. 0x00000000)
			else
			{
			    write_val = _decode_variable_data(line, &wlen, var_list);
			}

			if (!write_val)
			{
			    // no data to write
			    LOG("ERROR: No data to write");
			    return 0;
			}

//			for (int i=0; i < wlen; i++)
//				LOG("%x", write_val[i]);

			char* write = data + off;
			memcpy(write, write_val, wlen);
			free(write_val);

            LOG("Wrote %d bytes (%s) to 0x%X\n", wlen, line, off);
		}

		// insert *:*		2		(2)
		else if (wildcard_match_icase(line, "insert *:*"))
		{
			// insert data
			// insert next / insert at
		}

		// delete *:*
		else if (wildcard_match_icase(line, "delete *:*"))
		{
			// delete data
			// delete next / delete at
   			int off, dlen;
			u8 from_pointer = 0;
			char* tmp = NULL;
			
			line += strlen("delete");
		    skip_spaces(line);

			if (wildcard_match_icase(line, "at*"))
			{
			    from_pointer = 0;
			    line += strlen("at");
			}
			else if (wildcard_match_icase(line, "next*"))
			{
			    from_pointer = 1;
			    line += strlen("next");
			}
			else
			{
			    // invalid command
			    LOG("ERROR: Invalid delete command");
			    return 0;
			}

		    skip_spaces(line);

		    tmp = strchr(line, ':');
		    *tmp = 0;

			if (wildcard_match(line, "(*)"))
			    sscanf(line, "(%d)", &off);
			else
			    sscanf(line, "%x", &off);

			off += (from_pointer ? pointer : 0);

			line = tmp+1;
			*tmp = ':';

		    skip_spaces(line);

			// delete at/next *:until*
			if (wildcard_match_icase(line, "until*"))
			{
			    line += strlen("until");
    		    skip_spaces(line);

				int flen;
			    char* find = _decode_variable_data(line, &flen, var_list);
			    
			    if (!find)
			    {
			    	LOG("Error: no data to search");
			    	return 0;
			    }
			    
			    dlen = search_data(data + off, dsize - off, find, flen, 1);
			    free(find);
			}
			else
			{
			    dlen = _parse_int_value(line, pointer, dsize, var_list);
			}

			char* write = data + off;
			dsize -= dlen;
			bcopy(write + dlen, write, dsize - off);

            LOG("Deleted %d bytes (%s) from 0x%X to 0x%X\n", dlen, line, off, off + dlen);
		}

        // search *
		else if (wildcard_match_icase(line, "search *"))
		{
			// search "difficulty"
			// ;Searches for the word "difficulty" as text.
			//
			// search 646966666963756C7479:1
			// ;":1" means repeat search 1 time (default). ":3" at the end would set the pointer to the 3rd occurrence of the searched text

			int cnt, len;
			char* find;
			char* tmp = NULL;

			line += strlen("search");
			if (wildcard_match(line, "*:*"))
			{
			    tmp = strrchr(line, ':');
			    sscanf(tmp+1, "%d", &cnt);
			    *tmp = 0;
			}
			else
			{
			    cnt = 1;
			}

		    skip_spaces(line);

			find = _decode_variable_data(line, &len, var_list);

		    if (!find)
		    {
		        // error decoding
				LOG("Error parsing search pattern!");
		        return 0;
		    }
			
			if (tmp)
			    *tmp = ':';

			LOG("Searching {%s} ...\n", line);
			pointer = search_data(data, dsize, find, len, cnt);
			
			if (pointer < 0)
			{
				LOG("ERROR: SEARCH PATTERN NOT FOUND");
				return 0;
			}
			
			LOG("POINTER = %ld (0x%lX)\n", pointer, pointer);

			free(find);
		}

		else if (wildcard_match_icase(line, "copy *"))
		{
			// copy data
			// UNUSED
		}

		else if (wildcard_match_icase(line, "compress *"))
		{
			// compress data
		}

		else if (wildcard_match_icase(line, "decompress *"))
		{
			// decompress data
		}
	    line = strtok(NULL, "\n");
    }

//	write_buffer(APOLLO_PATH "PAYLOAD.bin", (uint8_t*) data, dsize);

	write_buffer(filepath, (uint8_t*) data, dsize);
	free(data);

	// remove 0x00 from previous strtok(...)
    remove_char(code->codes, codelen, '\0');

    _cleanup_var_list(var_list);

	return 1;
}

int apply_ggenie_patch_code(const char* filepath, code_entry_t* code)
{
    char *data;
	size_t dsize;
	long pointer = 0;
	char tmp3[4], tmp4[5], tmp6[7], tmp8[9];
	int codelen = strlen(code->codes);
    char *line = strtok(code->codes, "\n");
	
	LOG("Applying [%s] to '%s'...", code->name, filepath);
	read_buffer(filepath, (uint8_t**) &data, &dsize);

//	write_buffer(APOLLO_PATH "PAYLOAD.src", (uint8_t*) data, dsize);

    while (line)
    {
    	switch (line[0])
    	{
    		case '0':
    			//	8-bit write
    			//	0TXXXXXX 000000YY
    		case '1':
    			//	16-bit write
    			//	1TXXXXXX 0000YYYY
    		case '2':
    			//	32-bit write
    			//	2TXXXXXX YYYYYYYY
    			//	X= Address/Offset
    			//	Y= Value to write
    			//	T=Address/Offset type (0 = Normal / 8 = Offset From Pointer)
    		{
    			int off;
    			uint32_t val;
				uint8_t bytes = 1 << (line[0] - 0x30);

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off);
    			off += (line[1] == '8' ? pointer : 0);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);

    			char* write = data + off;
    			memcpy(write, (char*) &val + (4 - bytes), bytes);

    			LOG("Wrote %d bytes (%s) to 0x%X", bytes, tmp8 + (8 - bytes*2), off);
    		}
    			break;

    		case '4':
    			//	multi write
    			//	4TXXXXXX YYYYYYYY
    			//	4NNNWWWW VVVVVVVV
    			//	X= Address/Offset
    			//	Y= Value to write (Starting)
    			//	N=Times to Write
    			//	W=Increase Address By
    			//	V=Increase Value By
    			//	T=Address/Offset type
    			//	Normal/Pointer
    			//	0 / 8 = 8bit
    			//	1 / 9 = 16bit
    			//	2 / A = 32bit
    		{
    			int i, off, n, incoff;
    			uint32_t val, incval;
    			char t = line[1];
    			char* write;

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off);
    			off += ((t == '8' || t == '9' || t == 'A') ? pointer : 0);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);

			    line = strtok(NULL, "\n");

    			sprintf(tmp3, "%.3s", line+1);
    			sscanf(tmp3, "%x", &n);

    			sprintf(tmp4, "%.4s", line+4);
    			sscanf(tmp4, "%x", &incoff);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &incval);
			
				LOG("Multi-write at (0x%X) %d times, inc-addr (%d) inc-val (%X)", off, n, incoff, incval);

				for (i = 0; i < n; i++)
				{
	    			write = data + off + (incoff * i);

					switch (t)
					{
						case '0':
						case '8':
			    			memcpy(write, (char*) &val +3, 1);
			    			LOG("M-Wrote 1 byte (%02X) to 0x%lX", val, write - data);
							break;

						case '1':
						case '9':
			    			memcpy(write, (char*) &val +2, 2);
			    			LOG("M-Wrote 2 bytes (%04X) to 0x%lX", val, write - data);
							break;

						case '2':
						case 'A':
			    			memcpy(write, (char*) &val, 4);
			    			LOG("M-Wrote 4 bytes (%08X) to 0x%lX", val, write - data);
							break;
					}

	    			val += incval;
				}

//    			LOG("%s\n", line);
    		}
    			break;

    		case '5':
    			//	copy bytes
    			//	5TXXXXXX ZZZZZZZZ
    			//	5TYYYYYY 00000000
    			//	  XXXXXX = Offset to copy from
    			//	  YYYYYY = Offset to copy to
    			//	         ZZZZZZZZ = Number of bytes to copy
    			//	 T = Bit Size
    			//	 0 = From start of the data
    			//	 8 = From found from a search
    		{
    			int off_src, off_dst;
    			uint32_t val;

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off_src);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);

    			char* src = data + off_src + (line[1] == '8' ? pointer : 0);

			    line = strtok(NULL, "\n");

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off_dst);
    			
    			char* dst = data + off_dst + (line[1] == '8' ? pointer : 0);

    			memcpy(dst, src, val);
				LOG("Copied %d bytes from 0x%lX to 0x%lX", val, src, dst);
    		}
    			break;

    		case '6':
    			//	special mega code
    			//	6TWX0Y0Z VVVVVVVV <- Code Type 6
    			//	6 = Type 6: Pointer codes
    			//	T = Data size of VVVVVVVV: 0:8bit, 1:16bit, 2:32bit, search-> 8:8bit, 9:16bit, A:32bit
    			//	W = operator:
    			//	      0 = Read "address" from file
    			//	      1X = Move pointer from obtained address ?? (X = 0:add, 1:substract, 2:multiply)
    			//	      2X = Move pointer ?? (X = 0:add, 1:substract, 2:multiply)
    			//	      4X = Write value: X=0 at read address, X=1 at pointer address
    			//	Y = flag relative to read add (very tricky to understand)
    			//	Z = flag relative to current pointer (very tricky to understand)
    			//	V = Data
    		{
    		}
    			break;

    		case '7':
    			//	Add Write
    			//	7TXXXXXX YYYYYYYY 
    			//	 T: 0=1byte/1=2byte/2=4byte
    			//	T=Address/Offset type
    			//	Normal/Pointer
    			//	0 / 8 = 8bit
    			//	1 / 9 = 16bit
    			//	2 / A = 32bit
    			//	X= Address/Offset
    			//	Y=Increase Value By
    		{
    			int off;
    			uint32_t val;
    			char t = line[1];

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off);
				off += ((t == '8' || t == '9' || t == 'A') ? pointer : 0);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);

    			char* write = data + off;

				switch (t)
				{
					case '0':
					case '8':
						val += (uint8_t) write[0];
		    			memcpy(write, (char*) &val +3, 1);
		    			LOG("Add-Wrote 1 byte (%02X) to 0x%X", val, off);
						break;
					case '1':
					case '9':
						val += ((uint16_t*) write)[0];
		    			memcpy(write, (char*) &val +2, 2);
		    			LOG("Add-Wrote 2 bytes (%04X) to 0x%X", val, off);
						break;
					case '2':
					case 'A':
						val += ((uint32_t*) write)[0];
		    			memcpy(write, (char*) &val, 4);
		    			LOG("Add-Wrote 4 bytes (%08X) to 0x%X", val, off);
						break;
				}

    		}
    			break;

    		case '8':
    			//	Search Type
    			//	8ZZZXXXX YYYYYYYY
    			//	Z= Amount of times to find before Write
    			//	X= Amount of data to Match
    			//	Y= Seach For (note can be xtended for more just continue it like YYYYYYYY YYYYYYYY under it)
    			//	Once u have your Seach type done then place one of the standard code types under it with setting T to the Pointer type
    		{
    			int i, cnt, len;
    			uint32_t val;
    			char* find;

    			sprintf(tmp3, "%.3s", line+1);
    			sscanf(tmp3, "%x", &cnt);

    			sprintf(tmp4, "%.4s", line+4);
    			sscanf(tmp4, "%x", &len);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);

    			find = malloc(len);

				memcpy(find, (char*) &val, 4);
    			
    			for (i=4; i < len; i += 8)
    			{
				    line = strtok(NULL, "\n");

					sprintf(tmp8, "%.8s", line);
	    			sscanf(tmp8, "%x", &val);

					memcpy(find + i, (char*) &val, 4);

					sprintf(tmp8, "%.8s", line+9);
	    			sscanf(tmp8, "%x", &val);

					if (i+4 < len)
						memcpy(find + i+4, (char*) &val, 4);
    			}

//				for (i=0; i < len; i++)
//					LOG("%c", find[i]);

				pointer = search_data(data, dsize, find, len, cnt);
				
				if (pointer < 0)
				{
					LOG("SEARCH PATTERN NOT FOUND");
					free(find);
					free(data);

					return 0;
				}

				LOG("Search pointer = %ld (0x%lX)", pointer, pointer);

    			free(find);
    		}
    			break;

    		case '9':
    			//	Move pointer to offset in address XXXXXXXXX (CONFIRMED CODE)
    			//	90000000 XXXXXXXX
    			//	---
    			//	Step Forward Code (CONFIRMED CODE)
    			//	92000000 XXXXXXXX
    			//	---
    			//	Step Back Code (CONFIRMED CODE)
    			//	93000000 XXXXXXXX
    			//	---
    			//	Step Back From End of File Code (CONFIRMED CODE)
    			//	94000000 XXXXXXXX
    		{
    			uint32_t off, val;

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &off);

				switch (line[1])
				{
					case '0':
						val = *(uint32_t*)(data + off);
						pointer = val;
						break;
					case '2':
						pointer += off;
						break;
					case '3':
						pointer -= off;
						break;
					case '4':
						pointer = dsize - off;
						break;
				}
    		}
    			break;

    		case 'A':
    			//	Multi-write
    			//	Axxxxxxx 0000yyyy  (xxxxxxxx = address, yyyy = size)
    			//	zzzzzzzz zzzzzzzz  <-data to write at address
    		{
    			int i, off;
    			uint32_t val, size;
    			char* write;

    			sprintf(tmp8, "%.7s", line+1);
    			sscanf(tmp8, "%x", &off);

    			sprintf(tmp4, "%.4s", line+13);
    			sscanf(tmp4, "%x", &size);

				for (i = 0; i < size; i += 8)
				{
				    line = strtok(NULL, "\n");

    				sprintf(tmp8, "%.8s", line);
    				sscanf(tmp8, "%x", &val);

	    			write = data + off + i; //+ ((t == '8' || t == '9' || t == 'A') ? pointer : 0);
	    			memcpy(write, (char*) &val, 4);
					LOG("m-Wrote 4 bytes (%s) to 0x%lX", tmp8, write - data);

    				sprintf(tmp8, "%.8s", line+9);
    				sscanf(tmp8, "%x", &val);

	    			write += 4;
					if (i + 4 < size)
					{
		    			memcpy(write, (char*) &val, 4);
		    			LOG("m-Wrote 4 bytes (%s) to 0x%lX", tmp8, write - data);
					}

				}

    		}
    			break;
    		default:
    			break;
    	}
	    line = strtok(NULL, "\n");
    }

//	write_buffer(APOLLO_PATH "PAYLOAD.bin", (uint8_t*) data, dsize);

	write_buffer(filepath, (uint8_t*) data, dsize);
	free(data);

	// remove 0x00 from previous strtok(...)
    remove_char(code->codes, codelen, '\0');

	return 1;
}

int apply_cheat_patch_code(const char* fpath, code_entry_t* code)
{
	if (code->type == PATCH_GAMEGENIE)
	{
		LOG("Game Genie Code");
		return apply_ggenie_patch_code(fpath, code);
	}

	if (code->type == PATCH_BSD)
	{
		LOG("Bruteforce Save Data Code");
		return apply_bsd_patch_code(fpath, code);
	}

	return 0;
}
