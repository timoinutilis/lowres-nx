//
// Copyright 2025 WizzardSK
//
// Store - public cartridge browser for LowRes NX
// Similar to PICO-8 splore and TIC-80 surf
//

#ifndef store_h
#define store_h

#include "config.h"

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "core.h"
#include "runner.h"
#include "text_lib.h"

#define STORE_MAX_ENTRIES 2048
#define STORE_NAME_SIZE 20
#define STORE_URL_SIZE 256
#define STORE_NUM_CATEGORIES 4

enum StoreScreen {
    StoreScreenCategories,
    StoreScreenList,
};

struct StoreEntry {
    int topicId;
    char displayName[STORE_NAME_SIZE];
    char nxUrl[STORE_URL_SIZE];
    char screenshotUrl[STORE_URL_SIZE];
    char cachedPath[FILENAME_MAX];
    bool downloaded;
};

struct Store {
    struct Runner *runner;
    struct TextLib textLib;
    struct StoreEntry entries[STORE_MAX_ENTRIES];
    int numEntries;
    int selectedIndex;
    int scrollOffset;
    bool lastTouch;
    bool lastUp;
    bool lastDown;
    bool lastLeft;
    bool lastRight;
    bool lastButtonA;
    bool lastButtonB;
    char cacheDir[FILENAME_MAX];
    bool loading;
    bool downloading;
    enum StoreScreen screen;
    int selectedCategory;
    uint8_t *previewPixels;  // RGB 160x128 decoded screenshot
    int previewIndex;        // which entry the preview belongs to (-1 = none)
    // Async preview loading
    pthread_t previewThread;
    bool previewLoading;     // thread is running
    int previewRequestIndex; // index requested for async load
    uint8_t *previewPending; // pixels ready from background thread
    bool previewReady;       // pending pixels are ready to swap in
};

void store_init(struct Store *store, struct Runner *runner);
void store_show(struct Store *store, const char *storeFilePath);
void store_resume(struct Store *store);
void store_update(struct Store *store, struct CoreInput *input);
uint8_t *store_getPreviewPixels(struct Store *store, int *w, int *h);

#endif /* store_h */
