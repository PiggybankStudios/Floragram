/*
File:   game_helpers.cpp
Author: Taylor Robbins
Date:   12\20\2023
Description: 
	** Some functions that are helpful for all AppStates
*/

void SetSourceWord(MyStr_t word)
{
	FreeString(mainHeap, &gl->sourceWord);
	gl->sourceWord = AllocString(mainHeap, &word);
}
void SetSourceWord(const char* wordNt)
{
	SetSourceWord(NewStr(wordNt));
}
