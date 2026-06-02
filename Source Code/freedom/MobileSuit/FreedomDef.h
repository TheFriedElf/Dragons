#ifndef FREEDOM_DRAGONICA_PCH_FREEDOMDEF_H
#define FREEDOM_DRAGONICA_PCH_FREEDOMDEF_H

char const* const ATTACH_POINT_STAR = "p_ef_star";
char const* const ATTACH_POINT_HEART = "p_ef_heart";
char const* const ATTACH_POINT_01 = "p_ef_01";
char const* const ATTACH_POINT_CENTER = "p_ef_center";

char const* const ATTACH_POINT_PC_TO_PET = "char_root";
char const* const ATTACH_POINT_MOUNT_PET = "p_pt_Ride";
char const* const ATTACH_POINT_RIDENAME = "p_ef_ridename";

const NiColor DEF_BG_COLOR(0.5f, 0.5f, 0.5f);
const NiColorA DEF_BG_COLORA(0.5f, 0.5f, 0.5f, 0.0f);

char const* const XML_ELEMENT_CONFIG = "CONFIG";
char const* const XML_ELEMENT_GRAPHIC1 = "GRAPHIC1";
char const* const XML_ELEMENT_GRAPHIC2 = "GRAPHIC2";
char const* const XML_ELEMENT_GRAPHIC3 = "GRAPHIC3";
char const* const XML_ELEMENT_SOUND = "SOUND";
char const* const XML_ELEMENT_ETC = "ETC";
char const* const XML_ELEMENT_ITEM = "ITEM";
char const* const XML_ELEMENT_KEYSET = "KEYSET";
char const* const XML_ELEMENT_OPTION = "OPTION";
char const* const XML_ELEMENT_GAME = "GAME";
char const* const XML_ELEMENT_KEYSET_TABLE = "KEYSET_TABLE";
char const* const XML_ELEMENT_KEYMAP = "KEYMAP";
char const* const XML_ATTR_VALUE = "VALUE";
char const* const XML_ATTR_NAME = "NAME";
char const* const XML_ATTR_UKEY = "UKEY";
char const* const XML_ATTR_ICONNO = "ICONNO";
char const* const XML_ATTR_TTW = "TTW";
char const* const XML_ELEMENT_VERSION = "VERSION";
char const* const XML_ELEMENT_CONF_VER = "CONF_VER";
char const* const XML_ELEMENT_QUEST_BODY = "BODY";
char const* const XML_ELEMENT_QUEST_TEXT = "TEXT";
char const* const XML_ELEMENT_QUEST_OBJECT1 = "OBJECT1";
char const* const XML_ELEMENT_QUEST_OBJECT2 = "OBJECT2";
char const* const XML_ELEMENT_QUEST_OBJECT3 = "OBJECT3";
char const* const XML_ELEMENT_QUEST_OBJECT4 = "OBJECT4";
char const* const XML_ELEMENT_QUEST_OBJECT5 = "OBJECT5";
char const* const XML_ELEMENT_QUEST_ITEM = "ITEM";
char const* const XML_ELEMENT_QUEST_EXP = "EXP";
char const* const XML_ELEMENT_QUEST_GOLD = "GOLD";
char const* const XML_ELEMENT_QUEST_SELECT1 = "SELECT1";
char const* const XML_ELEMENT_QUEST_SELECT2 = "SELECT2";
char const* const XML_ELEMENT_QUEST_DIALOG = "DIALOG";
char const* const XML_ELEMENT_QUEST_SELECT = "SELECT";
char const* const XML_ATTR_QUEST_COUNT = "COUNT";
char const* const XML_ATTR_QUEST_TEXT = "TEXT";
char const* const XML_ATTR_QUEST_TYPE = "TYPE";
char const* const XML_ATTR_QUEST_MEMO = "MEMO";
char const* const XML_ATTR_QUEST_ID = "ID";
char const* const XML_ELEMENT_TEXT_TABLE = "TEXT_TABLE";
char const* const XML_ELEMENT_QUEST_TEXT_TABLE = "QUEST_TEXT_TABLE";
char const* const XML_ELEMENT_TEXT = "TEXT";
char const* const XML_ATTR_TextID = "ID";
char const* const XML_ATTR_TextTEXT = "Text";
char const* const XML_ELEMENT_QUEST_CHANGECLASS = "CHANGECLASS";
char const* const XML_ELEMENT_MAP_TEXT_TABLE = "MAP_TEXT_TABLE";
char const* const XML_ELEMENT_STITCH_IMAGE = "STITCH_IMAGE";
char const* const XML_ELEMENT_IMPORTANCE_QUEST = "IMPORTANCE_QUEST";
char const* const XML_ELEMENT_CUSTOM_QUEST_MARKER = "CUSTOM_QUEST_MARKER";
char const* const XML_ELEMENT_TEXT_DIALOG = "TEXT_DIALOG";
char const* const XML_ELEMENT_MOVIE = "MOVIE";
char const* const XML_ATTR_MOVIE_OPENING = "OPENING";
char const* const XML_ELEMENT_LEGEND_ITEM_MAKE_HELP = "LEGEND_ITEM_MAKE_HELP";
char const* const XML_ELEMENT_SUPER_GROUND_ENTER_SETTING = "SUPER_GROUND_ENTER_SETTING";
char const* const XML_ELEMENT_RAGNAROK_GROUND_ENTER_SETTING = "RAGNAROK_GROUND_ENTER_SETTING";
char const* const XML_ELEMENT_BALLOON_FORM = "BALLOON_FORM";

char const* const XML_ELEMENT_ERROR_CODE_TABLE = "ERROR";

char const* const STR_USER_UI= "USER_UI";
char const* const STR_DEFAULT_BOLD= "DEFAULT_BOLD";
char const* const STR_OPTION_GRAPHIC_SET= "GRAPHIC_SET";
char const* const STR_GAME_RESOLUTION_WIDTH= "RESOLUTION_WIDTH";
char const* const STR_GAME_RESOLUTION_HEIGHT= "RESOLUTION_HEIGHT";
char const* const STR_GAME_FULL_SCREEN= "FULL_SCREEN";
char const* const STR_OPTION_ANTI_ALIAS= "ANTI_ALIAS";
char const* const STR_OPTION_WORLD_QUALITY= "WORLD_QUALITY";
char const* const STR_OPTION_VIEW_DISTANCE= "VIEW_DISTANCE";
char const* const STR_OPTION_TEXTURE_RES= "TEXTURE_RES";
char const* const STR_OPTION_TEXTURE_FILTERING= "TEXTURE_FILTERING";
char const* const STR_OPTION_GLOW= "GLOW_EFFECT";
char const* const STR_OPTION_KEYSET= XML_ELEMENT_KEYSET;
char const* const STR_OPTION_PASSED_KEY= "0";
char const* const STR_OPTION_SEPARATER= "/";
char const* const STR_OPTION_USER_FILE= "./Config.xml";
char const* const STR_OPTION_DISPLAY_HELMET= "DISPLAY_HELMET";
char const* const STR_OPTION_SHOW_ALL_QUEST= "SHOW_ALL_QUEST";

char const* const STR_FIRE_START_NODE= "FIRE_START_NODE";
char const* const STR_FIRE_EFFECT= "FIRE_EFFECT";
char const* const STR_TARGET_EFFECT_NODE= "TARGET_EFFECT_NODE";
char const* const STR_TARGET_EFFECT= "TARGET_EFFECT";
std::string const STR_COMMON_EFFECT_ID = "COMMON_EFFECT_ID";
std::string const STR_COMMON_EFFECT_NODE = "COMMON_EFFECT_NODE";
std::string const STR_COMMON_EFFECT_SCALE = "COMMON_EFFECT_SCALE";
std::string const STR_COMMON_EFFECT_TARGET = "COMMON_EFFECT_TARGET";
std::string const STR_COMMON_EFFECT_DETACH_SKIP = "COMMON_EFFECT_DETACH_SKIP";
char const* const STR_PROJECTILE_ID= "PROJECTILE_ID";
char const* const STR_SPEED="SPEED";

char const* const STR_SAVE_INFO = "SAVE_INFO";
char const* const STR_ID_AUTO_SAVE = "ID_AUTO_SAVE";
char const* const STR_CONNECT_SERVER_SAVE = "CONNECT_SERVER_SAVE";
char const* const STR_SHOW_EXTEND_QUICK_SLOT = "SHOW_EXTEND_QUICK_SLOT";

char const* const OBJ_TYPE_ACTOR = "ACTOR";
char const* const OBJ_TYPE_NPC = "NPC";
char const* const OBJ_TYPE_MONSTER = "MONSTER";
char const* const OBJ_TYPE_PET = "PET";
char const* const OBJ_TYPE_OBJECT = "OBJECT";
char const* const OBJ_TYPE_MYHOME = "MYHOME";
char const* const OBJ_TYPE_SUMMONED = "SUMMONED";
char const* const OBJ_TYPE_SUB_PLAYER = "SUB_PLAYER";

const wchar_t* const WSTR_REWARD_ICON_FORMAT= _T("REWARD_ITEM%d_%d");
const wchar_t* const WSTR_REWARD_SFRM_FORMAT= _T("SFRM_REWARD_BG%d_%d");
const wchar_t* const WSTR_REWARD_BTN_FORMAT= _T("BTN_REWARD%d_%d");
const wchar_t* const WSTR_REWARD_CHANGE_LEVEL= _T("SFRM_REWARD_CHANGE_LEVEL");
const wchar_t* const WSTR_DLG_TYPE_COMPLETE= _T("COMPLETE");
const wchar_t* const WSTR_DLG_TYPE_INFO= _T("INFO");
const wchar_t* const WSTR_DLG_TYPE_PROLOG= _T("PROLOG");
const wchar_t* const WSTR_IMG_OVER_FORMAT= _T("IMG_OVER%d");
const wchar_t* const WSTR_KEYSET_BTN_HEADER= _T("ICON_KEY_MAP_");
const wchar_t* const WSTR_XUI_CURSOR= _T("Cursor");
const wchar_t* const WSTR_FORM_MY_QUEST_LIST= _T("FORM_MY_QUEST_LIST");//WSTR_FRM_MY_QUEST_LIST
const wchar_t* const WSTR_QUEST_INFO= _T("QUEST_INFO");//WSTR_QUEST_INFO

const wchar_t* const WSTR_SELECTIVE_QUEST_LIST= _T("SELECTIVE_QUEST_LIST");//WSTR_QUEST_INFO
const wchar_t* const WSTR_MY_QUEST_INFO= _T("MY_QUEST_INFO");//WSTR_QUEST_INFO

const wchar_t* const WSTR_ATTR_EXPLAIN= _T(":   ");//속성 보여주는거 사이에 들어감 ex> 내구도 :   xxx
const wchar_t* const WSTR_ATTR_OPTION_PLUS=_T("   +");

std::wstring const WSTR_CLOSEUI( _T("CloseUI") );

std::wstring const WSTR_UI_REGULATIONSTR(_T("..."));	//UI의 글자범위 초과시, 대체하는 문장
int const REGULATION_UI_WIDTH = -10;					//UI의 글자범위 초과시 문자 출력 영역을 보정하는 수치

int const ACTOR_ATTACH_SLOT_BIND_EFFECT = 90331;		//BIND, UNBIND 성공시 Player Actor에 이펙트를 attatch할 슬롯번호

DWORD const ITEM_NO_TOPAZ		= 50900010;
DWORD const ITEM_NO_EMERALD		= 50900020;
DWORD const ITEM_NO_SAPPHIER	= 50900030;
DWORD const ITEM_NO_RUBY		= 50900040;
DWORD const ITEM_NO_DIAMOND		= 50900050;

DWORD const ITEM_NO_SEAL_SCROLL = 79000050;
DWORD const ITEM_NO_SEAL_REMOVER_SCROLL = 79000040;
DWORD const ITEM_NO_SEAL_REMOVER_SCROLL_CASH_NOMAL = 79000060;
DWORD const ITEM_NO_SEAL_REMOVER_SCROLL_CASH_GOOD  = 79000070;
DWORD const ITEM_NO_SEAL_REMOVER_SCROLL_CASH_BEST  = 79000080;

DWORD const ITEM_NO_BIND_ITEM		= 98000370;
DWORD const ITEM_NO_UNBIND_GOLD		= 98000380;
DWORD const ITEM_NO_UNBIND_PLATINUM = 98000382;

int const g_iMsgNoAnswerServer = 18996; // 서버로 부터 응답이 없습니다.

int const iTOOLTIP_X_SIZE_MAX = 230;

//공용 함수도.
extern void Notice_Show(std::wstring const &rkString, int const iLevel, bool const bCheckEqualMessage = false);

int const ACTION_NO_IDLE = 100000101;
int const ACTION_NO_BATTLE_IDLE = 100000201;
int const ACTION_NO_TWIN_SUB_REPOS = 400000005;
int const ACTION_NO_TWIN_SUB_TRACE_GROUND = 400000001;
int const TRANSFORM_ID_NINJA = 170001101;
NxVec3 const NX_ZERO(0.0f,0.0f,0.0f);
NxVec3 const NX_UNIT_X(1.0f,0.0f,0.0f);
NxVec3 const NX_UNIT_Y(0.0f,1.0f,0.0f);
NxVec3 const NX_UNIT_Z(0.0f,0.0f,1.0f);
NxVec3 const NX_NEGATIVE_UNIT_Z(0.0f,0.0f,-1.0f);
#endif // FREEDOM_DRAGONICA_PCH_FREEDOMDEF_H