#include <QStandardItemModel>
#include <QStandardItem>

#include "hidtables.h"

static const char *usages[] = {
	"",
	"Error Rollover",                 // 0x01
	"Post Fail",                      // 0x02
	"Error Undefined",                // 0x03
	"A",                              // 0x04
	"B",                              // 0x05
	"C",                              // 0x06
	"D",                              // 0x07
	"E",                              // 0x08
	"F",                              // 0x09
	"G",                              // 0x0a
	"H",                              // 0x0b
	"I",                              // 0x0c
	"J",                              // 0x0d
	"K",                              // 0x0e
	"L",                              // 0x0f
	"M",                              // 0x10
	"N",                              // 0x11
	"O",                              // 0x12
	"P",                              // 0x13
	"Q",                              // 0x14
	"R",                              // 0x15
	"S",                              // 0x16
	"T",                              // 0x17
	"U",                              // 0x18
	"V",                              // 0x19
	"W",                              // 0x1a
	"X",                              // 0x1b
	"Y",                              // 0x1c
	"Z",                              // 0x1d
	"! 1",                            // 0x1e
	"@ 2",                            // 0x1f
	"# 3",                            // 0x20
	"$ 4",                            // 0x21
	"% 5",                            // 0x22
	"^ 6",                            // 0x23
	"& 7",                            // 0x24
	"* 8",                            // 0x25
	"( 9",                            // 0x26
	") 0",                            // 0x27
	"Enter",                          // 0x28
	"Esc",                            // 0x29
	"BkSp",                           // 0x2a
	"Tab",                            // 0x2b
	"Space",                          // 0x2c
	"_ -",                            // 0x2d
	"+ =",                            // 0x2e
	"{ [",                            // 0x2f
	"} ]",                            // 0x30
	"| \\",                             // 0x31
	"NonUS /~",                       // 0x32
	": ;",                            // 0x33
	"\" '",                           // 0x34
	"~ `",                            // 0x35
	"< ,",                            // 0x36
	"> .",                            // 0x37
	"? /",                            // 0x38
	"Caps Lck",                       // 0x39
	"F1",                             // 0x3a
	"F2",                             // 0x3b
	"F3",                             // 0x3c
	"F4",                             // 0x3d
	"F5",                             // 0x3e
	"F6",                             // 0x3f
	"F7",                             // 0x40
	"F8",                             // 0x41
	"F9",                             // 0x42
	"F10",                            // 0x43
	"F11",                            // 0x44
	"F12",                            // 0x45
	"Prnt Scrn",                      // 0x46
	"Scrl Lck",                       // 0x47
	"Pause Break",                    // 0x48
	"Insert",                         // 0x49
	"Home",                           // 0x4a
	"PgUp",                           // 0x4b
	"Del",                            // 0x4c
	"End",                            // 0x4d
	"PgDn",                           // 0x4e
	"Right",                          // 0x4f
	"Left",                           // 0x50
	"Down",                           // 0x51
	"Up",                             // 0x52
	"Num Lck",                        // 0x53
	"Kpd/",                           // 0x54
	"Kpd*",                           // 0x55
	"Kpd-",                           // 0x56
	"Kpd+",                           // 0x57
	"Kpd Enter",                      // 0x58
	"Kpd1 End",                       // 0x59
	"Kpd2 Down",                      // 0x5a
	"Kpd3 PgDn",                      // 0x5b
	"Kpd4 Left",                      // 0x5c
	"Kpd5",                           // 0x5d
	"Kpd6 Right",                     // 0x5e
	"Kpd7 Home",                      // 0x5f
	"Kpd8 Up",                        // 0x60
	"Kpd9 PgUp",                      // 0x61
	"Kpd0 Insert",                    // 0x62
	"Kpd. Del",                       // 0x63
	"NonUS |\\",                      // 0x64
	"Appli- cation",                  // 0x65
	"Power",                          // 0x66
	"Kpd=",                           // 0x67
	"F13",                            // 0x68
	"F14",                            // 0x69
	"F15",                            // 0x6a
	"F16",                            // 0x6b
	"F17",                            // 0x6c
	"F18",                            // 0x6d
	"F19",                            // 0x6e
	"F20",                            // 0x6f
	"F21",                            // 0x70
	"F22",                            // 0x71
	"F23",                            // 0x72
	"F24",                            // 0x73
	"Execute",                        // 0x74
	"Help",                           // 0x75
	"Manu",                           // 0x76
	"Select",                         // 0x77
	"Stop",                           // 0x78
	"Again",                          // 0x79
	"Undo",                           // 0x7a
	"Cut",                            // 0x7b
	"Copy",                           // 0x7c
	"Paste",                          // 0x7d
	"Find",                           // 0x7e
	"Mute",                           // 0x7f
	"Volume Up",                      // 0x80
	"Volume Down",                    // 0x81
	"Locking CapsLock",               // 0x82
	"Locking NumLock",                // 0x83
	"Locking ScrlLock",               // 0x84
	"Kpd,",                           // 0x85
	"Kpd=",                           // 0x86
	"International1",                 // 0x87
	"International2",                 // 0x88
	"International3",                 // 0x89
	"International4",                 // 0x8a
	"International5",                 // 0x8b
	"International6",                 // 0x8c
	"International7",                 // 0x8d
	"International8",                 // 0x8e
	"International9",                 // 0x8f
	"Lang1",                          // 0x90
	"Lang2",                          // 0x91
	"Lang3",                          // 0x92
	"Lang4",                          // 0x93
	"Lang5",                          // 0x94
	"Lang6",                          // 0x95
	"Lang7",                          // 0x96
	"Lang8",                          // 0x97
	"Lang9",                          // 0x98
	"Alternate Erase",                // 0x99
	"Print SysReq",                   // 0x9a
	"Cancel",                         // 0x9b
	"Clear",                          // 0x9c
	"Prior",                          // 0x9d
	"Return",                         // 0x9e
	"Separator",                      // 0x9f
	"Out",                            // 0xa0
	"Oper",                           // 0xa1
	"Clear Again",                    // 0xa2
	"CrSel Props",                    // 0xa3
	"ExSel",                          // 0xa4
	"", "", "", "", "", "", "", "", "", "", "", //A5 To Af Reserved
	"Kpd00",                          // 0xb0
	"Kpd000",                         // 0xb1
	"Thousands Separator",            // 0xb2
	"Decimal Separator",              // 0xb3
	"Currency Unit",                  // 0xb4
	"Currency SubUnit",               // 0xb5
	"Kpd(",                           // 0xb6
	"Kpd)",                           // 0xb7
	"Kpd{",                           // 0xb8
	"Kpd}",                           // 0xb9
	"Kpd Tab",                        // 0xba
	"Kpd BkSp",                       // 0xbb
	"KpdA",                           // 0xbc
	"KpdB",                           // 0xbd
	"KpdC",                           // 0xbe
	"KpdD",                           // 0xbf
	"KpdE",                           // 0xc0
	"KpdF",                           // 0xc1
	"Kpd Xor",                        // 0xc2
	"Kpd^",                           // 0xc3
	"Kpd%",                           // 0xc4
	"Kpd<",                           // 0xc5
	"Kpd>",                           // 0xc6
	"Kpd&",                           // 0xc7
	"Kpd&&",                          // 0xc8
	"Kpd|",                           // 0xc9
	"Kpd||",                          // 0xca
	"Kpd:",                           // 0xcb
	"Kpd#",                           // 0xcc
	"Kpd Space",                      // 0xcd
	"Kpd@",                           // 0xce
	"Kpd!",                           // 0xcf
	"Kpd MemStore",                   // 0xd0
	"Kpd MemRecall",                  // 0xd1
	"Kpd MemClear",                   // 0xd2
	"Kpd Mem+",                       // 0xd3
	"Kpd Mem-",                       // 0xd4
	"Kpd Mem*",                       // 0xd5
	"Kpd Mem/",                       // 0xd6
	"Kpd +/-",                        // 0xd7
	"Kpd Clear",                      // 0xd8
	"Kpd ClearEntry",                 // 0xd9
	"Kpd Binary",                     // 0xda
	"Kpd Octal",                      // 0xdb
	"Kpd Decimal",                    // 0xdc
	"Kpd Hexadec",                    // 0xdd
	"", "",                           // de, df reserved
	"Left Ctrl",                      // 0xe0
	"Left Shift",                     // 0xe1
	"Left Alt",                       // 0xe2
	"Left Win",                       // 0xe3
	"Right Ctrl",                     // 0xe4
	"Right Shift",                    // 0xe5
	"Right Alt",                      // 0xe6
	"Right Win",                      // 0xe7
	//// special non-hid keys for my keyboard
	"mouse 1",                        //0xe8
	"mouse 2",                        //0xe9
	"mouse 3",                        //0xea
	"mouse 4",                        //0xeb
	"mouse 5",                        //0xec
	"mouse up",                       //0xed
	"mouse down",                     //0xee
	"mouse left",                     //0xef
	"mouse right",                    //0xf0
	"prog 1",                         //0xf1
	"prog 2",                         //0xf2
	"prog 3",                         //0xf3
	"prog 4",                         //0xf4
	"prog 5",                         //0xf5
	"prog 6",                         //0xf6
	"macro",                          //0xf7
	"", "",                           // f8-f9 reserved
	"layer lock",                     // 0xfa (not remappable interactively)
	"keypad shift",                   // 0xfb (not remappable interactively)
	"funct shift",                    // 0xfc (not remappable interactively)
	"macro shift",                    // 0xfd (not remappable interactively)
	"prog- ram",                      // 0xfe (not remappable interactively)
	"no key"                          // 0xff
};

const char *HIDTables::nameUsage(uint8_t usage) {
	return usages[usage];
}

QStandardItemModel *HIDTables::newUsageModel(QObject *parent) {
	QStandardItemModel *model = new QStandardItemModel(parent);
	const int nUsages = sizeof(usages) / sizeof(*usages);
	for (int i = 0; i < nUsages; ++i) {
		if (*usages[i] != '\0') {
			QStandardItem *item = new QStandardItem;
			item->setText(QString(usages[i]));
			item->setData(QVariant(i), UsageCode);
			model->appendRow(item);
		}
	}
	return model;
}
