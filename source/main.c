#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <switch.h>

#define BCT_KEY_OFF 0x210
#define BCT_SZ 0x4000
#define NUM_KBS 32
#define KB_SZ 0xB0

#define RCM_XOR 0x77

int toggle_rcm() {
	FsStorage store;
	int boot0_part = 0;
	Result rc = fsOpenBisStorage(&store, boot0_part);
	printf("Result1: %d\n", rc);
	u64 size = 0;
	fsStorageGetSize(&store, &size);
	if(size==0) {
		printf("size is zero for some gay reason\n");
		return -1;
	}
	printf("BOOT0 size: %ld\n", size);
	
	char * buf = malloc(size);
	Result rc2 = fsStorageRead(&store, 0, buf, size);
	printf("Result2: %d\n", rc2);
	for(int i=0; i< 4; i++) {
		int off = BCT_KEY_OFF + i*BCT_SZ;
		buf[off]  ^= RCM_XOR;
	}
	/* This code logs the modified boot0 to SD file for checking it.
	FILE * fp = fopen("sdmc:/altboot0.bin", "wb");
	if(fp==NULL) {
		printf("fp is null wtf\n");
		return -2;
	}
	fwrite(buf, 1, size, fp);
	fclose(fp); */
	fsStorageWrite(&store, 0, buf, size);
	fsStorageClose(&store);
	free(buf);
	return 0;
}

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(NULL);

	printf("press L+X to toggle autorcm...\n");
	printf("If RCM is already enabled this will disable it\n");
    // Main loop
    while(appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        u64 kDown = hidKeysHeld(CONTROLLER_P1_AUTO);
		
		if((kDown & KEY_X) && (kDown & KEY_L)) {
			toggle_rcm();
			printf("FINISHED! Press B to exit...\n");
			while(!(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_B)) {
				hidScanInput();
			}
			break;
		}

        if (kDown & KEY_PLUS) break; // break in order to return to hbmenu

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    gfxExit();
    return 0;
}

