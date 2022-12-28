//
// Copyright (c) 2022-2022 the rbfx project.
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

#include "../Math/InverseKinematics.h"
#include "../Math/Ray.h"
#include "../Math/Transform.h"
#include "../Scene/Component.h"

namespace Urho3D
{

using IKNodeCache = ea::unordered_map<WeakPtr<Node>, IKNode>;

/// Base component for all IK solvers.
class IKSolverComponent : public Component
{
    URHO3D_OBJECT(IKSolverComponent, Component);

public:
    explicit IKSolverComponent(Context* context);
    ~IKSolverComponent() override;
    static void RegisterObject(Context* context);

    bool Initialize(IKNodeCache& nodeCache);
    void NotifyPositionsReady();
    void Solve(const IKSettings& settings, float timeStep);

    /// Internal. Marks chain tree as dirty.
    void OnTreeDirty();

protected:
    virtual bool InitializeNodes(IKNodeCache& nodeCache) = 0;
    virtual void UpdateChainLengths(const Transform& inverseFrameOfReference) = 0;
    virtual void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) = 0;

    void OnNodeSet(Node* previousNode, Node* currentNode) override;

    /// Add node to cache by name. Return null if the node is not found.
    IKNode* AddSolverNode(IKNodeCache& nodeCache, const ea::string& name);
    /// Add node that should be checked for existence before solving.
    Node* AddCheckedNode(IKNodeCache& nodeCache, const ea::string& name) const;
    /// Find Node corresponding to solver IKNode. Suboptimal, prefer to call it on initialization only!
    Node* FindNode(const IKNode& node) const;
    /// Set frame of reference Node used for calculations.
    void SetFrameOfReference(Node* node);
    void SetFrameOfReference(const IKNode& node);
    /// Same as SetFrameOfReference, except it accepts first child of the node.
    void SetParentAsFrameOfReference(const IKNode& childNode);

    /// Draw IK node in DebugRenderer.
    void DrawIKNode(DebugRenderer* debug, const IKNode& node, bool oriented) const;
    /// Draw IK segment line in DebugRenderer.
    void DrawIKSegment(DebugRenderer* debug, const IKNode& beginNode, const IKNode& endNode) const;
    /// Draw IK target in DebugRenderer.
    void DrawIKTarget(DebugRenderer* debug, const Vector3& position, const Quaternion& rotation, bool oriented) const;
    void DrawIKTarget(DebugRenderer* debug, const Node* node, bool oriented) const;
    /// Draw direction in DebugRenderer.
    void DrawDirection(DebugRenderer* debug, const Vector3& position, const Vector3& direction,
        bool markBegin = false, bool markEnd = true) const;

private:
    Transform GetFrameOfReferenceTransform() const;

    ea::vector<ea::pair<Node*, IKNode*>> solverNodes_;
    WeakPtr<Node> frameOfReferenceNode_;
};

class IKIdentitySolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKIdentitySolver, IKSolverComponent);

public:
    explicit IKIdentitySolver(Context* context);
    ~IKIdentitySolver() override;
    static void RegisterObject(Context* context);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

    void UpdateProperties();

private:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

    void EnsureInitialized();
    void UpdateRotationOffset();

    ea::string boneName_;
    ea::string targetName_;
    Quaternion rotationOffset_{Quaternion::ZERO};

    IKNode* boneNode_{};
    WeakPtr<Node> target_;
};

class IKLimbSolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKLimbSolver, IKSolverComponent);

public:
    explicit IKLimbSolver(Context* context);
    ~IKLimbSolver() override;
    static void RegisterObject(Context* context);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

private:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

    void EnsureInitialized();
    Vector3 GetTargetPosition() const;
    ea::pair<Vector3, Vector3> CalculateBendDirections(
        const Transform& frameOfReference, const Vector3& toeTargetPosition) const;

    ea::string firstBoneName_;
    ea::string secondBoneName_;
    ea::string thirdBoneName_;

    ea::string targetName_;
    ea::string bendTargetName_;

    float positionWeight_{1.0f};
    float rotationWeight_{0.0f};
    float bendTargetWeight_{1.0f};
    float minAngle_{0.0f};
    float maxAngle_{180.0f};
    Vector3 bendDirection_{Vector3::FORWARD};

    IKTrigonometricChain chain_;
    WeakPtr<Node> target_;
    WeakPtr<Node> bendTarget_;

    struct LocalCache
    {
        Vector3 defaultDirection_;
    } local_;

    Vector3 latestTargetPosition_;
};

class IKLegSolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKLegSolver, IKSolverComponent);

public:
    explicit IKLegSolver(Context* context);
    ~IKLegSolver() override;
    static void RegisterObject(Context* context);

    void UpdateProperties();

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

private:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

    void EnsureInitialized();
    void UpdateHeelGroundOffset();

    void RotateFoot(const Vector3& toeToHeel);

    Vector3 GetTargetPosition() const;
    Plane GetGroundPlane() const;
    Vector2 ProjectOnGround(const Vector3& position) const;
    float GetHeelReachDistance() const;
    float GetToeReachDistance() const;
    Vector3 RecoverFromGroundPenetration(const Vector3& toeToHeel, const Vector3& toePosition) const;
    Vector3 SnapToReachablePosition(const Vector3& toeToHeel, const Vector3& toePosition) const;

    ea::pair<Vector3, Vector3> CalculateBendDirections(
        const Transform& frameOfReference, const Vector3& toeTargetPosition) const;
    Quaternion CalculateLegRotation(
        const Vector3& toeTargetPosition, const Vector3& originalDirection, const Vector3& currentDirection) const;
    Vector2 CalculateLegOffset(
        const Transform& frameOfReference, const Vector3& toeTargetPosition, const Vector3& bendDirection);
    float CalculateTiptoeFactor(const Vector3& toeTargetPosition) const;

    Vector3 CalculateToeToHeel(const Transform& frameOfReference, float tiptoeFactor,
        const Vector3& toeTargetPosition, const Vector3& originalDirection, const Vector3& currentDirection) const;
    Vector3 CalculateToeToHeelBent(
        const Vector3& toeTargetPosition, const Vector3& approximateBendDirection) const;

    /// Attributes.
    /// @{
    ea::string thighBoneName_;
    ea::string calfBoneName_;
    ea::string heelBoneName_;
    ea::string toeBoneName_;

    ea::string targetName_;
    ea::string bendTargetName_;
    ea::string groundTargetName_;

    float positionWeight_{1.0f};
    float rotationWeight_{0.0f};
    float bendTargetWeight_{1.0f};
    float minKneeAngle_{0.0f};
    float maxKneeAngle_{180.0f};
    Vector2 baseTiptoe_{0.5f, 0.0f};
    Vector4 groundTiptoeTweaks_{0.2f, 0.2f, 0.2f, 0.2f};
    Vector3 bendDirection_{Vector3::FORWARD};

    float heelGroundOffset_{-1.0f};
    /// @}

    /// IK nodes and effectors.
    /// @{
    IKTrigonometricChain legChain_;
    IKNodeSegment footSegment_;
    WeakPtr<Node> target_;
    WeakPtr<Node> bendTarget_;
    WeakPtr<Node> groundTarget_;
    /// @}

    struct LocalCache
    {
        Vector3 toeToHeel_;
        float defaultThighToToeDistance_{};
        float tiptoeTweakOffset_{};

        Vector3 defaultDirection_;
        Quaternion defaultFootRotation_;
        Vector3 defaultToeOffset_;
        Quaternion defaultToeRotation_;
    } local_;

    Vector3 latestTargetPosition_;
    float latestTiptoeFactor_{};
};

class IKSpineSolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKSpineSolver, IKSolverComponent);

public:
    explicit IKSpineSolver(Context* context);
    ~IKSpineSolver() override;
    static void RegisterObject(Context* context);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

    /// Return the bone names.
    const StringVector& GetBoneNames() const { return boneNames_; }
    /// Return the target name.
    const ea::string& GetTargetName() const { return targetName_; }

protected:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

private:
    void EnsureInitialized();

    void SetOriginalTransforms(const Transform& frameOfReference);
    float GetTwistAngle(const IKNodeSegment& segment, Node* targetNode) const;

    StringVector boneNames_;
    ea::string twistTargetName_;
    ea::string targetName_;

    float positionWeight_{1.0f};
    float rotationWeight_{0.0f};
    float twistWeight_{1.0f};
    float maxAngle_{90.0f};

    IKSpineChain chain_;
    WeakPtr<Node> target_;
    WeakPtr<Node> twistTarget_;

    // TODO: Rename "local cache"
    struct LocalCache
    {
        ea::vector<Transform> defaultTransforms_;
        Vector3 baseDirection_;
    } local_;

    ea::vector<Quaternion> originalBoneRotations_;
};

class IKArmSolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKArmSolver, IKSolverComponent);

public:
    explicit IKArmSolver(Context* context);
    ~IKArmSolver() override;
    static void RegisterObject(Context* context);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

private:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

    void EnsureInitialized();

    void RotateShoulder(const Quaternion& rotation);

    ea::pair<Vector3, Vector3> CalculateBendDirections(
        const Transform& frameOfReference, const Vector3& toeTargetPosition) const;
    Quaternion CalculateMaxShoulderRotation(const Vector3& handTargetPosition) const;

    /// Attributes.
    /// @{
    ea::string shoulderBoneName_;
    ea::string armBoneName_;
    ea::string forearmBoneName_;
    ea::string handBoneName_;

    ea::string targetName_;
    ea::string bendTargetName_;

    float positionWeight_{1.0f};
    float rotationWeight_{0.0f};
    float bendTargetWeight_{1.0f};
    float minElbowAngle_{0.0f};
    float maxElbowAngle_{180.0f};
    Vector2 shoulderWeight_{};
    Vector3 bendDirection_{Vector3::FORWARD};
    Vector3 upDirection_{Vector3::UP};
    /// @}

    /// IK nodes and effectors.
    /// @{
    IKTrigonometricChain armChain_;
    IKNodeSegment shoulderSegment_;
    WeakPtr<Node> target_;
    WeakPtr<Node> bendTarget_;
    /// @}

    struct LocalCache
    {
        Vector3 defaultDirection_;
        Vector3 up_;

        Quaternion shoulderRotation_;
        Vector3 armOffset_;
        Quaternion armRotation_;
    } local_;
};

class IKChainSolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKChainSolver, IKSolverComponent);

public:
    explicit IKChainSolver(Context* context);
    ~IKChainSolver() override;
    static void RegisterObject(Context* context);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

    /// Return the bone names.
    const StringVector& GetBoneNames() const { return boneNames_; }
    /// Return the target name.
    const ea::string& GetTargetName() const { return targetName_; }

protected:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

private:
    StringVector boneNames_;
    ea::string targetName_;

    IKFabrikChain chain_;
    WeakPtr<Node> target_;
};

class IKLookAtSolver : public IKSolverComponent
{
    URHO3D_OBJECT(IKLookAtSolver, IKSolverComponent);

public:
    explicit IKLookAtSolver(Context* context);
    ~IKLookAtSolver() override;
    static void RegisterObject(Context* context);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

protected:
    /// Implement IKSolverComponent
    /// @{
    bool InitializeNodes(IKNodeCache& nodeCache) override;
    void UpdateChainLengths(const Transform& inverseFrameOfReference) override;
    void SolveInternal(const Transform& frameOfReference, const IKSettings& settings, float timeStep) override;
    /// @}

private:
    void EnsureInitialized();
    Ray GetEyeRay() const;

    ea::string neckBoneName_;
    ea::string headBoneName_;
    ea::string headTargetName_;
    ea::string lookAtTargetName_;

    float lookAtWeight_{0.0f};
    float rotationWeight_{1.0f};
    Vector3 eyeDirection_{Vector3::FORWARD};
    Vector3 eyeOffset_;
    float neckWeight_{0.5f};

    IKNodeSegment neckSegment_;
    IKEyeChain neckChain_;
    IKEyeChain headChain_;

    WeakPtr<Node> headTarget_;
    WeakPtr<Node> lookAtTarget_;

    struct LocalCache
    {
        Transform defaultNeckTransform_;
        Transform defaultHeadTransform_;
    } local_;
};

}