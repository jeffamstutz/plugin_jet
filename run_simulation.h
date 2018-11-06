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

#include <tuple>
#include "ospcommon/containers/AlignedVector.h"
#include "ospcommon/vec.h"

namespace ospray {
  namespace jet_plugin {

    using SimResults = std::tuple<ospcommon::containers::AlignedVector<float>,
                                  ospcommon::vec3i>;

    void simulation_init(size_t resolutionX, double fps);

    SimResults simulation_compute_timestep();

    void simulation_cleanup();

  }  // namespace jet_plugin
}  // namespace ospray
