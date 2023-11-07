/*
 * sd_card.c
 *
 *  Created on: Sep 25, 2023
 *      Author: janok
 */

#include "fatfs.h"
#include "sd_card.h"


SD_card::SD_card() {
	mount_sd_card();
	is_file_opened = false;
}

SD_card::~SD_card() {
	unmount_sd_card();
	if (is_file_opened) {
		close_file();
	}
}

bool SD_card::mount_sd_card() {
	FRESULT fres = f_mount(&sd_fs, "0:", 1);
	if (fres != FR_OK) {
		sd_card_error_handler();
		return false;
	}
	return true;
}

bool SD_card::unmount_sd_card() {
	FRESULT fres = f_mount(NULL, "0:", 0);
	if (fres != FR_OK) {
		sd_card_error_handler();
		return false;
	}
	return true;
}

bool SD_card::get_free_sector_size() {
	DWORD free_clusters, free_sectors, total_sectors;
	FATFS* getFreeFs;
	FRESULT fres = f_getfree("", &free_clusters, &getFreeFs);

	total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
	free_sectors = free_clusters * getFreeFs->csize;
	if (fres != FR_OK) {
		sd_card_error_handler();
		return false;
	}
	return true;
}

bool SD_card::open_file(const char* file_name) {
	FRESULT fres = f_open(&file, file_name, FA_READ);
	if (fres != FR_OK) {
		sd_card_error_handler();
		return false;
	}
	is_file_opened = true;
	return true;
}

bool SD_card::close_file() {
	FRESULT fres = f_close(&file);
	if (fres != FR_OK) {
		sd_card_error_handler();
		return false;
	}
	return true;
}

bool SD_card::read_one_line(char* file_content, uint32_t buff_len) {
	if (!is_file_opened) {
		sd_card_error_handler();
		return false;
	}
	if (f_gets(file_content, buff_len, &file) == nullptr) {
		sd_card_error_handler();
		return false;
	}
	return true;
}

FIL* SD_card::get_file() {
	return &file;
}

void SD_card::sd_card_error_handler() {
	//Error! Coulnd't mount/read sd card
}

