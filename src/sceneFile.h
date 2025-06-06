#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__

#include "data.h"
#include <filesystem>

void parseSceneFile(std::filesystem::path path, Scene *editingScene);

#endif /* __SCENEFILE_H__ */
