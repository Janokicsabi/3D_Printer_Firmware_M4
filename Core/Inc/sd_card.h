/*
 * sd_card.h
 *
 *  Created on: Oct 3, 2023
 *      Author: janok
 */

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_

#include "fatfs.h"

class SD_card {
private:
	FATFS sd_fs;
	FIL file;
	bool is_file_opened;
	void sd_card_error_handler();

public:
	SD_card();
	~SD_card();
	bool mount_sd_card();
	bool unmount_sd_card();
	bool get_free_sector_size();
	bool open_file(const char* file_name);
	bool close_file();
	bool read_one_line(char* file_content, uint32_t buff_len);
	FIL* get_file();

};


#endif /* INC_SD_CARD_H_ */
