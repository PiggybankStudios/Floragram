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

bool IsValidWord(MyStr_t sourceWord, MyStr_t word)
{
	for (u64 wIndex = 0; wIndex < word.length; wIndex++)
	{
		char c = word.chars[wIndex];
		bool isValidChar = false;
		for (u64 sIndex = 0; sIndex < sourceWord.length; sIndex++)
		{
			if (sourceWord.chars[sIndex] == c) { isValidChar = true; break; }
		}
		if (!isValidChar) { return false; }
	}
	return true;
}

ValidWord_t* FindValidWordById(u64 id)
{
	VarArrayLoop(&game->validWords, wIndex)
	{
		VarArrayLoopGet(ValidWord_t, validWord, &game->validWords, wIndex);
		if (validWord->id == id) { return validWord; }
	}
	return nullptr;
}

// +==============================+
// |       WordsSortingFunc       |
// +==============================+
// i32 WordsSortingFunc(const void* left, const void* right, void* contextPntr)
COMPARE_FUNC_DEFINITION(WordsSortingFunc)
{
	UNUSED(contextPntr);
	ValidWord_t* leftWord = (ValidWord_t*)left;
	ValidWord_t* rightWord = (ValidWord_t*)right;
	if (leftWord->word.length > rightWord->word.length) { return 1; }
	else if (leftWord->word.length < rightWord->word.length) { return -1; }
	else
	{
		return MyStrCompare(leftWord->word.chars, rightWord->word.chars, leftWord->word.length);
	}
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
		game->sourceWord = NewStr(NUM_LEAVES, &game->letters[0]);
		
		game->currentWord.chars = &game->wordEntryBuffer[0];
		game->currentWord.length = 0;
		
		game->loadingDictionary = true;
		game->renderedLoadingScreen = false;
		CreateWordTree(&game->dictionary, mainHeap, NUM_WORDS_EXPECTED_IN_DICTIONARY);
		
		CreateVarArray(&game->validWords, mainHeap, sizeof(ValidWord_t), NUM_VALID_WORDS_EXPECTED);
		game->nextValidWordId = 1;
		
		game->numValidWordsFound = 0;
		
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
		VarArrayLoop(&game->validWords, wIndex)
		{
			VarArrayLoopGet(ValidWord_t, validWord, &game->validWords, wIndex);
			FreeString(mainHeap, &validWord->word);
		}
		FreeVarArray(&game->validWords);
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
	MemArena_t* scratch = GetScratchArena();
	
	game->flowerRadius = (r32)game->leavesSheet.frameSize.width;
	game->flowerRec.size = Vec2iFill((i32)game->flowerRadius * 2);
	game->flowerRec.x = ScreenSize.width/2 - game->flowerRec.width/2;
	game->flowerRec.y = ScreenSize.height/2 - game->flowerRec.height/2;
	
	const char* validityText = (game->currentWordIsValid ? (game->currentWordIsNotGuessed ? "Valid!" : "Already Found") : "Nope");
	game->validityTextRec.size = MeasureText(game->mainFont.font, NewStr(validityText));
	game->validityTextRec.x = VALIDITY_MARGIN_LEFT;
	game->validityTextRec.y = VALIDITY_MARGIN_TOP;
	
	MyStr_t numFoundText = PrintInArenaStr(scratch, "%llu/%llu", game->numValidWordsFound, game->validWords.length);
	game->numFoundTextRec.size = MeasureText(game->mainFont.font, numFoundText);
	game->numFoundTextRec.x = ScreenSize.width - NUM_FOUND_MARGIN_RIGHT - game->numFoundTextRec.width;
	game->numFoundTextRec.y = NUM_FOUND_MARGIN_TOP;
	
	game->currentWordRec.size = MeasureText(game->mainFont.font, game->currentWord);
	game->currentWordRec.x = CURRENT_WORD_MARGIN_LEFT;
	game->currentWordRec.y = ScreenSize.height - CURRENT_WORD_MARGIN_BOTTOM - game->currentWordRec.height;
	
	game->wordListRec.x = WORD_LIST_MARGIN_LEFT;
	game->wordListRec.width = game->flowerRec.x - WORD_LIST_MARGIN_RIGHT - game->wordListRec.x;
	game->wordListRec.y = game->validityTextRec.y + game->validityTextRec.height + VALIDITY_BOB_AMOUNT;
	game->wordListRec.height = game->currentWordRec.y - game->wordListRec.y;
	
	i32 wordHeight = pig->debugFont.lineHeight;
	game->wordListNumRows = FloorR32i((r32)game->wordListRec.height / (r32)wordHeight);
	game->wordListRec.y = game->wordListRec.y + game->wordListRec.height/2;
	game->wordListRec.height = game->wordListNumRows * wordHeight;
	game->wordListRec.y -= game->wordListRec.height/2;
	game->numWordListPages = MaxU64(1, CeilDivU64(game->validWords.length, game->wordListNumRows));
	game->currentWordListPage = ClampU64(game->currentWordListPage, 0, game->numWordListPages-1);
	
	FreeScratchArena(scratch);
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
				u64 numValidWordsInFile = 0;
				u8 addWordsTimerIndex = StartPerfTime();
				for (u64 cIndex = 0; cIndex < dictionaryFileContents.length; cIndex++)
				{
					char c = dictionaryFileContents.chars[cIndex];
					if (c == '\r' || c == '\n')
					{
						if (cIndex > lineStartIndex)
						{
							MyStr_t word = NewStr(cIndex - lineStartIndex, &dictionaryFileContents.chars[lineStartIndex]);
							if (IsValidWord(game->sourceWord, word))
							{
								// PrintLine_D("Adding \"%.*s\"", StrPrint(word));
								WordTreeLeaf_t* newLeaf = WordTreeAddLeaf(&game->dictionary, word);
								if (newLeaf != nullptr)
								{
									ValidWord_t* newValidWord = VarArrayAdd(&game->validWords, ValidWord_t);
									NotNull(newValidWord);
									ClearPointer(newValidWord);
									newValidWord->guessed = false;
									newValidWord->id = game->nextValidWordId;
									game->nextValidWordId++;
									newValidWord->word = AllocString(mainHeap, &word);
									
									newLeaf->value64 = newValidWord->id;
									
									numWordsInDictionary++;
								}
								
								numValidWordsInFile++;
							}
						}
						lineStartIndex = cIndex+1;
					}
					else { Assert(c >= 'a' && c <= 'z'); }
				}
				u64 addWordsTime = EndPerfTime(addWordsTimerIndex);
				PrintLine_I("Loaded %llu word%s from \"%s\" (read=" PERF_FORMAT_STR " parse=" PERF_FORMAT_STR ")",
					numValidWordsInFile,
					((numValidWordsInFile == 1) ? "" : "s"),
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
		
		u8 sortValidWordsTimerIndex = StartPerfTime();
		VarArraySort(&game->validWords, WordsSortingFunc, nullptr);
		u64 sortValidWordsTime = EndPerfTime(sortValidWordsTimerIndex);
		
		PrintLine_I("Loaded %u dictionaries (%llu words total) in " PERF_FORMAT_STR " (sorted in " PERF_FORMAT_STR ")", ArrayCount(dictionaryPaths), numWordsInDictionary, PERF_FORMAT(totalDictionaryTime), PERF_FORMAT(sortValidWordsTime));
		game->loadingDictionary = false;
	}
	
	if (game->loadingDictionary) { return; } // <==== Early out!
	
	// +==============================+
	// |    Update Incorrect Anim     |
	// +==============================+
	if (game->incorrectAnimProgress > 0.0f)
	{
		UpdateAnimationDown(&game->incorrectAnimProgress, INCORRECT_SHAKE_ANIM_TIME);
	}
	
	// +==============================+
	// |  Update Word List Page Anim  |
	// +==============================+
	if (game->currentWordListPageAnim < (r32)game->currentWordListPage)
	{
		UpdateAnimationUpTo(&game->currentWordListPageAnim, WORD_LIST_PAGE_TURN_ANIM_TIME, (r32)game->currentWordListPage);
	}
	else if (game->currentWordListPageAnim > (r32)game->currentWordListPage)
	{
		UpdateAnimationDownTo(&game->currentWordListPageAnim, WORD_LIST_PAGE_TURN_ANIM_TIME, (r32)game->currentWordListPage);
	}
	
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
		if (game->currentWord.length >= MIN_WORD_LENGTH)
		{
			WordTreeLeaf_t* dictionaryLeaf = WordTreeGetLeaf(&game->dictionary, game->currentWord);
			game->currentWordIsValid = (dictionaryLeaf != nullptr && dictionaryLeaf->value64 != 0);
			game->currentWordIsNotGuessed = (dictionaryLeaf != nullptr && dictionaryLeaf->value64 < game->nextValidWordId);
			PrintLine_D("\"%.*s\" %s a valid word%s", StrPrint(game->currentWord), game->currentWordIsValid ? "is" : "is NOT", game->currentWordIsNotGuessed ? "" : " (but was already guessed)");
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
		if (game->currentWord.length >= MIN_WORD_LENGTH)
		{
			if (game->currentWordIsValid && game->currentWordIsNotGuessed)
			{
				WordTreeLeaf_t* leaf = WordTreeGetLeaf(&game->dictionary, game->currentWord);
				Assert(leaf->value64 < game->nextValidWordId);
				ValidWord_t* validWord = FindValidWordById(leaf->value64);
				NotNull(validWord);
				Assert(!validWord->guessed);
				
				validWord->guessed = true;
				leaf->value64 = game->nextValidWordId;
				game->numValidWordsFound++;
				
				game->currentWord.length = 0;
				game->currentWordChanged = true;
				
			}
			else
			{
				game->incorrectAnimProgress = 1.0f;
			}
		}
		else
		{
			PrintLine_W("Please enter a %d character or more word!", MIN_WORD_LENGTH);
		}
	}
	
	// +==================================+
	// | Btn_Right/Btn_Left Change Pages  |
	// +==================================+
	if (BtnPressed(Btn_Right))
	{
		HandleBtnExtended(Btn_Right);
		if (game->currentWordListPage+1 < game->numWordListPages)
		{
			game->currentWordListPage++;
		}
	}
	if (BtnPressed(Btn_Left))
	{
		HandleBtnExtended(Btn_Left);
		if (game->currentWordListPage > 0)
		{
			game->currentWordListPage--;
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
		PdSetDrawMode(isLeafSelected ? kDrawModeInverted : kDrawModeCopy);
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
		if (game->incorrectAnimProgress > 0.0f)
		{
			i32 shakeStrength = RoundR32i(LerpR32(INCORRECT_SHAKE_STRENGTH_MIN, INCORRECT_SHAKE_STRENGTH_MAX, game->incorrectAnimProgress));
			currentWordPos.x += GetRandI32(&pig->random, 0, shakeStrength);
		}
		PdBindFont(&game->mainFont);
		PdSetDrawMode(kDrawModeXOR);
		PdDrawText(game->currentWord, currentWordPos);
		PdSetDrawMode(kDrawModeInverted);
	}
	
	// +==============================+
	// |       Render Validity        |
	// +==============================+
	if (game->currentWord.length >= MIN_WORD_LENGTH)
	{
		const char* validityText = (game->currentWordIsValid ? (game->currentWordIsNotGuessed ? "Valid!" : "Already Found") : "Nope");
		v2i validityTextPos = NewVec2i(VALIDITY_MARGIN_LEFT, VALIDITY_MARGIN_TOP);
		if (game->currentWordIsValid && game->currentWordIsNotGuessed) { validityTextPos.y = RoundR32i(Oscillate(VALIDITY_MARGIN_TOP, VALIDITY_MARGIN_TOP + VALIDITY_BOB_AMOUNT, VALIDITY_BOB_PERIOD)); }
		PdBindFont(&game->mainFont);
		PdSetDrawMode(kDrawModeXOR);
		PdDrawText(validityText, validityTextPos);
		PdSetDrawMode(kDrawModeInverted);
	}
	
	// +==============================+
	// |  Render numValidWordsFound   |
	// +==============================+
	{
		MyStr_t numFoundText = PrintInArenaStr(scratch, "%llu/%llu", game->numValidWordsFound, game->validWords.length);
		v2i numFoundTextSize = MeasureText(game->mainFont.font, numFoundText);
		v2i numFoundTextPos = NewVec2i(ScreenSize.width - NUM_FOUND_MARGIN_RIGHT - numFoundTextSize.width, NUM_FOUND_MARGIN_TOP);
		PdBindFont(&game->mainFont);
		PdSetDrawMode(kDrawModeXOR);
		PdDrawText(numFoundText, numFoundTextPos);
		PdSetDrawMode(kDrawModeInverted);
	}
	
	// +==============================+
	// |       Render Word List       |
	// +==============================+
	{
		char underscores[MAX_VALID_WORD_LENGTH];
		MyMemSet(&underscores[0], '_', MAX_VALID_WORD_LENGTH);
		
		reci oldClipRec = PdAddClipRec(game->wordListRec);
		PdBindFont(&pig->debugFont);
		
		u64 currentPage = (u64)FloorR32i(game->currentWordListPageAnim);
		for (u64 pIndex = 0; pIndex < 2; pIndex++)
		{
			v2i textPos = NewVec2i(
				game->wordListRec.x + RoundR32i(game->wordListRec.width * ((r32)currentPage - game->currentWordListPageAnim)),
				game->wordListRec.y
			);
			
			u64 startWordIndex = currentPage * (u64)game->wordListNumRows;
			u64 endWordIndex = MinU64((currentPage+1) * (u64)game->wordListNumRows, game->validWords.length);
			for (u64 wIndex = startWordIndex; wIndex < endWordIndex; wIndex++)
			{
				ValidWord_t* validWord = VarArrayGet(&game->validWords, wIndex, ValidWord_t);
				NotNull(validWord);
				if (validWord->guessed)
				{
					PdDrawText(validWord->word, textPos);
				}
				else if (pig->debugEnabled)
				{
					PdDrawTextPrint(textPos, "(%.*s)", StrPrint(validWord->word));
				}
				else
				{
					PdDrawText(NewStr(validWord->word.length, &underscores[0]), textPos);
				}
				textPos.y += pig->debugFont.lineHeight;
			}
			
			if (BasicallyEqualR32(game->currentWordListPageAnim, (r32)currentPage)) { break; } //no need to draw the second page
			currentPage++;
		}
		
		PdSetClipRec(oldClipRec);
	}
	
	// +==============================+
	// |         Debug Render         |
	// +==============================+
	if (pig->debugEnabled)
	{
		LCDBitmapDrawMode oldDrawMode = PdSetDrawMode(kDrawModeNXOR);
		
		// PdDrawRec(game->wordListRec, kColorBlack); //TODO: Remove me!
		
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
