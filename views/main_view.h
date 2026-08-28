#pragma once

#include <imgui.h>
#include "../state.h"
#include "../widgets/picker.h"
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
    void compose_stream_combo();
    void compose_viewport_control();

    widget::Viewport _viewport;
    widget::Picker _picker;

    std::array<std::pair<std::string, float>, 4> _frame_scale_items{
        std::pair{"Fit", 0.0f}, std::pair{"200%", 2.0f},
        std::pair{"100%", 1.0f}, std::pair{"50%", 0.5f}};
    int _frame_scale_id = 0;

    std::array<std::string, 2> _stream_items{"Color", "Depth"};
    SelectedStream _selected_stream = SelectedStream::Color;

    ImVec2 _viewport_size = ImVec2{960, 400};
};

}  // namespace app
