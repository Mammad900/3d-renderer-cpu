#ifndef __FOG_H__
#define __FOG_H__

#include "data.h"

Color sampleFog(Vec3 start, Vec3 end, Color background, Scene &scene, shared_ptr<Volume> volume);

#endif /* __FOG_H__ */
