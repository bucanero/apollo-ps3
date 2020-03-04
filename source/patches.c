#include "saves.h"
#include "util.h"

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

int apply_bsd_patch_code(const char* fpath, const char* fname, code_entry_t* code)
{
	return 0;
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
    			//	X= Address/Offset
    			//	Y= Value to write
    			//	T=Address/Offset type (0 = Normal / 8 = Offset From Pointer)
    		{
    			int off;
    			uint32_t val;

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);
//			val = ES32(val);

    			char* write = data + off + (line[1] == '8' ? pointer : 0);
    			memcpy(write, (char*) &val +3, 1);

    			LOG("Wrote 1 byte (%s) to 0x%lX", tmp8, write - data);
    		}
    			break;
    		case '1':
    			//	16-bit write
    			//	1TXXXXXX 0000YYYY
    			//	X= Address/Offset
    			//	Y= Value to write
    			//	T=Address/Offset type (0 = Normal / 8 = Offset From Pointer)
    		{
    			int off;
    			uint32_t val;

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);
//			val = ES32(val);

    			char* write = data + off + (line[1] == '8' ? pointer : 0);
    			memcpy(write, (char*) &val +2, 2);

    			LOG("Wrote 2 bytes (%s) to 0x%lX", tmp8, write - data);
    		}
    			break;
    		case '2':
    			//	32-bit write
    			//	2TXXXXXX YYYYYYYY
    			//	X= Address/Offset
    			//	Y= Value to write
    			//	T=Address/Offset type (0 = Normal / 8 = Offset From Pointer)
    		{
    			int off;
    			uint32_t val;

    			sprintf(tmp6, "%.6s", line+2);
    			sscanf(tmp6, "%x", &off);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);
//			val = ES32(val);

    			char* write = data + off + (line[1] == '8' ? pointer : 0);
    			memcpy(write, (char*) &val, 4);

    			LOG("Wrote 4 bytes (%s) to 0x%lX", tmp8, write - data);
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

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);

			    line = strtok(NULL, "\n");

    			sprintf(tmp3, "%.3s", line+1);
    			sscanf(tmp3, "%x", &n);

    			sprintf(tmp4, "%.4s", line+4);
    			sscanf(tmp4, "%x", &incoff);

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &incval);
			
				for (i = 0; i < n; i++)
				{
	    			write = data + off + (incoff * i) + ((t == '8' || t == '9' || t == 'A') ? pointer : 0);
//			val = ES32(val);

					switch (t)
					{
						case '0':
						case '8':
			    			memcpy(write, (char*) &val +3, 1);
							break;

						case '1':
						case '9':
			    			memcpy(write, (char*) &val +2, 2);
							break;

						case '2':
						case 'A':
			    			memcpy(write, (char*) &val, 4);
							break;
					}

//			val = ES32(val);
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

    			sprintf(tmp8, "%.8s", line+9);
    			sscanf(tmp8, "%x", &val);
//			val = ES32(val);

    			char* write = data + off + ((t == '8' || t == '9' || t == 'A') ? pointer : 0);
//				val += ((uint32_t*) write)[0];

				switch (t)
				{
					case '0':
					case '8':
						val += (uint8_t) write[0];
		    			memcpy(write, (char*) &val +3, 1);
						break;
					case '1':
					case '9':
						val += ((uint16_t*) write)[0];
		    			memcpy(write, (char*) &val +2, 2);
						break;
					case '2':
					case 'A':
						val += ((uint32_t*) write)[0];
		    			memcpy(write, (char*) &val, 4);
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
//			val = ES32(val);

    			find = malloc(len);

				memcpy(find, (char*) &val, 4);
    			
    			for (i=4; i < len; i += 8)
    			{
				    line = strtok(NULL, "\n");

					sprintf(tmp8, "%.8s", line);
	    			sscanf(tmp8, "%x", &val);
//			val = ES32(val);

					memcpy(find + i, (char*) &val, 4);

					sprintf(tmp8, "%.8s", line+9);
	    			sscanf(tmp8, "%x", &val);
//			val = ES32(val);

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

					return -1;
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
//			val = ES32(val);
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
//			val = ES32(val);
	    			memcpy(write, (char*) &val, 4);

    				sprintf(tmp8, "%.8s", line+9);
    				sscanf(tmp8, "%x", &val);

	    			write += 4;
//			val = ES32(val);
					if (i + 4 < size)
		    			memcpy(write, (char*) &val, 4);

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
