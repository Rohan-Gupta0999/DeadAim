#pragma once

namespace deadaim {

// The fixed draw order for a frame: Background is drawn first (furthest
// back), Debug last (always on top, for development visuals only).
// Systems submit drawables to Renderer tagged with one of these; Renderer
// draws each layer in this order regardless of submission order.
enum class RenderLayer {
    Background,
    World,
    Fog,     // atmosphere OVER the world so it can obscure zombies, but
             // under the weapon and HUD, which must always stay readable
    Weapon,
    UI,
    Debug,
    Count // sentinel -- never submit to this one; used only for sizing
};

} // namespace deadaim