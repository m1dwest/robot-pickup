#pragma once

#include "../widgets/viewport.h"
#include "view.h"

namespace app {

class MainView : public View {
   public:
    void on_enter() override;
    void update(app::State& state) override;
    void compose() override;

   private:
    void compose_frame_scale();

    widget::Viewport _viewport;

    std::array<std::pair<std::string, float>, 4> _frame_scale_items{
        std::pair{"Fit", 0.0f}, std::pair{"200%", 2.0f},
        std::pair{"100%", 1.0f}, std::pair{"50%", 0.5f}};

    int _frame_scale_id = 0;
};

}  // namespace app
