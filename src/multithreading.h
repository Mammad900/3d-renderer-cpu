#ifndef __MULTITHREADING_H__
#define __MULTITHREADING_H__
#include "camera.h"

void startThreads(Camera *camera, bool secondPass);
void shutdownThreads();

#endif /* __MULTITHREADING_H__ */
