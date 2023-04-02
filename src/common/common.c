#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "common.h"
#include <msettings.h>

// TODO: move any functions not also used by libmmenu out of here
// TODO: one wrinkle, functions that require static variables defined in here...

// TODO: standardize function casing, eg. Object_methodName vs function_name vs functionName
// NOTE: currently thinking it should be Object_methodName() and functionName() but instance->property_name and variable_name
// NOTE: constants can be CONSTANT_NAME or kConstantName, not sure what distinguishes the two types...
// NOTE: CONSTANT_NAME matches C and feels lower level, kConstantName is recognized by my IDE's C syntax coloring :sweat_smile:

UnionScreen Screen = {
	.width	= 640,
	.height	= 480,

	.font = {
		.small_size		= 24,
		.medium_size	= 28,
		.large_size 	= 32,
		.small_oy		= 8,
		.medium_oy 		= 10,
		.large_oy 		= 10,

		.shadow = {
			.ox = 2,
			.oy = 3,
		},
	},

	.button = {
		.size = 22*2,
		.text = {
			.oy		= -2,
			.ox_A	= 1,
			.ox_B	= 2,
			.ox_X	= 1,
			.ox_Y	= 2,
		},
	},

	.pill = {
		.pad_width	= 12,
		.text_oy	= 4,
	},

	.hint = {
		.ox			= 6,
		.text_oy	= 2,
	},

	.settings = {
		.width	= 292,
		.height	= 52,
		.icon = {
			.ox = 12,
			.oy = 8,
		},
		.bar = {
			.ox = 60,
			.oy = 22,
		},
	},

	.buttons = {
		.top	= 422,
		.left	= 16,
		.right	= 20,
		.gutter	= 16,
	},

	.main = {
		.settings = {
			.x = 342,
			.y = 8,
		},
		.battery = {
			.x = 600,
			.y = 18,
		},
		.rule = {
			.top_y		= 70,
			.bottom_y	= 406,
		},

		.logo = {
			.x = 20,
			.y = 20,
		},
		.list = {
			.row_count	= 5,
			.y			= 76,
			.ox			= 32,
			.oy			= 8,
			.row_height	= 64,
			.alt_ox		= 64, // from right
			.alt_oy		= 20,
		},
	},

	.menu = {
		.settings = {
			.x = 174,
			.y = 74,
		},
		.battery = {
			.x = 606,
			.y = 10,
		},
		.rule = {
			.top_y		= 54,
			.bottom_y	= 422,
		},

		.buttons = {
			.top = 434,
		},
		.slots = {
			.x	= 400,
			.y	= 394,
			.ox	= 16,
		},
		.disc = {
			.ox = 130,
			.oy = 8,
		},
		.preview = {
			.x		= 296,
			.y		= 142,
			.width	= 332,
			.height	= 270,
			.inset	= 6,
		},

		.title = {
			.x 		= 48,
			.y 		= 6,
			.width	= 544,
		},
		.window = {
			.x		= 12,
			.y		= 142,
			.width	= 280,
			.height	= 270,
		},
		.list = {
			.x				=  28,
			.y				=  152,
			.oy				=  0,
			.line_height	=  50,
			.row_height		=  54,
		},
		.arrow = {
			.ox = 12, // from right
			.oy = 14,
		},

		.bar_height = 60,
	},

	.body = {
		.line_height = 48, // paired with Screen.font.large_size
	},
};

UnionPaths Paths = {
	.rootDir 		= "/mnt/SDCARD",
	.resDir			= "/mnt/SDCARD/.system/res",
	.paksDir		= "/mnt/SDCARD/.system/paks",
	.userdataDir	= "/mnt/SDCARD/.userdata",
	.romsDir		= "/mnt/SDCARD/Roms",
	.recentPath		= "/mnt/SDCARD/.userdata/.miniui/recent.txt",
	.fauxRecentDir 	= "/mnt/SDCARD/Recently Played",
	.collectionsDir = "/mnt/SDCARD/Collections",
};

///////////////////////////////////////

struct ButtonState {
	int is_pressed;
	int just_pressed;
	int just_repeated;
	int just_released;
};
static struct ButtonState buttons[kButtonCount];

void Input_reset(void) {
	// reset all
	for (int i=0; i<kButtonCount; i++) {
		buttons[i].just_pressed = 0;
		buttons[i].is_pressed = 0;
		buttons[i].just_repeated = 0;
		buttons[i].just_released = 0;
	}
}
void Input_poll(void) {
	// reset transient values
	for (int i=0; i<kButtonCount; i++) {
		buttons[i].just_pressed = 0;
		buttons[i].just_repeated = 0;
		buttons[i].just_released = 0;
	}
	
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ButtonIndex i;
		if (event.type==SDL_KEYDOWN || event.type==SDL_KEYUP) {
			SDLKey key = event.key.keysym.sym;
			// NOTE: can't use switch because all missing buttons have the same value
				 if (key==MINUI_A) 		i = kButtonA;
			else if (key==MINUI_B) 		i = kButtonB;
			else if (key==MINUI_X) 		i = kButtonX;
			else if (key==MINUI_Y) 		i = kButtonY;
			else if (key==MINUI_START) 	i = kButtonStart;
			else if (key==MINUI_SELECT)	i = kButtonSelect;
			else if (key==MINUI_UP)		i = kButtonUp;
			else if (key==MINUI_DOWN)	i = kButtonDown;
			else if (key==MINUI_LEFT)	i = kButtonLeft;
			else if (key==MINUI_RIGHT)	i = kButtonRight;
			else if (key==MINUI_L)		i = kButtonL;
			else if (key==MINUI_R)		i = kButtonR;
			else if (key==MINUI_MENU)	i = kButtonMenu;
			else if (key==MINUI_L2)		i = kButtonL2;
			else if (key==MINUI_R2)		i = kButtonR2;
			else if (key==MINUI_L3)		i = kButtonL3;
			else if (key==MINUI_R3)		i = kButtonR3;
			else if (key==MINUI_POWER)	i = kButtonPower;
			else if (key==MINUI_VOLDN)	i = kButtonVolDn;
			else if (key==MINUI_VOLUP)	i = kButtonVolUp;
			else continue;
			
			if (event.type==SDL_KEYDOWN) {
				buttons[i].just_repeated = 1;
				if (!buttons[i].is_pressed) {
					buttons[i].just_pressed = 1;
					buttons[i].is_pressed = 1;
				}
			}
			else {
				buttons[i].is_pressed = 0;
				buttons[i].just_released = 1;
			}
		}
	}
}
int Input_anyPressed(void) {
	for (int i=0; i<kButtonCount; i++) {
		if (buttons[i].is_pressed) return 1;
	}
	return 0;
}
int Input_justPressed(ButtonIndex btn) 	{ return  buttons[btn].just_pressed; }
int Input_justRepeated(ButtonIndex btn) { return  buttons[btn].just_repeated; }
int Input_isPressed(ButtonIndex btn) 	{ return  buttons[btn].is_pressed; }
int Input_justReleased(ButtonIndex btn) { return  buttons[btn].just_released; }

///////////////////////////////////////

int prefixMatch(char* pre, char* str) {
	return (strncasecmp(pre,str,strlen(pre))==0);
}
int suffixMatch(char* suf, char* str) {
	int len = strlen(suf);
	int offset = strlen(str)-len;
	return (offset>=0 && strncasecmp(suf, str+offset, len)==0);
}
int exactMatch(char* str1, char* str2) {
	int len1 = strlen(str1);
	if (len1!=strlen(str2)) return 0;
	return (strncmp(str1,str2,len1)==0);
}
int hide(char* file_name) {
	return file_name[0]=='.';
}

void getDisplayName(const char* in_name, char* out_name) {
	char* tmp;
	strcpy(out_name, in_name);
	
	// extract just the filename if necessary
	tmp = strrchr(in_name, '/');
	if (tmp) strcpy(out_name, tmp+1);
	
	// remove extension
	tmp = strrchr(out_name, '.');
	if (tmp && strlen(tmp)<=4) tmp[0] = '\0'; // 3 letter extension plus dot
	
	// remove trailing parens (round and square)
	char safe_name[256];
	strcpy(safe_name,out_name);
	while ((tmp=strrchr(out_name, '('))!=NULL || (tmp=strrchr(out_name, '['))!=NULL) {
		if (tmp==out_name) break;
		tmp[0] = '\0';
		tmp = out_name;
	}
	
	// make sure we haven't nuked the entire name
	if (out_name[0]=='\0') strcpy(out_name, safe_name);
	
	// remove trailing whitespace
	tmp = out_name + strlen(out_name) - 1;
    while(tmp>out_name && isspace((unsigned char)*tmp)) tmp--;
    tmp[1] = '\0';
}
void getEmuName(const char* in_name, char* out_name) {
	char* tmp;
	strcpy(out_name, in_name);
	tmp = out_name;
	
	// extract just the Roms folder name if necessary
	if (prefixMatch(Paths.romsDir, tmp)) {
		tmp += strlen(Paths.romsDir) + 1;
		char* tmp2 = strchr(tmp, '/');
		if (tmp2) tmp2[0] = '\0';
	}

	// finally extract pak name from parenths if present
	tmp = strrchr(tmp, '(');
	if (tmp) {
		tmp += 1;
		strcpy(out_name, tmp);
		tmp = strchr(out_name,')');
		tmp[0] = '\0';
	}
}

void normalizeNewline(char* line) {
	int len = strlen(line);
	if (len>1 && line[len-1]=='\n' && line[len-2]=='\r') { // windows!
		line[len-2] = '\n';
		line[len-1] = '\0';
	}
}
void trimTrailingNewlines(char* line) {
	int len = strlen(line);
	while (len>0 && line[len-1]=='\n') {
		line[len-1] = '\0'; // trim newline
		len -= 1;
	}
}

///////////////////////////////////////

int exists(char* path) {
	return access(path, F_OK)==0;
}
void touch(char* path) {
	close(open(path, O_RDWR|O_CREAT, 0777));
}
void putFile(char* path, char* contents) {
	FILE* file = fopen(path, "w");
	if (file) {
		fputs(contents, file);
		fclose(file);
	}
}
void getFile(char* path, char* buffer, size_t buffer_size) {
	FILE *file = fopen(path, "r");
	if (file) {
		fseek(file, 0L, SEEK_END);
		size_t size = ftell(file);
		if (size>buffer_size-1) size = buffer_size - 1;
		rewind(file);
		fread(buffer, sizeof(char), size, file);
		fclose(file);
		buffer[size] = '\0';
	}
}
int getInt(char* path) {
	int i = 0;
	FILE *file = fopen(path, "r");
	if (file!=NULL) {
		fscanf(file, "%i", &i);
		fclose(file);
	}
	return i;
}
void putInt(char* path, int value) {
	char buffer[8];
	sprintf(buffer, "%d", value);
	putFile(path, buffer);
}

///////////////////////////////////////

static TTF_Font* font_s;
static TTF_Font* font_m;
static TTF_Font* font_l;
static SDL_Surface* screen; // video surface
static SDL_Surface* rule;
static SDL_Surface* button;
static SDL_Surface* slot_overlay;
static SDL_Surface* bg_white;
static SDL_Surface* bg_black;
static SDL_Surface* settings_bar_full;
static SDL_Surface* settings_bar_empty;
static SDL_Surface* settings_brightness;
static SDL_Surface* settings_volume;
static SDL_Surface* settings_mute;

static SDL_Color pink = {PINK_TRIAD};
static SDL_Color bronze = {BRONZE_TRIAD};
static SDL_Color white = {WHITE_TRIAD};
static SDL_Color gray = {GRAY_TRIAD};
static SDL_Color shadow25 = {SHADOW25_TRIAD};
static SDL_Color shadow50 = {SHADOW50_TRIAD};
static SDL_Color disabled = {DISABLED_TRIAD};
static SDL_Color disabled_shadow = {DISABLED_SHADOW_TRIAD};

static SDL_Surface* battery_charging;
static SDL_Surface* battery_fill;
static SDL_Surface* battery_line;
static SDL_Surface* battery_fill_bad;
static SDL_Surface* battery_line_bad;
void GFX_init(void) {
	char font_path[256];
	sprintf(font_path, "%s/%s", Paths.resDir, "BPreplayBold-unhinted.otf");
	
	TTF_Init();
	font_s = TTF_OpenFont(font_path, Screen.font.small_size);
	font_m = TTF_OpenFont(font_path, Screen.font.medium_size);
	font_l = TTF_OpenFont(font_path, Screen.font.large_size);
	
	if (!font_s || !font_m || !font_l) printf("TTF_OpenFont: %s\n", TTF_GetError());

	rule = GFX_loadImage("rule.png");
	button = GFX_loadImage("btn.png");
	bg_white = GFX_loadImage("bg-white.png");
	bg_black = GFX_loadImage("bg-black.png");
	
	settings_bar_full = GFX_loadImage("settings-bar-full.png");
	settings_bar_empty = GFX_loadImage("settings-bar-empty.png");
	settings_brightness = GFX_loadImage("settings-brightness.png");
	settings_volume = GFX_loadImage("settings-volume.png");
	settings_mute = GFX_loadImage("settings-mute.png");
		
	battery_charging = GFX_loadImage("battery-charging.png");
	battery_fill = GFX_loadImage("battery-fill.png");
	battery_line = GFX_loadImage("battery-line.png");
	battery_fill_bad = GFX_loadImage("bad-battery-fill.png");
	battery_line_bad = GFX_loadImage("bad-battery-line.png");
	
	puts("GFX_init"); fflush(stdout);
}
void GFX_ready(void) {
	screen = SDL_GetVideoSurface(); // :cold_sweat:
}
SDL_Surface* GFX_loadImage(char* path) {
	static char full_path[256];
	sprintf(full_path, "%s/%s", Paths.resDir, path);
	
	SDL_Surface* image = IMG_Load(full_path);
	if (!image) printf("IMG_Load: %s\n", IMG_GetError());
	return image;
}

void GFX_quit(void) {
	puts("GFX_quit"); fflush(stdout);
	
	SDL_FreeSurface(rule);
	SDL_FreeSurface(button);
	SDL_FreeSurface(bg_white);
	SDL_FreeSurface(bg_black);
	SDL_FreeSurface(settings_bar_full);
	SDL_FreeSurface(settings_bar_empty);
	SDL_FreeSurface(settings_brightness);
	SDL_FreeSurface(settings_volume);
	SDL_FreeSurface(settings_mute);
	
	SDL_FreeSurface(battery_charging);
	SDL_FreeSurface(battery_fill);
	SDL_FreeSurface(battery_line);
	SDL_FreeSurface(battery_fill_bad);
	SDL_FreeSurface(battery_line_bad);
	
	TTF_CloseFont(font_s);
	TTF_CloseFont(font_m);
	TTF_CloseFont(font_l);
	
	if (screen) SDL_FreeSurface(screen);
	
	TTF_Quit();
}

void GFX_clear(void) {
	SDL_FillRect(screen, NULL, 0);
	SDL_Flip(screen);
}
void GFX_blitRule(SDL_Surface* surface, int y) {
	SDL_BlitSurface(rule, NULL, surface, &(SDL_Rect){0,y});
}
int GFX_blitHint(SDL_Surface* surface, char* htxt, int x, int y) {
	SDL_Surface* hint_text = TTF_RenderUTF8_Blended(font_m, htxt, white);
	SDL_BlitSurface(hint_text, NULL, surface, &(SDL_Rect){x,y});
	SDL_FreeSurface(hint_text);
}
int GFX_blitPill(SDL_Surface* surface, char* btxt, char* htxt, int x, int y) {
	SDL_Surface* button_text = TTF_RenderUTF8_Blended(font_s, btxt, bronze);
	SDL_Surface* hint_text = TTF_RenderUTF8_Blended(font_m, htxt, white);

	int pill_width = Screen.pill.pad_width + button_text->w + Screen.pill.pad_width;
	int total_width = pill_width + Screen.hint.ox + hint_text->w;
	if (x<0) x += Screen.width - total_width;

	int hw = Screen.button.size/2;
	int fill_width = pill_width - hw - hw;
	SDL_BlitSurface(button, &(SDL_Rect){0,0,hw,Screen.button.size}, surface, &(SDL_Rect){x,y});
	SDL_FillRect(surface, &(SDL_Rect){x+hw,y,fill_width,Screen.button.size}, SDL_MapRGB(button->format, WHITE_TRIAD));
	SDL_BlitSurface(button, &(SDL_Rect){hw,0,hw,Screen.button.size}, surface, &(SDL_Rect){x+hw+fill_width,y});
	SDL_BlitSurface(button_text, NULL, surface, &(SDL_Rect){x+Screen.pill.pad_width,y+Screen.pill.text_oy});
	SDL_BlitSurface(hint_text, NULL, surface, &(SDL_Rect){x+pill_width+Screen.hint.ox,y+Screen.hint.text_oy});
	SDL_FreeSurface(button_text);
	SDL_FreeSurface(hint_text);
	return total_width;
}
int GFX_blitButton(SDL_Surface* surface, char* btxt, char* htxt, int x, int y, int bx) {
	SDL_Surface* button_text = TTF_RenderUTF8_Blended(font_l, btxt, bronze);
	SDL_Surface* hint_text = TTF_RenderUTF8_Blended(font_m, htxt, white);

	int total_width = Screen.button.size + Screen.hint.ox + hint_text->w;
	if (x<0) x += Screen.width - total_width;

	int button_text_ox = (Screen.button.size - button_text->w) / 2 + bx;
	SDL_BlitSurface(button, NULL, surface, &(SDL_Rect){x,y});
	SDL_BlitSurface(button_text, NULL, surface, &(SDL_Rect){x+button_text_ox,y+Screen.button.text.oy});
	SDL_BlitSurface(hint_text, NULL, surface, &(SDL_Rect){x+Screen.button.size+Screen.hint.ox,y+Screen.hint.text_oy});
	SDL_FreeSurface(button_text);
	SDL_FreeSurface(hint_text);
	return total_width;
}

static void trimSortingMeta(char** str) { // eg. `001) `
	char* safe = *str;
	while(isdigit(**str)) *str += 1; // ignore leading numbers

	if (*str[0]==')') { // then match a closing parenthesis
		*str += 1;
	}
	else { //  or bail, restoring the string to its original value
		*str = safe;
		return;
	}
	
	while(isblank(**str)) *str += 1; // ignore leading space
}
void GFX_blitMenu(SDL_Surface* surface, char* name, char* path, char* unique, int row, int selected_row) {
	int max_width = Screen.width - (2 * Screen.main.list.ox);
	SDL_Surface* text;
	if (row==selected_row) {
		char* display_name = unique ? unique : name;
		trimSortingMeta(&display_name);

		// bar
		SDL_FillRect(surface, &(SDL_Rect){0,Screen.main.list.y+(row*Screen.main.list.row_height),Screen.width,Screen.main.list.row_height}, SDL_MapRGB(surface->format, PINK_TRIAD));
		
		// shadow
		text = TTF_RenderUTF8_Blended(font_l, display_name, shadow50);
		
		// if (text->w>max_width) needs_scrolling = 1; // TODO: restore
		SDL_BlitSurface(text, &(SDL_Rect){0,0,max_width,text->h}, surface, &(SDL_Rect){Screen.main.list.ox+Screen.font.shadow.ox,Screen.main.list.y+(row*Screen.main.list.row_height)+Screen.main.list.oy+Screen.font.shadow.oy});
		SDL_FreeSurface(text);
		
		text = TTF_RenderUTF8_Blended(font_l, display_name, white);
		SDL_BlitSurface(text, &(SDL_Rect){0,0,max_width,text->h}, surface, &(SDL_Rect){Screen.main.list.ox,Screen.main.list.y+(row*Screen.main.list.row_height)+Screen.main.list.oy});
		SDL_FreeSurface(text);
	}
	else {
		if (unique) {
			trimSortingMeta(&unique);
			text = TTF_RenderUTF8_Blended(font_l, unique, gray);
			SDL_BlitSurface(text, &(SDL_Rect){0,0,max_width,text->h}, surface, &(SDL_Rect){Screen.main.list.ox,Screen.main.list.y+(row*Screen.main.list.row_height)+Screen.main.list.oy});
			SDL_FreeSurface(text);
		}

		trimSortingMeta(&name);
		text = TTF_RenderUTF8_Blended(font_l, name, white);
		SDL_BlitSurface(text, &(SDL_Rect){0,0,max_width,text->h}, surface, &(SDL_Rect){Screen.main.list.ox,Screen.main.list.y+(row*Screen.main.list.row_height)+Screen.main.list.oy});
		SDL_FreeSurface(text);
	}
}
// copy/paste/mod
static int scroll_selected = -1;
static int scroll_ticks = 0;
static int scroll_delay = 30;
static int scroll_ox = 0;
int GFX_scrollMenu(SDL_Surface* surface, char* name, char* path, char* unique, int row, int selected, int reset, int force) {
	// reset is used when changing directories (otherwise returning from the first row to the first row above wouldn't reset the scroll)
	if (reset || selected!=scroll_selected) {
		scroll_ticks = 0;
		scroll_ox = 0;
		scroll_selected = selected;
	}
	
	scroll_ticks += 1;
	if (scroll_ticks<scroll_delay) return 0; // nothing to do yet
	scroll_ox += 1;
	
	int max_width = Screen.width - (2 * Screen.main.list.ox);
	SDL_Surface* text;
	
	char* display_name = unique ? unique : name;
	trimSortingMeta(&display_name);
		
	text = TTF_RenderUTF8_Blended(font_l, display_name, shadow50);
	if (text->w<=max_width) {
		SDL_FreeSurface(text);
		return 0;
	}
	
	// prevent overscroll
	if (scroll_ox>text->w-max_width) {
		scroll_ox = text->w-max_width;
		if (!force) { // nothing to draw unless something outside of this function dirtied the screen
			SDL_FreeSurface(text);
			return 0;
		}
	}
	
	// bar
	SDL_FillRect(surface, &(SDL_Rect){0,Screen.main.list.y+(row*Screen.main.list.row_height),Screen.width,Screen.main.list.row_height}, SDL_MapRGB(surface->format, PINK_TRIAD));
	
	// shadow
	SDL_BlitSurface(text, &(SDL_Rect){scroll_ox,0,max_width,text->h}, surface, &(SDL_Rect){Screen.main.list.ox+Screen.font.shadow.ox,Screen.main.list.y+(row*Screen.main.list.row_height)+Screen.main.list.oy+Screen.font.shadow.oy});
	SDL_FreeSurface(text);
	
	// actual text
	text = TTF_RenderUTF8_Blended(font_l, display_name, white);
	SDL_BlitSurface(text, &(SDL_Rect){scroll_ox,0,max_width,text->h}, surface, &(SDL_Rect){Screen.main.list.ox,Screen.main.list.y+(row*Screen.main.list.row_height)+Screen.main.list.oy});
	SDL_FreeSurface(text);
	return 1;
}
void GFX_blitWindow(SDL_Surface* surface, int x, int y, int width, int height, int color) {
	SDL_Surface* bg = color ? bg_white : bg_black;
	int color_rgb = color ? SDL_MapRGB(bg->format, WHITE_TRIAD) : 0;
	int hw = (bg->w / 2);
	int hh = (bg->h / 2);
	
	// corners
	SDL_BlitSurface(bg, &(SDL_Rect){0,0,hw,hh}, surface, &(SDL_Rect){x,y});
	SDL_BlitSurface(bg, &(SDL_Rect){hw,0,hw,hh}, surface, &(SDL_Rect){x+width-hw,y});
	SDL_BlitSurface(bg, &(SDL_Rect){0,hh,hw,hh}, surface, &(SDL_Rect){x,y+height-hh});
	SDL_BlitSurface(bg, &(SDL_Rect){hw,hh,hw,hh}, surface, &(SDL_Rect){x+width-hw,y+height-hh});
	
	// fill
	SDL_FillRect(surface, &(SDL_Rect){x+hw,y,width-bg->w,hh}, color_rgb);
	SDL_FillRect(surface, &(SDL_Rect){x,y+hh,width,height-bg->h}, color_rgb);
	SDL_FillRect(surface, &(SDL_Rect){x+hw,y+height-hh,width-bg->w,hh}, color_rgb);
}

void GFX_blitBodyCopy(SDL_Surface* surface, char* str, int ox, int oy, int width, int height) {
	SDL_Surface* text;
#define kTextBoxMaxRows 16
	char* rows[kTextBoxMaxRows];
	int row_count = 0;

	char* tmp;
	rows[row_count++] = str;
	while ((tmp=strchr(rows[row_count-1], '\n'))!=NULL) {
		if (row_count+1>=kTextBoxMaxRows) return; // TODO: bail
		rows[row_count++] = tmp+1;
	}
	
	int rendered_height = Screen.body.line_height * row_count;
	int y = oy;
	y += (height - rendered_height) / 2;
	
	char line[256];
	for (int i=0; i<row_count; i++) {
		int len;
		if (i+1<row_count) {
			len = rows[i+1]-rows[i]-1;
			if (len) strncpy(line, rows[i], len);
			line[len] = '\0';
		}
		else {
			len = strlen(rows[i]);
			strcpy(line, rows[i]);
		}
		
		
		if (len) {
			text = TTF_RenderUTF8_Blended(font_l, line, pink);
			int x = ox;
			x += (width - text->w) / 2;
			SDL_BlitSurface(text, NULL, surface, &(SDL_Rect){x,y});
			SDL_FreeSurface(text);
		}
		y += Screen.body.line_height;
	}
}
// this function is completely asinine, wtf was I thinking
// TODO: this needs serious refactoring...I mean, yikes!
int GFX_blitText(SDL_Surface* surface, char* str, int size, int x, int y, int width, int color, int has_shadow) {
	// size=0:small,1:medium,2:large width=0:left,>0:center,<0:right, color=-1:gray,0:white,1:gold, no multiline
	SDL_Surface* text;
	
	SDL_Color shadow_color = (x==Screen.menu.list.x?(color==-1?disabled_shadow:shadow25):shadow50);
	if (has_shadow && color==-1) color = 0;
	
	TTF_Font* font = size==0?font_s:(size==1?font_m:font_l);
	int oy = size=0?Screen.font.small_oy:(size==1?Screen.font.medium_oy:Screen.font.large_oy);
	text = TTF_RenderUTF8_Blended(font, str, has_shadow?shadow_color:(color==-1?disabled:(color?pink:white)));
	int w = text->w;
	
	if (width>0) x += (width - w) / 2;
	else if (width<0) x += width - w;
	
	if (has_shadow) {
		SDL_BlitSurface(text, NULL, surface, &(SDL_Rect){x+Screen.font.shadow.ox,y+Screen.font.shadow.oy}); 
		SDL_FreeSurface(text);
		text = TTF_RenderUTF8_Blended(font, str, color==-1?gray:(color?pink:white));
	}
	
	// SDL_FillRect(surface, &(SDL_Rect){x,y,text->w,text->h}, white_rgb); // TODO: tmp
	
	SDL_BlitSurface(text, NULL, surface, &(SDL_Rect){x,y});
	SDL_FreeSurface(text);
	return w;
}

SDL_Surface* GFX_getText(char* text, int size, int color) {
	TTF_Font* font = size==0?font_s:(size==1?font_m:font_l);
	return TTF_RenderUTF8_Blended(font, text, color?pink:white);
}

void GFX_blitBattery(SDL_Surface* surface, int x, int y) {
	if (isCharging()) SDL_BlitSurface(battery_charging, NULL, surface, &(SDL_Rect){x,y});
	else {
		int charge = getInt("/tmp/battery");
		SDL_Surface* fill = charge<=17 ? battery_fill_bad : battery_fill;
		SDL_Surface* line = charge<=10 ? battery_line_bad : battery_line;
		SDL_BlitSurface(line, NULL, surface, &(SDL_Rect){x,y});
		
		x += 4;
		y += 6;
		
		int h = fill->h * (float)charge / 100;
		int oy = fill->h - h;
		SDL_BlitSurface(fill, &(SDL_Rect){0,oy,fill->w,h}, surface, &(SDL_Rect){x,y+oy});
	}
}
void GFX_blitSettings(SDL_Surface* surface, int x, int y, int icon, int value, int min_value, int max_value) {
	if (x==Screen.menu.settings.x) GFX_blitWindow(surface, x,y,Screen.settings.width,Screen.settings.height, 0);
	SDL_BlitSurface(icon==0?settings_brightness:(icon==1?settings_volume:settings_mute), NULL, surface, &(SDL_Rect){x+Screen.settings.icon.ox,y+Screen.settings.icon.oy});
	SDL_BlitSurface(settings_bar_empty, NULL, surface, &(SDL_Rect){x+Screen.settings.bar.ox,y+Screen.settings.bar.oy});
	int w = settings_bar_full->w * ((float)(value-min_value) / (max_value-min_value));
	int h = settings_bar_full->h;
	SDL_BlitSurface(settings_bar_full, &(SDL_Rect){0,0,w,h}, surface, &(SDL_Rect){x+Screen.settings.bar.ox,y+Screen.settings.bar.oy,w,h});
}

///////////////////////////////////////

static int can_poweroff = 1;
void disablePoweroff(void) {
	can_poweroff = 0;
}

void waitForWake(void) {
	SDL_Event event;
	int wake = 0;
	unsigned long sleep_ticks = SDL_GetTicks();
	while (!wake) {
		while (SDL_PollEvent(&event)) {
			if (event.type==SDL_KEYUP) {
				SDLKey key = event.key.keysym.sym;
				if (key==MINUI_POWER) {
					wake = 1;
					break;
				}
			}
		}
		SDL_Delay(200);
		if (can_poweroff && SDL_GetTicks()-sleep_ticks>=120000) { // increased to two minutes
			if (isCharging()) sleep_ticks += 60000; // check again in a minute
			else powerOff();
		}
	}
	return;
}

void fauxSleep(void) {
	GFX_clear();
	Input_reset();
	
	enterSleep();
	system("killall -s STOP keymon");
	waitForWake();
	system("killall -s CONT keymon");
	exitSleep();
}

int isCharging(void) {
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

#define GOVERNOR_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
static char governor[128];

void enterSleep(void) {
	SetRawVolume(-60);
	// SetRawBrightness(0);
	putInt("/sys/class/gpio/export", 4);
	putFile("/sys/class/gpio/gpio4/direction", "out");
	putInt("/sys/class/gpio/gpio4/value", 0);
	
	// save current governor (either ondemand or performance)
	getFile(GOVERNOR_PATH, governor, 128);
	trimTrailingNewlines(governor);

	putFile(GOVERNOR_PATH, "powersave");
	sync();
}
void exitSleep(void) {
	putInt("/sys/class/gpio/gpio4/value", 1);
	putInt("/sys/class/gpio/unexport", 4);
	putInt("/sys/class/pwm/pwmchip0/export", 0);
	putInt("/sys/class/pwm/pwmchip0/pwm0/enable",0);
	putInt("/sys/class/pwm/pwmchip0/pwm0/enable",1);
	// SetBrightness(GetBrightness());
	SetVolume(GetVolume());
	
	// restore previous governor
	putFile(GOVERNOR_PATH, governor);
}

int preventAutosleep(void) {
	return isCharging();
}

void powerOff(void) {
	if (can_poweroff) {
		char* msg = exists(kAutoResumePath) ? "Quicksave created,\npowering off" : "Powering off";
		SDL_FillRect(screen, NULL, 0);
		GFX_blitBodyCopy(screen, msg, 0,0,Screen.width,Screen.height);
		SDL_Flip(screen);
		sleep(1);
		system("shutdown");
		while (1) pause();
	}
}