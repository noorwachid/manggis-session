#pragma once

#include "wx/core/Flags.hpp"
#include <cstdint>

// NOTE: subset copy of xkb-keysyms with nicer symbol names

enum struct ModKey {
	None  = 0,
	Shift = 1 << 0,
	Ctrl  = 1 << 1,
	Alt   = 1 << 2,
	Super = 1 << 3,
};

WX_FLAGS(ModKey);

enum struct Key: uint32_t {
	/* Control character */

	Backspace                     = 0xff08,  /* U+0008 BACKSPACE */
	Tab                           = 0xff09,  /* U+0009 CHARACTER TABULATION */
	Linefeed                      = 0xff0a,  /* U+000A LINE FEED */
	Clear                         = 0xff0b,  /* U+000B LINE TABULATION */
	Return                        = 0xff0d,  /* U+000D CARRIAGE RETURN */
	Pause                         = 0xff13,  /* Pause, hold */
	ScrollLock                    = 0xff14,
	SysReq                        = 0xff15,
	Escape                        = 0xff1b,  /* U+001B ESCAPE */
	Delete                        = 0xffff,  /* U+007F DELETE */

	/* International & multi-key character composition */

	MultiKeyCompose               = 0xff20,  /* Multi-key character compose */
	Codeinput                     = 0xff37,
	SingleCandidate               = 0xff3c,
	MultipleCandidate             = 0xff3d,
	PreviousCandidate             = 0xff3e,

	/* Cursor control & motion */

	Home                          = 0xff50,
	Left                          = 0xff51,  /* Move left, left arrow */
	Up                            = 0xff52,  /* Move up, up arrow */
	Right                         = 0xff53,  /* Move right, right arrow */
	Down                          = 0xff54,  /* Move down, down arrow */
	Prior                         = 0xff55,  /* Prior, previous */
	PageUp                        = 0xff55,  /* deprecated alias for Prior */
	Next                          = 0xff56,  /* Next */
	PageDown                      = 0xff56,  /* deprecated alias for Next */
	End                           = 0xff57,  /* EOL */
	Begin                         = 0xff58,  /* BOL */


	/* Misc functions */

	Select                        = 0xff60,  /* Select, mark */
	Print                         = 0xff61,
	Execute                       = 0xff62,  /* Execute, run, do */
	Insert                        = 0xff63,  /* Insert, insert here */
	Undo                          = 0xff65,
	Redo                          = 0xff66,  /* Redo, again */
	Menu                          = 0xff67,
	Find                          = 0xff68,  /* Find, search */
	Cancel                        = 0xff69,  /* Cancel, stop, abort, exit */
	Help                          = 0xff6a,  /* Help */
	Break                         = 0xff6b,
	ModeSwitch                    = 0xff7e,  /* non-deprecated alias for ISO_Group_Shift */
	ScriptSwitch                  = 0xff7e,  /* non-deprecated alias for ISO_Group_Shift */
	NumLock                       = 0xff7f,

	/* Keypad functions, keypad numbers cleverly chosen to map to ASCII */

	KP_Space                      = 0xff80,  /*<U+0020 SPACE>*/
	KP_Tab                        = 0xff89,  /*<U+0009 CHARACTER TABULATION>*/
	KP_Enter                      = 0xff8d,  /*<U+000D CARRIAGE RETURN>*/
	KP_F1                         = 0xff91,  /* PF1, KP_A, ... */
	KP_F2                         = 0xff92,
	KP_F3                         = 0xff93,
	KP_F4                         = 0xff94,
	KP_Home                       = 0xff95,
	KP_Left                       = 0xff96,
	KP_Up                         = 0xff97,
	KP_Right                      = 0xff98,
	KP_Down                       = 0xff99,
	KP_Prior                      = 0xff9a,
	KP_PageUp                     = 0xff9a,  /* deprecated alias for KP_Prior */
	KP_Next                       = 0xff9b,
	KP_PageDown                   = 0xff9b,  /* deprecated alias for KP_Next */
	KP_End                        = 0xff9c,
	KP_Begin                      = 0xff9d,
	KP_Insert                     = 0xff9e,
	KP_Delete                     = 0xff9f,
	KP_Equal                      = 0xffbd,  /*<U+003D EQUALS SIGN>*/
	KP_Multiply                   = 0xffaa,  /*<U+002A ASTERISK>*/
	KP_Add                        = 0xffab,  /*<U+002B PLUS SIGN>*/
	KP_Separator                  = 0xffac,  /*<U+002C COMMA>*/
	KP_Subtract                   = 0xffad,  /*<U+002D HYPHEN-MINUS>*/
	KP_Decimal                    = 0xffae,  /*<U+002E FULL STOP>*/
	KP_Divide                     = 0xffaf,  /*<U+002F SOLIDUS>*/

	KP_0                          = 0xffb0,  /*<U+0030 DIGIT ZERO>*/
	KP_1                          = 0xffb1,  /*<U+0031 DIGIT ONE>*/
	KP_2                          = 0xffb2,  /*<U+0032 DIGIT TWO>*/
	KP_3                          = 0xffb3,  /*<U+0033 DIGIT THREE>*/
	KP_4                          = 0xffb4,  /*<U+0034 DIGIT FOUR>*/
	KP_5                          = 0xffb5,  /*<U+0035 DIGIT FIVE>*/
	KP_6                          = 0xffb6,  /*<U+0036 DIGIT SIX>*/
	KP_7                          = 0xffb7,  /*<U+0037 DIGIT SEVEN>*/
	KP_8                          = 0xffb8,  /*<U+0038 DIGIT EIGHT>*/
	KP_9                          = 0xffb9,  /*<U+0039 DIGIT NINE>*/

	/*
	 * Auxiliary functions; note the duplicate definitions for left and right
	 * function keys;  Sun keyboards and a few other manufacturers have such
	 * function key groups on the left and/or right sides of the keyboard.
	 * We've not found a keyboard with more than 35 function keys total.
	 */

	F1                            = 0xffbe,
	F2                            = 0xffbf,
	F3                            = 0xffc0,
	F4                            = 0xffc1,
	F5                            = 0xffc2,
	F6                            = 0xffc3,
	F7                            = 0xffc4,
	F8                            = 0xffc5,
	F9                            = 0xffc6,
	F10                           = 0xffc7,
	F11                           = 0xffc8,
	F12                           = 0xffc9,
	F13                           = 0xffca,
	F14                           = 0xffcb,
	F15                           = 0xffcc,
	F16                           = 0xffcd,
	F17                           = 0xffce,
	F18                           = 0xffcf,
	F19                           = 0xffd0,
	F20                           = 0xffd1,
	F21                           = 0xffd2,
	F22                           = 0xffd3,
	F23                           = 0xffd4,
	F24                           = 0xffd5,
	F25                           = 0xffd6,
	F26                           = 0xffd7,
	F27                           = 0xffd8,
	F28                           = 0xffd9,
	F29                           = 0xffda,
	F30                           = 0xffdb,
	F31                           = 0xffdc,
	F32                           = 0xffdd,
	F33                           = 0xffde,
	F34                           = 0xffdf,
	F35                           = 0xffe0,

	/* Modifiers */

	ShiftL                        = 0xffe1,  /* Left shift */
	ShiftR                        = 0xffe2,  /* Right shift */
	ControlL                      = 0xffe3,  /* Left control */
	ControlR                      = 0xffe4,  /* Right control */
	CapsLock                      = 0xffe5,  /* Caps lock */
	ShiftLock                     = 0xffe6,  /* Shift lock */

	MetaL                         = 0xffe7,  /* Left meta */
	MetaR                         = 0xffe8,  /* Right meta */
	AltL                          = 0xffe9,  /* Left alt */
	AltR                          = 0xffea,  /* Right alt */
	SuperL                        = 0xffeb,  /* Left super */
	SuperR                        = 0xffec,  /* Right super */
	HyperL                        = 0xffed,  /* Left hyper */
	HyperR                        = 0xffee,  /* Right hyper */

	/*
	 * Keyboard (XKB) Extension function and modifier keys
	 * (from Appendix C of "The X Keyboard Extension: Protocol Specification")
	 * Byte 3 = 0xfe
	 */

	ISO_Lock                      = 0xfe01,
	ISO_Level2Latch               = 0xfe02,
	ISO_Level3Shift               = 0xfe03,
	ISO_Level3Latch               = 0xfe04,
	ISO_Level3Lock                = 0xfe05,
	ISO_Level5Shift               = 0xfe11,
	ISO_Level5Latch               = 0xfe12,
	ISO_Level5Lock                = 0xfe13,
	ISO_GroupShift                = 0xff7e,
	ISO_GroupLatch                = 0xfe06,
	ISO_GroupLock                 = 0xfe07,
	ISO_NextGroup                 = 0xfe08,
	ISO_NextGroupLock             = 0xfe09,
	ISO_PrevGroup                 = 0xfe0a,
	ISO_PrevGroupLock             = 0xfe0b,
	ISO_FirstGroup                = 0xfe0c,
	ISO_FirstGroupLock            = 0xfe0d,
	ISO_LastGroup                 = 0xfe0e,
	ISO_LastGroupLock             = 0xfe0f,

	ISO_LeftTab                   = 0xfe20,
	ISO_MoveLineUp                = 0xfe21,
	ISO_MoveLineDown              = 0xfe22,
	ISO_PartialLineUp             = 0xfe23,
	ISO_PartialLineDown           = 0xfe24,
	ISO_PartialSpaceLeft          = 0xfe25,
	ISO_PartialSpaceRight         = 0xfe26,
	ISO_SetMarginLeft             = 0xfe27,
	ISO_SetMarginRight            = 0xfe28,
	ISO_ReleaseMarginLeft         = 0xfe29,
	ISO_ReleaseMarginRight        = 0xfe2a,
	ISO_ReleaseBothMargins        = 0xfe2b,
	ISO_FastCursorLeft            = 0xfe2c,
	ISO_FastCursorRight           = 0xfe2d,
	ISO_FastCursorUp              = 0xfe2e,
	ISO_FastCursorDown            = 0xfe2f,
	ISO_ContinuousUnderline       = 0xfe30,
	ISO_DiscontinuousUnderline    = 0xfe31,
	ISO_Emphasize                 = 0xfe32,
	ISO_CenterObject              = 0xfe33,
	ISO_Enter                     = 0xfe34,

	DeadGrave                     = 0xfe50,
	DeadAcute                     = 0xfe51,
	DeadCircumflex                = 0xfe52,
	DeadTilde                     = 0xfe53,
	DeadPerispomeni               = 0xfe53,  /* non-deprecated alias for dead_tilde */
	DeadMacron                    = 0xfe54,
	DeadBreve                     = 0xfe55,
	DeadAboveDot                  = 0xfe56,
	DeadDiaeresis                 = 0xfe57,
	DeadAboveRing                 = 0xfe58,
	DeadDoubleAcute               = 0xfe59,
	DeadCaron                     = 0xfe5a,
	DeadCedilla                   = 0xfe5b,
	DeadOgonek                    = 0xfe5c,
	DeadIota                      = 0xfe5d,
	DeadVoicedSound               = 0xfe5e,
	DeadSemivoicedSound           = 0xfe5f,
	DeadBelowDot                  = 0xfe60,
	DeadHook                      = 0xfe61,
	DeadHorn                      = 0xfe62,
	DeadStroke                    = 0xfe63,
	DeadAboveComma                = 0xfe64,
	DeadPsili                     = 0xfe64,  /* non-deprecated alias for dead_abovecomma */
	DeadAboveReversedComma        = 0xfe65,
	DeadDasia                     = 0xfe65,  /* non-deprecated alias for dead_abovereversedcomma */
	DeadDoubleGrave               = 0xfe66,
	DeadBelowRing                 = 0xfe67,
	DeadBelowMacron               = 0xfe68,
	DeadBelowCircumflex           = 0xfe69,
	DeadBelowTilde                = 0xfe6a,
	DeadBelowBreve                = 0xfe6b,
	DeadBelowDiaeresis            = 0xfe6c,
	DeadInvertedBreve             = 0xfe6d,
	DeadBelowComma                = 0xfe6e,
	DeadCurrency                  = 0xfe6f,

	/* exTra dead elements for German T3 layout */
	DeadLowLine                   = 0xfe90,
	DeadAboveVerticalLine         = 0xfe91,
	DeadBelowVerticalLine         = 0xfe92,
	DeadLongSolidusOverlay        = 0xfe93,

	/* dead vowels for universal syllable entry */
	DeadSmallA                    = 0xfe80,
	DeadA                         = 0xfe81,
	DeadSmallE                    = 0xfe82,
	DeadE                         = 0xfe83,
	DeadSmallI                    = 0xfe84,
	DeadI                         = 0xfe85,
	DeadSmallO                    = 0xfe86,
	DeadO                         = 0xfe87,
	DeadSmallU                    = 0xfe88,
	DeadU                         = 0xfe89,
	DeadSmallSchwa                = 0xfe8a, 
	DeadSchwa                     = 0xfe8b,

	DeadGreek                     = 0xfe8c,
	DeadHamza                     = 0xfe8d,
	DeadApostrophe                = 0xfe8e,

	FirstVirtualScreen            = 0xfed0,
	PrevVirtualScreen             = 0xfed1,
	NextVirtualScreen             = 0xfed2,
	LastVirtualScreen             = 0xfed4,
	TerminateServer               = 0xfed5,

	AccessX_Enable                = 0xfe70,
	AccessX_Feedback_Enable       = 0xfe71,
	RepeatKeys_Enable             = 0xfe72,
	SlowKeys_Enable               = 0xfe73,
	BounceKeys_Enable             = 0xfe74,
	StickyKeys_Enable             = 0xfe75,
	MouseKeys_Enable              = 0xfe76,
	MouseKeys_Accel_Enable        = 0xfe77,
	Overlay1_Enable               = 0xfe78,
	Overlay2_Enable               = 0xfe79,
	AudibleBell_Enable            = 0xfe7a,

	Pointer_Left                  = 0xfee0,
	Pointer_Right                 = 0xfee1,
	Pointer_Up                    = 0xfee2,
	Pointer_Down                  = 0xfee3,
	Pointer_UpLeft                = 0xfee4,
	Pointer_UpRight               = 0xfee5,
	Pointer_DownLeft              = 0xfee6,
	Pointer_DownRight             = 0xfee7,
	Pointer_Button_Dflt           = 0xfee8,
	Pointer_Button1               = 0xfee9,
	Pointer_Button2               = 0xfeea,
	Pointer_Button3               = 0xfeeb,
	Pointer_Button4               = 0xfeec,
	Pointer_Button5               = 0xfeed,
	Pointer_DblClick_Dflt         = 0xfeee,
	Pointer_DblClick1             = 0xfeef,
	Pointer_DblClick2             = 0xfef0,
	Pointer_DblClick3             = 0xfef1,
	Pointer_DblClick4             = 0xfef2,
	Pointer_DblClick5             = 0xfef3,
	Pointer_Drag_Dflt             = 0xfef4,
	Pointer_Drag1                 = 0xfef5,
	Pointer_Drag2                 = 0xfef6,
	Pointer_Drag3                 = 0xfef7,
	Pointer_Drag4                 = 0xfef8,
	Pointer_Drag5                 = 0xfefd,

	Pointer_EnableKeys            = 0xfef9,
	Pointer_Accelerate            = 0xfefa,
	Pointer_DfltBtnNext           = 0xfefb,
	Pointer_DfltBtnPrev           = 0xfefc,

	/* Single-Stroke Multiple-Character N-Graph Keysyms For The X Input Method */

	DigraphSmallSmall             = 0xfea0,
	DigraphCapSmall               = 0xfea1,
	DigraphCapCap                 = 0xfea2,
	NGraphSmallSmall              = 0xfea3,
	NGraphCapSmall                = 0xfea4,
	NGraphCapCap                  = 0xfea5,


	/*
	 * 3270 Terminal Keys
	 * Byte 3 = 0xfd
	 */

	T3270_Duplicate                = 0xfd01,
	T3270_FieldMark                = 0xfd02,
	T3270_Right2                   = 0xfd03,
	T3270_Left2                    = 0xfd04,
	T3270_BackTab                  = 0xfd05,
	T3270_EraseEOF                 = 0xfd06,
	T3270_EraseInput               = 0xfd07,
	T3270_Reset                    = 0xfd08,
	T3270_Quit                     = 0xfd09,
	T3270_PA1                      = 0xfd0a,
	T3270_PA2                      = 0xfd0b,
	T3270_PA3                      = 0xfd0c,
	T3270_Test                     = 0xfd0d,
	T3270_Attn                     = 0xfd0e,
	T3270_CursorBlink              = 0xfd0f,
	T3270_AltCursor                = 0xfd10,
	T3270_KeyClick                 = 0xfd11,
	T3270_Jump                     = 0xfd12,
	T3270_Ident                    = 0xfd13,
	T3270_Rule                     = 0xfd14,
	T3270_Copy                     = 0xfd15,
	T3270_Play                     = 0xfd16,
	T3270_Setup                    = 0xfd17,
	T3270_Record                   = 0xfd18,
	T3270_ChangeScreen             = 0xfd19,
	T3270_DeleteWord               = 0xfd1a,
	T3270_ExSelect                 = 0xfd1b,
	T3270_CursorSelect             = 0xfd1c,
	T3270_PrintScreen              = 0xfd1d,
	T3270_Enter                    = 0xfd1e,

	/*
	 * Latin 1
	 * (ISO/IEC 8859-1 = Unicode U+0020..U+00FF)
	 * Byte 3 = 0
	 */
	Space                         = 0x0020,  /* U+0020 SPACE */
	Exclam                        = 0x0021,  /* U+0021 EXCLAMATION MARK */
	DoubleQuote                   = 0x0022,  /* U+0022 QUOTATION MARK */
	Numbersign                    = 0x0023,  /* U+0023 NUMBER SIGN */
	Dollar                        = 0x0024,  /* U+0024 DOLLAR SIGN */
	Percent                       = 0x0025,  /* U+0025 PERCENT SIGN */
	Ampersand                     = 0x0026,  /* U+0026 AMPERSAND */
	Apostrophe                    = 0x0027,  /* U+0027 APOSTROPHE */
	ParenL                        = 0x0028,  /* U+0028 LEFT PARENTHESIS */
	ParenR                        = 0x0029,  /* U+0029 RIGHT PARENTHESIS */
	Asterisk                      = 0x002a,  /* U+002A ASTERISK */
	Plus                          = 0x002b,  /* U+002B PLUS SIGN */
	Comma                         = 0x002c,  /* U+002C COMMA */
	Minus                         = 0x002d,  /* U+002D HYPHEN-MINUS */
	Period                        = 0x002e,  /* U+002E FULL STOP */
	Slash                         = 0x002f,  /* U+002F SOLIDUS */
	Num0                          = 0x0030,  /* U+0030 DIGIT ZERO */
	Num1                          = 0x0031,  /* U+0031 DIGIT ONE */
	Num2                          = 0x0032,  /* U+0032 DIGIT TWO */
	Num3                          = 0x0033,  /* U+0033 DIGIT THREE */
	Num4                          = 0x0034,  /* U+0034 DIGIT FOUR */
	Num5                          = 0x0035,  /* U+0035 DIGIT FIVE */
	Num6                          = 0x0036,  /* U+0036 DIGIT SIX */
	Num7                          = 0x0037,  /* U+0037 DIGIT SEVEN */
	Num8                          = 0x0038,  /* U+0038 DIGIT EIGHT */
	Num9                          = 0x0039,  /* U+0039 DIGIT NINE */
	Colon                         = 0x003a,  /* U+003A COLON */
	Semicolon                     = 0x003b,  /* U+003B SEMICOLON */
	Less                          = 0x003c,  /* U+003C LESS-THAN SIGN */
	Equal                         = 0x003d,  /* U+003D EQUALS SIGN */
	Greater                       = 0x003e,  /* U+003E GREATER-THAN SIGN */
	Question                      = 0x003f,  /* U+003F QUESTION MARK */
	At                            = 0x0040,  /* U+0040 COMMERCIAL AT */
	A                             = 0x0041,  /* U+0041 LATIN CAPITAL LETTER A */
	B                             = 0x0042,  /* U+0042 LATIN CAPITAL LETTER B */
	C                             = 0x0043,  /* U+0043 LATIN CAPITAL LETTER C */
	D                             = 0x0044,  /* U+0044 LATIN CAPITAL LETTER D */
	E                             = 0x0045,  /* U+0045 LATIN CAPITAL LETTER E */
	F                             = 0x0046,  /* U+0046 LATIN CAPITAL LETTER F */
	G                             = 0x0047,  /* U+0047 LATIN CAPITAL LETTER G */
	H                             = 0x0048,  /* U+0048 LATIN CAPITAL LETTER H */
	I                             = 0x0049,  /* U+0049 LATIN CAPITAL LETTER I */
	J                             = 0x004a,  /* U+004A LATIN CAPITAL LETTER J */
	K                             = 0x004b,  /* U+004B LATIN CAPITAL LETTER K */
	L                             = 0x004c,  /* U+004C LATIN CAPITAL LETTER L */
	M                             = 0x004d,  /* U+004D LATIN CAPITAL LETTER M */
	N                             = 0x004e,  /* U+004E LATIN CAPITAL LETTER N */
	O                             = 0x004f,  /* U+004F LATIN CAPITAL LETTER O */
	P                             = 0x0050,  /* U+0050 LATIN CAPITAL LETTER P */
	Q                             = 0x0051,  /* U+0051 LATIN CAPITAL LETTER Q */
	R                             = 0x0052,  /* U+0052 LATIN CAPITAL LETTER R */
	S                             = 0x0053,  /* U+0053 LATIN CAPITAL LETTER S */
	T                             = 0x0054,  /* U+0054 LATIN CAPITAL LETTER T */
	U                             = 0x0055,  /* U+0055 LATIN CAPITAL LETTER U */
	V                             = 0x0056,  /* U+0056 LATIN CAPITAL LETTER V */
	W                             = 0x0057,  /* U+0057 LATIN CAPITAL LETTER W */
	X                             = 0x0058,  /* U+0058 LATIN CAPITAL LETTER X */
	Y                             = 0x0059,  /* U+0059 LATIN CAPITAL LETTER Y */
	Z                             = 0x005a,  /* U+005A LATIN CAPITAL LETTER Z */
	BracketL                      = 0x005b,  /* U+005B LEFT SQUARE BRACKET */
	Backslash                     = 0x005c,  /* U+005C REVERSE SOLIDUS */
	BracketR                      = 0x005d,  /* U+005D RIGHT SQUARE BRACKET */
	Circumflex                    = 0x005e,  /* U+005E CIRCUMFLEX ACCENT */
	Underscore                    = 0x005f,  /* U+005F LOW LINE */
	Grave                         = 0x0060,  /* U+0060 GRAVE ACCENT */
	SmallA                        = 0x0061,  /* U+0061 LATIN SMALL LETTER A */
	SmallB                        = 0x0062,  /* U+0062 LATIN SMALL LETTER B */
	SmallC                        = 0x0063,  /* U+0063 LATIN SMALL LETTER C */
	SmallD                        = 0x0064,  /* U+0064 LATIN SMALL LETTER D */
	SmallE                        = 0x0065,  /* U+0065 LATIN SMALL LETTER E */
	SmallF                        = 0x0066,  /* U+0066 LATIN SMALL LETTER F */
	SmallG                        = 0x0067,  /* U+0067 LATIN SMALL LETTER G */
	SmallH                        = 0x0068,  /* U+0068 LATIN SMALL LETTER H */
	SmallI                        = 0x0069,  /* U+0069 LATIN SMALL LETTER I */
	SmallJ                        = 0x006a,  /* U+006A LATIN SMALL LETTER J */
	SmallK                        = 0x006b,  /* U+006B LATIN SMALL LETTER K */
	SmallL                        = 0x006c,  /* U+006C LATIN SMALL LETTER L */
	SmallM                        = 0x006d,  /* U+006D LATIN SMALL LETTER M */
	SmallN                        = 0x006e,  /* U+006E LATIN SMALL LETTER N */
	SmallO                        = 0x006f,  /* U+006F LATIN SMALL LETTER O */
	SmallP                        = 0x0070,  /* U+0070 LATIN SMALL LETTER P */
	SmallQ                        = 0x0071,  /* U+0071 LATIN SMALL LETTER Q */
	SmallR                        = 0x0072,  /* U+0072 LATIN SMALL LETTER R */
	SmallS                        = 0x0073,  /* U+0073 LATIN SMALL LETTER S */
	SmallT                        = 0x0074,  /* U+0074 LATIN SMALL LETTER T */
	SmallU                        = 0x0075,  /* U+0075 LATIN SMALL LETTER U */
	SmallV                        = 0x0076,  /* U+0076 LATIN SMALL LETTER V */
	SmallW                        = 0x0077,  /* U+0077 LATIN SMALL LETTER W */
	SmallX                        = 0x0078,  /* U+0078 LATIN SMALL LETTER X */
	SmallY                        = 0x0079,  /* U+0079 LATIN SMALL LETTER Y */
	SmallZ                        = 0x007a,  /* U+007A LATIN SMALL LETTER Z */
	CurlyL                        = 0x007b,  /* U+007B LEFT CURLY BRACKET */
	Bar                           = 0x007c,  /* U+007C VERTICAL LINE */
	Pipe                          = 0x007c,  /* U+007C VERTICAL LINE */
	CurlyR                        = 0x007d,  /* U+007D RIGHT CURLY BRACKET */
	Tilde                         = 0x007e,  /* U+007E TILDE */

	NoBreakSpace                  = 0x00a0,  /* U+00A0 NO-BREAK SPACE */
	ExclamDown                    = 0x00a1,  /* U+00A1 INVERTED EXCLAMATION MARK */
	Cent                          = 0x00a2,  /* U+00A2 CENT SIGN */
	Sterling                      = 0x00a3,  /* U+00A3 POUND SIGN */
	Currency                      = 0x00a4,  /* U+00A4 CURRENCY SIGN */
	Yen                           = 0x00a5,  /* U+00A5 YEN SIGN */
	BrokenBar                     = 0x00a6,  /* U+00A6 BROKEN BAR */
	Section                       = 0x00a7,  /* U+00A7 SECTION SIGN */
	Diaeresis                     = 0x00a8,  /* U+00A8 DIAERESIS */
	Copyright                     = 0x00a9,  /* U+00A9 COPYRIGHT SIGN */
	OrdFeminine                   = 0x00aa,  /* U+00AA FEMININE ORDINAL INDICATOR */
	GuillemetL                    = 0x00ab,  /* U+00AB LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
	NotSign                       = 0x00ac,  /* U+00AC NOT SIGN */
	Hyphen                        = 0x00ad,  /* U+00AD SOFT HYPHEN */
	Registered                    = 0x00ae,  /* U+00AE REGISTERED SIGN */
	Macron                        = 0x00af,  /* U+00AF MACRON */
	Degree                        = 0x00b0,  /* U+00B0 DEGREE SIGN */
	PlusMinus                     = 0x00b1,  /* U+00B1 PLUS-MINUS SIGN */
	TwoSuperior                   = 0x00b2,  /* U+00B2 SUPERSCRIPT TWO */
	ThreeSuperior                 = 0x00b3,  /* U+00B3 SUPERSCRIPT THREE */
	Acute                         = 0x00b4,  /* U+00B4 ACUTE ACCENT */
	Mu                            = 0x00b5,  /* U+00B5 MICRO SIGN */
	Paragraph                     = 0x00b6,  /* U+00B6 PILCROW SIGN */
	PeriodCentered                = 0x00b7,  /* U+00B7 MIDDLE DOT */
	Cedilla                       = 0x00b8,  /* U+00B8 CEDILLA */
	OneSuperior                   = 0x00b9,  /* U+00B9 SUPERSCRIPT ONE */
	OrdMasculine                  = 0x00ba,  /* U+00BA MASCULINE ORDINAL INDICATOR */
	GuillemetR                    = 0x00bb,  /* U+00BB RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
	OneQuarter                    = 0x00bc,  /* U+00BC VULGAR FRACTION ONE QUARTER */
	OneHalf                       = 0x00bd,  /* U+00BD VULGAR FRACTION ONE HALF */
	ThreeQuarters                 = 0x00be,  /* U+00BE VULGAR FRACTION THREE QUARTERS */
	QuestionDown                  = 0x00bf,  /* U+00BF INVERTED QUESTION MARK */
	Agrave                        = 0x00c0,  /* U+00C0 LATIN CAPITAL LETTER A WITH GRAVE */
	Aacute                        = 0x00c1,  /* U+00C1 LATIN CAPITAL LETTER A WITH ACUTE */
	Acircumflex                   = 0x00c2,  /* U+00C2 LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
	Atilde                        = 0x00c3,  /* U+00C3 LATIN CAPITAL LETTER A WITH TILDE */
	Adiaeresis                    = 0x00c4,  /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */
	Aring                         = 0x00c5,  /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */
	Ae                            = 0x00c6,  /* U+00C6 LATIN CAPITAL LETTER AE */
	Ccedilla                      = 0x00c7,  /* U+00C7 LATIN CAPITAL LETTER C WITH CEDILLA */
	Egrave                        = 0x00c8,  /* U+00C8 LATIN CAPITAL LETTER E WITH GRAVE */
	Eacute                        = 0x00c9,  /* U+00C9 LATIN CAPITAL LETTER E WITH ACUTE */
	Ecircumflex                   = 0x00ca,  /* U+00CA LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
	Ediaeresis                    = 0x00cb,  /* U+00CB LATIN CAPITAL LETTER E WITH DIAERESIS */
	Igrave                        = 0x00cc,  /* U+00CC LATIN CAPITAL LETTER I WITH GRAVE */
	Iacute                        = 0x00cd,  /* U+00CD LATIN CAPITAL LETTER I WITH ACUTE */
	Icircumflex                   = 0x00ce,  /* U+00CE LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
	Idiaeresis                    = 0x00cf,  /* U+00CF LATIN CAPITAL LETTER I WITH DIAERESIS */
	Eth                           = 0x00d0,  /* U+00D0 LATIN CAPITAL LETTER ETH */
	Ntilde                        = 0x00d1,  /* U+00D1 LATIN CAPITAL LETTER N WITH TILDE */
	Ograve                        = 0x00d2,  /* U+00D2 LATIN CAPITAL LETTER O WITH GRAVE */
	Oacute                        = 0x00d3,  /* U+00D3 LATIN CAPITAL LETTER O WITH ACUTE */
	Ocircumflex                   = 0x00d4,  /* U+00D4 LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
	Otilde                        = 0x00d5,  /* U+00D5 LATIN CAPITAL LETTER O WITH TILDE */
	Odiaeresis                    = 0x00d6,  /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */
	multiply                      = 0x00d7,  /* U+00D7 MULTIPLICATION SIGN */
	Oslash                        = 0x00d8,  /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
	Ugrave                        = 0x00d9,  /* U+00D9 LATIN CAPITAL LETTER U WITH GRAVE */
	Uacute                        = 0x00da,  /* U+00DA LATIN CAPITAL LETTER U WITH ACUTE */
	Ucircumflex                   = 0x00db,  /* U+00DB LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
	Udiaeresis                    = 0x00dc,  /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */
	Yacute                        = 0x00dd,  /* U+00DD LATIN CAPITAL LETTER Y WITH ACUTE */
	Thorn                         = 0x00de,  /* U+00DE LATIN CAPITAL LETTER THORN */
	SmallSsharp                   = 0x00df,  /* U+00DF LATIN SMALL LETTER SHARP S */
	SmallAgrave                   = 0x00e0,  /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */
	SmallAacute                   = 0x00e1,  /* U+00E1 LATIN SMALL LETTER A WITH ACUTE */
	SmallAcircumflex              = 0x00e2,  /* U+00E2 LATIN SMALL LETTER A WITH CIRCUMFLEX */
	SmallAtilde                   = 0x00e3,  /* U+00E3 LATIN SMALL LETTER A WITH TILDE */
	SmallAdiaeresis               = 0x00e4,  /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */
	SmallAring                    = 0x00e5,  /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */
	SmallAe                       = 0x00e6,  /* U+00E6 LATIN SMALL LETTER AE */
	SmallCcedilla                 = 0x00e7,  /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */
	SmallEgrave                   = 0x00e8,  /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */
	SmallEacute                   = 0x00e9,  /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */
	SmallEcircumflex              = 0x00ea,  /* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */
	SmallEdiaeresis               = 0x00eb,  /* U+00EB LATIN SMALL LETTER E WITH DIAERESIS */
	SmallIgrave                   = 0x00ec,  /* U+00EC LATIN SMALL LETTER I WITH GRAVE */
	SmallIacute                   = 0x00ed,  /* U+00ED LATIN SMALL LETTER I WITH ACUTE */
	SmallIcircumflex              = 0x00ee,  /* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */
	SmallIdiaeresis               = 0x00ef,  /* U+00EF LATIN SMALL LETTER I WITH DIAERESIS */
	SmallEth                      = 0x00f0,  /* U+00F0 LATIN SMALL LETTER ETH */
	SmallNtilde                   = 0x00f1,  /* U+00F1 LATIN SMALL LETTER N WITH TILDE */
	SmallOgrave                   = 0x00f2,  /* U+00F2 LATIN SMALL LETTER O WITH GRAVE */
	SmallOacute                   = 0x00f3,  /* U+00F3 LATIN SMALL LETTER O WITH ACUTE */
	SmallOcircumflex              = 0x00f4,  /* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */
	SmallOtilde                   = 0x00f5,  /* U+00F5 LATIN SMALL LETTER O WITH TILDE */
	SmallOdiaeresis               = 0x00f6,  /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */
	SmallDivision                 = 0x00f7,  /* U+00F7 DIVISION SIGN */
	SmallOslash                   = 0x00f8,  /* U+00F8 LATIN SMALL LETTER O WITH STROKE */
	SmallUgrave                   = 0x00f9,  /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */
	SmallUacute                   = 0x00fa,  /* U+00FA LATIN SMALL LETTER U WITH ACUTE */
	SmallUcircumflex              = 0x00fb,  /* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */
	SmallUdiaeresis               = 0x00fc,  /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */
	SmallYacute                   = 0x00fd,  /* U+00FD LATIN SMALL LETTER Y WITH ACUTE */
	SmallThorn                    = 0x00fe,  /* U+00FE LATIN SMALL LETTER THORN */
	SmallYdiaeresis               = 0x00ff,  /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */

	/*
	 * XFree86 vendor specific keysyms.
	 *
	 * The XFree86 keysym range is 0x10080001 - 0x1008ffff.
	 *
	 * The XF86 set of keysyms is a catch-all set of defines for keysyms found
	 * on various multimedia keyboards. Originally specific to XFree86 they have
	 * been been adopted over time and are considered a "standard" part of X
	 * keysym definitions.
	 * XFree86 never properly commented these keysyms, so we have done our
	 * best to explain the semantic meaning of these keys.
	 *
	 * XFree86 has removed their mail archives of the period, that might have
	 * shed more light on some of these definitions. Until/unless we resurrect
	 * these archives, these are from memory and usage.
	 */
	
	/*
	 * ModeLock
	 *
	 * This one is old, and not really used any more since XKB offers this
	 * functionality.
	 */

	XF86ModeLock              = 0x1008ff01,  /* Mode Switch Lock */

	/* Backlight controls. */
	XF86MonBrightnessUp       = 0x1008ff02,  /* Monitor/panel brightness */
	XF86MonBrightnessDown     = 0x1008ff03,  /* Monitor/panel brightness */
	XF86KbdLightOnOff         = 0x1008ff04,  /* Keyboards may be lit     */
	XF86KbdBrightnessUp       = 0x1008ff05,  /* Keyboards may be lit     */
	XF86KbdBrightnessDown     = 0x1008ff06,  /* Keyboards may be lit     */
	XF86MonBrightnessCycle    = 0x1008ff07,  /* Monitor/panel brightness */

	/*
	 * Keys found on some "Internet" keyboards.
	 */
	XF86Standby               = 0x1008ff10,  /* System into standby mode   */
	XF86AudioLowerVolume      = 0x1008ff11,  /* Volume control down        */
	XF86AudioMute             = 0x1008ff12,  /* Mute sound from the system */
	XF86AudioRaiseVolume      = 0x1008ff13,  /* Volume control up          */
	XF86AudioPlay             = 0x1008ff14,  /* Start playing of audio >   */
	XF86AudioStop             = 0x1008ff15,  /* Stop playing audio         */
	XF86AudioPrev             = 0x1008ff16,  /* Previous track             */
	XF86AudioNext             = 0x1008ff17,  /* Next track                 */
	XF86HomePage              = 0x1008ff18,  /* Display user's home page   */
	XF86Mail                  = 0x1008ff19,  /* Invoke user's mail program */
	XF86Start                 = 0x1008ff1a,  /* Start application          */
	XF86Search                = 0x1008ff1b,  /* Search                     */
	XF86AudioRecord           = 0x1008ff1c,  /* Record audio application   */

	/* These are sometimes found on PDA's (e.g. Palm, PocketPC or elsewhere)   */
	XF86Calculator            = 0x1008ff1d,  /* Invoke calculator program  */
	XF86Memo                  = 0x1008ff1e,  /* Invoke Memo taking program */
	XF86ToDoList              = 0x1008ff1f,  /* Invoke To Do List program  */
	XF86Calendar              = 0x1008ff20,  /* Invoke Calendar program    */
	XF86PowerDown             = 0x1008ff21,  /* Deep sleep the system      */
	XF86ContrastAdjust        = 0x1008ff22,  /* Adjust screen contrast     */
	XF86RockerUp              = 0x1008ff23,  /* Rocker switches exist up   */
	XF86RockerDown            = 0x1008ff24,  /* and down                   */
	XF86RockerEnter           = 0x1008ff25,  /* and let you press them     */

	/* Some more "Internet" keyboard symbols */
	XF86Back                  = 0x1008ff26,  /* Like back on a browser     */
	XF86Forward               = 0x1008ff27,  /* Like forward on a browser  */
	XF86Stop                  = 0x1008ff28,  /* Stop current operation     */
	XF86Refresh               = 0x1008ff29,  /* Refresh the page           */
	XF86PowerOff              = 0x1008ff2a,  /* Power off system entirely  */
	XF86WakeUp                = 0x1008ff2b,  /* Wake up system from sleep  */
	XF86Eject                 = 0x1008ff2c,  /* Eject device (e.g. DVD)    */
	XF86ScreenSaver           = 0x1008ff2d,  /* Invoke screensaver         */
	XF86WWW                   = 0x1008ff2e,  /* Invoke web browser         */
	XF86Sleep                 = 0x1008ff2f,  /* Put system to sleep        */
	XF86Favorites             = 0x1008ff30,  /* Show favorite locations    */
	XF86AudioPause            = 0x1008ff31,  /* Pause audio playing        */
	XF86AudioMedia            = 0x1008ff32,  /* Launch media collection app */
	XF86MyComputer            = 0x1008ff33,  /* Display "My Computer" window */
	XF86VendorHome            = 0x1008ff34,  /* Display vendor home web site */
	XF86LightBulb             = 0x1008ff35,  /* Light bulb keys exist       */
	XF86Shop                  = 0x1008ff36,  /* Display shopping web site   */
	XF86History               = 0x1008ff37,  /* Show history of web surfing */
	XF86OpenURL               = 0x1008ff38,  /* Open selected URL           */
	XF86AddFavorite           = 0x1008ff39,  /* Add URL to favorites list   */
	XF86HotLinks              = 0x1008ff3a,  /* Show "hot" links            */
	XF86BrightnessAdjust      = 0x1008ff3b,  /* Invoke brightness adj. UI   */
	XF86Finance               = 0x1008ff3c,  /* Display financial site      */
	XF86Community             = 0x1008ff3d,  /* Display user's community    */
	XF86AudioRewind           = 0x1008ff3e,  /* "rewind" audio track        */
	XF86BackForward           = 0x1008ff3f,  /* ??? */
	XF86Launch0               = 0x1008ff40,  /* Launch Application          */
	XF86Launch1               = 0x1008ff41,  /* Launch Application          */
	XF86Launch2               = 0x1008ff42,  /* Launch Application          */
	XF86Launch3               = 0x1008ff43,  /* Launch Application          */
	XF86Launch4               = 0x1008ff44,  /* Launch Application          */
	XF86Launch5               = 0x1008ff45,  /* Launch Application          */
	XF86Launch6               = 0x1008ff46,  /* Launch Application          */
	XF86Launch7               = 0x1008ff47,  /* Launch Application          */
	XF86Launch8               = 0x1008ff48,  /* Launch Application          */
	XF86Launch9               = 0x1008ff49,  /* Launch Application          */
	XF86LaunchA               = 0x1008ff4a,  /* Launch Application          */
	XF86LaunchB               = 0x1008ff4b,  /* Launch Application          */
	XF86LaunchC               = 0x1008ff4c,  /* Launch Application          */
	XF86LaunchD               = 0x1008ff4d,  /* Launch Application          */
	XF86LaunchE               = 0x1008ff4e,  /* Launch Application          */
	XF86LaunchF               = 0x1008ff4f,  /* Launch Application          */

	XF86ApplicationLeft       = 0x1008ff50,  /* switch to application, left */
	XF86ApplicationRight      = 0x1008ff51,  /* switch to application, right*/
	XF86Book                  = 0x1008ff52,  /* Launch bookreader           */
	XF86CD                    = 0x1008ff53,  /* Launch CD/DVD player        */
	XF86MediaSelectCD         = 0x1008ff53,  /* Alias for XF86CD            */
	XF86Calculater            = 0x1008ff54,  /* Launch Calculater           */
	XF86Clear                 = 0x1008ff55,  /* Clear window, screen        */
	XF86Close                 = 0x1008ff56,  /* Close window                */
	XF86Copy                  = 0x1008ff57,  /* Copy selection              */
	XF86Cut                   = 0x1008ff58,  /* Cut selection               */
	XF86Display               = 0x1008ff59,  /* Output switch key           */
	XF86DOS                   = 0x1008ff5a,  /* Launch DOS (emulation)      */
	XF86Documents             = 0x1008ff5b,  /* Open documents window       */
	XF86Excel                 = 0x1008ff5c,  /* Launch spread sheet         */
	XF86Explorer              = 0x1008ff5d,  /* Launch file explorer        */
	XF86Game                  = 0x1008ff5e,  /* Launch game                 */
	XF86Go                    = 0x1008ff5f,  /* Go to URL                   */
	XF86iTouch                = 0x1008ff60,  /* Logitech iTouch- don't use  */
	XF86LogOff                = 0x1008ff61,  /* Log off system              */
	XF86Market                = 0x1008ff62,  /* ??                          */
	XF86Meeting               = 0x1008ff63,  /* enter meeting in calendar   */
	XF86MenuKB                = 0x1008ff65,  /* distinguish keyboard from PB */
	XF86MenuPB                = 0x1008ff66,  /* distinguish PB from keyboard */
	XF86MySites               = 0x1008ff67,  /* Favourites                  */
	XF86New                   = 0x1008ff68,  /* New (folder, document...    */
	XF86News                  = 0x1008ff69,  /* News                        */
	XF86OfficeHome            = 0x1008ff6a,  /* Office home (old Staroffice)*/
	XF86Open                  = 0x1008ff6b,  /* Open                        */
	XF86Option                = 0x1008ff6c,  /* ?? */
	XF86Paste                 = 0x1008ff6d,  /* Paste                       */
	XF86Phone                 = 0x1008ff6e,  /* Launch phone; dial number   */
	XF86Q                     = 0x1008ff70,  /* Compaq's Q - don't use      */
	XF86Reply                 = 0x1008ff72,  /* Reply e.g., mail            */
	XF86Reload                = 0x1008ff73,  /* Reload web page, file, etc. */
	XF86RotateWindows         = 0x1008ff74,  /* Rotate windows e.g. xrandr  */
	XF86RotationPB            = 0x1008ff75,  /* don't use                   */
	XF86RotationKB            = 0x1008ff76,  /* don't use                   */
	XF86Save                  = 0x1008ff77,  /* Save (file, document, state */
	XF86ScrollUp              = 0x1008ff78,  /* Scroll window/contents up   */
	XF86ScrollDown            = 0x1008ff79,  /* Scrool window/contentd down */
	XF86ScrollClick           = 0x1008ff7a,  /* Use XKB mousekeys instead   */
	XF86Send                  = 0x1008ff7b,  /* Send mail, file, object     */
	XF86Spell                 = 0x1008ff7c,  /* Spell checker               */
	XF86SplitScreen           = 0x1008ff7d,  /* Split window or screen      */
	XF86Support               = 0x1008ff7e,  /* Get support (??)            */
	XF86TaskPane              = 0x1008ff7f,  /* Show tasks */
	XF86Terminal              = 0x1008ff80,  /* Launch terminal emulator    */
	XF86Tools                 = 0x1008ff81,  /* toolbox of desktop/app.     */
	XF86Travel                = 0x1008ff82,  /* ?? */
	XF86UserPB                = 0x1008ff84,  /* ?? */
	XF86User1KB               = 0x1008ff85,  /* ?? */
	XF86User2KB               = 0x1008ff86,  /* ?? */
	XF86Video                 = 0x1008ff87,  /* Launch video player       */
	XF86WheelButton           = 0x1008ff88,  /* button from a mouse wheel */
	XF86Word                  = 0x1008ff89,  /* Launch word processor     */
	XF86Xfer                  = 0x1008ff8a,
	XF86ZoomIn                = 0x1008ff8b,  /* zoom in view, map, etc.   */
	XF86ZoomOut               = 0x1008ff8c,  /* zoom out view, map, etc.  */

	XF86Away                  = 0x1008ff8d,  /* mark yourself as away     */
	XF86Messenger             = 0x1008ff8e,  /* as in instant messaging   */
	XF86WebCam                = 0x1008ff8f,  /* Launch web camera app.    */
	XF86MailForward           = 0x1008ff90,  /* Forward in mail           */
	XF86Pictures              = 0x1008ff91,  /* Show pictures             */
	XF86Music                 = 0x1008ff92,  /* Launch music application  */

	XF86Battery               = 0x1008ff93,  /* Display battery information */
	XF86Bluetooth             = 0x1008ff94,  /* Enable/disable Bluetooth    */
	XF86WLAN                  = 0x1008ff95,  /* Enable/disable WLAN         */
	XF86UWB                   = 0x1008ff96,  /* Enable/disable UWB	    */

	XF86AudioForward          = 0x1008ff97,  /* fast-forward audio track    */
	XF86AudioRepeat           = 0x1008ff98,  /* toggle repeat mode          */
	XF86AudioRandomPlay       = 0x1008ff99,  /* toggle shuffle mode         */
	XF86Subtitle              = 0x1008ff9a,  /* cycle through subtitle      */
	XF86AudioCycleTrack       = 0x1008ff9b,  /* cycle through audio tracks  */
	XF86CycleAngle            = 0x1008ff9c,  /* cycle through angles        */
	XF86FrameBack             = 0x1008ff9d,  /* video: go one frame back    */
	XF86FrameForward          = 0x1008ff9e,  /* video: go one frame forward */
	XF86Time                  = 0x1008ff9f,  /* display, or shows an entry for time seeking */
	XF86Select                = 0x1008ffa0,  /* Select button on joypads and remotes */
	XF86View                  = 0x1008ffa1,  /* Show a view options/properties */
	XF86TopMenu               = 0x1008ffa2,  /* Go to a top-level menu in a video */

	XF86Red                   = 0x1008ffa3,  /* Red button                  */
	XF86Green                 = 0x1008ffa4,  /* Green button                */
	XF86Yellow                = 0x1008ffa5,  /* Yellow button               */
	XF86Blue                  = 0x1008ffa6,  /* Blue button                 */

	XF86Suspend               = 0x1008ffa7,  /* Sleep to RAM                */
	XF86Hibernate             = 0x1008ffa8,  /* Sleep to disk               */
	XF86TouchpadToggle        = 0x1008ffa9,  /* Toggle between touchpad/trackstick */
	XF86TouchpadOn            = 0x1008ffb0,  /* The touchpad got switched on */
	XF86TouchpadOff           = 0x1008ffb1,  /* The touchpad got switched off */

	XF86AudioMicMute          = 0x1008ffb2,  /* Mute the Mic from the system */

	XF86Keyboard              = 0x1008ffb3,  /* User defined keyboard related action */

	XF86WWAN                  = 0x1008ffb4,  /* Toggle WWAN (LTE, UMTS, etc.) radio */
	XF86RFKill                = 0x1008ffb5,  /* Toggle radios on/off */

	XF86AudioPreset           = 0x1008ffb6,  /* Select equalizer preset, e.g. theatre-mode */

	XF86RotationLockToggle    = 0x1008ffb7,  /* Toggle screen rotation lock on/off */

	XF86FullScreen            = 0x1008ffb8,  /* Toggle fullscreen */

	/* Keys for special action keys (hot keys) */
	/* Virtual terminals on some operating systems */
	XF86Switch_VT_1           = 0x1008fe01,
	XF86Switch_VT_2           = 0x1008fe02,
	XF86Switch_VT_3           = 0x1008fe03,
	XF86Switch_VT_4           = 0x1008fe04,
	XF86Switch_VT_5           = 0x1008fe05,
	XF86Switch_VT_6           = 0x1008fe06,
	XF86Switch_VT_7           = 0x1008fe07,
	XF86Switch_VT_8           = 0x1008fe08,
	XF86Switch_VT_9           = 0x1008fe09,
	XF86Switch_VT_10          = 0x1008fe0a,
	XF86Switch_VT_11          = 0x1008fe0b,
	XF86Switch_VT_12          = 0x1008fe0c,

	XF86Ungrab                = 0x1008fe20,  /* force ungrab               */
	XF86ClearGrab             = 0x1008fe21,  /* kill application with grab */
	XF86Next_VMode            = 0x1008fe22,  /* next video mode available  */
	XF86Prev_VMode            = 0x1008fe23,  /* prev. video mode available */
	XF86LogWindowTree         = 0x1008fe24,  /* print window tree to log   */
	XF86LogGrabInfo           = 0x1008fe25,  /* print all active grabs to log */


	/*
	 * Reserved range for evdev symbols: 0x10081000-0x10081FFF
	 */
	XF86MediaPlayPause           = 0x100810a4,      /*         KEY_PLAYPAUSE */
	XF86Exit                     = 0x100810ae,      /*         KEY_EXIT */
	XF86AudioBassBoost           = 0x100810d1,      /*         KEY_BASSBOOST */
	XF86Sport                    = 0x100810dc,      /*         KEY_SPORT */
	XF86MonBrightnessAuto        = 0x100810f4,      /* v3.16   KEY_BRIGHTNESS_AUTO */
	XF86DisplayOff               = 0x100810f5,      /* v2.6.23 KEY_DISPLAY_OFF */
	XF86OK                       = 0x10081160,      /* v2.5.26 KEY_OK */
	XF86GoTo                     = 0x10081162,      /* v2.5.26 KEY_GOTO */
	XF86Info                     = 0x10081166,      /* v2.5.26 KEY_INFO */
	XF86VendorLogo               = 0x10081168,      /* v2.5.26 KEY_VENDOR */
	XF86MediaSelectProgramGuide  = 0x1008116a,      /* v2.5.26 KEY_PROGRAM */
	XF86MediaSelectHome          = 0x1008116e,      /* v2.5.26 KEY_PVR */
	XF86MediaLanguageMenu        = 0x10081170,      /* v2.5.26 KEY_LANGUAGE */
	XF86MediaTitleMenu           = 0x10081171,      /* v2.5.26 KEY_TITLE */
	XF86AudioChannelMode         = 0x10081175,      /* v2.5.26 KEY_MODE */
	XF86AspectRatio              = 0x10081177,      /* v5.1    KEY_ASPECT_RATIO */
	XF86MediaSelectPC            = 0x10081178,      /* v2.5.26 KEY_PC */
	XF86MediaSelectTV            = 0x10081179,      /* v2.5.26 KEY_TV */
	XF86MediaSelectCable         = 0x1008117a,      /* v2.5.26 KEY_TV2 */
	XF86MediaSelectVCR           = 0x1008117b,      /* v2.5.26 KEY_VCR */
	XF86MediaSelectVCRPlus       = 0x1008117c,      /* v2.5.26 KEY_VCR2 */
	XF86MediaSelectSatellite     = 0x1008117d,      /* v2.5.26 KEY_SAT */
	XF86MediaSelectTape          = 0x10081180,      /* v2.5.26 KEY_TAPE */
	XF86MediaSelectRadio         = 0x10081181,      /* v2.5.26 KEY_RADIO */
	XF86MediaSelectTuner         = 0x10081182,      /* v2.5.26 KEY_TUNER */
	XF86MediaPlayer              = 0x10081183,      /* v2.5.26 KEY_PLAYER */
	XF86MediaSelectTeletext      = 0x10081184,      /* v2.5.26 KEY_TEXT */
	XF86DVD                      = 0x10081185,      /* v2.5.26 KEY_DVD */
	XF86MediaSelectDVD           = 0x10081185,      /* Alias for XF86DVD */
	XF86MediaSelectAuxiliary     = 0x10081186,      /* v2.5.26 KEY_AUX */
	XF86Audio                    = 0x10081188,      /* v2.5.26 KEY_AUDIO */
	XF86ChannelUp                = 0x10081192,      /* v2.5.26 KEY_CHANNELUP */
	XF86ChannelDown              = 0x10081193,      /* v2.5.26 KEY_CHANNELDOWN */
	XF86MediaPlaySlow            = 0x10081199,      /* v2.5.26 KEY_SLOW */
	XF86Break                    = 0x1008119b,      /* v2.5.26 KEY_BREAK */
	XF86NumberEntryMode          = 0x1008119d,      /* v2.5.26 KEY_DIGITS */
	XF86VideoPhone               = 0x100811a0,      /* v2.6.20 KEY_VIDEOPHONE */
	XF86ZoomReset                = 0x100811a4,      /* v2.6.20 KEY_ZOOMRESET */
	XF86Editor                   = 0x100811a6,      /* v2.6.20 KEY_EDITOR */
	XF86GraphicsEditor           = 0x100811a8,      /* v2.6.20 KEY_GRAPHICSEDITOR */
	XF86Presentation             = 0x100811a9,      /* v2.6.20 KEY_PRESENTATION */
	XF86Database                 = 0x100811aa,      /* v2.6.20 KEY_DATABASE */
	XF86Voicemail                = 0x100811ac,      /* v2.6.20 KEY_VOICEMAIL */
	XF86Addressbook              = 0x100811ad,      /* v2.6.20 KEY_ADDRESSBOOK */
	XF86DisplayToggle            = 0x100811af,      /* v2.6.20 KEY_DISPLAYTOGGLE */
	XF86SpellCheck               = 0x100811b0,      /* v2.6.24 KEY_SPELLCHECK */
	XF86ContextMenu              = 0x100811b6,      /* v2.6.24 KEY_CONTEXT_MENU */
	XF86MediaRepeat              = 0x100811b7,      /* v2.6.26 KEY_MEDIA_REPEAT */
	XF8610ChannelsUp             = 0x100811b8,      /* v2.6.38 KEY_10CHANNELSUP */
	XF8610ChannelsDown           = 0x100811b9,      /* v2.6.38 KEY_10CHANNELSDOWN */
	XF86Images                   = 0x100811ba,      /* v2.6.39 KEY_IMAGES */
	XF86NotificationCenter       = 0x100811bc,      /* v5.10   KEY_NOTIFICATION_CENTER */
	XF86PickupPhone              = 0x100811bd,      /* v5.10   KEY_PICKUP_PHONE */
	XF86HangupPhone              = 0x100811be,      /* v5.10   KEY_HANGUP_PHONE */
	XF86LinkPhone                = 0x100811bf,      /* v6.14   KEY_LINK_PHONE */
	XF86Fn                       = 0x100811d0,      /*         KEY_FN */
	XF86Fn_Esc                   = 0x100811d1,      /*         KEY_FN_ESC */
	XF86Fn_F1                    = 0x100811d2,      /*         KEY_FN_F1 */
	XF86Fn_F2                    = 0x100811d3,      /*         KEY_FN_F2 */
	XF86Fn_F3                    = 0x100811d4,      /*         KEY_FN_F3 */
	XF86Fn_F4                    = 0x100811d5,      /*         KEY_FN_F4 */
	XF86Fn_F5                    = 0x100811d6,      /*         KEY_FN_F5 */
	XF86Fn_F6                    = 0x100811d7,      /*         KEY_FN_F6 */
	XF86Fn_F7                    = 0x100811d8,      /*         KEY_FN_F7 */
	XF86Fn_F8                    = 0x100811d9,      /*         KEY_FN_F8 */
	XF86Fn_F9                    = 0x100811da,      /*         KEY_FN_F9 */
	XF86Fn_F10                   = 0x100811db,      /*         KEY_FN_F10 */
	XF86Fn_F11                   = 0x100811dc,      /*         KEY_FN_F11 */
	XF86Fn_F12                   = 0x100811dd,      /*         KEY_FN_F12 */
	XF86Fn_1                     = 0x100811de,      /*         KEY_FN_1 */
	XF86Fn_2                     = 0x100811df,      /*         KEY_FN_2 */
	XF86Fn_D                     = 0x100811e0,      /*         KEY_FN_D */
	XF86Fn_E                     = 0x100811e1,      /*         KEY_FN_E */
	XF86Fn_F                     = 0x100811e2,      /*         KEY_FN_F */
	XF86Fn_S                     = 0x100811e3,      /*         KEY_FN_S */
	XF86Fn_B                     = 0x100811e4,      /*         KEY_FN_B */
	XF86FnRightShift             = 0x100811e5,      /* v5.10   KEY_FN_RIGHT_SHIFT */
	XF86Numeric0                 = 0x10081200,      /* v2.6.28 KEY_NUMERIC_0 <U+0030 DIGIT ZERO> */
	XF86Numeric1                 = 0x10081201,      /* v2.6.28 KEY_NUMERIC_1 <U+0031 DIGIT ONE> */
	XF86Numeric2                 = 0x10081202,      /* v2.6.28 KEY_NUMERIC_2 <U+0032 DIGIT TWO> */
	XF86Numeric3                 = 0x10081203,      /* v2.6.28 KEY_NUMERIC_3 <U+0033 DIGIT THREE> */
	XF86Numeric4                 = 0x10081204,      /* v2.6.28 KEY_NUMERIC_4 <U+0034 DIGIT FOUR> */
	XF86Numeric5                 = 0x10081205,      /* v2.6.28 KEY_NUMERIC_5 <U+0035 DIGIT FIVE> */
	XF86Numeric6                 = 0x10081206,      /* v2.6.28 KEY_NUMERIC_6 <U+0036 DIGIT SIX> */
	XF86Numeric7                 = 0x10081207,      /* v2.6.28 KEY_NUMERIC_7 <U+0037 DIGIT SEVEN> */
	XF86Numeric8                 = 0x10081208,      /* v2.6.28 KEY_NUMERIC_8 <U+0038 DIGIT EIGHT> */
	XF86Numeric9                 = 0x10081209,      /* v2.6.28 KEY_NUMERIC_9 <U+0039 DIGIT NINE> */
	XF86NumericStar              = 0x1008120a,      /* v2.6.28 KEY_NUMERIC_STAR <U+002A ASTERISK> */
	XF86NumericPound             = 0x1008120b,      /* v2.6.28 KEY_NUMERIC_POUND <U+0023 NUMBER SIGN> */
	XF86NumericA                 = 0x1008120c,      /* v4.1    KEY_NUMERIC_A */
	XF86NumericB                 = 0x1008120d,      /* v4.1    KEY_NUMERIC_B */
	XF86NumericC                 = 0x1008120e,      /* v4.1    KEY_NUMERIC_C */
	XF86NumericD                 = 0x1008120f,      /* v4.1    KEY_NUMERIC_D */
	XF86CameraFocus              = 0x10081210,      /* v2.6.33 KEY_CAMERA_FOCUS */
	XF86WPSButton                = 0x10081211,      /* v2.6.34 KEY_WPS_BUTTON */
	XF86CameraZoomIn             = 0x10081215,      /* v2.6.39 KEY_CAMERA_ZOOMIN */
	XF86CameraZoomOut            = 0x10081216,      /* v2.6.39 KEY_CAMERA_ZOOMOUT */
	XF86CameraUp                 = 0x10081217,      /* v2.6.39 KEY_CAMERA_UP */
	XF86CameraDown               = 0x10081218,      /* v2.6.39 KEY_CAMERA_DOWN */
	XF86CameraLeft               = 0x10081219,      /* v2.6.39 KEY_CAMERA_LEFT */
	XF86CameraRight              = 0x1008121a,      /* v2.6.39 KEY_CAMERA_RIGHT */
	XF86AttendantOn              = 0x1008121b,      /* v3.10   KEY_ATTENDANT_ON */
	XF86AttendantOff             = 0x1008121c,      /* v3.10   KEY_ATTENDANT_OFF */
	XF86AttendantToggle          = 0x1008121d,      /* v3.10   KEY_ATTENDANT_TOGGLE */
	XF86LightsToggle             = 0x1008121e,      /* v3.10   KEY_LIGHTS_TOGGLE */
	XF86ALSToggle                = 0x10081230,      /* v3.13   KEY_ALS_TOGGLE */
	XF86RefreshRateToggle        = 0x10081232,      /* v6.9    KEY_REFRESH_RATE_TOGGLE */
	XF86Buttonconfig             = 0x10081240,      /* v3.16   KEY_BUTTONCONFIG */
	XF86Taskmanager              = 0x10081241,      /* v3.16   KEY_TASKMANAGER */
	XF86Journal                  = 0x10081242,      /* v3.16   KEY_JOURNAL */
	XF86ControlPanel             = 0x10081243,      /* v3.16   KEY_CONTROLPANEL */
	XF86AppSelect                = 0x10081244,      /* v3.16   KEY_APPSELECT */
	XF86Screensaver              = 0x10081245,      /* v3.16   KEY_SCREENSAVER */
	XF86VoiceCommand             = 0x10081246,      /* v3.16   KEY_VOICECOMMAND */
	XF86Assistant                = 0x10081247,      /* v4.13   KEY_ASSISTANT */
	XF86EmojiPicker              = 0x10081249,      /* v5.13   KEY_EMOJI_PICKER */
	XF86Dictate                  = 0x1008124a,      /* v5.17   KEY_DICTATE */
	XF86CameraAccessEnable       = 0x1008124b,      /* v6.2    KEY_CAMERA_ACCESS_ENABLE */
	XF86CameraAccessDisable      = 0x1008124c,      /* v6.2    KEY_CAMERA_ACCESS_DISABLE */
	XF86CameraAccessToggle       = 0x1008124d,      /* v6.2    KEY_CAMERA_ACCESS_TOGGLE */
	XF86Accessibility            = 0x1008124e,      /* v6.10   KEY_ACCESSIBILITY */
	XF86DoNotDisturb             = 0x1008124f,      /* v6.10   KEY_DO_NOT_DISTURB */
	XF86BrightnessMin            = 0x10081250,      /* v3.16   KEY_BRIGHTNESS_MIN */
	XF86BrightnessMax            = 0x10081251,      /* v3.16   KEY_BRIGHTNESS_MAX */
	XF86ElectronicPrivacyScreenOn  = 0x10081252,      /* v6.18   KEY_EPRIVACY_SCREEN_ON */
	XF86ElectronicPrivacyScreenOff = 0x10081253,      /* v6.18   KEY_EPRIVACY_SCREEN_OFF */
	XF86ActionOnSelection        = 0x10081254,      /* v7.0    KEY_ACTION_ON_SELECTION */
	XF86ContextualInsert         = 0x10081255,      /* v7.0    KEY_CONTEXTUAL_INSERT */
	XF86ContextualQuery          = 0x10081256,      /* v7.0    KEY_CONTEXTUAL_QUERY */
	XF86KbdInputAssistPrev       = 0x10081260,      /* v3.18   KEY_KBDINPUTASSIST_PREV */
	XF86KbdInputAssistNext       = 0x10081261,      /* v3.18   KEY_KBDINPUTASSIST_NEXT */
	XF86KbdInputAssistPrevgroup  = 0x10081262,      /* v3.18   KEY_KBDINPUTASSIST_PREVGROUP */
	XF86KbdInputAssistNextgroup  = 0x10081263,      /* v3.18   KEY_KBDINPUTASSIST_NEXTGROUP */
	XF86KbdInputAssistAccept     = 0x10081264,      /* v3.18   KEY_KBDINPUTASSIST_ACCEPT */
	XF86KbdInputAssistCancel     = 0x10081265,      /* v3.18   KEY_KBDINPUTASSIST_CANCEL */
	XF86RightUp                  = 0x10081266,      /* v4.7    KEY_RIGHT_UP */
	XF86RightDown                = 0x10081267,      /* v4.7    KEY_RIGHT_DOWN */
	XF86LeftUp                   = 0x10081268,      /* v4.7    KEY_LEFT_UP */
	XF86LeftDown                 = 0x10081269,      /* v4.7    KEY_LEFT_DOWN */
	XF86RootMenu                 = 0x1008126a,      /* v4.7    KEY_ROOT_MENU */
	XF86MediaTopMenu             = 0x1008126b,      /* v4.7    KEY_MEDIA_TOP_MENU */
	XF86Numeric11                = 0x1008126c,      /* v4.7    KEY_NUMERIC_11 */
	XF86Numeric12                = 0x1008126d,      /* v4.7    KEY_NUMERIC_12 */
	XF86AudioDesc                = 0x1008126e,      /* v4.7    KEY_AUDIO_DESC */
	XF863DMode                   = 0x1008126f,      /* v4.7    KEY_3D_MODE */
	XF86NextFavorite             = 0x10081270,      /* v4.7    KEY_NEXT_FAVORITE */
	XF86StopRecord               = 0x10081271,      /* v4.7    KEY_STOP_RECORD */
	XF86PauseRecord              = 0x10081272,      /* v4.7    KEY_PAUSE_RECORD */
	XF86VOD                      = 0x10081273,      /* v4.7    KEY_VOD */
	XF86Unmute                   = 0x10081274,      /* v4.7    KEY_UNMUTE */
	XF86FastReverse              = 0x10081275,      /* v4.7    KEY_FASTREVERSE */
	XF86SlowReverse              = 0x10081276,      /* v4.7    KEY_SLOWREVERSE */
	XF86Data                     = 0x10081277,      /* v4.7    KEY_DATA */
	XF86OnScreenKeyboard         = 0x10081278,      /* v4.12   KEY_ONSCREEN_KEYBOARD */
	XF86PrivacyScreenToggle      = 0x10081279,      /* v5.5    KEY_PRIVACY_SCREEN_TOGGLE */
	XF86SelectiveScreenshot      = 0x1008127a,      /* v5.6    KEY_SELECTIVE_SCREENSHOT */
	XF86NextElement              = 0x1008127b,      /* v5.18   KEY_NEXT_ELEMENT */
	XF86PreviousElement          = 0x1008127c,      /* v5.18   KEY_PREVIOUS_ELEMENT */
	XF86AutopilotEngageToggle    = 0x1008127d,      /* v5.18   KEY_AUTOPILOT_ENGAGE_TOGGLE */
	XF86MarkWaypoint             = 0x1008127e,      /* v5.18   KEY_MARK_WAYPOINT */
	XF86Sos                      = 0x1008127f,      /* v5.18   KEY_SOS */
	XF86NavChart                 = 0x10081280,      /* v5.18   KEY_NAV_CHART */
	XF86FishingChart             = 0x10081281,      /* v5.18   KEY_FISHING_CHART */
	XF86SingleRangeRadar         = 0x10081282,      /* v5.18   KEY_SINGLE_RANGE_RADAR */
	XF86DualRangeRadar           = 0x10081283,      /* v5.18   KEY_DUAL_RANGE_RADAR */
	XF86RadarOverlay             = 0x10081284,      /* v5.18   KEY_RADAR_OVERLAY */
	XF86TraditionalSonar         = 0x10081285,      /* v5.18   KEY_TRADITIONAL_SONAR */
	XF86ClearvuSonar             = 0x10081286,      /* v5.18   KEY_CLEARVU_SONAR */
	XF86SidevuSonar              = 0x10081287,      /* v5.18   KEY_SIDEVU_SONAR */
	XF86NavInfo                  = 0x10081288,      /* v5.18   KEY_NAV_INFO */
	XF86Macro1                   = 0x10081290,      /* v5.5    KEY_MACRO1 */
	XF86Macro2                   = 0x10081291,      /* v5.5    KEY_MACRO2 */
	XF86Macro3                   = 0x10081292,      /* v5.5    KEY_MACRO3 */
	XF86Macro4                   = 0x10081293,      /* v5.5    KEY_MACRO4 */
	XF86Macro5                   = 0x10081294,      /* v5.5    KEY_MACRO5 */
	XF86Macro6                   = 0x10081295,      /* v5.5    KEY_MACRO6 */
	XF86Macro7                   = 0x10081296,      /* v5.5    KEY_MACRO7 */
	XF86Macro8                   = 0x10081297,      /* v5.5    KEY_MACRO8 */
	XF86Macro9                   = 0x10081298,      /* v5.5    KEY_MACRO9 */
	XF86Macro10                  = 0x10081299,      /* v5.5    KEY_MACRO10 */
	XF86Macro11                  = 0x1008129a,      /* v5.5    KEY_MACRO11 */
	XF86Macro12                  = 0x1008129b,      /* v5.5    KEY_MACRO12 */
	XF86Macro13                  = 0x1008129c,      /* v5.5    KEY_MACRO13 */
	XF86Macro14                  = 0x1008129d,      /* v5.5    KEY_MACRO14 */
	XF86Macro15                  = 0x1008129e,      /* v5.5    KEY_MACRO15 */
	XF86Macro16                  = 0x1008129f,      /* v5.5    KEY_MACRO16 */
	XF86Macro17                  = 0x100812a0,      /* v5.5    KEY_MACRO17 */
	XF86Macro18                  = 0x100812a1,      /* v5.5    KEY_MACRO18 */
	XF86Macro19                  = 0x100812a2,      /* v5.5    KEY_MACRO19 */
	XF86Macro20                  = 0x100812a3,      /* v5.5    KEY_MACRO20 */
	XF86Macro21                  = 0x100812a4,      /* v5.5    KEY_MACRO21 */
	XF86Macro22                  = 0x100812a5,      /* v5.5    KEY_MACRO22 */
	XF86Macro23                  = 0x100812a6,      /* v5.5    KEY_MACRO23 */
	XF86Macro24                  = 0x100812a7,      /* v5.5    KEY_MACRO24 */
	XF86Macro25                  = 0x100812a8,      /* v5.5    KEY_MACRO25 */
	XF86Macro26                  = 0x100812a9,      /* v5.5    KEY_MACRO26 */
	XF86Macro27                  = 0x100812aa,      /* v5.5    KEY_MACRO27 */
	XF86Macro28                  = 0x100812ab,      /* v5.5    KEY_MACRO28 */
	XF86Macro29                  = 0x100812ac,      /* v5.5    KEY_MACRO29 */
	XF86Macro30                  = 0x100812ad,      /* v5.5    KEY_MACRO30 */
	XF86MacroRecordStart         = 0x100812b0,      /* v5.5    KEY_MACRO_RECORD_START */
	XF86MacroRecordStop          = 0x100812b1,      /* v5.5    KEY_MACRO_RECORD_STOP */
	XF86MacroPresetCycle         = 0x100812b2,      /* v5.5    KEY_MACRO_PRESET_CYCLE */
	XF86MacroPreset1             = 0x100812b3,      /* v5.5    KEY_MACRO_PRESET1 */
	XF86MacroPreset2             = 0x100812b4,      /* v5.5    KEY_MACRO_PRESET2 */
	XF86MacroPreset3             = 0x100812b5,      /* v5.5    KEY_MACRO_PRESET3 */
	XF86KbdLcdMenu1              = 0x100812b8,      /* v5.5    KEY_KBD_LCD_MENU1 */
	XF86KbdLcdMenu2              = 0x100812b9,      /* v5.5    KEY_KBD_LCD_MENU2 */
	XF86KbdLcdMenu3              = 0x100812ba,      /* v5.5    KEY_KBD_LCD_MENU3 */
	XF86KbdLcdMenu4              = 0x100812bb,      /* v5.5    KEY_KBD_LCD_MENU4 */
	XF86KbdLcdMenu5              = 0x100812bc,      /* v5.5    KEY_KBD_LCD_MENU5 */
	XF86PerformanceMode          = 0x100812bd,      /* v6.17   KEY_PERFORMANCE */

};
