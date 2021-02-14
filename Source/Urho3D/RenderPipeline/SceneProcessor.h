//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Graphics/Drawable.h"
#include "../RenderPipeline/BatchCompositor.h"
#include "../RenderPipeline/DrawableProcessor.h"
#include "../RenderPipeline/LightProcessor.h"

namespace Urho3D
{

class RenderPipelineInterface;
class RenderSurface;
class ShadowMapAllocator;
class Viewport;

/// Scene processor settings.
struct SceneProcessorSettings : public DrawableProcessorSettings
{
    /// Whether to render shadows.
    bool enableShadows_{ true };
    /// Whether to enable deferred rendering.
    bool deferred_{ false };

    /// Whether to render occlusion triangles in multiple threads.
    bool threadedOcclusion_{};
    /// Max number of occluder triangles.
    unsigned maxOccluderTriangles_{ 5000 };
    /// Occlusion buffer width.
    unsigned occlusionBufferSize_{ 256 };
    /// Occluder screen size threshold.
    float occluderSizeThreshold_{ 0.025f };
};

/// Scene processor for RenderPipeline
class URHO3D_API SceneProcessor : public Object, public LightProcessorCallback
{
    URHO3D_OBJECT(SceneProcessor, Object);

public:
    /// Construct.
    SceneProcessor(RenderPipelineInterface* renderPipeline, const ea::string& shadowTechnique);
    /// Destruct.
    ~SceneProcessor() override;

    /// Set passes.
    void SetPasses(ea::vector<SharedPtr<BatchCompositorPass>> passes);
    /// Set settings.
    void SetSettings(const SceneProcessorSettings& settings);

    /// Define before RenderPipeline update.
    void Define(RenderSurface* renderTarget, Viewport* viewport);
    /// Update frame info.
    void UpdateFrameInfo(const FrameInfo& frameInfo);
    /// Update drawables and batches.
    void Update();

    /// Return whether is valid.
    bool IsValid() const { return frameInfo_.camera_ && frameInfo_.octree_; }
    /// Return frame info.
    const FrameInfo& GetFrameInfo() const { return frameInfo_; }
    /// Return whether the pass object in callback corresponds to internal pass.
    bool IsInternalPass(Object* pass) const { return batchCompositor_ == pass; }
    /// Return drawable processor.
    DrawableProcessor* GetDrawableProcessor() const { return drawableProcessor_; }
    /// Return transient shadow map allocator.
    ShadowMapAllocator* GetShadowMapAllocator() const { return shadowMapAllocator_; }
    /// Return light volume batches.
    const auto& GetLightVolumeBatches() const { return batchCompositor_->GetLightVolumeBatches(); }

private:
    /// Called when update begins.
    void OnUpdateBegin(const FrameInfo& frameInfo);
    /// Return whether light needs shadow.
    bool IsLightShadowed(Light* light) override;
    /// Allocate shadow map for one frame.
    ShadowMap AllocateTransientShadowMap(const IntVector2& size) override;
    /// Draw occluders.
    void DrawOccluders();

    /// Drawable processor.
    SharedPtr<DrawableProcessor> drawableProcessor_;
    /// Batch compositor.
    SharedPtr<BatchCompositor> batchCompositor_;
    /// Transient shadow map allocator.
    SharedPtr<ShadowMapAllocator> shadowMapAllocator_;

    /// Settings.
    SceneProcessorSettings settings_;
    /// Frame info.
    FrameInfo frameInfo_;

    /// Occlusion buffer.
    SharedPtr<OcclusionBuffer> occlusionBuffer_;
    /// Currently active and filled occlusion buffer.
    OcclusionBuffer* activeOcclusionBuffer_{};
    /// Occluders in current frame.
    ea::vector<Drawable*> occluders_;
    /// Drawables in current frame.
    ea::vector<Drawable*> drawables_;
};

}