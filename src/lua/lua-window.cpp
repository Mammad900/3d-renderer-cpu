#include "lua-state.h"
#include "../data.h"
#include "../main.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaWindow() {
        Lua.new_usertype<Window>("Window",
        sol::meta_function::construct, [](sol::table props) {
            Vector2u size{
                props.get<sol::table>("size").get<uint>(1), 
                props.get<sol::table>("size").get<uint>(2)
            };
            std::function<void(sol::table)> onEvent = 
                props.get_or<std::function<void(sol::table)>>("on_event", [](sol::table _){});

            shared_ptr<Window> window = std::make_shared<Window>(Window{
                .quitWhenClosed = props.get_or("quit_when_closed", false),
                .frame = props["camera"].valid() ? std::make_shared<RenderTarget>(size, props.get_or("deferred", true)) : nullptr,
                .camera = props.get_or<shared_ptr<Camera>>("camera", nullptr),
                .scene = props.get_or<shared_ptr<Scene>>("scene", nullptr),
                .toolWindowFor = props.get_or<shared_ptr<Window>>("tool_window_for", nullptr),
                .hasGui = props.get_or("has_gui", false),
                .name = props["name"],
                .size = size,
                .syncFrameSize = props.get_or("sync_frame_size", true),
                .gui = props.get_or<std::function<void()>>("on_gui", [](){}),
                .onEvent = props["on_event"].valid() ? (std::function<void(std::optional<sf::Event>)>)(
                    [onEvent](std::optional<sf::Event> event) {
                        sol::table t = Lua.create_table();
                        if(const auto *resized = event->getIf<sf::Event::Resized>()) {
                            t["type"] = "resized";
                            t["x"] = resized->size.x;
                            t["y"] = resized->size.y;
                        }
                        else if(const auto *press = event->getIf<sf::Event::KeyPressed>()) {
                            t["type"] = "key_pressed";
                            t["key"] = (int)press->code;
                            t["alt"] = press->alt;
                            t["ctrl"] = press->control;
                            t["super"] = press->system;
                            t["shift"] = press->shift;
                        }
                        else if(const auto *release = event->getIf<sf::Event::KeyReleased>()) {
                            t["type"] = "key_released";
                            t["key"] = (int)release->code;
                            t["alt"] = release->alt;
                            t["ctrl"] = release->control;
                            t["super"] = release->system;
                            t["shift"] = release->shift;
                        }
                        else if(const auto *moved = event->getIf<sf::Event::MouseMoved>()) {
                            t["type"] = "mouse_moved";
                            t["position_x"] = moved->position.x;
                            t["position_y"] = moved->position.y;
                        }
                        else if(const auto *movedRaw = event->getIf<sf::Event::MouseMovedRaw>()) {
                            t["type"] = "mouse_moved_raw";
                            t["delta_x"] = movedRaw->delta.x;
                            t["delta_y"] = movedRaw->delta.y;
                        }
                        else if(const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                            t["type"] = "mouse_button_pressed";
                            t["position_x"] = mousePressed->position.x;
                            t["position_y"] = mousePressed->position.y;
                            t["button"] = (int)mousePressed->button + 1;
                        }
                        else if(const auto *mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                            t["type"] = "mouse_button_released";
                            t["position_x"] = mouseReleased->position.x;
                            t["position_y"] = mouseReleased->position.y;
                            t["button"] = (int)mouseReleased->button + 1;
                        } else {
                            return;
                        }
                        onEvent(t);
                    }
                ) : [](std::optional<sf::Event> _){}
            });
            if((window->camera != nullptr) + (window->scene != nullptr) == 1)
                throw std::runtime_error("One of window camera/scene was specified but not the other");
            if(window->toolWindowFor && !window->hasGui)
                throw std::runtime_error("has_gui has to be true when tool_window_for is set");
            if(window->frame)
                window->camera->frame = window->frame.get();
            if(initComplete)
                window->init();
            windows.push_back(window);
            return window;
        },
        "close", [](shared_ptr<Window> self) {
            self->window.close();
        },
        "size", sol::property(
            [](Window& self) { 
                sol::table res = Lua.create_table();
                res[1] = self.size.x;
                res[2] = self.size.y;
                return res;
            },
            [](Window& self, sol::table value) { 
                self.changeSize({value.get<uint>(1), value.get<uint>(2)});
            }
        ),
        "frame_size", sol::property(
            [](Window& self)-> sol::object { 
                sol::table res = Lua.create_table();
                if(!self.frame)
                    return sol::nil;
                res[1] = self.frame->size.x;
                res[2] = self.frame->size.y;
                return res;
            },
            [](Window& self, sol::table value) { 
                if(!self.frame)
                    throw std::runtime_error("Cannot change frame_size to a camera-less window, use set_camera first or set 'size' instead");
                Vector2u size{value.get<uint>(1), value.get<uint>(2)};
                if(self.syncFrameSize)
                    self.changeSize(size);
                else
                    self.changeFrameSize(size);
            }
        ),
        "tool_window_for", sol::property(
            [](Window& self) { return self.toolWindowFor; },
            [](Window& self, shared_ptr<Window> value) {
                if(value != nullptr && !self.hasGui)
                    throw std::runtime_error("Tried to set tool_window_for without setting has_gui to true");
                self.toolWindowFor = value;
            }
        ),
        "has_gui", sol::property(
            [](Window& self) { return self.hasGui; },
            [](Window& self, bool value) {
                if(self.toolWindowFor != nullptr && !value)
                    throw std::runtime_error("Tried to set has_gui to false before unsetting tool_window_for");
                self.hasGui = value;
            }
        ),
        "name", sol::property(
            [](Window& self) { return self.name; },
            [](Window& self, std::string value) { self.name = value; self.window.setTitle(value); }
        ),
        "camera", sol::property(
            [](Window& self) { return self.camera; },
            [](Window& self, shared_ptr<Camera> value) {
                if(!self.camera || !self.scene || !self.frame)
                    throw std::runtime_error("Cannot set camera for a camera-less window, use set_camera instead.");
                if(!value)
                    throw std::runtime_error("Tried to set camera to nil, use remove_camera instead.");
                self.camera->frame = self.frame.get();
                self.camera = value;
            }
        ),
        "scene", sol::readonly(&Window::scene),
        "quit_when_closed", &Window::quitWhenClosed,
        "sync_frame_size", &Window::syncFrameSize,
        "set_camera", [](shared_ptr<Window> self, shared_ptr<Scene> scene, shared_ptr<Camera> camera) {
            if(!scene)
                throw std::runtime_error("Scene is nil");
            if(!camera)
                throw std::runtime_error("Camera is nil");
            if(self->camera)
                self->camera->frame = nullptr;
            if(!self->frame) {
                self->frame = std::make_shared<RenderTarget>(Vector2u{0, 0}, true);
                self->changeFrameSize(self->size);
            }
            self->scene = scene;
            self->camera = camera;
            camera->frame = self->frame.get();
        },
        "remove_camera", [](shared_ptr<Window> self) {
            if(!self->camera || !self->scene || !self->frame)
                throw std::runtime_error("Tried to remove camera from a window that doesn't have one");
            self->camera->frame = nullptr;
            self->frame = nullptr;
            self->camera = nullptr;
            self->scene = nullptr;
        },
        "deferred", sol::property(
            [](Window& self)-> sol::object { 
                if(self.frame)
                    return sol::make_object(Lua, self.frame->deferred);
                else
                    return sol::nil;
            },
            [](Window& self, bool value) {
                if(!self.frame)
                    throw std::runtime_error("Cannot set deferred for a camera-less window, use set_camera first.");
                self.frame->changeSize(self.frame->size, value);
            }
        )
    );
}