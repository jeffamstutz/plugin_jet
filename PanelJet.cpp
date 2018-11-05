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

// imgui
#include "imgui.h"
#include "../../app/widgets/sg_ui/ospray_sg_ui.h"
// jobs
#include "../../app/jobs/JobScheduler.h"
// jet_plugin
#include "PanelJet.h"
#include "run_simulation.h"
// ospcommon
#include "ospcommon/utility/OnScopeExit.h"

namespace ospray {
  namespace jet_plugin {

    PanelJet::PanelJet() : Panel("Jet Panel - Plugin") {}

    void PanelJet::buildUI()
    {
      auto flags = g_defaultWindowFlags | ImGuiWindowFlags_AlwaysAutoResize;
      if (ImGui::Begin("Jet Panel", nullptr, flags)) {
        ui_SimulationParameters();

        ImGui::NewLine();
        ImGui::Separator();

        ui_RenderingParameters();

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Separator();

        if (current_frame == -1)
          ui_SimulationStart();
        else
          ui_SimulationStatus();

        if (ImGui::Button("Close"))
          setShown(false);

        ImGui::End();
      }
    }

    void PanelJet::ui_SimulationParameters()
    {
      ImGui::Text("Simulation Parameters:");

      ImGui::DragInt("resolutionX", &resolution, .1f, 10, 1000000);
      ImGui::DragInt("# frames", &numFrames, .1f, 1, 1000000);
      ImGui::DragFloat("sim FPS", &fps, .1f, 1.f, 120.f);
      ImGui::Checkbox("add volume if canceled", &addIfCanceled);
    }

    void PanelJet::ui_RenderingParameters()
    {
      ImGui::Text("Initial Volume Rendering Parameters:");

      ImGui::DragFloat("sampling rate", &samplingRate, .01f, 0.01, 10.f);
      ImGui::Checkbox("gradient shading", &gradientShading);
    }

    void PanelJet::ui_SimulationStart()
    {
      ImGui::NewLine();
      ImGui::NewLine();
      if (ImGui::Button("Launch New Simulation")) {
        cancelSimulation = false;
        job_scheduler::schedule_job([&]() {
          job_scheduler::Nodes retval;
          current_frame = 0;

          auto [data, dims] = run_simulation(
              resolution, numFrames, fps, current_frame, cancelSimulation);

          utility::OnScopeExit([&]() { current_frame = -1; });

          if (cancelSimulation && !addIfCanceled)
            return retval;

          // create sg nodes

          auto volume_node = sg::createNode("basic_volume", "StructuredVolume");

          auto voxel_data = std::make_shared<sg::DataVector1f>();
          voxel_data->v   = std::move(data);

          voxel_data->setName("voxelData");

          volume_node->add(voxel_data);

          // volume attributes

          volume_node->child("voxelType")  = std::string("float");
          volume_node->child("dimensions") = vec3i(dims);

          if (volume_node->hasChildRecursive("gradientShadingEnabled")) {
            volume_node->childRecursive("gradientShadingEnabled") =
                gradientShading;
          }

          if (volume_node->hasChildRecursive("samplingRate"))
            volume_node->childRecursive("samplingRate") = samplingRate;

          if (volume_node->hasChildRecursive("adaptiveMaxSamplingRate")) {
            volume_node->childRecursive("adaptiveMaxSamplingRate") =
                5 * samplingRate;
          }

          retval.push_back(volume_node);

          return retval;
        });
      }
    }

    void PanelJet::ui_SimulationStatus()
    {
      ImGui::Text("running sim...");
      ImGui::Text("current frame: %i", current_frame);
      if (ImGui::Button("Cancel Simulation"))
        cancelSimulation = true;
    }

  }  // namespace jet_plugin
}  // namespace ospray
