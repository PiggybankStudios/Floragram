@echo off

set OUTPUT_PATH=..\data\Resources\Text\Dictionaries
set SCOWL_PATH=F:\gamedev\resources\Other\SCOWL\final
set SCOWL_MISC_PATH=F:\gamedev\resources\Other\SCOWL\misc

set INPUT_DICTIONARIES=missing_words.txt
set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.10
set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.20
set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.35
rem set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.40
rem set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.50
rem set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.55
rem set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.60
rem set INPUT_DICTIONARIES=%INPUT_DICTIONARIES% %SCOWL_PATH%\english-words.70

set BLACKLIST_DICTIONARIES=-words_i_dont_know.txt -words_i_find_offensive.txt
set BLACKLIST_DICTIONARIES=%BLACKLIST_DICTIONARIES% -%SCOWL_MISC_PATH%\offensive.1
set BLACKLIST_DICTIONARIES=%BLACKLIST_DICTIONARIES% -%SCOWL_MISC_PATH%\offensive.2
set BLACKLIST_DICTIONARIES=%BLACKLIST_DICTIONARIES% -%SCOWL_MISC_PATH%\profane.1

python ..\tools\FilterWordList.py #3 @%OUTPUT_PATH%\common_words3.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #4 @%OUTPUT_PATH%\common_words4.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #5 @%OUTPUT_PATH%\common_words5.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #6 @%OUTPUT_PATH%\common_words6.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #7 @%OUTPUT_PATH%\common_words7.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #8 @%OUTPUT_PATH%\common_words8.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #9 @%OUTPUT_PATH%\common_words9.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #10 @%OUTPUT_PATH%\common_words10.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #11 @%OUTPUT_PATH%\common_words11.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
python ..\tools\FilterWordList.py #12 @%OUTPUT_PATH%\common_words12.txt %INPUT_DICTIONARIES% %BLACKLIST_DICTIONARIES%
