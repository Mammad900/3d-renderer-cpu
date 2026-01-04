# Renders

Each of the following renders is rendered at 5000x5000 resolution with maximum configuration, scaled down to 1000x1000 with GIMP, a technique called super-sampling. The downscaled versions are shown here. Click on the images to view at original resolution.

## Earth & Moon

[![Earth from space, with Africa at the center, the sun shining from behind right, the moon visible to the right, stars visible in the background.](Earth%201k.webp)](Earth.webp)

Taken with 0.6.1

Features used:

- Nested object transforms
- Sky-box
- Earth material
- Normal maps

## Earth Disco

[![A black earth with a vibrant blue light at the center, shining through what's supposed to be land, with the black oceans casting shadows and god rays](Earth%20Disco%201k.webp)](Earth%20Disco.webp)

Taken with 0.8.1

Features used:

- Point light shadow mapping
- God rays (volumetric lighting)
- Alpha cutout

## Glass Bottle

[![An empty glass bottle mostly filled with red liquid in the middle of the sky](Bottle%202%201k.webp)](Bottle.webp)

Taken with 0.8.1

Features used:

- Cubemap skybox
- Order independent transparency (OIT)
- Volumetric transparency with exponential attenuation
- Environment (skybox) reflections
