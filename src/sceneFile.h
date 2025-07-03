#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__

#include "data.h"
#include <filesystem>

void parseSceneFile(std::filesystem::path path, Scene *editingScene);
void serializeSceneToFile(Scene *scene, const std::filesystem::path &path);
void serializeEverything(const std::filesystem::path &path);

#endif /* __SCENEFILE_H__ */
