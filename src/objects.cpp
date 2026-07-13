#include "objects.h"
#include "raylib.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>

static void ParseJsonFile(const char *path, Objects::ObjectTemplate *tpl)
{
    FILE *f = fopen(path, "r");
    if (!f) return;

    char buf[4096];
    int len = fread(buf, 1, sizeof(buf) - 1, f);
    buf[len] = '\0';
    fclose(f);

    char *p = buf;
    while (*p && *p != '{') p++;
    if (!*p) return;
    p++;

    while (*p && tpl->attrCount < Objects::MAX_ATTRS) {
        while (*p && *p != '"') p++;
        if (!*p) break;
        p++;

        char *key = p;
        while (*p && *p != '"') p++;
        if (!*p) break;
        *p = '\0';
        p++;

        while (*p && *p != ':') p++;
        if (!*p) break;
        p++;

        while (*p && *p != '"') p++;
        if (!*p) break;
        p++;

        char *val = p;
        while (*p && *p != '"') p++;
        if (!*p) break;
        *p = '\0';
        p++;

        strncpy(tpl->attrs[tpl->attrCount].key, key, sizeof(tpl->attrs[0].key) - 1);
        strncpy(tpl->attrs[tpl->attrCount].value, val, sizeof(tpl->attrs[0].value) - 1);
        tpl->attrCount++;

        while (*p && *p != ',' && *p != '}') p++;
        if (*p == ',') p++;
    }
}

namespace {

    Objects::ObjectTemplate templates[Objects::MAX_TEMPLATES];
    int templateCount = 0;

    int nextId = 1;

    inline bool isDirectory(const dirent *entry) { return entry->d_type == DT_DIR; }
    inline bool isNavDirectory(const dirent *entry) { return strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0; }
    inline bool isJsonFile(const dirent *entry) { return strcmp(entry->d_name + strlen(entry->d_name) - 5, ".json") == 0; }

    void ScanObjectsDir()
    {
        DIR *dir = opendir("objects");
        if (!dir) return;

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr && templateCount < Objects::MAX_TEMPLATES) {
            if (!isDirectory(entry)) continue;
            if (isNavDirectory(entry)) continue; // avoids recursion

            char dirpath[256];
            snprintf(dirpath, sizeof(dirpath), "objects/%s", entry->d_name);

            DIR *sub = opendir(dirpath);
            if (!sub) continue;

            struct dirent *file;
            while ((file = readdir(sub)) != nullptr) {
                if (!isJsonFile(file)) continue;

                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, file->d_name);

                Objects::ObjectTemplate *tpl = &templates[templateCount];
                tpl->attrCount = 0;
                strncpy(tpl->name, entry->d_name, sizeof(tpl->name) - 1);
                ParseJsonFile(filepath, tpl);
                templateCount++;
                break;
            }
            closedir(sub);
        }
        closedir(dir);
    }

}

Objects::ObjectRegistry Objects::registry = {};

void Objects::InitCache()
{
    ScanObjectsDir();
    printf("[Objects] Loaded %d template(s)\n", templateCount);
}

int Objects::Create(const char *name)
{
    for (int i = 0; i < templateCount; i++) {
        if (strcmp(templates[i].name, name) == 0) {
            if (registry.instanceCount >= MAX_INSTANCES) return -1;
            ObjectInstance *inst = &registry.instances[registry.instanceCount];
            inst->id = nextId++;
            inst->attrCount = templates[i].attrCount;
            memcpy(inst->attrs, templates[i].attrs, sizeof(Attribute) * inst->attrCount);
            registry.instanceCount++;
            return inst->id;
        }
    }
    return -1;
}

const int *Objects::Get(const char *key, const char *value, int *outCount)
{
    static int results[MAX_INSTANCES];
    int count = 0;
    for (int i = 0; i < registry.instanceCount; i++) {
        for (int j = 0; j < registry.instances[i].attrCount; j++) {
            if (strcmp(registry.instances[i].attrs[j].key, key) == 0 &&
                strcmp(registry.instances[i].attrs[j].value, value) == 0) {
                results[count++] = registry.instances[i].id;
                break;
            }
        }
    }
    *outCount = count;
    return count > 0 ? results : nullptr;
}

bool Objects::HasType(int id, const char *type)
{
    for (int i = 0; i < registry.instanceCount; i++) {
        if (registry.instances[i].id != id) continue;
        for (int j = 0; j < registry.instances[i].attrCount; j++) {
            if (strcmp(registry.instances[i].attrs[j].key, "type") == 0 &&
                strcmp(registry.instances[i].attrs[j].value, type) == 0)
                return true;
        }
        return false;
    }
    return false;
}

void Objects::SetAttr(int id, const char *key, const char *value)
{
    for (int i = 0; i < registry.instanceCount; i++) {
        if (registry.instances[i].id != id) continue;
        for (int j = 0; j < registry.instances[i].attrCount; j++) {
            if (strcmp(registry.instances[i].attrs[j].key, key) == 0) {
                strncpy(registry.instances[i].attrs[j].value, value, sizeof(registry.instances[i].attrs[0].value) - 1);
                return;
            }
        }
        if (registry.instances[i].attrCount < MAX_ATTRS) {
            strncpy(registry.instances[i].attrs[registry.instances[i].attrCount].key, key, sizeof(registry.instances[i].attrs[0].key) - 1);
            strncpy(registry.instances[i].attrs[registry.instances[i].attrCount].value, value, sizeof(registry.instances[i].attrs[0].value) - 1);
            registry.instances[i].attrCount++;
        }
        return;
    }
}

const char *Objects::GetAttr(int id, const char *key)
{
    for (int i = 0; i < registry.instanceCount; i++) {
        if (registry.instances[i].id != id) continue;
        for (int j = 0; j < registry.instances[i].attrCount; j++) {
            if (strcmp(registry.instances[i].attrs[j].key, key) == 0)
                return registry.instances[i].attrs[j].value;
        }
        return nullptr;
    }
    return nullptr;
}

int Objects::Spawn(int id, Vector3 position, Vector3 scale, float rotation)
{
    if (registry.bodyCount >= MAX_OBJECT_INSTANCES) return -1;

    ObjectInstance *inst = nullptr;
    for (int i = 0; i < registry.instanceCount; i++) {
        if (registry.instances[i].id == id) {
            inst = &registry.instances[i];
            break;
        }
    }
    if (!inst) return -1;

    const char *modelPath = nullptr;
    for (int i = 0; i < inst->attrCount; i++) {
        if (strcmp(inst->attrs[i].key, "model") == 0) {
            modelPath = inst->attrs[i].value;
            break;
        }
    }
    if (!modelPath) return -1;

    StaticBody &body = registry.bodies[registry.bodyCount];
    body.model = LoadModel(modelPath);
    body.position = position;
    body.scale = scale;
    body.rotation = rotation;
    body.UpdateAABB();
    body.ExtractTriangles(registry);
    body.instanceId = id;

    return registry.bodyCount++;
}


bool Objects::Despawn(int id)
{
    for (int i = 0; i < registry.bodyCount; i++) {
        if (registry.bodies[i].instanceId == id) {
            UnloadModel(registry.bodies[i].model);
            registry.bodies[i] = registry.bodies[registry.bodyCount - 1];
            registry.bodyCount--;
            return true;
        }
    }
    return false;
}

void Objects::UnloadObjectInstances()
{
    for (int i = 0; i < registry.bodyCount; i++)
        UnloadModel(registry.bodies[i].model);
    registry.bodyCount = 0;
    registry.trianglePoolCount = 0;
}

void Objects::UnloadAll()
{
    UnloadObjectInstances();
    templateCount = 0;
    registry.instanceCount = 0;
    nextId = 1;
}
