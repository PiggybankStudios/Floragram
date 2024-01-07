/*
File:   game_state.h
Author: Taylor Robbins
Date:   12\19\2023
*/

#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#define GAME_MAIN_FONT_PATH "Resources/Fonts/whacky_joe"

#if 0
#define DICTIONARY_FILE_PATHS { "Resources/Text/Dictionaries/common_words3.txt" } //TODO: Remove me!
#else
#define DICTIONARY_FILE_PATHS {                      \
	"Resources/Text/Dictionaries/common_words3.txt", \
	"Resources/Text/Dictionaries/common_words4.txt", \
	"Resources/Text/Dictionaries/common_words5.txt", \
	"Resources/Text/Dictionaries/common_words6.txt", \
	"Resources/Text/Dictionaries/common_words7.txt", \
	"Resources/Text/Dictionaries/common_words8.txt", \
	"Resources/Text/Dictionaries/common_words9.txt", \
}
#endif

#define NUM_LEAVES 8
#define CRANK_ANGLE_OFFSET (-HalfPi32)
#define LEAVES_ARC_LENGTH (TwoPi32 / NUM_LEAVES) //radians
#define LEAF_LETTER_SPREAD_RADIUS 0.7f //percentage of total flower radius
#define POINTER_TO_CIRCLE_MARGIN 10 //px
#define POINTER_FRAME_TIME 300 //ms
#define CURRENT_WORD_MARGIN_LEFT 5 //px
#define CURRENT_WORD_MARGIN_BOTTOM 5 //px
#define NUM_WORDS_EXPECTED_IN_DICTIONARY 30000 //words

#define MAX_WORD_ENTRY_LENGTH 32 //chars

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
	
	char letters[NUM_LEAVES];
	
	char wordEntryBuffer[MAX_WORD_ENTRY_LENGTH];
	MyStr_t currentWord;
	bool currentWordChanged;
	bool currentWordIsValid;
};

#endif //  _GAME_STATE_H
