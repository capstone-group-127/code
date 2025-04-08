#include "sdcard.h"
#include "main.h"
#include "spi.h"

FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];

/* initialization function */
void sdcard_init(void)
{
	MX_FATFS_Init();
}

void sdcard_mount(void) {
	f_mount(&fs, "", 1);
}

void sdcard_log(char *message) {
	f_open(&fil, "log.txt", FA_OPEN_ALWAYS | FA_WRITE);
	f_write(&fil, message, strlen(message), &fres);
	f_close(&fil);
}

void sdcard_unmount(void) {
	f_mount(NULL, "", 1);
}
