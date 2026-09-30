//
// Copyright 2025 WizzardSK
//
// Store - public cartridge browser for LowRes NX
// Fetches public cartridges from lowresnx.inutilis.com
//

#include "config.h"

#include "store.h"
#include "main.h"
#include "text_lib.h"
#include "string_utils.h"
#include "system_paths.h"
#include "utils.h"
#include "sdl_include.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include "machine.h"
#include <curl/curl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

extern const char dev_characters[];

#define VISIBLE_ROWS 14
#define LIST_START_Y 1
#define SCREEN_COLS 20
#define BASE_URL "https://lowresnx.inutilis.com/"
#define PARALLEL_FETCHES 10

static const char *categoryNames[] = {"GAMES", "ART", "TOOLS", "EXAMPLES"};
static const char *categorySlugs[] = {"game", "art", "tool", "example"};

// Palette: color 1 = text foreground, color 2 = text background
// Color byte format: 00RRGGBB (R bits 5-4, G bits 3-2, B bits 1-0)
// 0x00=black, 0x3F=white, 0x03=blue, 0x0C=green, 0x30=red
static const char store_colors[] = {
    0x05, 0x3F, 0x05, 0x15, // 0: normal - white text on teal bg
    0x05, 0x3F, 0x06, 0x15, // 1: title bar - white text on cyan bg
    0x05, 0x3F, 0x09, 0x2A, // 2: selected - white text on brighter teal bg
    0x05, 0x15, 0x05, 0x05, // 3: dim - gray text on teal bg
    0x05, 0x02, 0x05, 0x00, // 4: scrollbar track - teal
    0x05, 0x2A, 0x04, 0x15, // 5: status bar - gray text on dark teal bg
    0x05, 0x3F, 0x09, 0x00, // 6: scrollbar thumb - white on brighter teal
    0x05, 0x0F, 0x05, 0x15, // 7: accent - yellow text on teal bg
};

// --- HTTP helpers ---

struct MemBuffer {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total = size * nmemb;
    struct MemBuffer *buf = (struct MemBuffer *)userp;
    char *ptr = realloc(buf->data, buf->size + total + 1);
    if (!ptr) return 0;
    buf->data = ptr;
    memcpy(buf->data + buf->size, contents, total);
    buf->size += total;
    buf->data[buf->size] = '\0';
    return total;
}

static char *http_get(const char *url)
{
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    struct MemBuffer buf = {0};
    buf.data = malloc(1);
    buf.data[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "LowResNX-Store/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) { free(buf.data); return NULL; }
    return buf.data;
}

static bool http_download(const char *url, const char *path)
{
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    FILE *fp = fopen(path, "wb");
    if (!fp) { curl_easy_cleanup(curl); return false; }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "LowResNX-Store/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK) { remove(path); return false; }
    return true;
}

// --- HTML parsing ---

static void html_decode(char *s)
{
    char *r = s, *w = s;
    while (*r)
    {
        if (*r == '&')
        {
            if (strncmp(r, "&amp;", 5) == 0) { *w++ = '&'; r += 5; }
            else if (strncmp(r, "&lt;", 4) == 0) { *w++ = '<'; r += 4; }
            else if (strncmp(r, "&gt;", 4) == 0) { *w++ = '>'; r += 4; }
            else if (strncmp(r, "&quot;", 6) == 0) { *w++ = '"'; r += 6; }
            else if (strncmp(r, "&apos;", 6) == 0) { *w++ = '\''; r += 6; }
            else if (strncmp(r, "&#", 2) == 0)
            {
                // Numeric entity: &#39; or &#x27;
                int code = 0;
                const char *p = r + 2;
                if (*p == 'x' || *p == 'X')
                {
                    p++;
                    while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))
                    {
                        code *= 16;
                        if (*p >= '0' && *p <= '9') code += *p - '0';
                        else if (*p >= 'a' && *p <= 'f') code += *p - 'a' + 10;
                        else code += *p - 'A' + 10;
                        p++;
                    }
                }
                else
                {
                    while (*p >= '0' && *p <= '9') { code = code * 10 + (*p - '0'); p++; }
                }
                if (*p == ';') p++;
                if (code > 0 && code < 128) *w++ = (char)code;
                else *w++ = '?';
                r = (char *)p;
            }
            else { *w++ = *r++; }
        }
        else { *w++ = *r++; }
    }
    *w = '\0';
}

static void store_parseProgramList(struct Store *store, const char *html)
{
    const char *p = html;
    while ((p = strstr(p, "topic.php?id=")) != NULL && store->numEntries < STORE_MAX_ENTRIES)
    {
        p += 13;
        int id = atoi(p);
        if (id <= 0) continue;

        bool dup = false;
        for (int i = 0; i < store->numEntries; i++)
            if (store->entries[i].topicId == id) { dup = true; break; }
        if (dup) continue;

        const char *h3 = strstr(p, "<h3>");
        if (!h3) continue;
        const char *nextTopic = strstr(p, "topic.php?id=");
        if (nextTopic && h3 > nextTopic) continue;

        h3 += 4;
        const char *h3end = strstr(h3, "</h3>");
        if (!h3end) continue;

        struct StoreEntry *e = &store->entries[store->numEntries];
        e->topicId = id;
        e->downloaded = false;
        e->nxUrl[0] = '\0';
        e->screenshotUrl[0] = '\0';
        e->cachedPath[0] = '\0';

        // Look for screenshot image: src="uploads/...png"
        const char *imgSearch = p;
        const char *nextTopicLimit = strstr(p, "topic.php?id=");
        const char *imgSrc = strstr(imgSearch, "src=\"uploads/");
        if (imgSrc && (!nextTopicLimit || imgSrc < nextTopicLimit))
        {
            imgSrc += 5; // skip src="
            const char *imgEnd = strchr(imgSrc, '"');
            if (imgEnd)
            {
                int imgLen = (int)(imgEnd - imgSrc);
                if (imgLen < STORE_URL_SIZE)
                {
                    strncpy(e->screenshotUrl, imgSrc, imgLen);
                    e->screenshotUrl[imgLen] = '\0';
                }
            }
        }

        int titleLen = (int)(h3end - h3);
        if (titleLen >= STORE_NAME_SIZE) titleLen = STORE_NAME_SIZE - 1;
        strncpy(e->displayName, h3, titleLen);
        e->displayName[titleLen] = '\0';
        html_decode(e->displayName);

        for (int i = 0; e->displayName[i]; i++)
            if (e->displayName[i] >= 'a' && e->displayName[i] <= 'z')
                e->displayName[i] -= 32;

        store->numEntries++;
    }
}

static bool store_parseTopicPage(const char *html, char *nxUrlOut, size_t maxLen)
{
    const char *p = strstr(html, "href=\"uploads/");
    while (p)
    {
        p += 6;
        const char *end = strchr(p, '"');
        if (end && end - p > 3 && strncmp(end - 3, ".nx", 3) == 0)
        {
            int len = (int)(end - p);
            if (len < (int)maxLen)
            {
                strncpy(nxUrlOut, p, len);
                nxUrlOut[len] = '\0';
                return true;
            }
        }
        p = strstr(p, "href=\"uploads/");
    }
    return false;
}

// --- Parallel fetch ---

struct PageFetch {
    CURL *curl;
    struct MemBuffer buf;
    char url[512];
    int page;
};

static void store_fetchCategory(struct Store *store, int categoryIndex)
{
    store->numEntries = 0;
    const char *slug = categorySlugs[categoryIndex];

    char firstUrl[512];
    snprintf(firstUrl, sizeof(firstUrl), BASE_URL "programs.php?category=%s&sort=new&page=1", slug);
    char *firstHtml = http_get(firstUrl);
    if (!firstHtml) return;

    store_parseProgramList(store, firstHtml);
    free(firstHtml);
    if (store->numEntries == 0) return;

    int page = 2;
    bool done = false;

    while (!done && page <= 100)
    {
        CURLM *multi = curl_multi_init();
        struct PageFetch fetches[PARALLEL_FETCHES];
        int numFetches = 0;

        for (int i = 0; i < PARALLEL_FETCHES && page <= 100; i++, page++)
        {
            struct PageFetch *pf = &fetches[numFetches];
            pf->buf.data = malloc(1);
            pf->buf.data[0] = '\0';
            pf->buf.size = 0;
            pf->page = page;
            snprintf(pf->url, sizeof(pf->url), BASE_URL "programs.php?category=%s&sort=new&page=%d", slug, page);

            pf->curl = curl_easy_init();
            curl_easy_setopt(pf->curl, CURLOPT_URL, pf->url);
            curl_easy_setopt(pf->curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(pf->curl, CURLOPT_WRITEDATA, &pf->buf);
            curl_easy_setopt(pf->curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(pf->curl, CURLOPT_TIMEOUT, 15L);
            curl_easy_setopt(pf->curl, CURLOPT_USERAGENT, "LowResNX-Store/1.0");
            curl_multi_add_handle(multi, pf->curl);
            numFetches++;
        }

        int running = 0;
        do {
            curl_multi_perform(multi, &running);
            if (running) curl_multi_wait(multi, NULL, 0, 1000, NULL);
        } while (running);

        for (int i = 0; i < numFetches; i++)
        {
            struct PageFetch *pf = &fetches[i];
            int before = store->numEntries;
            if (pf->buf.data && pf->buf.size > 0)
                store_parseProgramList(store, pf->buf.data);
            if (store->numEntries == before)
                done = true;
            curl_multi_remove_handle(multi, pf->curl);
            curl_easy_cleanup(pf->curl);
            free(pf->buf.data);
        }
        curl_multi_cleanup(multi);
        if (store->numEntries >= STORE_MAX_ENTRIES) break;
    }
}

// --- Download ---

static bool store_downloadProgram(struct Store *store, int index)
{
    struct StoreEntry *e = &store->entries[index];

    if (e->downloaded && e->cachedPath[0] != '\0')
    {
        struct stat st;
        if (stat(e->cachedPath, &st) == 0 && st.st_size > 0)
            return true;
    }

    if (e->nxUrl[0] == '\0')
    {
        char topicUrl[512];
        snprintf(topicUrl, sizeof(topicUrl), BASE_URL "topic.php?id=%d", e->topicId);
        char *html = http_get(topicUrl);
        if (!html) return false;
        bool found = store_parseTopicPage(html, e->nxUrl, STORE_URL_SIZE);
        free(html);
        if (!found) return false;
    }

    char downloadUrl[512];
    snprintf(downloadUrl, sizeof(downloadUrl), BASE_URL "%s", e->nxUrl);

    const char *fname = strrchr(e->nxUrl, '/');
    fname = fname ? fname + 1 : e->nxUrl;
    snprintf(e->cachedPath, FILENAME_MAX, "%s%s", store->cacheDir, fname);

    struct stat st;
    if (stat(e->cachedPath, &st) == 0 && st.st_size > 0)
    {
        e->downloaded = true;
        return true;
    }

    if (http_download(downloadUrl, e->cachedPath))
    {
        e->downloaded = true;
        return true;
    }
    return false;
}

// --- Navigation with wrap-around ---

static void store_moveUp(struct Store *store)
{
    if (store->numEntries == 0) return;
    store->selectedIndex = (store->selectedIndex > 0) ? store->selectedIndex - 1 : store->numEntries - 1;
    if (store->selectedIndex < store->scrollOffset)
        store->scrollOffset = store->selectedIndex;
    if (store->selectedIndex >= store->scrollOffset + VISIBLE_ROWS)
        store->scrollOffset = store->selectedIndex - VISIBLE_ROWS + 1;
}

static void store_moveDown(struct Store *store)
{
    if (store->numEntries == 0) return;
    store->selectedIndex = (store->selectedIndex < store->numEntries - 1) ? store->selectedIndex + 1 : 0;
    if (store->selectedIndex >= store->scrollOffset + VISIBLE_ROWS)
        store->scrollOffset = store->selectedIndex - VISIBLE_ROWS + 1;
    if (store->selectedIndex < store->scrollOffset)
        store->scrollOffset = store->selectedIndex;
}

static void store_pageUp(struct Store *store)
{
    if (store->numEntries == 0) return;
    store->selectedIndex -= VISIBLE_ROWS;
    if (store->selectedIndex < 0) store->selectedIndex = store->numEntries - 1;
    store->scrollOffset = store->selectedIndex - VISIBLE_ROWS / 2;
    if (store->scrollOffset > store->numEntries - VISIBLE_ROWS)
        store->scrollOffset = store->numEntries - VISIBLE_ROWS;
    if (store->scrollOffset < 0) store->scrollOffset = 0;
}

static void store_pageDown(struct Store *store)
{
    if (store->numEntries == 0) return;
    store->selectedIndex += VISIBLE_ROWS;
    if (store->selectedIndex >= store->numEntries) store->selectedIndex = 0;
    store->scrollOffset = store->selectedIndex - VISIBLE_ROWS / 2;
    if (store->scrollOffset > store->numEntries - VISIBLE_ROWS)
        store->scrollOffset = store->numEntries - VISIBLE_ROWS;
    if (store->scrollOffset < 0) store->scrollOffset = 0;
}

// --- UI drawing ---

static void store_setupScreen(struct Store *store)
{
    struct Core *core = store->runner->core;
    struct TextLib *textLib = &store->textLib;
    textLib->core = core;

    itp_endProgram(core);
    machine_reset(core, true);
    overlay_reset(core);

    core->machine->ioRegisters.attr.touchEnabled = 1;
    core->machine->ioRegisters.attr.gamepadsEnabled = 1;
    core->machineInternals->isEnergySaving = true;

    txtlib_clearScreen(textLib);
    textLib->fontCharOffset = 192;

    memcpy(&core->machine->colorRegisters, store_colors, sizeof(store_colors));
    memcpy(&core->machine->videoRam.characters, dev_characters, 4096);
}

static void store_fillRow(struct Store *store, int y, int palette)
{
    store->textLib.charAttr.palette = palette;
    txtlib_setCells(&store->textLib, 0, y, SCREEN_COLS - 1, y, ' ' - 32 + 192);
}

static void store_drawTitleBar(struct Store *store, const char *title)
{
    struct TextLib *textLib = &store->textLib;
    store_fillRow(store, 0, 1);
    textLib->charAttr.palette = 1;
    txtlib_writeText(textLib, title, 1, 0);
}

static void store_drawStatusBar(struct Store *store, const char *left, const char *right)
{
    struct TextLib *textLib = &store->textLib;
    store_fillRow(store, 15, 5);
    textLib->charAttr.palette = 5;
    if (left)  txtlib_writeText(textLib, left, 0, 15);
    if (right) txtlib_writeText(textLib, right, SCREEN_COLS - (int)strlen(right), 15);
}

static void store_showMessage(struct Store *store, const char *msg, int y)
{
    struct TextLib *textLib = &store->textLib;
    textLib->charAttr.palette = 3;
    int x = (SCREEN_COLS - (int)strlen(msg)) / 2;
    if (x < 0) x = 0;
    txtlib_writeText(textLib, msg, x, y);
    machine_suspendEnergySaving(store->runner->core, 60);
}

static void store_drawCategories(struct Store *store)
{
    struct TextLib *textLib = &store->textLib;

    store_setupScreen(store);

    // Fill entire screen with dark blue background (palette 0)
    textLib->charAttr.palette = 0;
    for (int y = 0; y < 16; y++)
        store_fillRow(store, y, 0);

    store_drawTitleBar(store, "LOWRES NX STORE");

    // Centered subtitle
    textLib->charAttr.palette = 3;
    txtlib_writeText(textLib, "BROWSE & PLAY", 3, 3);
    txtlib_writeText(textLib, "COMMUNITY PROGRAMS", 1, 4);

    for (int i = 0; i < STORE_NUM_CATEGORIES; i++)
    {
        int y = 6 + i * 2;

        if (i == store->selectedCategory)
        {
            store_fillRow(store, y, 2);
            textLib->charAttr.palette = 2;
            char line[SCREEN_COLS + 1];
            snprintf(line, sizeof(line), " > %s", categoryNames[i]);
            txtlib_writeText(textLib, line, 2, y);
        }
        else
        {
            textLib->charAttr.palette = 0;
            char line[SCREEN_COLS + 1];
            snprintf(line, sizeof(line), "   %s", categoryNames[i]);
            txtlib_writeText(textLib, line, 2, y);
        }
    }

    store_drawStatusBar(store, " ]:SELECT", "\\:QUIT ");
    machine_suspendEnergySaving(store->runner->core, 120);
}

static void store_drawList(struct Store *store)
{
    struct TextLib *textLib = &store->textLib;

    // Clear list area
    for (int y = LIST_START_Y; y < LIST_START_Y + VISIBLE_ROWS; y++)
        store_fillRow(store, y, 0);

    if (store->numEntries == 0)
    {
        store_showMessage(store, "NO PROGRAMS FOUND", 8);
        return;
    }

    int maxVisible = VISIBLE_ROWS;
    if (maxVisible > store->numEntries) maxVisible = store->numEntries;

    for (int i = 0; i < maxVisible; i++)
    {
        int idx = store->scrollOffset + i;
        if (idx >= store->numEntries) break;

        int y = LIST_START_Y + i;
        struct StoreEntry *e = &store->entries[idx];

        if (idx == store->selectedIndex)
        {
            store_fillRow(store, y, 2);
            textLib->charAttr.palette = 2;
            txtlib_writeText(textLib, e->displayName, 1, y);
        }
        else
        {
            textLib->charAttr.palette = 0;
            txtlib_writeText(textLib, e->displayName, 1, y);
        }
    }

    // Scrollbar
    if (store->numEntries > VISIBLE_ROWS)
    {
        int barHeight = VISIBLE_ROWS;
        int thumbSize = (VISIBLE_ROWS * VISIBLE_ROWS + store->numEntries - 1) / store->numEntries;
        if (thumbSize < 1) thumbSize = 1;
        int thumbPos = (store->scrollOffset * (barHeight - thumbSize) + (store->numEntries - VISIBLE_ROWS) / 2) / (store->numEntries - VISIBLE_ROWS);
        if (thumbPos < 0) thumbPos = 0;
        if (thumbPos + thumbSize > barHeight) thumbPos = barHeight - thumbSize;

        for (int i = 0; i < barHeight; i++)
        {
            int y = LIST_START_Y + i;
            bool isThumb = (i >= thumbPos && i < thumbPos + thumbSize);
            textLib->charAttr.palette = isThumb ? 1 : 3;
            txtlib_writeText(textLib, ".", SCREEN_COLS - 1, y);
        }
    }

    // Status bar with position and page
    int page = store->selectedIndex / VISIBLE_ROWS + 1;
    int totalPages = (store->numEntries + VISIBLE_ROWS - 1) / VISIBLE_ROWS;

    char left[SCREEN_COLS + 1];
    char right[SCREEN_COLS + 1];
    snprintf(left, sizeof(left), " %d/%d", store->selectedIndex + 1, store->numEntries);
    snprintf(right, sizeof(right), "%d/%d ", page, totalPages);
    store_drawStatusBar(store, left, right);
}

static void store_showListScreen(struct Store *store)
{
    store_setupScreen(store);

    store->textLib.charAttr.palette = 0;
    for (int y = 0; y < 16; y++)
        store_fillRow(store, y, 0);

    // Title bar with category name and count
    struct TextLib *textLib = &store->textLib;
    store_fillRow(store, 0, 1);
    textLib->charAttr.palette = 1;
    txtlib_writeText(textLib, categoryNames[store->selectedCategory], 1, 0);
    char countStr[10];
    snprintf(countStr, sizeof(countStr), "%d", store->numEntries);
    txtlib_writeText(textLib, countStr, SCREEN_COLS - (int)strlen(countStr) - 1, 0);

    store_drawList(store);
    machine_suspendEnergySaving(store->runner->core, 120);
}

// --- Public API ---

static void store_freePreview(struct Store *store)
{
    // Wait for any in-flight preview thread to finish
    if (store->previewLoading)
    {
        pthread_join(store->previewThread, NULL);
        store->previewLoading = false;
        if (store->previewPending)
        {
            free(store->previewPending);
            store->previewPending = NULL;
        }
        store->previewReady = false;
    }
    if (store->previewPixels)
    {
        free(store->previewPixels);
        store->previewPixels = NULL;
    }
    store->previewIndex = -1;
    store->previewRequestIndex = -1;
}

struct PreviewThreadArg {
    char url[512];
    int index;
    struct Store *store;
};

static void *preview_thread_func(void *arg)
{
    struct PreviewThreadArg *pta = (struct PreviewThreadArg *)arg;

    CURL *curl = curl_easy_init();
    if (!curl) { free(pta); return NULL; }

    struct MemBuffer buf = {0};
    buf.data = malloc(1);
    buf.data[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, pta->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "LowResNX-Store/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && buf.size > 0)
    {
        int w, h, channels;
        uint8_t *pixels = stbi_load_from_memory((uint8_t *)buf.data, (int)buf.size, &w, &h, &channels, 3);
        if (pixels && w == SCREEN_WIDTH && h == SCREEN_HEIGHT)
        {
            pta->store->previewPending = pixels;
            pta->store->previewRequestIndex = pta->index;
            pta->store->previewReady = true;
        }
        else if (pixels)
        {
            stbi_image_free(pixels);
        }
    }
    free(buf.data);
    free(pta);
    return NULL;
}

static void store_loadPreview(struct Store *store, int index)
{
    if (index == store->previewIndex && !store->previewLoading) return;

    if (index < 0 || index >= store->numEntries)
    {
        store_freePreview(store);
        return;
    }
    struct StoreEntry *e = &store->entries[index];
    if (e->screenshotUrl[0] == '\0')
    {
        store_freePreview(store);
        return;
    }

    // If a thread is already running, let it finish (we'll ignore its result if index changed)
    if (store->previewLoading)
    {
        // Just update the desired index; we'll start a new load when the current one finishes
        store->previewRequestIndex = index;
        return;
    }

    struct PreviewThreadArg *pta = malloc(sizeof(struct PreviewThreadArg));
    if (!pta) return;

    snprintf(pta->url, sizeof(pta->url), BASE_URL "%s", e->screenshotUrl);
    pta->index = index;
    pta->store = store;

    store->previewLoading = true;
    store->previewRequestIndex = index;
    store->previewReady = false;

    if (pthread_create(&store->previewThread, NULL, preview_thread_func, pta) != 0)
    {
        free(pta);
        store->previewLoading = false;
    }
}

static void store_checkPreview(struct Store *store)
{
    if (!store->previewLoading) return;

    if (store->previewReady)
    {
        pthread_join(store->previewThread, NULL);
        store->previewLoading = false;

        if (store->previewPending)
        {
            if (store->previewRequestIndex == store->selectedIndex)
            {
                if (store->previewPixels) free(store->previewPixels);
                store->previewPixels = store->previewPending;
                store->previewIndex = store->previewRequestIndex;
            }
            else
            {
                free(store->previewPending);
            }
            store->previewPending = NULL;
        }

        // If user moved cursor while loading, start new load
        if (store->selectedIndex != store->previewIndex)
        {
            store_loadPreview(store, store->selectedIndex);
        }
    }
}

uint8_t *store_getPreviewPixels(struct Store *store, int *w, int *h)
{
    if (!store->previewPixels) return NULL;
    *w = SCREEN_WIDTH;
    *h = SCREEN_HEIGHT;
    return store->previewPixels;
}

void store_init(struct Store *store, struct Runner *runner)
{
    memset(store, 0, sizeof(struct Store));
    store->runner = runner;
    store->previewIndex = -1;
    store->previewRequestIndex = -1;
    store->previewLoading = false;
    store->previewReady = false;
    store->previewPending = NULL;
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void store_show(struct Store *store, const char *storeFilePath)
{
    store->selectedIndex = 0;
    store->scrollOffset = 0;
    store->lastTouch = false;
    store->lastUp = false;
    store->lastDown = false;
    store->lastLeft = false;
    store->lastRight = false;
    store->lastButtonA = true;
    store->lastButtonB = true;
    store->loading = false;
    store->screen = StoreScreenCategories;
    store->selectedCategory = 0;
    store->numEntries = 0;

    snprintf(store->cacheDir, FILENAME_MAX, "%s%clowresnx-store%c", P_tmpdir, PATH_SEPARATOR_CHAR, PATH_SEPARATOR_CHAR);
    mkdir(store->cacheDir, 0755);

    store_drawCategories(store);
    setMouseEnabled(true);
}

void store_resume(struct Store *store)
{
    // Return to list, keep position
    store->lastTouch = false;
    store->lastUp = false;
    store->lastDown = false;
    store->lastLeft = false;
    store->lastRight = false;
    store->lastButtonA = true;
    store->lastButtonB = true;
    store->loading = false;

    if (store->screen == StoreScreenList && store->numEntries > 0)
    {
        store_showListScreen(store);
        store_loadPreview(store, store->selectedIndex);
    }
    else
    {
        store_drawCategories(store);
    }
    setMouseEnabled(true);
}

static void store_enterCategory(struct Store *store)
{
    store_setupScreen(store);

    store->textLib.charAttr.palette = 0;
    for (int y = 0; y < 16; y++)
        store_fillRow(store, y, 0);

    store_drawTitleBar(store, categoryNames[store->selectedCategory]);
    store_showMessage(store, "LOADING...", 7);
    machine_suspendEnergySaving(store->runner->core, 120);

    store->screen = StoreScreenList;
    store->loading = true;
    store->selectedIndex = 0;
    store->scrollOffset = 0;
}

static void store_finishLoading(struct Store *store)
{
    store->loading = false;
    store_fetchCategory(store, store->selectedCategory);
    store_showListScreen(store);
    if (store->numEntries > 0)
        store_loadPreview(store, store->selectedIndex);
}

static void store_runSelected(struct Store *store)
{
    for (int y = LIST_START_Y; y < LIST_START_Y + VISIBLE_ROWS; y++)
        store_fillRow(store, y, 0);
    store_showMessage(store, "DOWNLOADING...", 7);
    machine_suspendEnergySaving(store->runner->core, 120);
    store->downloading = true;
}

static void store_finishDownloading(struct Store *store)
{
    store->downloading = false;

    if (store_downloadProgram(store, store->selectedIndex))
    {
        selectProgram(store->entries[store->selectedIndex].cachedPath);
    }
    else
    {
        for (int y = LIST_START_Y; y < LIST_START_Y + VISIBLE_ROWS; y++)
            store_fillRow(store, y, 0);
        store_showMessage(store, "DOWNLOAD FAILED", 7);
    }
}

void store_update(struct Store *store, struct CoreInput *input)
{
    struct Core *core = store->runner->core;

    // Check if async preview download finished
    store_checkPreview(store);

    // Save key before core_handleInput clears it
    char key = input->key;
    core_handleInput(core, input);

    if (store->loading)
    {
        store_finishLoading(store);
        return;
    }

    if (store->downloading)
    {
        store_finishDownloading(store);
        return;
    }

    bool touch = core->machine->ioRegisters.status.touch;
    int cy = core->machine->ioRegisters.touchY / 8;
    bool redraw = false;
    struct CoreInputGamepad *gp = &input->gamepads[0];

    if (store->screen == StoreScreenCategories)
    {
        bool catChanged = false;

        if ((gp->up && !store->lastUp) || key == CoreInputKeyUp)
        {
            store->selectedCategory = (store->selectedCategory > 0) ? store->selectedCategory - 1 : STORE_NUM_CATEGORIES - 1;
            catChanged = true;
        }
        if ((gp->down && !store->lastDown) || key == CoreInputKeyDown)
        {
            store->selectedCategory = (store->selectedCategory < STORE_NUM_CATEGORIES - 1) ? store->selectedCategory + 1 : 0;
            catChanged = true;
        }
        if (catChanged) store_drawCategories(store);

        if ((gp->buttonA && !store->lastButtonA) || key == CoreInputKeyReturn)
        {
            store_enterCategory(store);
            goto save_state;
        }

        if (touch && !store->lastTouch)
        {
            for (int i = 0; i < STORE_NUM_CATEGORIES; i++)
            {
                int y = 6 + i * 2;
                if (cy == y)
                {
                    if (i == store->selectedCategory)
                    {
                        store_enterCategory(store);
                        goto save_state;
                    }
                    store->selectedCategory = i;
                    store_drawCategories(store);
                }
            }
        }

        if ((gp->buttonB && !store->lastButtonB) || key == 27)
        {
            rebootNX();
            goto save_state;
        }
    }
    else // StoreScreenList
    {
        if (store->numEntries > 0)
        {
            if ((gp->up && !store->lastUp) || key == CoreInputKeyUp)
                { store_moveUp(store); redraw = true; }
            if ((gp->down && !store->lastDown) || key == CoreInputKeyDown)
                { store_moveDown(store); redraw = true; }
            if ((gp->left && !store->lastLeft) || key == CoreInputKeyLeft)
                { store_pageUp(store); redraw = true; }
            if ((gp->right && !store->lastRight) || key == CoreInputKeyRight)
                { store_pageDown(store); redraw = true; }

            if ((gp->buttonA && !store->lastButtonA) || key == CoreInputKeyReturn)
            {
                store_runSelected(store);
                goto save_state;
            }

            if (touch && !store->lastTouch && cy >= LIST_START_Y && cy < LIST_START_Y + VISIBLE_ROWS)
            {
                int idx = store->scrollOffset + (cy - LIST_START_Y);
                if (idx < store->numEntries)
                {
                    if (idx == store->selectedIndex) { store_runSelected(store); goto save_state; }
                    store->selectedIndex = idx;
                    redraw = true;
                }
            }
        }

        if ((gp->buttonB && !store->lastButtonB) || key == 27)
        {
            store_freePreview(store);
            store->screen = StoreScreenCategories;
            store_drawCategories(store);
            goto save_state;
        }
    }

    if (redraw)
    {
        store_drawList(store);
        store_loadPreview(store, store->selectedIndex);
        machine_suspendEnergySaving(core, 120);
    }

    overlay_draw(core, false);

save_state:
    store->lastTouch = touch;
    store->lastUp = gp->up;
    store->lastDown = gp->down;
    store->lastLeft = gp->left;
    store->lastRight = gp->right;
    store->lastButtonA = gp->buttonA;
    store->lastButtonB = gp->buttonB;
}
