/*
 * SmartMatrix Library - Teensy 4 HUB75 Panel Calculation Class
 *
 * Copyright (c) 2020 Eric Eason and Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SMARTMATRIXCALCT4_H
#define SMARTMATRIXCALCT4_H

template <UINT16 refreshDepth, UINT16 matrixWidth, UINT16 matrixHeight, UINT8 panelType, uint32_t optionFlags>
class SmartMatrixHub75Calc {
    public:
        typedef typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct rowDataStruct;

        // init
        SmartMatrixHub75Calc(UINT8 bufferrows, volatile rowDataStruct * rowDataBuf);
        void begin(void);
        void addLayer(SM_Layer * newlayer);

        // configuration
        void setRotation(rotationDegrees newrotation);
        void setBrightness(uint8_t newBrightness);
        void setRefreshRate(uint16_t newRefreshRate);

        // get info
        uint16_t getScreenWidth(void) const;
        uint16_t getScreenHeight(void) const;
        uint16_t getRefreshRate(void);
        bool getdmaBufferUnderrunFlag(void);
        bool getRefreshRateLoweredFlag(void);

        // debug
        int countFPS(void);

        // functions called by ISR
        static void matrixCalculations(bool initial);
        static void dmaBufferUnderrunCallback(void);

    private:
        static SM_Layer * baseLayer;

        // functions for refreshing
        static void loadMatrixBuffers(UINT16 currentRow);
        static void loadMatrixBuffers48(volatile rowDataStruct * currentRowDataPtr, UINT16 currentRow);
        static void resetMultiRowRefreshMapPosition(void);
        static void resetMultiRowRefreshMapPositionPixelGroupToStartOfRow(void);
        static void advanceMultiRowRefreshMapToNextRow(void);
        static void advanceMultiRowRefreshMapToNextPixelGroup(void);
        static UINT16 getMultiRowRefreshRowOffset(void);
        static UINT16 getMultiRowRefreshNumPixelsToMap(void);
        static UINT16 getMultiRowRefreshPixelGroupOffset(void);

        // configuration
        static volatile bool brightnessChange;
        static volatile bool rotationChange;
        static volatile bool dmaBufferUnderrun;
        static uint8_t brightness;
        static rotationDegrees rotation;
        static UINT16 calc_refreshRate;
        static bool dmaBufferUnderrunSinceLastCheck;
        static bool refreshRateLowered;
        static bool refreshRateChanged;

        static INT16 multiRowRefresh_mapIndex_CurrentRowGroups;
        static INT16 multiRowRefresh_mapIndex_CurrentPixelGroup;
        static INT16 multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped;
        static INT16 multiRowRefresh_NumPanelsAlreadyMapped;
};

#endif
