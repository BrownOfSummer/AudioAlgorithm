/*************************************************************************
    > File Name: utils.c
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-08-16 14:52:49
 ************************************************************************/

#include<stdio.h>
#include<stdbool.h>
#include<string.h>

#define MAXWIDTH	2000
#define MAXHEIGHT	1200

int edge_step = 1;
int subtitles = 0;
int	logo_at_bottom = 0;
int	num_logo_buffers = 50;				// How many frames to compare at a time for logo detection;
int height = 1200;
int width = 2000;
int videowidth = 2000;
int edge_radius = 2;
int border = 10;

int tlogoMinX = 0;
int tlogoMaxX = 0;
int tlogoMinY = 0;
int tlogoMaxY = 0;

#define LOGOBORDER	4*edge_step
#define LOGO_X_LOOP for (x = edge_radius + border + LOGOBORDER; x < (videowidth - edge_radius - border- LOGOBORDER); x = (x==videowidth/3 ? 2*videowidth/3 : x+edge_step))

#define LOGO_Y_LOOP	int y_max_test = (subtitles? height/2 : (height - edge_radius - border - LOGOBORDER)); \
                    int y_step_test = height/3; \
                    for (y = (logo_at_bottom ? height/2 : edge_radius + border + LOGOBORDER); y < y_max_test; y = (y==y_step_test ? 2*height/3 : y+edge_step))
unsigned char thoriz_edgemask[MAXHEIGHT*MAXWIDTH];
unsigned char tvert_edgemask[MAXHEIGHT*MAXWIDTH];
char haslogo[MAXWIDTH*MAXHEIGHT];

bool SearchForLogoEdges(void)
{
    int		i;
    int		x;
    int		y;
    double scale = ((double)height / 572) * ( (double) videowidth / 720 );
    double	logoPercentageOfScreen;
    bool	LogoIsThere;
    int		sum;
    int		tempMinX;
    int		tempMaxX;
    int		tempMinY;
    int		tempMaxY;
    int		last_non_logo_frame;
    int		logoFound = false;
    tlogoMinX = edge_radius + border;
    tlogoMaxX = videowidth - edge_radius - border;
    tlogoMinY = edge_radius + border;
    tlogoMaxY = height - edge_radius - border;

    memset(thoriz_edgemask, 0, width * height);
    memset(tvert_edgemask, 0, width * height);
//	minY = (logo_at_bottom ? height/2 : edge_radius + (int)(height * borderIgnore));
//	if (framearray) minY = max(minY, frame[frame_count].minY);
//	maxY = (subtitles? height/2 : height - edge_radius - (int)(height * borderIgnore));
//	if (framearray) maxY = min(maxY, frame[frame_count].maxY);

    // loop width with step, add by lpj
    LOGO_X_LOOP
    {
        LOGO_Y_LOOP {
//	for (y = minY; y < maxY; y++) {
//		for (x = edge_radius + (int)(width * borderIgnore); x < videowidth - edge_radius + (int)(width * borderIgnore); x++) {
            if (hor_edgecount[y * width + x] >= num_logo_buffers * 0.95 ) // must get the hor_edgecount number in pre-process
            {
                thoriz_edgemask[y * width + x] = 1;
            }
            if (ver_edgecount[y * width + x] >= num_logo_buffers * 0.95 )
            {
                tvert_edgemask[y * width + x] = 1;
            }
        }
    }

    ClearEdgeMaskArea(thoriz_edgemask, tvert_edgemask);
    ClearEdgeMaskArea(tvert_edgemask, thoriz_edgemask);


    SetEdgeMaskArea(thoriz_edgemask);
    tempMinX = tlogoMinX;
    tempMaxX = tlogoMaxX;
    tempMinY = tlogoMinY;
    tempMaxY = tlogoMaxY;
    tlogoMinX = edge_radius + border;
    tlogoMaxX = videowidth - edge_radius - border;
    tlogoMinY = edge_radius + border;
    tlogoMaxY = height - edge_radius - border;
    SetEdgeMaskArea(tvert_edgemask);
    if (tempMinX < tlogoMinX) tlogoMinX = tempMinX;
    if (tempMaxX > tlogoMaxX) tlogoMaxX = tempMaxX;
    if (tempMinY < tlogoMinY) tlogoMinY = tempMinY;
    if (tempMaxY > tlogoMaxY) tlogoMaxY = tempMaxY;
    edgemask_filled = 1;
    logoPercentageOfScreen = (double)((tlogoMaxY - tlogoMinY) * (tlogoMaxX - tlogoMinX)) / (double)(height * width);
    if (logoPercentageOfScreen > logo_max_percentage_of_screen)
    {
//			Debug(
//				3,
//				"Reducing logo search area!\tPercentage of screen - %.2f%% TOO BIG.\n",
//				logoPercentageOfScreen * 100
//			);

//        if (tempMinX > tlogoMinX+50) tlogoMinX = tempMinX;
//        if (tempMaxX < tlogoMaxX-50) tlogoMaxX = tempMaxX;
//        if (tempMinY > tlogoMinY+50) tlogoMinY = tempMinY;
//        if (tempMaxY < tlogoMaxY-50) tlogoMaxY = tempMaxY;
    }

    i = CountEdgePixels();
//printf("Edges=%d\n",i);
//	if (i > 350/(lowres+1)/(edge_step)) {
    if ( i > 150 * scale /edge_step)
    {
        logoPercentageOfScreen = (double)((tlogoMaxY - tlogoMinY) * (tlogoMaxX - tlogoMinX)) / (double)(height * width);
        if (i > 40000 || logoPercentageOfScreen > logo_max_percentage_of_screen)
        {
            Debug(
                3,
                "Edge count - %i\tPercentage of screen - %.2f%% TOO BIG, CAN'T BE A LOGO.\n",
                i,
                logoPercentageOfScreen * 100
            );
//			logoInfoAvailable = false;
        }
        else
        {
            Debug(3, "Edge count - %i\tPercentage of screen - %.2f%%, Check: %i\n", i, logoPercentageOfScreen * 100,doublCheckLogoCount);
//			logoInfoAvailable = true;
            logoFound = true;
        }
    }
    else
        Debug(3, "Not enough edge count - %i\n", i);


    if (logoFound)
    {
        doublCheckLogoCount++;
        Debug(3, "Double checking - %i\n", doublCheckLogoCount );

        if (doublCheckLogoCount > 1)
        {
            // Final check done, found
        }
        else
            logoFound = false;
    }
    else
    {
        doublCheckLogoCount = 0;
    }


    sum = 0;
    oldestLogoBuffer = 0;
    for (i = 0; i < num_logo_buffers; i++)
    {
        if (logoFrameNum[i]  && logoFrameNum[i] < logoFrameNum[oldestLogoBuffer]) oldestLogoBuffer = i;
    }
    last_non_logo_frame = logoFrameNum[oldestLogoBuffer];
    if (logoFound)
    {
        Debug(3, "Doublechecking frames %i to %i for logo.\n", logoFrameNum[oldestLogoBuffer], logoFrameNum[newestLogoBuffer]);
        for (i = 0; i < num_logo_buffers; i++)
        {
            currentGoodEdge = DoubleCheckStationLogoEdge(logoFrameBuffer[i]);
            LogoIsThere = (currentGoodEdge > logo_threshold);

            for (x = logoFrameNum[i]; x < logoFrameNum[i] + (int)( logoFreq * fps ); x++)
            {
                frame[x].currentGoodEdge = currentGoodEdge;
                frame[x].logo_present = LogoIsThere;
                if (!LogoIsThere)
                {
                    if (x > last_non_logo_frame)
                        last_non_logo_frame = x;
                }
            }
            if (LogoIsThere)
            {
//				Debug(7, "Logo present in frame %i.\n", logoFrameNum[i]);
                sum++;
            }
            else
            {
                Debug(7, "Logo not present in frame %i.\n", logoFrameNum[i]);
            }
        }
    }


    if (logoFound && (sum >= (int)(num_logo_buffers * .9)))
    {

        clogoMinX = tlogoMinX;
        clogoMaxX = tlogoMaxX;
        clogoMinY = tlogoMinY;
        clogoMaxY = tlogoMaxY;
        memcpy(choriz_edgemask, thoriz_edgemask, width * height);
        memcpy(cvert_edgemask, tvert_edgemask, width * height);


        logoTrendCounter = num_logo_buffers;
        lastLogoTest = true;
        curLogoTest = true;

        logo_block[logo_block_count].start = last_non_logo_frame+1;
        DumpEdgeMasks();
//		DumpEdgeMask(choriz_edgemask, HORIZ);
//		DumpEdgeMask(cvert_edgemask, VERT);
//		for (i = 0; i < num_logo_buffers; i++) {
#if MULTI_EDGE_BUFFER
//			free(vert_edges[i]);
//			free(horiz_edges[i]);
#endif
//			free(logoFrameBuffer[i]);
//		}
#if MULTI_EDGE_BUFFER
//		free(vert_edges);
//		vert_edges = NULL
//		free(horiz_edges);
//		horiz_edges = NULL;
#else
//		free(horiz_count);
//		horiz_count = NULL;
//		free(vert_count);
//		vert_count = NULL;
#endif
//		free(logoFrameBuffer);
//		logoFrameBuffer = NULL;
        InitScanLines();
        InitHasLogo();

        logoInfoAvailable = true; //xxxxxxx
    }
    else
    {
//		logoInfoAvailable = false; //xxxxxxx
        currentGoodEdge = 0.0;
    }

    if (!logoInfoAvailable && startOverAfterLogoInfoAvail && (framenum_real > (int)(giveUpOnLogoSearch * fps)))
    {
        Debug(1, "No logo was found after %i frames.\nGiving up", framenum_real);
        commDetectMethod -= LOGO;
    }
    if (added_recording > 0)
        giveUpOnLogoSearch += added_recording * 60;

    if (logoInfoAvailable && startOverAfterLogoInfoAvail)
    {
        Debug(3, "Logo found at frame %i\tlogoMinX=%i\tlogoMaxX=%i\tlogoMinY=%i\tlogoMaxY=%i\n", framenum_real, clogoMinX, clogoMaxX, clogoMinY, clogoMaxY);
        SaveLogoMaskData();
        Debug(3, "******************* End of Logo Processing ***************\n");
        return false;
    }

    return true;
}

