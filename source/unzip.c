#include <zip.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "saves.h"
#include "common.h"

static inline uint64_t min64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

int _mkdirs(const char* dir)
{
    char path[256];
    snprintf(path, sizeof(path), "%s", dir);
    LOG("mkdirs for %s", path);
    char* ptr = path;
    ptr++;
    while (*ptr)
    {
        while (*ptr && *ptr != '/')
        {
            ptr++;
        }
        char last = *ptr;
        *ptr = 0;

        if (dir_exists(path) == FAILED)
        {
            LOG("mkdir %s", path);
            int err = mkdir(path, 0777);
            if (err < 0)
            {
                LOG("mkdir %s err=0x%08x", path, (uint32_t)err);
                return 0;
            }
        }
        
        *ptr++ = last;
        if (last == 0)
        {
            break;
        }
    }

    return 1;
}

int extract_zip(const char* zip_file, const char* dest_path)
{
	char path[256];
	struct zip* archive = zip_open(zip_file, ZIP_CHECKCONS, NULL);
	int files = zip_get_num_files(archive);

	if (files <= 0) {
		LOG("Empty ZIP file.");
		return 0;
	}

	uint64_t progress = 0;
	init_progress_bar("Extracting files...", " ");

	LOG("Installing ZIP to <%s>...", dest_path);

	for (int i = 0; i < files; i++) {
		progress++;
		const char* filename = zip_get_name(archive, i, 0);

		update_progress_bar(&progress, files, filename);

		if (!filename)
			continue;

		if (filename[0] == '/')
			filename++;

		snprintf(path, sizeof(path)-1, "%s%s", dest_path, filename);

		if (filename[strlen(filename) - 1] == '/') {
			if (!_mkdirs(path))
				return 0;

			continue;
		}

		struct zip_stat st;
		if (zip_stat_index(archive, i, 0, &st)) {
			LOG("Unable to access file %s in zip.", filename);
			continue;
		}
		struct zip_file* zfd = zip_fopen_index(archive, i, 0);
		if (!zfd) {
			LOG("Unable to open file %s in zip.", filename);
			continue;
		}

		FILE* tfd = fopen(path, "wb");
		if(!tfd) {
			zip_fclose(zfd);
            zip_close(archive);
			end_progress_bar();
            LOG("Error opening temporary file.");
            return 0;
		}

		uint64_t pos = 0, count;
		uint8_t* buffer = malloc(0x1000);
		while (pos < st.size) {
			count = min64(0x1000, st.size - pos);
			if (zip_fread(zfd, buffer, count) != count) {
				free(buffer);
                fclose(tfd);
                zip_fclose(zfd);
                zip_close(archive);
				end_progress_bar();
                LOG("Error reading from zip.");
                return 0;
			}

			fwrite(buffer, count, 1, tfd);
			pos += count;
		}
		free(buffer);

		zip_fclose(zfd);
		fclose(tfd);

	}

	if (archive) {
		zip_close(archive);
	}

	end_progress_bar();

	return 1;
}
