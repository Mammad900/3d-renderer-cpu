#include "lua-state.h"
#include "../audio.h"
#include <memory>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaAudio() {
    Lua.new_usertype<AudioComponent>("AudioComponent",
        sol::meta_function::construct, sol::overload(
            [](std::shared_ptr<sf::SoundBuffer> buffer) { return std::make_shared<AudioComponent>(buffer); },
            [](sol::this_state s, std::string file) { 
                sol::state_view lua(s);
                return std::make_shared<AudioComponent>(get_calling_script_path(lua) / file); 
            },
            [](std::shared_ptr<sf::SoundBuffer> buffer, bool loop) { 
                shared_ptr<AudioComponent> audio = std::make_shared<AudioComponent>(buffer); 
                audio->sound.setLooping(loop);
                return audio;
            },
            [](sol::this_state s, std::string file, bool loop) { 
                sol::state_view lua(s);
                shared_ptr<AudioComponent> audio = std::make_shared<AudioComponent>(get_calling_script_path(lua) / file); 
                audio->sound.setLooping(loop);
                return audio;
            }
        ),
        // "velocity", sol::property(
        //     [](AudioComponent &self)->Vec3 { sf::Vector3f i = self.sound.getVelocity(); return {i.x, i.y, i.z}; },
        //     [](AudioComponent &self, Vec3 value) { self.sound.setVelocity(value * self.obj->transformRotation); }
        // ),
        "sound", sol::property(
            [](AudioComponent &self) { return self.buffer; },
            [](AudioComponent &self, shared_ptr<sf::SoundBuffer> value) { self.sound.setBuffer(*value); self.buffer = value; }
        ),
        "pitch", sol::property(
            [](AudioComponent &self) { return self.sound.getPitch(); },
            [](AudioComponent &self, float value) { self.sound.setPitch(value); }
        ),
        "volume", sol::property(
            [](AudioComponent &self) { return self.sound.getVolume(); },
            [](AudioComponent &self, float value) { self.sound.setVolume(value); }
        ),
        "loop", sol::property(
            [](AudioComponent &self) { return self.sound.isLooping(); },
            [](AudioComponent &self, bool value) { self.sound.setLooping(value); }
        ),
        "play", &AudioComponent::play,
        "pause", &AudioComponent::pause,
        "stop", &AudioComponent::stop,
        "is_playing", &AudioComponent::isPlaying,
        "as_component", [](std::shared_ptr<AudioComponent>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<AudioListenerComponent>("AudioListenerComponent",
        sol::meta_function::construct, []() {
            return std::make_shared<AudioListenerComponent>();
        },
        "as_component", [](std::shared_ptr<AudioListenerComponent>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<sf::SoundBuffer>("Sound",
        sol::meta_function::construct, [](sol::this_state s, std::string file) { 
            sol::state_view lua(s);
            return std::make_shared<sf::SoundBuffer>(get_calling_script_path(lua) / file);
        }
    );
}