/*************************************************************************
    > File Name: tmp.c
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-08-16 14:44:12
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
// Function Prototypes
bool				BuildBlocks(bool recalc);
void				Recalc(void);
double				ValidateBlackFrames(long reason, double ratio, int remove);
int					DetectCommercials(int, double);
bool				BuildMasterCommList(void);
void				WeighBlocks(void);
bool				OutputBlocks(void);
void        OutputAspect(void);
void        OutputTraining(void);
bool ProcessLogoTest(int framenum_real, int curLogoTest, int close);
void        OutputStrict(double len, double delta, double tol);
int					InputReffer(char *ext, int setfps);
bool				IsStandardCommercialLength(double length, double tolerance, bool strict);
bool				LengthWithinTolerance(double test_length, double expected_length, double tolerance);
double				FindNumber(char* str1, char* str2, double v);
char *				FindString(char* str1, char* str2, char *v);
void				AddIniString( char *s);
char*				intSecondsToStrMinutes(int seconds);
char*				dblSecondsToStrMinutes(double seconds);
FILE*				LoadSettings(int argc, char ** argv);
int					GetAvgBrightness(void);
bool				CheckFrameIsBlack(void);
void				BuildBlackFrameCommList(void);
bool				CheckSceneHasChanged(void);
#if 0
void				BuildSceneChangeCommList(void);
void				BuildSceneChangeCommList2(void);
#endif
void                backfill_frame_volumes();
void				PrintLogoFrameGroups(void);
void				PrintCCBlocks(void);
void				ResetLogoBuffers(void);
void				EdgeDetect(unsigned char* frame_ptr, int maskNumber);
void				EdgeCount(unsigned char* frame_ptr);
void				FillLogoBuffer(void);
bool				SearchForLogoEdges(void);
double				CheckStationLogoEdge(unsigned char* testFrame);
double				DoubleCheckStationLogoEdge(unsigned char* testFrame);
void				SetEdgeMaskArea(unsigned char* temp);
int					ClearEdgeMaskArea(unsigned char* temp, unsigned char* test);
int					CountEdgePixels(void);
void				DumpEdgeMask(unsigned char* buffer, int direction);
void				DumpEdgeMasks(void);
void				BuildBlackFrameAndLogoCommList(void);
bool				CheckFramesForLogo(int start, int end);
char				CheckFramesForCommercial(int start, int end);
char				CheckFramesForReffer(int start, int end);
void				SaveLogoMaskData(void);
void				LoadLogoMaskData(void);
double				CalculateLogoFraction(int start, int end);
bool				CheckFrameForLogo(int i);
int					CountSceneChanges(int StartFrame, int EndFrame);
void				Debug(int level, char* fmt, ...);
void				InitProcessLogoTest(void);
void				InitComSkip(void);
void				InitLogoBuffers(void);
void				FindIniFile(void);
double				FindScoreThreshold(double percentile);
void				OutputLogoHistogram(int buckets);
void				OutputbrightHistogram(void);
void				OutputuniformHistogram(void);
void				OutputHistogram(int *histogram, int scale, char *title, bool truncate);
int					FindBlackThreshold(double percentile);
int					FindUniformThreshold(double percentile);
void				OutputFrameArray(bool screenOnly);
void                OutputBlackArray();
void				OutputFrame();
void				OpenOutputFiles();
void				InitializeFrameArray(long i);
void				InitializeBlackArray(long i);
void				InitializeSchangeArray(long i);
void				InitializeLogoBlockArray(long i);
void				InitializeARBlockArray(long i);
void				InitializeACBlockArray(long i);
void				InitializeBlockArray(long i);
void				InitializeCCBlockArray(long i);
void				InitializeCCTextArray(long i);
void				PrintArgs(void);
void        close_dump(void);
void				OutputCommercialBlock(int i, long prev, long start, long end, bool last);
void				ProcessCSV(FILE *);
void				OutputCCBlock(long i);
void				ProcessCCData(void);
bool				CheckOddParity(unsigned char ch);
void				AddNewCCBlock(long current_frame, int type, bool cc_on_screen, bool cc_in_memory);
char*				CCTypeToStr(int type);
int					DetermineCCTypeForBlock(long start, long end);
double				AverageARForBlock(int start, int end);
void				SetARofBlocks(void);
bool				ProcessCCDict(void);
int					FindBlock(long frame);
void				BuildCommListAsYouGo(void);
void				BuildCommercial(void);
int					RetreiveVolume (int f);
void InsertBlackFrame(int f, int b, int u, int v, int c);
extern void DecodeOnePicture(FILE * f, double pts);
