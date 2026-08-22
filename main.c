/*
 * Sala do Neston - FIX da demo do Claude
 * PSn00bSDK - versão mínima funcionando
 */

#include <sys/types.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxapi.h>
#include <inline_c.h>
#include <psxetc.h>

#include "neston_tex.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define OT_LEN   8

typedef struct {
    DISPENV disp;
    DRAWENV draw;
} DB;

static DB db[2];
static int db_active = 0;
static uint32_t ot[2][OT_LEN];
static uint8_t pribuf[2][32768];
static char *nextpri;

static int overlayOpen = 0;
static int nearFrame = 1; // por enquanto sempre perto pra testar

static void init_graphics(void) {
    ResetGraph(0);

    SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_W, SCREEN_H);
    SetDefDrawEnv(&db[0].draw, 0, SCREEN_H, SCREEN_W, SCREEN_H);
    SetDefDispEnv(&db[1].disp, 0, SCREEN_H, SCREEN_W, SCREEN_H);
    SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_W, SCREEN_H);

    db[0].draw.isbg = 1;
    db[1].draw.isbg = 1;
    setRGB0(&db[0].draw, 20, 18, 24);
    setRGB0(&db[1].draw, 20, 18, 24);

    PutDispEnv(&db[0].disp);
    PutDrawEnv(&db[0].draw);

    // Sobe textura do Neston pra VRAM
    RECT texRect = {640, 0, NESTON_TEX_W, NESTON_TEX_H};
    LoadImage(&texRect, (u_long*)neston_tex);
    DrawSync(0);

    RECT clutRect = {640, 480, 16, 1};
    LoadImage(&clutRect, (u_long*)neston_clut);
    DrawSync(0);

    FntLoad(960, 0);
    FntOpen(0, 0, 320, 240, 0, 512);
}

int main(void) {
    // Pad correto do PSn00bSDK - não usa endereço 0x1F801000 direto
    uint8_t padBuf[2][34];
    PADTYPE *pad;

    InitPAD(padBuf[0], 34, padBuf[1], 34);
    StartPAD();
    ChangeClearPAD(1);

    init_graphics();

    int prevCross = 0;

    while (1) {
        pad = (PADTYPE*)padBuf[0];

        // --- INPUT ---
        if(pad->stat == 0) {
            int crossNow = !(pad->btn & PAD_CROSS);
            int circleNow = !(pad->btn & PAD_CIRCLE);
            int crossEdge = crossNow && !prevCross;
            prevCross = crossNow;

            if(crossEdge && nearFrame && !overlayOpen) overlayOpen = 1;
            if(circleNow && overlayOpen) overlayOpen = 0;
        }

        // --- DRAW ---
        db_active ^= 1;
        nextpri = (char*)pribuf[db_active];
        ClearOTagR(ot[db_active], OT_LEN);

        // Fundo (parede escura)
        POLY_F4 *bg = (POLY_F4*)nextpri;
        setPolyF4(bg);
        setRGB0(bg, 30, 20, 15);
        setXY4(bg, 0, 0, SCREEN_W, 0, 0, SCREEN_H, SCREEN_W, SCREEN_H);
        addPrim(ot[db_active], bg);
        nextpri += sizeof(POLY_F4);

        // Quadro do Neston - 128x128 no meio
        POLY_FT4 *quad = (POLY_FT4*)nextpri;
        setPolyFT4(quad);
        setRGB0(quad, 128, 128, 128);
        // centralizado
        setXY4(quad, 96, 56, 224, 56, 96, 184, 224, 184);
        setUV4(quad, 0, 0, 127, 0, 0, 127, 127, 127);
        quad->tpage = getTPage(0, 1, 640, 0); // 4bpp
        quad->clut = getClut(640, 480);
        addPrim(ot[db_active], quad);
        nextpri += sizeof(POLY_FT4);

        // Moldura do quadro (só 4 linhas em volta)
        POLY_F4 *mold = (POLY_F4*)nextpri;
        setPolyF4(mold);
        setRGB0(mold, 110, 70, 40);
        // top
        setXY4(mold, 90, 50, 230, 50, 90, 56, 230, 56);
        addPrim(ot[db_active]+1, mold);
        nextpri += sizeof(POLY_F4);

        // Texto
        if(!overlayOpen && nearFrame) {
            FntPrint("APERTE X - QUADRO DO NESTON\n");
        }

        if(overlayOpen) {
            // caixa preta por tras do texto
            POLY_F4 *box = (POLY_F4*)nextpri;
            setPolyF4(box);
            setRGB0(box, 0, 0, 0);
            setXY4(box, 20, 90, 300, 90, 20, 150, 300, 150);
            addPrim(ot[db_active]+2, box);
            nextpri += sizeof(POLY_F4);

            FntPrint("\n\n\n  SE CADA UM DER 1 REAL\n   -NESTON QUE JOGA\n\n   [O] FECHAR");
        }

        FntFlush(-1);

        DrawSync(0);
        VSync(0);
        PutDispEnv(&db[db_active].disp);
        PutDrawEnv(&db[db_active].draw);
        DrawOTag(ot[db_active] + OT_LEN - 1);
    }

    return 0;
}
