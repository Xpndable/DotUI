// miyoomini/keymon.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>

#include <msettings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pthread.h>

//	Button Defines
#define	BUTTON_MENU		KEY_ESC
#define	BUTTON_POWER	KEY_POWER
#define	BUTTON_SELECT	KEY_RIGHTCTRL
#define	BUTTON_START	KEY_ENTER
#define	BUTTON_L1		KEY_E
#define	BUTTON_R1		KEY_T
#define	BUTTON_L2		KEY_TAB
#define	BUTTON_R2		KEY_BACKSPACE
#define BUTTON_VOLUP	KEY_VOLUMEUP
#define BUTTON_VOLDOWN	KEY_VOLUMEDOWN

//	for keyshm
#define VOLUME		0
#define BRIGHTNESS	1
#define VOLMAX		20
#define BRIMAX		10

//	for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

//	for button_flag
#define SELECT_BIT	0
#define START_BIT	1
#define SELECT		(1<<SELECT_BIT)
#define START		(1<<START_BIT)

//	for DEBUG
//#define	DEBUG
#ifdef	DEBUG
#define ERROR(str)	fprintf(stderr,str"\n"); quit(EXIT_FAILURE)
#else
#define ERROR(str)	quit(EXIT_FAILURE)
#endif

//	Global Variables
typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)

static SAR_ADC_CONFIG_READ  adc_config = {0,0};
static int is_charging = 0;
static int eased_charge = 0;
static int sar_fd = 0;
static struct input_event	ev;
static int	input_fd = 0;
static pthread_t adc_pt;

void quit(int exitcode) {
	pthread_cancel(adc_pt);
	pthread_join(adc_pt, NULL);
	QuitSettings();
	
	if (input_fd > 0) close(input_fd);
	if (sar_fd > 0) close(sar_fd);
	exit(exitcode);
}

static int isCharging(void) {
    // Code adapted from OnionOS
	char *cmd = "cd /customer/app/ ; ./axp_test";  
	int batJsonSize = 100;
	char buf[batJsonSize];
	int charge_number;
	int result;

	FILE *fp;      
	fp = popen(cmd, "r");
	if (fgets(buf, batJsonSize, fp) != NULL) {
		sscanf(buf,  "{\"battery\":%*d, \"voltage\":%*d, \"charging\":%d}", &charge_number);
		result = (charge_number==3);
	}
	pclose(fp); 

	return result;
}

void checkAXP() {
	// Code adapted from OnionOS
    char *cmd = "cd /customer/app/ ; ./axp_test";  
    int batJsonSize = 100;
    char buf[batJsonSize];
    int battery_number;

    FILE *fp;      
    fp = popen(cmd, "r");
        if (fgets(buf, batJsonSize, fp) != NULL) {
           sscanf(buf,  "{\"battery\":%d, \"voltage\":%*d, \"charging\":%*d}", &battery_number);
        }
    pclose(fp);

    int bat_fd = open("/tmp/battery", O_CREAT | O_WRONLY | O_TRUNC);
	if (bat_fd>0) {
		char value[3];
		sprintf(value, "%d", battery_number);
		write(bat_fd, value, strlen(value));
		close(bat_fd);
	}
}

static void* runAXP(void *arg) {
	while(1) {
		sleep(5);
		checkAXP();
	}
	return 0;
}

int main (int argc, char *argv[]) {
	checkAXP();
	pthread_create(&adc_pt, NULL, &runAXP, NULL);
	
	// Set Initial Volume / Brightness
	InitSettings();
	
	input_fd = open("/dev/input/event0", O_RDONLY);

	// Main Loop
	register uint32_t val;
	register uint32_t button_flag = 0;
	register uint32_t menu_pressed = 0;
	register uint32_t power_pressed = 0;
	uint32_t repeat_vol = 0;
	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		val = ev.value;
		if (( ev.type != EV_KEY ) || ( val > REPEAT )) continue;
		switch (ev.code) {
		case BUTTON_MENU:
			if ( val != REPEAT ) menu_pressed = val;
			break;
		case BUTTON_POWER:
			if ( val != REPEAT ) power_pressed = val;
			break;
		case BUTTON_START:
			if ( val != REPEAT ) {
				button_flag = button_flag & (~START) | (val<<START_BIT);
			} 
			break;
		case BUTTON_VOLUP:
			if ( val == REPEAT ) {
				// Adjust repeat speed to 1/2
				val = repeat_vol;
				repeat_vol ^= PRESSED;
			} else {
				repeat_vol = 0;
			}
			if ( val == PRESSED ) {
				if (menu_pressed > 0) {
					val = GetBrightness();
					if (val<BRIMAX) SetBrightness(++val);
				} else {
					val = GetVolume();
					if (val<VOLMAX) SetVolume(++val);
				}
			}
			break;
		case BUTTON_VOLDOWN:
			if ( val == REPEAT ) {
				// Adjust repeat speed to 1/2
				val = repeat_vol;
				repeat_vol ^= PRESSED;
			} else {
				repeat_vol = 0;
			}
			if ( val == PRESSED ) {
				if (menu_pressed > 0) {
					val = GetBrightness();
					if (val>0) SetBrightness(--val);
				} else {
					val = GetVolume();
					if (val>0) SetVolume(--val);
				}
			}
			break;
		default:
			break;
		}
		
		if (menu_pressed && power_pressed) {
			menu_pressed = power_pressed = 0;
			system("shutdown");
			while (1) pause();
		}
	}
	ERROR("Failed to read input event");
}
