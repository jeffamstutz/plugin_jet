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
#include "../../app/widgets/sg_ui/ospray_sg_ui.h"
#include "imgui.h"
// jobs
#include "../../app/app_utility/AsyncRenderEngine.h"
#include "../../app/jobs/JobScheduler.h"
// app sg utilities
#include "../../app/sg_utility/utility.h"
// ospray_sg
#include "ospray/sg/visitor/MarkAllAsModified.h"
// jet_plugin
#include "PanelJet.h"

namespace ospray {
  namespace jet_plugin {

    PanelJet::PanelJet() : Panel("Jet Panel") {}

    PanelJet::~PanelJet()
    {
      simulation_cleanup();
    }

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

        ui_TimeStepControls();

        if (simulationRunning) {
          ui_SimulationStatus();
        } else {
          ImGui::NewLine();
          ImGui::NewLine();
          ui_SimulationStart();
        }

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
      ImGui::Checkbox("auto show latest time step", &showLatestTimeStep);
    }

    void PanelJet::ui_RenderingParameters()
    {
      ImGui::Text("Initial Volume Rendering Parameters:");

      ImGui::DragFloat("sampling rate", &samplingRate, .01f, 0.01, 10.f);
      ImGui::Checkbox("gradient shading", &gradientShading);
      ImGui::Checkbox("auto update tf value range", &autoUpdateTfValueRange);
    }

    void PanelJet::ui_SimulationStart()
    {
      if (ImGui::Button("Launch New Simulation")) {
        cancelSimulation = false;

        // create new time-series node (sg::Selector) and do first sim time step
        job_scheduler::scheduleJob([&]() {
          job_scheduler::Nodes retval;
          simulationRunning = true;
          currentFrame      = 0;

          simulation_init(resolution, fps);

          selector_ptr   = sg::createNode("Jet Simulation", "Selector");
          auto &selector = *selector_ptr;

          auto [first_data, first_dims] = simulation_compute_timestep();
          auto first_volume = createSgVolume(first_data, first_dims, 0);
          selector.add(first_volume);

          if (autoUpdateTfValueRange)
            resetVoxelRangeOfMasterTfn(*selector_ptr);

          // run additional simulation time steps on a background task
          job_scheduler::detail::schedule([&]() {
            for (int i = 1; i < numFrames; ++i) {
              auto [data, dims] = simulation_compute_timestep();
              auto volume       = createSgVolume(data, dims, i);

              currentFrame++;

              // task to create a new volume for the latest time step
              job_scheduler::scheduleNodeOp([=]() {
                volume->traverse(sg::MarkAllAsModified{});
                volume->verify();
                volume->commit();
                selector_ptr->add(volume);
                if (autoUpdateTfValueRange)
                  resetVoxelRangeOfMasterTfn(*selector_ptr);
              });

              if (showLatestTimeStep) {
                currentTimeStep = i;
                job_scheduler::scheduleNodeValueChange(selector["index"], i);
              }

              if (cancelSimulation)
                break;
            }

            simulationRunning = false;
          });

          retval.push_back(selector_ptr);

          return retval;
        });
      }
    }

    void PanelJet::ui_SimulationStatus()
    {
      ImGui::Text("running sim...");
      ImGui::Text("current frame: %i", currentFrame);
      if (ImGui::Button("Cancel Simulation"))
        cancelSimulation = true;
    }

    void PanelJet::ui_TimeStepControls()
    {
      if (ImGui::SliderInt("Timestep", &currentTimeStep, 0, currentFrame)) {
        showLatestTimeStep = false;

        job_scheduler::scheduleNodeValueChange(selector_ptr->child("index"),
                                               currentTimeStep);
      }
    }

    std::shared_ptr<sg::Node> PanelJet::createSgVolume(SimData &data,
                                                       SimDims dims,
                                                       int whichTimeStep)
    {
      std::stringstream name;

      name << "jet_volume_";
      name << std::to_string(whichTimeStep / 10000).back();
      name << std::to_string(whichTimeStep / 1000).back();
      name << std::to_string(whichTimeStep / 100).back();
      name << std::to_string(whichTimeStep / 10).back();
      name << std::to_string(whichTimeStep % 10).back();

      auto volume_node = sg::createNode(name.str(), "StructuredVolume");

      auto voxel_data = std::make_shared<sg::DataVector1f>();
      voxel_data->v   = std::move(data);

      voxel_data->setName("voxelData");

      volume_node->add(voxel_data);

      volume_node->child("voxelType")  = std::string("float");
      volume_node->child("dimensions") = vec3i(dims);

      if (volume_node->hasChildRecursive("gradientShadingEnabled")) {
        volume_node->childRecursive("gradientShadingEnabled") = gradientShading;
      }

      if (volume_node->hasChildRecursive("samplingRate"))
        volume_node->childRecursive("samplingRate") = samplingRate;

      if (volume_node->hasChildRecursive("adaptiveMaxSamplingRate")) {
        volume_node->childRecursive("adaptiveMaxSamplingRate") =
            5 * samplingRate;
      }

      replaceAllTFsWithMasterTF(*volume_node);

      return volume_node;
    }

  }  // namespace jet_plugin
}  // namespace ospray
