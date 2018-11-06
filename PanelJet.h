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

#include "ospray/sg/common/Node.h"

#include "run_simulation.h"

namespace ospray {
  namespace jet_plugin {

    struct PanelJet : public Panel
    {
      PanelJet();
      ~PanelJet();

      void buildUI() override;

     private:
      // Helper functions //

      void ui_SimulationParameters();
      void ui_RenderingParameters();
      void ui_SimulationStart();
      void ui_SimulationStatus();
      void ui_TimeStepControls();

      std::shared_ptr<sg::Node> createSgVolume(SimData &data,
                                               SimDims dims,
                                               int whichTimeStep);

      // Data //

      // simulation data
      int resolution          = 25;
      int numFrames           = 250;
      float fps               = 60.f;
      bool showLatestTimeStep = true;

      // rendering data
      float samplingRate   = 0.25f;
      bool gradientShading = false;

      // misc. UI data
      int currentFrame       = -1;
      bool simulationRunning = false;
      bool cancelSimulation  = false;

      // time series data
      int currentTimeStep = 0;

      // scene graph data
      std::shared_ptr<sg::Node> selector_ptr;
    };

  }  // namespace jet_plugin
}  // namespace ospray
