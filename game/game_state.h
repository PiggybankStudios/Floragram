/*
File:   game_state.h
Author: Taylor Robbins
Date:   12\19\2023
*/

#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#define GAME_MAIN_FONT_PATH "Resources/Fonts/border_basic"

#if 0
#define DICTIONARY_FILE_PATHS { "Resources/Text/Dictionaries/common_words3.txt" } //TODO: Remove me!
#else
#define DICTIONARY_FILE_PATHS {                       \
	"Resources/Text/Dictionaries/common_words3.txt",  \
	"Resources/Text/Dictionaries/common_words4.txt",  \
	"Resources/Text/Dictionaries/common_words5.txt",  \
	"Resources/Text/Dictionaries/common_words6.txt",  \
	"Resources/Text/Dictionaries/common_words7.txt",  \
	"Resources/Text/Dictionaries/common_words8.txt",  \
	"Resources/Text/Dictionaries/common_words9.txt",  \
	"Resources/Text/Dictionaries/common_words10.txt", \
	"Resources/Text/Dictionaries/common_words11.txt", \
	"Resources/Text/Dictionaries/common_words12.txt", \
}
#endif

#define NUM_NODES_EXPECTED_IN_DICTIONARY 512 //words
#define NUM_VALID_WORDS_EXPECTED         256 //words
#define CRANK_ANGLE_OFFSET (-HalfPi32)

#define MIN_WORD_LENGTH       3 //chars
#define MAX_VALID_WORD_LENGTH 12 //chars
#define MAX_WORD_ENTRY_LENGTH 12 //chars

#define POINTER_TO_CIRCLE_MARGIN   4 //px
#define NUM_FOUND_MARGIN_RIGHT     5 //px
#define NUM_FOUND_MARGIN_TOP       5 //px
#define VALIDITY_MARGIN_LEFT       5 //px
#define VALIDITY_MARGIN_TOP        5 //px
#define VALIDITY_BOB_AMOUNT        8 //px
#define CURRENT_WORD_MARGIN_LEFT   5 //px
#define CURRENT_WORD_MARGIN_BOTTOM 5 //px
#define WORD_LIST_MARGIN_LEFT      5 //px
#define WORD_LIST_MARGIN_RIGHT     1 //px
#define LEAF_LETTER_SPREAD_RADIUS  0.7f //percentage of total flower radius
#define LEAVES_ARC_LENGTH          (TwoPi32 / NUM_LEAVES) //radians

#define POINTER_FRAME_TIME 300 //ms
#define INCORRECT_SHAKE_ANIM_TIME  400 //ms
#define INCORRECT_SHAKE_STRENGTH_MIN 2
#define INCORRECT_SHAKE_STRENGTH_MAX 10
#define VALIDITY_BOB_PERIOD 2000 //ms
#define WORD_LIST_PAGE_TURN_ANIM_TIME 100 //ms

struct ValidWord_t
{
	bool guessed;
	u64 id;
	MyStr_t word;
};

struct GameState_t
{
	bool initialized;
	
	PDMenuItem* mainMenuItem;
	bool mainMenuRequested;
	
	Font_t mainFont;
	SpriteSheet_t pointerSheet;
	SpriteSheet_t leavesSheet;
	
	bool loadingDictionary;
	bool renderedLoadingScreen;
	WordTree_t dictionary;
	VarArray_t validWords; //ValidWord_t
	u64 nextValidWordId;
	
	MyStr_t sourceWord;
	char letters[NUM_LEAVES];
	
	char wordEntryBuffer[MAX_WORD_ENTRY_LENGTH];
	
	MyStr_t previousWord;
	MyStr_t currentWord;
	bool currentWordChanged;
	bool currentWordIsValid;
	bool currentWordIsNotGuessed;
	r32 incorrectAnimProgress;
	u64 numValidWordsFound;
	
	i32 wordListNumRows;
	u64 numWordListPages;
	u64 currentWordListPage;
	r32 currentWordListPageAnim;
	
	r32 flowerRadius;
	reci flowerRec;
	reci validityTextRec;
	reci numFoundTextRec;
	reci currentWordRec;
	reci wordListRec;
};

#endif //  _GAME_STATE_H
