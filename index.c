#include "index.h"
#include "tree.h"
#include "pes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LOAD
int index_load(Index *index) {
    memset(index, 0, sizeof(Index));

    FILE *f = fopen(".pes/index", "rb");
    if (!f) return 0;

    fread(&index->count, sizeof(int), 1, f);

    if (index->count < 0 || index->count > MAX_INDEX_ENTRIES) {
        fclose(f);
        return -1;
    }

    fread(index->entries, sizeof(IndexEntry), index->count, f);

    fclose(f);
    return 0;
}

// SAVE
int index_save(const Index *index) {
    FILE *f = fopen(".pes/index", "wb");
    if (!f) return -1;

    fwrite(&index->count, sizeof(int), 1, f);
    fwrite(index->entries, sizeof(IndexEntry), index->count, f);

    fclose(f);
    return 0;
}

// ADD FILE
int index_add(Index *index, const char *path) {
    if (!index || !path) return -1;

    if (index->count >= MAX_INDEX_ENTRIES)
        return -1;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    void *data = malloc(size);
    if (!data) {
        fclose(f);
        return -1;
    }

    fread(data, 1, size, f);
    fclose(f);

    ObjectID id;
    if (object_write(OBJ_BLOB, data, size, &id) != 0) {
        free(data);
        return -1;
    }

    free(data);

    IndexEntry *e = &index->entries[index->count];
    memset(e, 0, sizeof(IndexEntry));

    snprintf(e->path, sizeof(e->path), "%s", path);
    e->mode = get_file_mode(path);
    memcpy(&e->hash, &id, sizeof(ObjectID));

    index->count++;

    return index_save(index);
}

// STATUS
int index_status(const Index *index) {
    printf("Staged changes:\n");

    if (index->count == 0) {
        printf("  (nothing to show)\n");
        return 0;
    }

    for (int i = 0; i < index->count; i++) {
        printf("  staged: %s\n", index->entries[i].path);
    }

    printf("\nUnstaged changes:\n  (nothing to show)\n");
    printf("\nUntracked files:\n  (nothing to show)\n");

    return 0;
}