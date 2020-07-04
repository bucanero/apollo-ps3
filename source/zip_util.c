#include <zip.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#include "saves.h"
#include "common.h"

static inline uint64_t min64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

void walk_zip_directory(const char* startdir, const char* inputdir, struct zip *zipper)
{
	char fullname[256];	
	struct dirent *dirp;
	int len = strlen(startdir) + 1;
	DIR *dp = opendir(inputdir);

	if (!dp) {
		LOG("Failed to open input directory: '%s'", inputdir);
		return;
	}

	if (strlen(inputdir) > len)
	{
		LOG("Adding folder '%s'", inputdir+len);
		if (zip_add_dir(zipper, inputdir+len) < 0)
		{
			LOG("Failed to add directory to zip: %s", inputdir);
			return;
		}
	}

	while ((dirp = readdir(dp)) != NULL) {
		if ((strcmp(dirp->d_name, ".")  != 0) && (strcmp(dirp->d_name, "..") != 0)) {
  			snprintf(fullname, sizeof(fullname), "%s%s", inputdir, dirp->d_name);

  			if (dirp->d_type == DT_DIR) {
    			strcat(fullname, "/");
    			walk_zip_directory(startdir, fullname, zipper);
  			} else {
    			struct zip_source *source = zip_source_file(zipper, fullname, 0, 0);
    			if (!source) {
      				LOG("Failed to source file to zip: %s", fullname);
      				continue;
    			}
    			LOG("Adding file '%s'", fullname+len);
    			if (zip_add(zipper, &fullname[len], source) < 0) {
      				zip_source_free(source);
      				LOG("Failed to add file to zip: %s", fullname);
    			}
  			}
		}
	}
	closedir(dp);
}

int zip_directory(const char* basedir, const char* inputdir, const char* output_filename)
{
    int ret;
    struct zip* archive = zip_open(output_filename, ZIP_CREATE | ZIP_EXCL, &ret);

    LOG("Zipping <%s> to %s...", inputdir, output_filename);
    if (!archive) {
        LOG("Failed to open output file '%s'", output_filename);
        return 0;
    }

    walk_zip_directory(basedir, inputdir, archive);
    ret = zip_close(archive);

    return (ret == ZIP_ER_OK);
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
		mkdirs(path);

		if (filename[strlen(filename) - 1] == '/')
			continue;

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
            LOG("Error opening temporary file '%s'.", path);
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
