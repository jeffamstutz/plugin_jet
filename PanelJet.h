// ======================================================================== //
// Copyright 2009-2018 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "app/widgets/Panel.h"

namespace ospray {
  namespace jet_plugin {

    struct PanelJet : public Panel
    {
      PanelJet();

      void buildUI() override;

     private:
      // Helper functions //

      void ui_SimulationParameters();
      void ui_RenderingParameters();
      void ui_SimulationStart();
      void ui_SimulationStatus();

      // Data //

      // simulation data
      int resolution     = 50;
      int numFrames      = 100;
      float fps          = 60.f;
      bool addIfCanceled = false;

      // rendering data
      float samplingRate   = 0.25f;
      bool gradientShading = false;

      // misc. UI data
      int current_frame     = -1;
      bool cancelSimulation = false;
    };

  }  // namespace jet_plugin
}  // namespace ospray