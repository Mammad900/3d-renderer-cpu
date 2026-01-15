#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "object.h"
#include <SFML/Audio.hpp>

class AudioListenerComponent : public Component {
  public:
    void update() override {
        sf::Listener::setPosition(obj->globalPosition);
        sf::Listener::setUpVector(Vec3{0, 1, 0} * obj->transformRotation);
        sf::Listener::setDirection(Vec3{0, 0, 1} * obj->transformRotation);
    }

    std::string name() override { return "Audio Listener"; }
};

class AudioComponent : public Component {
  public:
    sf::Sound sound;
    std::shared_ptr<sf::SoundBuffer> buffer;

    AudioComponent(std::shared_ptr<sf::SoundBuffer> buffer): sound(*buffer), buffer(buffer) {}
    AudioComponent(std::string file): AudioComponent(std::make_shared<sf::SoundBuffer>(file)) {}

    void update() override {
        sound.setPosition(obj->globalPosition);
        sound.setDirection(Vec3{0,0,1} * obj->transformRotation);
    }

    void play() {
        sound.play();
    }
    void pause() {
        sound.pause();
    }
    void stop() {
        sound.stop();
    }
    bool isPlaying() {
        return sound.getStatus() == sf::Sound::Status::Playing;
    }

    std::string name() override { return "Audio"; }
};


#endif /* __AUDIO_H__ */
