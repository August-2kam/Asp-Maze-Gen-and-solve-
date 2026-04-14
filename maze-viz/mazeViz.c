#include "mazeApp.h"
#include <SDL2/SDL.h>
#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANEL_WIDTH          360
#define WINDOW_MARGIN        20
#define CONTENT_GAP          18
#define TOP_BAR_HEIGHT       22
#define FOOTER_HEIGHT        180
#define LEFT_MIN_WIDTH       480
#define MIN_CELL_SIZE        3
#define MAX_CELL_SIZE        34
#define MAZE_CARD_PAD        10
#define FOOTER_MIN_HEIGHT    156

// footer layout constants
#define FOOTER_STATUS_H      28
#define FOOTER_BTN_H         40
#define FOOTER_BTN_W         128
#define FOOTER_BTN_GAP       14
#define FOOTER_HINT_H        20
#define FOOTER_PAD           10

typedef struct
{
    SDL_Rect rect;
    bool hovered;
    bool pressed;
} Button;

typedef enum
{
    INFO_NONE,
    INFO_EDGES,
    INFO_SOLVER
} InfoType;

typedef struct
{
    uint8_t **mazes;
    PathSet *paths;
    AnswerSets *parentSets;
    bool *showPath;

    int mazeCount;
    int current;
    int rows;
    int cols;
    int cellSize;
    int winW;
    int winH;

    InfoType infoType;
    char **infoLines;
    int infoLineCount;
    int infoScrollOffset;
} AppState;

typedef struct
{
    SDL_Rect mazeCard;
    SDL_Rect mazeArea;
    SDL_Rect footerPanel;
    SDL_Rect infoPanel;
    int leftPaneW;
} Layout;

typedef struct
{
    int statusY;
    int div1Y;
    int btnRowY;
    int btnStartX;
    int btnW;
    int btnH;
    int btnGap;
    int div2Y;
    int hintY;
} FooterMetrics;


//Augustine Mochoeneng
//GMTK game jam code - 2021


//clamp an integer to be in the interval [lb,ub]
static inline
int clampi(int x, int lb, int ub)
{
    if (x < lb) return lb;
    if (x > ub) return ub;
    return x;
}

//2D coordinates to 1D index
static inline
int getIndexFromRowCol(int rows, int cols, int r, int c)
{
    (void)rows;
    return r * cols + c;
}


//Check if a point is located inside the rectangle r,
static inline bool
pointInRect(int x, int y, SDL_Rect r)
{
    return x >= r.x && x < r.x + r.w &&
           y >= r.y && y < r.y + r.h;
}

//Embed seperate r, g ,b values to one 4byte int
static inline uint32_t
rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

//A wrapper function to change the rederer draw color
static void
setColor(SDL_Renderer *ren, uint32_t color)
{
    SDL_SetRenderDrawColor(
        ren,
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF,
        255
    );
}

//A wrapper function to draw a filled rectangle
static void
fillRect(SDL_Renderer *ren, SDL_Rect r, uint32_t color)
{
    setColor(ren, color);
    SDL_RenderFillRect(ren, &r);
}

//A wrapper function to draw an unfilled rectangle
static void
strokeRect(SDL_Renderer *ren, SDL_Rect r, uint32_t color)
{
    setColor(ren, color);
    SDL_RenderDrawRect(ren, &r);
}



//Simpl text rendering lib  in SDL
//Augustine Mochoeneng
//07 June 2022

typedef struct
{
    char ch;
    const char *rows[7];
} Glyph;

static const Glyph GLYPHS[] = {
    {'0', {"11111","10001","10011","10101","11001","10001","11111"}},
    {'1', {"00100","01100","00100","00100","00100","00100","01110"}},
    {'2', {"11111","00001","00001","11111","10000","10000","11111"}},
    {'3', {"11111","00001","00001","01111","00001","00001","11111"}},
    {'4', {"10001","10001","10001","11111","00001","00001","00001"}},
    {'5', {"11111","10000","10000","11111","00001","00001","11111"}},
    {'6', {"11111","10000","10000","11111","10001","10001","11111"}},
    {'7', {"11111","00001","00010","00100","01000","01000","01000"}},
    {'8', {"11111","10001","10001","11111","10001","10001","11111"}},
    {'9', {"11111","10001","10001","11111","00001","00001","11111"}},
    {'A', {"01110","10001","10001","11111","10001","10001","10001"}},
    {'B', {"11110","10001","10001","11110","10001","10001","11110"}},
    {'C', {"01111","10000","10000","10000","10000","10000","01111"}},
    {'D', {"11110","10001","10001","10001","10001","10001","11110"}},
    {'E', {"11111","10000","10000","11110","10000","10000","11111"}},
    {'F', {"11111","10000","10000","11110","10000","10000","10000"}},
    {'G', {"01111","10000","10000","10011","10001","10001","01111"}},
    {'H', {"10001","10001","10001","11111","10001","10001","10001"}},
    {'I', {"11111","00100","00100","00100","00100","00100","11111"}},
    {'J', {"11111","00010","00010","00010","00010","10010","01100"}},
    {'K', {"10001","10010","10100","11000","10100","10010","10001"}},
    {'L', {"10000","10000","10000","10000","10000","10000","11111"}},
    {'M', {"10001","11011","10101","10001","10001","10001","10001"}},
    {'N', {"10001","11001","10101","10011","10001","10001","10001"}},
    {'O', {"01110","10001","10001","10001","10001","10001","01110"}},
    {'P', {"11110","10001","10001","11110","10000","10000","10000"}},
    {'Q', {"01110","10001","10001","10001","10101","10010","01101"}},
    {'R', {"11110","10001","10001","11110","10100","10010","10001"}},
    {'S', {"01111","10000","10000","01110","00001","00001","11110"}},
    {'T', {"11111","00100","00100","00100","00100","00100","00100"}},
    {'U', {"10001","10001","10001","10001","10001","10001","01110"}},
    {'V', {"10001","10001","10001","10001","10001","01010","00100"}},
    {'W', {"10001","10001","10001","10101","10101","11011","10001"}},
    {'X', {"10001","01010","00100","00100","00100","01010","10001"}},
    {'Y', {"10001","01010","00100","00100","00100","00100","00100"}},
    {'Z', {"11111","00001","00010","00100","01000","10000","11111"}},
    {'/', {"00001","00010","00100","00100","01000","10000","00000"}},
    {'-', {"00000","00000","00000","11111","00000","00000","00000"}},
    {' ', {"00000","00000","00000","00000","00000","00000","00000"}},
    {'(', {"00010","00100","01000","01000","01000","00100","00010"}},
    {')', {"01000","00100","00010","00010","00010","00100","01000"}},
    {',', {"00000","00000","00000","00000","00100","00100","01000"}},
    {'.', {"00000","00000","00000","00000","00000","01100","01100"}},
    {':', {"00000","01100","01100","00000","01100","01100","00000"}},
    {'+', {"00000","00100","00100","11111","00100","00100","00000"}},
    {'_', {"00000","00000","00000","00000","00000","00000","11111"}},
};


//search the glphs array to find a corrosponding bitmap for a char
//optimize if we add more characters
static const Glyph*
findGlyph(char ch)
{
    size_t n = sizeof(GLYPHS) / sizeof(GLYPHS[0]);
    for (size_t i = 0; i < n; i++)
        if (GLYPHS[i].ch == ch) return &GLYPHS[i];

    return &GLYPHS[n - 1];
}

//draw a char on to the display. (x,y) top left cordinates
static void
drawChar(SDL_Renderer *ren, int x, int y,
                                   int scale,
                                   char ch,
                                   uint32_t color)
{
    const Glyph *g = findGlyph(ch);
    setColor(ren, color);

    for (int r = 0; r < 7; r++)
    {
        for (int c = 0; c < 5; c++)
        {
            if (g->rows[r][c] == '1') {
                SDL_Rect px = {x + c * scale, y + r * scale, scale, scale};
                SDL_RenderFillRect(ren, &px);
            }
        }
    }
}


//Draw a null terminated string into the display , call the drawChar func above
static void
drawText(SDL_Renderer *ren, int x, int y,
                                   int scale,
                                   const char *text,
                                   uint32_t color)
{
    for (int i = 0; text[i] != '\0'; i++)
        drawChar(ren, x + i * 6 * scale, y, scale, text[i], color);

}


//determine the width of the text ...estimate really
static int
textWidth(const char *text, int scale)
{
    return (int)strlen(text) * 6 * scale;   // 6 -> bitmap is (7 by 5)
}

static int
getInfoTextScale(const AppState *app)
{
    if (!app) return 2;

    if (app->infoType == INFO_SOLVER || app->infoType == INFO_EDGES)
        return 1;

    return 2;
}

static int
getInfoWrapChars(const AppState *app)
{
    int scale = getInfoTextScale(app);
    int usableW = PANEL_WIDTH - 44;
    int cols = usableW / (6 * scale);

    if (cols < 12)
        cols = 12;

    return cols;
}

//turn lower case chars into upper and any chars that are not supported
//yet are replaced by " "
static void
sanitizeFontText(char *dst, size_t dstSize, const char *src)
{
    if (!dst || dstSize == 0) return;

    size_t i = 0;
    for (; src[i] != '\0' && i + 1 < dstSize; i++)
    {
        unsigned char ch = (unsigned char)src[i];

        if (ch >= 'a' && ch <= 'z')
            ch = (unsigned char)(ch - ('a' - 'A'));

        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == ' ' || ch == '/'   || ch == '-' ||
            ch == '(' || ch == ')'   || ch == ',' ||
            ch == '.' || ch == ':'   || ch == '+' ||
            ch == '_'
           ) dst[i] = (char)ch;
        else
            dst[i] = ' ';

    }
    dst[i] = '\0';
}

//A wrapper function for the drawing text to screen
static void
drawTextSafe(SDL_Renderer *ren, int x, int y,
                                       int scale,
                                       const char *text,
                                       uint32_t color)
{
    char buf[1024];
    sanitizeFontText(buf, sizeof(buf), text);
    drawText(ren, x, y, scale, buf, color);
}



//29-march-2026
//side panel stuff
//
static void freeInfoLines(AppState *app)
{
    if (!app) return;

    if (app->infoLines)
    {
        for (int i = 0; i < app->infoLineCount; i++)
            free(app->infoLines[i]);

        free(app->infoLines);
    }

    app->infoLines = NULL;
    app->infoLineCount = 0;
    app->infoScrollOffset = 0;
}

static void
clearInfoPanel(AppState *app)
{
    freeInfoLines(app);
    app->infoType = INFO_NONE;
}

static void
pushInfoLine(AppState *app, const char *s)
{
    if (!app || !s) return;

    char **newLines = realloc(app->infoLines, (size_t)(app->infoLineCount + 1) * sizeof(char *));
    if (!newLines) return;

    app->infoLines = newLines;
    app->infoLines[app->infoLineCount] = strdup(s);
    if (app->infoLines[app->infoLineCount]) {
        app->infoLineCount++;
    }
}

static void
pushWrappedInfoLine(AppState *app, const char *s, int maxChars)
{
    if (!app || !s) return;

    if (maxChars < 8 || (int)strlen(s) <= maxChars)
    {
        pushInfoLine(app, s);
        return;
    }

    size_t start = 0;
    size_t len = strlen(s);

    while (start < len)
    {
        size_t chunk = len - start;
        if ((int)chunk > maxChars)
            chunk = (size_t)maxChars;

        if (start + chunk < len)
        {
            size_t split = start + chunk;
            while (split > start && s[split] != ' ')
                split--;

            if (split > start + (size_t)(maxChars / 3))
                chunk = split - start;
        }

        char buf[1024];
        if (chunk >= sizeof(buf))
            chunk = sizeof(buf) - 1;

        memcpy(buf, s + start, chunk);
        buf[chunk] = '\0';

        pushInfoLine(app, buf);

        start += chunk;
        while (start < len && s[start] == ' ')
            start++;
    }
}

//window title- does not work on other window managers
static void
updateTitle(SDL_Window *window, const AppState *app)
{
    char buf[128];
    snprintf(
        buf, sizeof(buf),
        "Maze Viewer | Maze %d/%d | %dx%d | %s",
        app->mazeCount == 0 ? 0 : app->current + 1,
        app->mazeCount,
        app->rows,
        app->cols,
        (app->showPath && app->showPath[app->current]) ? "PATH ON" : "PATH OFF"
    );
    SDL_SetWindowTitle(window, buf);
}

static FooterMetrics
computeFooterMetrics(const Layout *L)
{
    FooterMetrics M;
    memset(&M, 0, sizeof(M));

    int innerW = L->footerPanel.w - FOOTER_PAD * 2;

    M.statusY = L->footerPanel.y + FOOTER_PAD + 2;
    M.div1Y   = M.statusY + FOOTER_STATUS_H + FOOTER_PAD;

    M.btnGap = FOOTER_BTN_GAP;
    M.btnW   = FOOTER_BTN_W;
    M.btnH   = FOOTER_BTN_H;

    if (5 * M.btnW + 4 * M.btnGap > innerW)
    {
        M.btnGap = 8;
        M.btnW = (innerW - 4 * M.btnGap) / 5;

        if (M.btnW < 76)
            M.btnW = 76;

        if (M.btnW < 96)
            M.btnH = 36;
    }

    {
        int totalW = 5 * M.btnW + 4 * M.btnGap;
        M.btnStartX = L->footerPanel.x + (L->footerPanel.w - totalW) / 2;
        if (M.btnStartX < L->footerPanel.x + FOOTER_PAD)
            M.btnStartX = L->footerPanel.x + FOOTER_PAD;
    }

    M.btnRowY = M.div1Y + FOOTER_PAD + 4;
    M.div2Y   = M.btnRowY + M.btnH + FOOTER_PAD + 4;
    M.hintY   = M.div2Y + FOOTER_PAD;

    return M;
}

static int
getButtonTextScale(const Button *btn, const char *label)
{
    if (!btn || !label) return 2;

    if (textWidth(label, 2) + 18 <= btn->rect.w)
        return 2;

    return 1;
}

static int
getInfoVisibleLines(const AppState *app, const Layout *L)
{
    int textScale = getInfoTextScale(app);
    int lineStep = (textScale == 1) ? 12 : 18;
    int usableH = L->infoPanel.h - 86;
    int maxLines = usableH / lineStep;

    if (maxLines < 1)
        maxLines = 1;

    return maxLines;
}


//web dev math
static Layout
computeLayout(const AppState *app)
{
    Layout L;
    memset(&L, 0, sizeof(L));

    L.leftPaneW = app->winW - PANEL_WIDTH - CONTENT_GAP;
    if (L.leftPaneW < LEFT_MIN_WIDTH)
        L.leftPaneW = LEFT_MIN_WIDTH;

    int mazePixelW = app->cols * app->cellSize;
    int mazePixelH = app->rows * app->cellSize;

    int mazeCardW = mazePixelW + MAZE_CARD_PAD * 2;
    int mazeCardH = mazePixelH + MAZE_CARD_PAD * 2;

    int leftInnerX = WINDOW_MARGIN;
    int leftInnerW = L.leftPaneW - WINDOW_MARGIN * 2;

    int mazeCardX = leftInnerX + (leftInnerW - mazeCardW) / 2;
    if (mazeCardX < WINDOW_MARGIN)
        mazeCardX = WINDOW_MARGIN;

    int mazeCardY = WINDOW_MARGIN;

    L.mazeCard = (SDL_Rect){mazeCardX, mazeCardY, mazeCardW, mazeCardH};
    L.mazeArea = (SDL_Rect){mazeCardX + MAZE_CARD_PAD, mazeCardY + MAZE_CARD_PAD, mazePixelW, mazePixelH};

    int footerY = L.mazeCard.y + L.mazeCard.h + 12;
    int footerH = app->winH - footerY - WINDOW_MARGIN;

    if (footerH < FOOTER_MIN_HEIGHT)
        footerH = FOOTER_MIN_HEIGHT;

    L.footerPanel = (SDL_Rect){
        WINDOW_MARGIN,
        footerY,
        L.leftPaneW - WINDOW_MARGIN * 2,
        footerH
    };

    L.infoPanel = (SDL_Rect){
        L.leftPaneW + CONTENT_GAP,
        WINDOW_MARGIN,
        PANEL_WIDTH - WINDOW_MARGIN,
        app->winH - WINDOW_MARGIN * 2
    };

    return L;
}


//draw buttons
//TOD0: make press and hover colors global
static void
drawButton(SDL_Renderer *ren, Button *btn, const char *label)
{
    uint32_t fill =
        btn->pressed ? rgb(70, 92, 138) :
        btn->hovered ? rgb(84, 104, 148) :
                       rgb(56, 68, 94);

    uint32_t top =
        btn->pressed ? rgb(54, 74, 112) :
        btn->hovered ? rgb(96, 118, 164) :
                       rgb(70, 84, 112);

    uint32_t edge =
        btn->pressed ? rgb(184, 198, 220) :
                       rgb(212, 218, 228);

    fillRect(ren, btn->rect, fill);

    SDL_Rect gloss = {
        btn->rect.x + 1,
        btn->rect.y + 1,
        btn->rect.w - 2,
        btn->rect.h / 2 - 1
    };
    if (gloss.w > 0 && gloss.h > 0)
        fillRect(ren, gloss, top);

    strokeRect(ren, btn->rect, edge);

    int scale = getButtonTextScale(btn, label);
    int tw = textWidth(label, scale);
    int tx = btn->rect.x + (btn->rect.w - tw) / 2;
    int ty = btn->rect.y + (btn->rect.h - 7 * scale) / 2;
    drawTextSafe(ren, tx, ty, scale, label, rgb(245, 247, 250));
}


static void
drawCellWalls(SDL_Renderer *ren, int x, int y, int cellSize, uint8_t walls)
{
    setColor(ren, rgb(236, 239, 244));

    if (walls & NORTH) SDL_RenderDrawLine(ren, x, y, x + cellSize, y);
    if (walls & EAST)  SDL_RenderDrawLine(ren, x + cellSize, y, x + cellSize, y + cellSize);
    if (walls & SOUTH) SDL_RenderDrawLine(ren, x, y + cellSize, x + cellSize, y + cellSize);
    if (walls & WEST)  SDL_RenderDrawLine(ren, x, y, x, y + cellSize);
}


//get the centeru cordinares  of a cell
static void
cellIdToCenter(int id1, int cols, SDL_Rect area,
                                  int cellSize,
                                  int *x,
                                  int *y)
{
    int id0 = id1 - 1;
    int r = id0 / cols;
    int c = id0 % cols;

    *x = area.x + c * cellSize + cellSize / 2;
    *y = area.y + r * cellSize + cellSize / 2;
}

static void
drawPathSegment(SDL_Renderer *ren, int x1, int y1, int x2, int y2, int thickness)
{
    SDL_RenderDrawLine(ren, x1, y1, x2, y2);

    if (thickness >= 2)
    {
        if (y1 == y2)
            SDL_RenderDrawLine(ren, x1, y1 + 1, x2, y2 + 1);
        else
            SDL_RenderDrawLine(ren, x1 + 1, y1, x2 + 1, y2);
    }

    if (thickness >= 3)
    {
        if (y1 == y2)
            SDL_RenderDrawLine(ren, x1, y1 - 1, x2, y2 - 1);
        else
            SDL_RenderDrawLine(ren, x1 - 1, y1, x2 - 1, y2);
    }
}

//paths visualization
//TODO:optimized for larger mazes
static void
drawSolution(SDL_Renderer *ren, const AppState *app, SDL_Rect area)
{
    // sanity checks
    if (!app || !app->paths || !app->showPath || !app->showPath[app->current]) return;
    if (app->current < 0 || app->current >= app->paths->count) return;

    const PathModel *m = &app->paths->mods[app->current];
    if (!m || m->count <= 0) return;

    // build adjacency list from the path edges
    int maxNode = app->rows * app->cols;   // nodes are 1-based IDs
    typedef struct NodeLink { int to; struct NodeLink *next; } NodeLink;
    NodeLink *adj[maxNode + 1];
    for (int i = 1; i <= maxNode; i++) adj[i] = NULL;

    for (int i = 0; i < m->count; i++) {
        int u = m->edges[i].from;
        int v = m->edges[i].to;
        // add v to adj[u]
        NodeLink *link = malloc(sizeof(NodeLink));
        if (!link) continue;
        link->to = v;
        link->next = adj[u];
        adj[u] = link;
        // add u to adj[v]
        link = malloc(sizeof(NodeLink));
        if (!link) continue;
        link->to = u;
        link->next = adj[v];
        adj[v] = link;
    }

    int start = 1;
    int goal = maxNode;

    int visited[maxNode + 1];
    memset(visited, 0, sizeof(visited));
    int parent[maxNode + 1];
    memset(parent, -1, sizeof(parent));

    int stack[maxNode + 1];
    int top = 0;
    stack[top++] = start;
    visited[start] = 1;

    while (top > 0) {
        int u = stack[--top];
        if (u == goal) break;
        for (NodeLink *l = adj[u]; l; l = l->next) {
            int v = l->to;
            if (!visited[v]) {
                visited[v] = 1;
                parent[v] = u;
                stack[top++] = v;
            }
        }
    }

    int thickness =
        app->cellSize >= 18 ? 3 :
        app->cellSize >= 11 ? 2 : 1;

    // If a path exists, draw a continuous polyline
    if (visited[goal]) {
        // reconstruct path from goal to start
        int path[maxNode];
        int pathLen = 0;
        int cur = goal;
        while (cur != -1) {
            path[pathLen++] = cur;
            cur = parent[cur];
        }
        // draw lines between consecutive nodes (in reverse order)
        setColor(ren, rgb(80, 200, 255));
        for (int i = pathLen - 1; i > 0; i--) {
            int x1, y1, x2, y2;
            cellIdToCenter(path[i],   app->cols, area, app->cellSize, &x1, &y1);
            cellIdToCenter(path[i-1], app->cols, area, app->cellSize, &x2, &y2);
            drawPathSegment(ren, x1, y1, x2, y2, thickness);
        }
    } else {
        // No continuous path found – fall back to drawing individual edges
        setColor(ren, rgb(80, 200, 255));
        for (int i = 0; i < m->count; i++) {
            int x1, y1, x2, y2;
            cellIdToCenter(m->edges[i].from, app->cols, area, app->cellSize, &x1, &y1);
            cellIdToCenter(m->edges[i].to,   app->cols, area, app->cellSize, &x2, &y2);
            drawPathSegment(ren, x1, y1, x2, y2, thickness);
        }
    }

    // free adjacency list
    for (int i = 1; i <= maxNode; i++) {
        NodeLink *l = adj[i];
        while (l) {
            NodeLink *tmp = l;
            l = l->next;
            free(tmp);
        }
    }
}

//wrapper functions
static void
drawMazeCard(SDL_Renderer *ren, SDL_Rect card)
{
    fillRect(ren, card, rgb(18, 22, 30));
    strokeRect(ren, card, rgb(78, 86, 102));

    SDL_Rect inner = {card.x + 4, card.y + 4, card.w - 8, card.h - 8};
    if (inner.w > 0 && inner.h > 0)
        strokeRect(ren, inner, rgb(34, 40, 54));
}


static void
drawMaze(SDL_Renderer *ren, const AppState *app, SDL_Rect area)
{
    if (!app || !app->mazes || app->mazeCount <= 0) return;

    //border rect
    fillRect(ren, area, rgb(16, 20, 28));

    //outer walls of the maze
    if (app->cellSize >= 10)
    {
        setColor(ren, rgb(42, 48, 64));
        for (int x = area.x; x <= area.x + app->cols * app->cellSize; x += app->cellSize)
            SDL_RenderDrawLine(ren, x, area.y, x, area.y + app->rows * app->cellSize);

        for (int y = area.y; y <= area.y + app->rows * app->cellSize; y += app->cellSize)
            SDL_RenderDrawLine(ren, area.x, y, area.x + app->cols * app->cellSize, y);
    }

    //individual cells
    uint8_t *maze = app->mazes[app->current];
    for (int r = 0; r < app->rows; r++)
    {
        for (int c = 0; c < app->cols; c++)
        {
            int idx = getIndexFromRowCol(app->rows, app->cols, r, c);
            int x = area.x + c * app->cellSize;
            int y = area.y + r * app->cellSize;
            drawCellWalls(ren, x, y, app->cellSize, maze[idx]);
        }
    }

    //solution if button was pressed
    drawSolution(ren, app, area);

    strokeRect(ren, area, rgb(255, 255, 255));

    {
        int markerPad = clampi(app->cellSize / 4, 1, 6);
        int markerSize = app->cellSize - markerPad * 2;
        if (markerSize < 1) markerSize = 1;

        SDL_Rect start = {area.x + markerPad, area.y + markerPad, markerSize, markerSize};
        SDL_Rect goal  = {
            area.x + (app->cols - 1) * app->cellSize + markerPad,
            area.y + (app->rows - 1) * app->cellSize + markerPad,
            markerSize,
            markerSize
        };

        fillRect(ren, start, rgb(76, 175, 80));
        fillRect(ren, goal, rgb(244, 67, 54));
    }
}


// draw the footer panel
// layout (top to bottom): dev math
//   FOOTER_PAD
//   status row  (FOOTER_STATUS_H px tall)
//   FOOTER_PAD
//   thin divider line
//   FOOTER_PAD
//   button row  (FOOTER_BTN_H px tall)
//   FOOTER_PAD
//   thin divider line
//   FOOTER_PAD
//   hint row    (FOOTER_HINT_H px tall)
//   FOOTER_PAD
static void
drawFooterPanel(SDL_Renderer *ren, const AppState *app, const Layout *L)
{
    FooterMetrics M = computeFooterMetrics(L);

    fillRect(ren, L->footerPanel, rgb(18, 22, 30));
    strokeRect(ren, L->footerPanel, rgb(70, 78, 94));

    SDL_Rect accent = {
        L->footerPanel.x + 1,
        L->footerPanel.y + 1,
        L->footerPanel.w - 2,
        3
    };
    fillRect(ren, accent, rgb(74, 142, 182));

    int px = L->footerPanel.x;
    int pw = L->footerPanel.w;


    char mazeInfo[64];
    snprintf(mazeInfo, sizeof(mazeInfo), "MAZE %d/%d", app->current + 1, app->mazeCount);

    char dimInfo[32];
    snprintf(dimInfo, sizeof(dimInfo), "DIM %dX%d", app->rows, app->cols);

    const char *mode = "NONE";
    if (app->infoType == INFO_EDGES) mode = "EDGES";
    else if (app->infoType == INFO_SOLVER) mode = "SOLVER";

    char modeInfo[64];
    snprintf(modeInfo, sizeof(modeInfo), "VIEW %s", mode);

    char pathInfo[32];
    snprintf(pathInfo, sizeof(pathInfo), "PATH %s",
             (app->showPath && app->showPath[app->current]) ? "ON" : "OFF");

    const char *tags[4] = {mazeInfo, dimInfo, modeInfo, pathInfo};
    uint32_t tagFill[4] = {
        rgb(38, 46, 60),
        rgb(34, 52, 42),
        rgb(28, 48, 58),
        rgb(52, 40, 58)
    };
    uint32_t tagTextColor[4] = {
        rgb(236, 239, 244),
        rgb(163, 190, 140),
        rgb(136, 192, 208),
        rgb(180, 142, 173)
    };

    int tagScale = 2;
    int tagPadX = 10;
    int tagPadY = 4;
    int tagGap = 10;
    int totalTagW = 0;

    for (int i = 0; i < 4; i++)
    {
        totalTagW += textWidth(tags[i], tagScale) + tagPadX * 2;
        if (i < 3) totalTagW += tagGap;
    }

    if (totalTagW > pw - 24)
    {
        tagScale = 1;
        tagPadX = 8;
        tagPadY = 5;
        tagGap = 8;
        totalTagW = 0;

        for (int i = 0; i < 4; i++)
        {
            totalTagW += textWidth(tags[i], tagScale) + tagPadX * 2;
            if (i < 3) totalTagW += tagGap;
        }
    }

    int chipH = 7 * tagScale + tagPadY * 2;
    int chipY = M.statusY + (FOOTER_STATUS_H - chipH) / 2;
    int tagX = px + (pw - totalTagW) / 2;

    if (tagX < px + 8)
        tagX = px + 8;

    for (int i = 0; i < 4; i++)
    {
        int chipW = textWidth(tags[i], tagScale) + tagPadX * 2;
        SDL_Rect chip = {tagX, chipY, chipW, chipH};

        fillRect(ren, chip, tagFill[i]);
        strokeRect(ren, chip, rgb(82, 90, 108));
        drawTextSafe(ren, chip.x + tagPadX, chip.y + tagPadY, tagScale,
                     tags[i], tagTextColor[i]);

        tagX += chipW + tagGap;
    }

    setColor(ren, rgb(46, 54, 70));
    SDL_RenderDrawLine(ren, px + 8, M.div1Y, px + pw - 8, M.div1Y);
    SDL_RenderDrawLine(ren, px + 8, M.div2Y, px + pw - 8, M.div2Y);

    // --- hint row ---
    drawTextSafe(ren, px + 12, M.hintY, 1,
                 "L/R A/D MOVE  S TOGGLE PATH  2 EDGES  3 SOLVER  WHEEL",
                 rgb(206, 212, 220));
}

//web dev math
//TODO: REFACTOR THIS FUNCTION
// 5 buttons: PREV  NEXT  SOLVE  EDGES  SOLVER
static void
layoutButtons(const Layout *L,
                          Button *prevBtn,
                          Button *nextBtn,
                          Button *solveBtn,
                          Button *edgesBtn,
                          Button *solverBtn)
{
    FooterMetrics M = computeFooterMetrics(L);

    prevBtn->rect   = (SDL_Rect){M.btnStartX,                           M.btnRowY, M.btnW, M.btnH};
    nextBtn->rect   = (SDL_Rect){M.btnStartX + 1 * (M.btnW + M.btnGap), M.btnRowY, M.btnW, M.btnH};
    solveBtn->rect  = (SDL_Rect){M.btnStartX + 2 * (M.btnW + M.btnGap), M.btnRowY, M.btnW, M.btnH};
    edgesBtn->rect  = (SDL_Rect){M.btnStartX + 3 * (M.btnW + M.btnGap), M.btnRowY, M.btnW, M.btnH};
    solverBtn->rect = (SDL_Rect){M.btnStartX + 4 * (M.btnW + M.btnGap), M.btnRowY, M.btnW, M.btnH};
}

//info panel
//for edges
static void updateEdgesInfo(AppState *app)
{
    clearInfoPanel(app);
    app->infoType = INFO_EDGES;

    uint8_t *maze = app->mazes[app->current];
    char *edgesStr = generateEdgesString(maze, app->rows, app->cols);
    if (!edgesStr)
    {
        pushInfoLine(app, "FAILED TO GENERATE EDGES");
        return;
    }

    char *line = strtok(edgesStr, "\n");
    while (line)
    {
        pushInfoLine(app, line);
        line = strtok(NULL, "\n");
    }

    free(edgesStr);

    if (app->infoLineCount == 0)
        pushInfoLine(app, "NO EDGES");
}

//solver - path info
//TODO - Parse output and draw nicely
static void updateSolverInfo(AppState *app)
{
    clearInfoPanel(app);
    app->infoType = INFO_SOLVER;

    uint8_t *maze = app->mazes[app->current];
    //run clingo
    char *output = runSolverOnMaze(maze, app->rows, app->cols,
                                            "maze_solver.lp");
    if (!output)
    {
        pushInfoLine(app, "FAILED TO RUN SOLVER");
        return;
    }

    int wrapChars = getInfoWrapChars(app);

    //get the lines
    char *line = strtok(output, "\n");
    while (line)
    {
        pushWrappedInfoLine(app, line, wrapChars);
        line = strtok(NULL, "\n");
    }

    free(output);

    if (app->infoLineCount == 0)
        pushInfoLine(app, "NO SOLVER OUTPUT");
}

//main function for the answer sets visualizer
//alkso web dev math
static void
drawInfoPanel(SDL_Renderer *ren, const AppState *app, const Layout *L)
{
    uint32_t accent = rgb(120, 128, 144);
    if (app->infoType == INFO_EDGES)
        accent = rgb(136, 192, 208);
    else if (app->infoType == INFO_SOLVER)
        accent = rgb(163, 190, 140);

    fillRect(ren, L->infoPanel, rgb(24, 28, 36));
    strokeRect(ren, L->infoPanel, rgb(94, 102, 118));

    SDL_Rect titleBar = {
        L->infoPanel.x,
        L->infoPanel.y,
        L->infoPanel.w,
        36
    };
    fillRect(ren, titleBar, rgb(34, 40, 54));
    strokeRect(ren, titleBar, rgb(96, 104, 120));

    SDL_Rect accentBar = {
        titleBar.x + 1,
        titleBar.y + 1,
        titleBar.w - 2,
        4
    };
    fillRect(ren, accentBar, accent);

    SDL_Rect body = {
        L->infoPanel.x + 8,
        L->infoPanel.y + 44,
        L->infoPanel.w - 16,
        L->infoPanel.h - 64
    };
    fillRect(ren, body, rgb(14, 18, 26));
    strokeRect(ren, body, rgb(60, 68, 84));

    const char *title = "INFO";
    if (app->infoType == INFO_EDGES) title = "EDGES";
    else if (app->infoType == INFO_SOLVER) title = "SOLVER";

    drawTextSafe(ren, titleBar.x + 10, titleBar.y + 10, 2, title, rgb(245, 247, 250));

    if (app->infoType == INFO_SOLVER)
        drawTextSafe(ren, titleBar.x + titleBar.w - 110, titleBar.y + 13, 1, "RAW UNPARSED", rgb(163, 190, 140));
    else if (app->infoType == INFO_EDGES)
        drawTextSafe(ren, titleBar.x + titleBar.w - 58, titleBar.y + 13, 1, "RAW", rgb(136, 192, 208));

    if (app->infoType == INFO_NONE || !app->infoLines || app->infoLineCount == 0)
    {
        drawTextSafe(ren, body.x + 10, body.y + 12, 2, "NO INFO SELECTED", rgb(170, 178, 188));
        return;
    }

    int textScale = getInfoTextScale(app);
    int lineStep = (textScale == 1) ? 12 : 18;
    int textX = body.x + 8;
    int textY = body.y + 8;
    int maxLines = getInfoVisibleLines(app, L);

    int start = app->infoScrollOffset;
    int end = start + maxLines;
    if (end > app->infoLineCount) end = app->infoLineCount;

    uint32_t textColor = rgb(236, 239, 244);
    if (app->infoType == INFO_SOLVER)
        textColor = rgb(196, 220, 172);
    else if (app->infoType == INFO_EDGES)
        textColor = rgb(200, 224, 255);

    SDL_RenderSetClipRect(ren, &body);

    for (int i = start; i < end; i++)
    {
        drawTextSafe(ren, textX, textY, textScale, app->infoLines[i], textColor);
        textY += lineStep;
    }

    SDL_RenderSetClipRect(ren, NULL);

    char buf[96];
    snprintf(buf, sizeof(buf), "ROWS %d-%d OF %d",
             app->infoLineCount == 0 ? 0 : start + 1,
             end,
             app->infoLineCount);

    drawTextSafe(ren, body.x + 8, body.y + body.h - 12, 1, buf, rgb(150, 160, 172));
}


//main function
int runApp(uint8_t **mazes, PathSet *paths, AnswerSets *parentSets,
           int mazeCount, int rows, int cols)
{
    if (!mazes || mazeCount <= 0 || rows <= 0 || cols <= 0)
    {
        fprintf(stderr, "runApp: invalid input\n");
        return 1;
    }

    AppState app;
    memset(&app, 0, sizeof(app));

    //init
    app.mazes = mazes;
    app.paths = paths;
    app.parentSets = parentSets;
    app.mazeCount = mazeCount;
    app.current = 0;
    app.rows = rows;
    app.cols = cols;
    app.infoType = INFO_NONE;

    app.showPath = calloc((size_t)mazeCount, sizeof(bool));
    if (!app.showPath) {
        fprintf(stderr, "runApp: failed to allocate showPath\n");
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        free(app.showPath);
        return 1;
    }

    //webdev math
    //todo: move to a seperate function
    SDL_Rect usable = {0, 0, 1366, 820};
    if (SDL_GetDisplayUsableBounds(0, &usable) != 0)
    {
        usable.w = 1366;
        usable.h = 820;
    }

    int longest = rows > cols ? rows : cols;
    int footerH = FOOTER_MIN_HEIGHT;

    int maxMazeW = usable.w
                 - PANEL_WIDTH
                 - CONTENT_GAP
                 - WINDOW_MARGIN * 2
                 - MAZE_CARD_PAD * 2
                 - 40;

    int maxMazeH = usable.h
                 - WINDOW_MARGIN * 2
                 - footerH
                 - 12
                 - MAZE_CARD_PAD * 2
                 - 40;

    if (maxMazeW < cols) maxMazeW = cols;
    if (maxMazeH < rows) maxMazeH = rows;

    int cellByBase = 560 / (longest > 0 ? longest : 1);
    int cellByW = maxMazeW / cols;
    int cellByH = maxMazeH / rows;
    int fitCell = cellByW < cellByH ? cellByW : cellByH;

    if (fitCell < cellByBase)
        cellByBase = fitCell;

    app.cellSize = clampi(cellByBase, MIN_CELL_SIZE, MAX_CELL_SIZE);

    int mazePixelW = cols * app.cellSize + MAZE_CARD_PAD * 2;
    int mazePixelH = rows * app.cellSize + MAZE_CARD_PAD * 2;

    int leftPaneW = mazePixelW + WINDOW_MARGIN * 2 + 20;
    if (leftPaneW < LEFT_MIN_WIDTH) leftPaneW = LEFT_MIN_WIDTH;

    app.winW = leftPaneW + CONTENT_GAP + PANEL_WIDTH;
    app.winH = WINDOW_MARGIN
             + mazePixelH
             + 12
             + footerH
             + WINDOW_MARGIN;

    if (app.winH < 640) app.winH = 640;

    SDL_Window *window = SDL_CreateWindow(
        "Maze Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        app.winW,
        app.winH,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        free(app.showPath);
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(app.showPath);
        return 1;
    }

    updateTitle(window, &app);

    //init buttons
    //todo: a context struct for the buttons
    Button prevBtn   = {0};
    Button nextBtn   = {0};
    Button solveBtn  = {0};
    Button edgesBtn  = {0};
    Button solverBtn = {0};

    bool running = true;

    while (running)
    {
        Layout L = computeLayout(&app);

        layoutButtons(&L, &prevBtn, &nextBtn, &solveBtn,
                      &edgesBtn, &solverBtn);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_MOUSEMOTION) {
                int mx = e.motion.x;
                int my = e.motion.y;

                //check if buttons are hovered
                prevBtn.hovered   = pointInRect(mx, my, prevBtn.rect);
                nextBtn.hovered   = pointInRect(mx, my, nextBtn.rect);
                solveBtn.hovered  = pointInRect(mx, my, solveBtn.rect);
                edgesBtn.hovered  = pointInRect(mx, my, edgesBtn.rect);
                solverBtn.hovered = pointInRect(mx, my, solverBtn.rect);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN &&
                     e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;

                //check if buttons are pressed
                prevBtn.pressed   = pointInRect(mx, my, prevBtn.rect);
                nextBtn.pressed   = pointInRect(mx, my, nextBtn.rect);
                solveBtn.pressed  = pointInRect(mx, my, solveBtn.rect);
                edgesBtn.pressed  = pointInRect(mx, my, edgesBtn.rect);
                solverBtn.pressed = pointInRect(mx, my, solverBtn.rect);

                if(solverBtn.pressed){printf("Solving\n");}
            }
            else if (e.type == SDL_MOUSEBUTTONUP &&
                     e.button.button == SDL_BUTTON_LEFT)
            { //advancing the mazes

                int mx = e.button.x;
                int my = e.button.y;
                bool mazeChanged = false;

                if (prevBtn.pressed && pointInRect(mx, my, prevBtn.rect)) {
                    app.current = (app.current - 1 + app.mazeCount) % app.mazeCount;
                    mazeChanged = true;
                }
                if (nextBtn.pressed && pointInRect(mx, my, nextBtn.rect)) {
                    app.current = (app.current + 1) % app.mazeCount;
                    mazeChanged = true;
                }
                if (solveBtn.pressed && pointInRect(mx, my, solveBtn.rect)) {
                    if (app.paths && app.current < app.paths->count)
                        app.showPath[app.current] = !app.showPath[app.current];
                }
                if (edgesBtn.pressed && pointInRect(mx, my, edgesBtn.rect))
                    updateEdgesInfo(&app);
                if (solverBtn.pressed && pointInRect(mx, my, solverBtn.rect))
                    updateSolverInfo(&app);

                if (mazeChanged)
                    clearInfoPanel(&app);

                prevBtn.pressed   = false;
                nextBtn.pressed   = false;
                solveBtn.pressed  = false;
                edgesBtn.pressed  = false;
                solverBtn.pressed = false;

                updateTitle(window, &app);
            }
            else if (e.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = e.key.keysym.sym;
                bool mazeChanged = false;

                if (key == SDLK_ESCAPE)
                    running = false;
                else if (key == SDLK_LEFT || key == SDLK_a)
                {
                    app.current = (app.current - 1 + app.mazeCount) % app.mazeCount;
                    mazeChanged = true;
                }
                else if (key == SDLK_RIGHT || key == SDLK_d)
                {
                    app.current = (app.current + 1) % app.mazeCount;
                    mazeChanged = true;
                }
                else if (key == SDLK_HOME)
                {
                    app.current = 0;
                    mazeChanged = true;
                }
                else if (key == SDLK_END)
                {
                    app.current = app.mazeCount - 1;
                    mazeChanged = true;
                }
                else if (key == SDLK_s || key == SDLK_RETURN)
                {
                    if (app.paths && app.current < app.paths->count)
                        app.showPath[app.current] = !app.showPath[app.current];
                }
                else if (key == SDLK_2)
                    updateEdgesInfo(&app);
                else if (key == SDLK_3)
                    updateSolverInfo(&app);

                if (mazeChanged)
                    clearInfoPanel(&app);

                updateTitle(window, &app);
            }
            else if (e.type == SDL_MOUSEWHEEL)
            { // mousewheel concerns the answer sets visualizer
                if (app.infoLines && app.infoLineCount > 0)
                {
                    int maxLines = getInfoVisibleLines(&app, &L);
                    int maxOffset = app.infoLineCount - maxLines;
                    if (maxOffset < 0) maxOffset = 0;

                    int newOffset = app.infoScrollOffset - e.wheel.y;
                    if (newOffset < 0) newOffset = 0;
                    if (newOffset > maxOffset) newOffset = maxOffset;

                    app.infoScrollOffset = newOffset;
                }
            }
        }

        setColor(ren, rgb(10, 12, 18));
        SDL_RenderClear(ren);

        drawMazeCard(ren, L.mazeCard);
        drawMaze(ren, &app, L.mazeArea);
        drawFooterPanel(ren, &app, &L);

        drawButton(ren, &prevBtn,   "PREV");
        drawButton(ren, &nextBtn,   "NEXT");
        drawButton(ren, &solveBtn,  "SOLVE");
        drawButton(ren, &edgesBtn,  "EDGES");
        drawButton(ren, &solverBtn, "SOLVER");

        drawInfoPanel(ren, &app, &L);

        SDL_RenderPresent(ren);
    }

    freeInfoLines(&app);
    free(app.showPath);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
