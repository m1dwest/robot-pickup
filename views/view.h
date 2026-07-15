#pragma once

namespace app {

class State;

class View {
   public:
    virtual ~View() = default;

    virtual void on_enter() = 0;
    virtual void update(app::State&) = 0;
    virtual void compose() = 0;
};

}  // namespace app
