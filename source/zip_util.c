#include <zip.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <un7zip.h>
#include <unrar.h>

#include "saves.h"
#include "common.h"

#define UNZIP_BUF_SIZE 0x80000

static inline uint64_t min64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

static void walk_zip_directory(const char* startdir, const char* inputdir, struct zip *zipper)
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
	file_chmod(output_filename);

    return (ret == ZIP_ER_OK);
}

int zip_savegame(const char* folder, const char* inputdir, const char* output_filename)
{
	char fullname[256];
	struct dirent *dirp;
	struct zip* archive;
	DIR *dp = opendir(inputdir);

	if (!dp) {
		LOG("Failed to open input directory: '%s'", inputdir);
		return 0;
	}

	unlink_secure(output_filename);
	archive = zip_open(output_filename, ZIP_CREATE, NULL);
	if (!archive) {
		LOG("Failed to create zip file '%s'", output_filename);
		closedir(dp);
		return 0;
	}

	LOG("Zipping <%s> to %s...", inputdir, output_filename);
	LOG("Adding folder '%s'", folder);
	zip_add_dir(archive, folder);

	while ((dirp = readdir(dp)) != NULL) {
		if ((strcmp(dirp->d_name, ".")  == 0) || (strcmp(dirp->d_name, "..") == 0))
			continue;

		snprintf(fullname, sizeof(fullname), "%s%s", inputdir, dirp->d_name);
		struct zip_source *source = zip_source_file(archive, fullname, 0, 0);
		if (!source) {
			LOG("Failed to source file to zip: %s", fullname);
			continue;
		}

		snprintf(fullname, sizeof(fullname), "%s/%s", folder, dirp->d_name);
		LOG("Adding file '%s'", fullname);
		if (zip_add(archive, fullname, source) < 0) {
			zip_source_free(source);
			LOG("Failed to add file to zip: %s", fullname);
		}
	}
	closedir(dp);

	return (zip_close(archive) == ZIP_ER_OK);
}

int zip_file(const char* input_file, const char* output_filename)
{
	char *fname;
	struct zip* archive;

	unlink_secure(output_filename);
	archive = zip_open(output_filename, ZIP_CREATE, NULL);
	if (!archive) {
		LOG("Failed to create zip file '%s'", output_filename);
		return 0;
	}

	LOG("Zipping <%s> to %s...", input_file, output_filename);
	struct zip_source *source = zip_source_file(archive, input_file, 0, 0);
	if (!source) {
		LOG("Failed to source file to zip: %s", input_file);
		return 0;
	}

	fname = strrchr(input_file, '/');
	if (!fname)
		fname = (char*)input_file;
	else
		fname++;

	LOG("Adding file '%s'", fname);
	if (zip_add(archive, fname, source) < 0) {
		zip_source_free(source);
		LOG("Failed to add file to zip: %s", input_file);
	}

	return (zip_close(archive) == ZIP_ER_OK);
}

int extract_zip(const char* zip_file, const char* dest_path)
{
	char path[256];
	uint8_t* buffer;
	struct zip* archive = zip_open(zip_file, ZIP_CHECKCONS, NULL);
	int files = zip_get_num_files(archive);

	if (files <= 0) {
		LOG("Empty ZIP file.");
		return 0;
	}

	buffer = malloc(UNZIP_BUF_SIZE);
	if (!buffer)
		return 0;

	init_progress_bar("Extracting files...", zip_file);

	LOG("Extracting %s to <%s>...", zip_file, dest_path);

	for (int i = 0; i < files; i++) {
		const char* filename = zip_get_name(archive, i, 0);

		update_progress_bar(i+1, files, filename);
		LOG("Unzip [%d/%d] '%s'...", i+1, files, filename);

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
			free(buffer);
			zip_fclose(zfd);
            zip_close(archive);
			end_progress_bar();
            LOG("Error opening temporary file '%s'.", path);
            return 0;
		}

		uint64_t pos = 0, count;
		while (pos < st.size) {
			count = min64(UNZIP_BUF_SIZE, st.size - pos);
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

		zip_fclose(zfd);
		fclose(tfd);
		file_chmod(path);
	}

	if (archive) {
		zip_close(archive);
	}

	end_progress_bar();
	free(buffer);

	return 1;
}

static void callback_7z(const char* fileName, unsigned long fileSize, uint32_t fileNum, uint32_t numFiles)
{
    LOG("Extracted: %s (%ld bytes)", fileName, fileSize);
    update_progress_bar(fileNum, numFiles, fileName);
}

int extract_7zip(const char* fpath, const char* dest_path)
{
	int ret;

	LOG("Extracting 7-Zip (%s) to <%s>...", fpath, dest_path);
	init_progress_bar("Extracting files...", fpath);

	// Extract 7-Zip archive contents
	ret = Extract7zFileEx(fpath, dest_path, &callback_7z, UNZIP_BUF_SIZE);
	end_progress_bar();

	return (ret == SUCCESS);
}

int extract_rar(const char* rarFilePath, const char* dstPath)
{
	int err = 0;
	uint64_t progress = 0, numFiles = 0;
	HANDLE hArcData; //Archive Handle
	struct RAROpenArchiveDataEx rarOpenArchiveData;
	struct RARHeaderDataEx rarHeaderData;

	memset(&rarOpenArchiveData, 0, sizeof(rarOpenArchiveData));
	memset(&rarHeaderData, 0, sizeof(rarHeaderData));
	rarOpenArchiveData.ArcName = (char*) rarFilePath;
	rarOpenArchiveData.OpenMode = RAR_OM_LIST;

	hArcData = RAROpenArchiveEx(&rarOpenArchiveData);
	if (rarOpenArchiveData.OpenResult != ERAR_SUCCESS)
	{
		LOG("OpenArchive '%s' Failed!", rarOpenArchiveData.ArcName);
		return 0;
	}

	while (RARReadHeaderEx(hArcData, &rarHeaderData) == ERAR_SUCCESS)
	{
		if (RARProcessFile(hArcData, RAR_SKIP, NULL, NULL) == ERAR_SUCCESS)
			numFiles++;
	}

	RARCloseArchive(hArcData);
	rarOpenArchiveData.OpenMode = RAR_OM_EXTRACT;
	hArcData = RAROpenArchiveEx(&rarOpenArchiveData);

	LOG("UnRAR Extract %s to '%s'...", rarFilePath, dstPath);
	init_progress_bar("Extracting files...", rarFilePath);

	while (RARReadHeaderEx(hArcData, &rarHeaderData) == ERAR_SUCCESS)
	{
		LOG("Extracting '%s' (%ld bytes)", rarHeaderData.FileName, rarHeaderData.UnpSize + (((uint64_t)rarHeaderData.UnpSizeHigh) << 32));
		update_progress_bar(progress++, numFiles, rarHeaderData.FileName);

		if (RARProcessFile(hArcData, RAR_EXTRACT, (char*) dstPath, NULL) != ERAR_SUCCESS)
		{
			err++;
			LOG("ERROR: UnRAR Extract Failed!");
			continue;
		}
		update_progress_bar(progress, numFiles, rarHeaderData.FileName);
	}
	end_progress_bar();

	RARCloseArchive(hArcData);
	return (err == ERAR_SUCCESS);
}
