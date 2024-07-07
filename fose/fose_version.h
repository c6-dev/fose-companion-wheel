#ifndef __FOSE_VERSION_H__
#define __FOSE_VERSION_H__

// these have to be macros so they can be used in the .rc
#define FOSE_VERSION_INTEGER		1
#define FOSE_VERSION_INTEGER_MINOR	3
#define FOSE_VERSION_INTEGER_BETA	2
#define FOSE_VERSION_VERSTRING		"0, 1, 3, 2"
#define FOSE_VERSION_PADDEDSTRING	"0001"

// build numbers do not appear to follow the same format as with oblivion
#define MAKE_FALLOUT_VERSION_EX(major, minor, build, sub)	(((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | ((build & 0xFFF) << 4) | (sub & 0xF))
#define MAKE_FALLOUT_VERSION(major, minor, build)			MAKE_FALLOUT_VERSION_EX(major, minor, build, 0)

#define FALLOUT_VERSION_1_7		MAKE_FALLOUT_VERSION(1, 7, 3)			// 0x01070030


#define PACKED_FOSE_VERSION		MAKE_FALLOUT_VERSION(FOSE_VERSION_INTEGER, FOSE_VERSION_INTEGER_MINOR, FOSE_VERSION_INTEGER_BETA)

#endif /* __FOSE_VERSION_H__ */
