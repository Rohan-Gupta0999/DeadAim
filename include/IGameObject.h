#pragma once

namespace deadaim {

class Renderer;


class IGameObject {
public:
    virtual ~IGameObject() = default;

    virtual void update(float dt) = 0;
    virtual void render(Renderer& renderer) const = 0;
    virtual bool isAlive() const = 0;
};

} 
