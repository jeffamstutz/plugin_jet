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

#include "run_simulation.h"
// ospcommon
#include "ospcommon/multidim_index_sequence.h"
#include "ospcommon/tasking/parallel_foreach.h"
#include "ospcommon/vec.h"
using namespace ospcommon;
// jet
#include <jet/jet.h>

namespace ospray {
  namespace jet_plugin {

    // Constants //////////////////////////////////////////////////////////////

    constexpr size_t kEdgeBlur = 3;
    constexpr float kEdgeBlurF = 3.f;

    // Helper Functions  //////////////////////////////////////////////////////

    static inline float smoothStep(float edge0, float edge1, float x)
    {
      float t = jet::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
      return t * t * (3.f - 2.f * t);
    }

    static inline containers::AlignedVector<float> extractVolumeData(
        const jet::ScalarGrid3Ptr &density)
    {
      const auto size = density->dataSize();

      multidim_index_sequence<3> index(vec3i(size.x, size.y, size.z));
      containers::AlignedVector<float> data(index.total_indices());

      tasking::serial_for(index.total_indices(), [&](size_t v) {
        auto current_index = index.reshape(v);

        const auto &i = current_index.x;
        const auto &j = current_index.y;
        const auto &k = current_index.z;

        float d = static_cast<float>((*density)(i, j, k));

        // Blur the edge for less-noisy rendering
        if (i < kEdgeBlur)
          d *= smoothStep(0.f, kEdgeBlurF, static_cast<float>(i));
        if (i > size.x - 1 - kEdgeBlur) {
          d *=
              smoothStep(0.f, kEdgeBlurF, static_cast<float>((size.x - 1) - i));
        }
        if (j < kEdgeBlur)
          d *= smoothStep(0.f, kEdgeBlurF, static_cast<float>(j));
        if (j > size.y - 1 - kEdgeBlur) {
          d *=
              smoothStep(0.f, kEdgeBlurF, static_cast<float>((size.y - 1) - j));
        }
        if (k < kEdgeBlur)
          d *= smoothStep(0.f, kEdgeBlurF, static_cast<float>(k));
        if (k > size.z - 1 - kEdgeBlur) {
          d *=
              smoothStep(0.f, kEdgeBlurF, static_cast<float>((size.z - 1) - k));
        }

        data[v] = d;
      });

      return data;
    }

    SimResults run_simulation(size_t resolutionX,
                              int numberOfFrames,
                              double fps)
    {
      jet::Logging::mute();

      vec3i dims(resolutionX, 2 * resolutionX, resolutionX);

      // Build solver
      auto solver =
          jet::GridSmokeSolver3::builder()
              .withResolution({dims.x, dims.y, dims.z})
              .withDomainSizeX(1.0)
              .makeShared();

      solver->setAdvectionSolver(std::make_shared<jet::CubicSemiLagrangian3>());

      auto grids  = solver->gridSystemData();
      auto domain = grids->boundingBox();

      // Build emitter
      auto box = jet::Box3::builder()
                     .withLowerCorner({0.45, -1, 0.45})
                     .withUpperCorner({0.55, 0.05, 0.55})
                     .makeShared();

      auto emitter = jet::VolumeGridEmitter3::builder()
                         .withSourceRegion(box)
                         .withIsOneShot(false)
                         .makeShared();

      solver->setEmitter(emitter);
      emitter->addStepFunctionTarget(solver->smokeDensity(), 0, 1);
      emitter->addStepFunctionTarget(solver->temperature(), 0, 1);

      // Build collider
      auto sphere = jet::Sphere3::builder()
                        .withCenter({0.5, 0.3, 0.5})
                        .withRadius(0.075 * domain.width())
                        .makeShared();

      auto collider =
          jet::RigidBodyCollider3::builder().withSurface(sphere).makeShared();

      solver->setCollider(collider);

      // Run simulation
      for (jet::Frame frame(0, 1.0 / fps); frame.index < numberOfFrames;
           ++frame) {
        printf("Calculating time step %i\n", frame.index);
        solver->update(frame);
      }

      auto density = solver->smokeDensity();
      return std::make_tuple(extractVolumeData(density), dims);
    }

  }  // namespace jet_plugin
}  // namespace ospray