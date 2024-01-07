/*
File:   game_state.cpp
Author: Taylor Robbins
Date:   12\19\2023
Description: 
	** Holds the AppState that runs the main game (where you actually play Sudoku)
*/

GameState_t* game = nullptr;

void GameMainMenuSelectedCallback(void* userPntr)
{
	UNUSED(userPntr);
	game->mainMenuRequested = true;
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartAppState_Game(bool initialize, AppState_t prevState, MyStr_t transitionStr)
{
	if (initialize)
	{
		MemArena_t* scratch = GetScratchArena();
		
		game->mainFont = LoadFont(NewStr(GAME_MAIN_FONT_PATH));
		Assert(game->mainFont.isValid);
		
		game->leavesSheet = LoadSpriteSheet(NewStr("Resources/Sheets/leaves"), 8);
		Assert(game->leavesSheet.isValid);
		Assert(game->leavesSheet.numFramesX == NUM_LEAVES);
		
		game->pointerSheet = LoadSpriteSheet(NewStr("Resources/Sheets/pointer"), 4);
		Assert(game->pointerSheet.isValid);
		
		Assert(gl->sourceWord.length == NUM_LEAVES); //TODO: Eventually this should be somewhat dynamic
		for (u64 lIndex = 0; lIndex < NUM_LEAVES; lIndex++)
		{
			game->letters[lIndex] = GetLowercaseAnsiiChar(gl->sourceWord.chars[lIndex]);
		}
		game->currentWord.chars = &game->wordEntryBuffer[0];
		game->currentWord.length = 0;
		
		game->loadingDictionary = true;
		game->renderedLoadingScreen = false;
		CreateWordTree(&game->dictionary, mainHeap, NUM_WORDS_EXPECTED_IN_DICTIONARY);
		
		game->initialized = true;
		FreeScratchArena(scratch);
	}
	
	game->mainMenuItem = pd->system->addMenuItem("Main Menu", GameMainMenuSelectedCallback, nullptr);
}

// +--------------------------------------------------------------+
// |                             Stop                             |
// +--------------------------------------------------------------+
void StopAppState_Game(bool deinitialize, AppState_t nextState)
{
	pd->system->removeMenuItem(game->mainMenuItem);
	game->mainMenuItem = nullptr;
	
	if (deinitialize)
	{
		FreeFont(&game->mainFont);
		FreeSpriteSheet(&game->leavesSheet);
		FreeSpriteSheet(&game->pointerSheet);
		FreeWordTree(&game->dictionary);
		ClearPointer(game);
	}
}

// +--------------------------------------------------------------+
// |                            Layout                            |
// +--------------------------------------------------------------+
void GameUiLayout()
{
	
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateAppState_Game()
{
	MemArena_t* scratch = GetScratchArena();
	
	// +==============================+
	// |     Return to Main Menu      |
	// +==============================+
	if (game->mainMenuRequested)
	{
		game->mainMenuRequested = false;
		PopAppState();
	}
	
	// +==============================+
	// |       Load Dictionary        |
	// +==============================+
	if (game->loadingDictionary && game->renderedLoadingScreen)
	{
		u8 totalDictionaryTimerIndex = StartPerfTime();
		u64 numWordsInDictionary = 0;
		const char* dictionaryPaths[] = DICTIONARY_FILE_PATHS;
		for (u64 dIndex = 0; dIndex < ArrayCount(dictionaryPaths); dIndex++)
		{
			u8 readFileTimerIndex = StartPerfTime();
			MyStr_t dictionaryFileContents;
			PushMemMark(scratch);
			if (ReadEntireFile(false, NewStr(dictionaryPaths[dIndex]), &dictionaryFileContents, scratch))
			{
				u64 readFileTime = EndPerfTime(readFileTimerIndex);
				u64 lineStartIndex = 0;
				u64 numWordsInFile = 0;
				u8 addWordsTimerIndex = StartPerfTime();
				for (u64 cIndex = 0; cIndex < dictionaryFileContents.length; cIndex++)
				{
					char c = dictionaryFileContents.chars[cIndex];
					if (c == '\r' || c == '\n')
					{
						if (cIndex > lineStartIndex)
						{
							MyStr_t word = NewStr(cIndex - lineStartIndex, &dictionaryFileContents.chars[lineStartIndex]);
							// PrintLine_D("Adding \"%.*s\"", StrPrint(word));
							WordTreeLeaf_t* newLeaf = WordTreeAddLeaf(&game->dictionary, word);
							if (newLeaf != nullptr)
							{
								newLeaf->value64 = 1;
								numWordsInDictionary++;
								numWordsInFile++;
							}
						}
						lineStartIndex = cIndex+1;
					}
					else { Assert(c >= 'a' && c <= 'z'); }
				}
				u64 addWordsTime = EndPerfTime(addWordsTimerIndex);
				PrintLine_I("Loaded %llu word%s from \"%s\" (read=" PERF_FORMAT_STR " parse=" PERF_FORMAT_STR ")",
					numWordsInFile,
					((numWordsInFile == 1) ? "" : "s"),
					dictionaryPaths[dIndex],
					PERF_FORMAT(readFileTime),
					PERF_FORMAT(addWordsTime)
				);
			}
			else 
			{
				u64 readFileTime = EndPerfTime(readFileTimerIndex);
				PrintLine_E("Failed to open/read dictionary at \"%s\" " PERF_FORMAT_STR, dictionaryPaths[dIndex], PERF_FORMAT(readFileTime));
			}
			PopMemMark(scratch);
		}
		u64 totalDictionaryTime = EndPerfTime(totalDictionaryTimerIndex);
		
		PrintLine_I("Loaded %u dictionaries in " PERF_FORMAT_STR " (%llu words total)", ArrayCount(dictionaryPaths), PERF_FORMAT(totalDictionaryTime), numWordsInDictionary);
		game->loadingDictionary = false;
	}
	
	if (game->loadingDictionary) { return; } // <==== Early out!
	
	// +==============================+
	// |    Btn_A Selects a Letter    |
	// +==============================+
	if (BtnPressed(Btn_A))
	{
		HandleBtnExtended(Btn_A);
		u32 selectedLetterIndex = (u32)(FloorR32i(AngleFixR32(input->crankAngleRadians + CRANK_ANGLE_OFFSET) / LEAVES_ARC_LENGTH) % NUM_LEAVES);
		char selectedLetter = game->letters[selectedLetterIndex];
		if (game->currentWord.length < MAX_WORD_ENTRY_LENGTH)
		{
			game->currentWord.chars[game->currentWord.length] = selectedLetter;
			game->currentWord.length++;
			game->currentWordChanged = true;
		}
	}
	
	// +==============================+
	// |    Btn_B Deletes a Letter    |
	// +==============================+
	if (BtnPressed(Btn_B))
	{
		HandleBtnExtended(Btn_B);
		if (game->currentWord.length > 0)
		{
			game->currentWord.length--;
			game->currentWordChanged = true;
		}
	}
	
	// +==============================+
	// |     Check Word Validity      |
	// +==============================+
	if (game->currentWordChanged)
	{
		game->currentWordChanged = false;
		if (game->currentWord.length > 0)
		{
			WordTreeLeaf_t* dictionaryLeaf = WordTreeGetLeaf(&game->dictionary, game->currentWord);
			game->currentWordIsValid = (dictionaryLeaf != nullptr && dictionaryLeaf->value64 != 0);
			PrintLine_D("\"%.*s\" %s a valid word", StrPrint(game->currentWord), game->currentWordIsValid ? "is" : "is NOT");
		}
		else
		{
			game->currentWordIsValid = false;
		}
	}
	
	// +==============================+
	// |     Btn_Up Submits Word      |
	// +==============================+
	if (BtnPressed(Btn_Up))
	{
		HandleBtnExtended(Btn_Up);
		if (game->currentWord.length > 0)
		{
			
		}
		else
		{
			
		}
	}
	
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderAppState_Game(bool isOnTop)
{
	MemArena_t* scratch = GetScratchArena();
	GameUiLayout();
	
	r32 crankAngle = AngleFixR32(input->crankAngleRadians + CRANK_ANGLE_OFFSET);
	v2i flowerCenter = NewVec2i(ScreenSize.width/2, ScreenSize.height/2);
	r32 flowerRadius = (r32)game->leavesSheet.frameSize.width;
	
	pd->graphics->clear(kColorBlack);
	PdSetDrawMode(kDrawModeInverted);
	
	// +==============================+
	// |     Render Loading Text      |
	// +==============================+
	if (game->loadingDictionary)
	{
		MyStr_t loadingText = NewStr("Loading...");
		v2i loadingTextSize = MeasureText(game->mainFont.font, loadingText);
		v2i loadingTextPos = NewVec2i(ScreenSize.width/2 - loadingTextSize.width/2, ScreenSize.height/2 - loadingTextSize.height/2);
		PdBindFont(&game->mainFont);
		PdDrawText(loadingText, loadingTextPos);
		
		FreeScratchArena(scratch);
		game->renderedLoadingScreen = true;
		return;
	}
	
	// +==============================+
	// |        Render Leaves         |
	// +==============================+
	PdBindFont(&game->mainFont);
	for (i32 lIndex = 0; lIndex < NUM_LEAVES; lIndex++)
	{
		r32 leafMinAngle = LEAVES_ARC_LENGTH * (r32)lIndex;
		r32 leafMaxAngle = leafMinAngle + LEAVES_ARC_LENGTH;
		r32 leafCenterAngle = leafMinAngle + LEAVES_ARC_LENGTH/2;
		bool isLeafSelected = (crankAngle >= leafMinAngle && crankAngle < leafMaxAngle);
		
		reci leafRec = NewReci(flowerCenter, game->leavesSheet.frameSize);
		v2 leafDirVec = Vec2FromAngle(leafCenterAngle);
		if (leafDirVec.x < 0) { leafRec.x -= leafRec.width-1; }
		if (leafDirVec.y < 0) { leafRec.y -= leafRec.height-1; }
		
		MyStr_t letterStr = NewStr(1, &game->letters[lIndex]);
		v2i letterSize = MeasureText(game->mainFont.font, letterStr);
		v2i letterDrawPos = Vec2Roundi(ToVec2(flowerCenter) + Vec2FromAngle(leafCenterAngle, flowerRadius * LEAF_LETTER_SPREAD_RADIUS));
		letterDrawPos.x -= letterSize.width/2;
		letterDrawPos.y -= letterSize.height/2;
		
		PdDrawSheetFrame(game->leavesSheet, NewVec2i(lIndex, 0), leafRec);
		PdSetDrawMode(isLeafSelected ? kDrawModeCopy : kDrawModeInverted);
		PdDrawSheetFrame(game->leavesSheet, NewVec2i(lIndex, 1), leafRec);
		// PdSetDrawMode(isLeafSelected ? kDrawModeInverted : kDrawModeCopy);
		PdDrawText(letterStr, letterDrawPos);
		PdSetDrawMode(kDrawModeInverted);
	}
	
	// +==============================+
	// |        Render Pointer        |
	// +==============================+
	{
		v2 pointerTipPos = ToVec2(flowerCenter) + Vec2FromAngle(crankAngle, flowerRadius + POINTER_TO_CIRCLE_MARGIN);
		v2 pointerCenterPos = pointerTipPos + Vec2FromAngle(input->crankAngleRadians, (r32)game->pointerSheet.frameSize.height/2);
		v2i pointerRotationOffset = NewVec2i(game->pointerSheet.frameSize.width/2, game->pointerSheet.frameSize.height);
		obb2 pointerObb = NewObb2D(pointerCenterPos, ToVec2(game->pointerSheet.frameSize), input->crankAngleRadians);
		i32 frameIndex = (ProgramTime % (POINTER_FRAME_TIME * game->pointerSheet.numFramesX)) / POINTER_FRAME_TIME;
		PdDrawRotatedSheetFrame(game->pointerSheet, NewVec2i(frameIndex, 0), pointerObb);
	}
	
	// +==============================+
	// |     Render Current Word      |
	// +==============================+
	{
		v2i currentWordPos = NewVec2i(CURRENT_WORD_MARGIN_LEFT, ScreenSize.height - CURRENT_WORD_MARGIN_BOTTOM - game->mainFont.lineHeight);
		PdBindFont(&game->mainFont);
		PdSetDrawMode(kDrawModeNXOR);
		PdDrawText(game->currentWord, currentWordPos);
		PdSetDrawMode(kDrawModeInverted);
	}
	
	// +==============================+
	// |       Render Validity        |
	// +==============================+
	{
		v2i validTextPos = NewVec2i(5, 5);
		if (game->currentWordIsValid) { validTextPos.y = RoundR32i(Oscillate(5, 10, 2000)); }
		PdBindFont(&game->mainFont);
		PdSetDrawMode(kDrawModeNXOR);
		PdDrawText(game->currentWordIsValid ? "Valid!" : "Nope", validTextPos);
		PdSetDrawMode(kDrawModeInverted);
	}
	
	// +==============================+
	// |         Debug Render         |
	// +==============================+
	if (pig->debugEnabled)
	{
		LCDBitmapDrawMode oldDrawMode = PdSetDrawMode(kDrawModeNXOR);
		
		v2i textPos = NewVec2i(1, 1);
		if (pig->perfGraph.enabled) { textPos.y += pig->perfGraph.mainRec.y + pig->perfGraph.mainRec.height + 1; }
		PdBindFont(&pig->debugFont);
		i32 stepY = pig->debugFont.lineHeight + 1;
		
		PdDrawTextPrint(textPos, "Memory: %.2lf%%", ((r64)mainHeap->used / (r64)MAIN_HEAP_MAX_SIZE) * 100.0);
		textPos.y += stepY;
		PdDrawTextPrint(textPos, "ProgramTime: %u (%u)", ProgramTime, input->realProgramTime);
		textPos.y += stepY;
		
		PdSetDrawMode(oldDrawMode);
	}
	
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                           Register                           |
// +--------------------------------------------------------------+
void RegisterAppState_Game()
{
	game = (GameState_t*)RegisterAppState(
		AppState_Game,
		sizeof(GameState_t),
		StartAppState_Game,
		StopAppState_Game,
		UpdateAppState_Game,
		RenderAppState_Game
	);
}
