#ifndef NOMINMAX
#define NOMINMAX
#endif
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <array>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "core/Application.h"
#include "core/Logger.h"
#include "core/ProjectPath.h"
#include "editor/SceneDocument.h"
#include "editor/SceneSerializer.h"
#include "input/Input.h"
#include "input/KeyCodes.h"
#include "math/MathUtils.h"
#include "math/Quaternion.h"
#include "renderer/Camera.h"
#include "renderer/DebugDraw.h"
#include "renderer/Material.h"
#include "renderer/Mesh.h"
#include "renderer/MeshCache.h"
#include "renderer/RenderBackend.h"
#include "renderer/RenderContext.h"
#include "renderer/Renderer.h"
#include "renderer/RenderViewUtils.h"
#include "renderer/Shader.h"
#include "scene/RuntimeSceneDefinition.h"
#include "scene/Scene.h"
#include "scene/SceneReferenceRuntime.h"
#include "scene/components/MeshComponent.h"
#include "scene/systems/PhysicsSystem.h"
#include "scene/systems/RenderSystem.h"

using namespace Monolith;
using namespace Monolith::Editor;
namespace fs = std::filesystem;

namespace {

struct PanelVisual {
  Mesh mesh;
  Vec3 center = Vec3::Zero();
};

fs::path FilePathOrEmpty(const fs::path& path) {
  std::error_code ec;
  return fs::exists(path, ec) && !ec ? path : fs::path{};
}

fs::path ResolveShaderPath(const char* fileName) {
  const fs::path root = ProjectPath::Root();
  const std::array<fs::path, 4> candidates = {
      root / "engine" / "renderer" / "shaders" / fileName,
      root.parent_path() / "horo-engine" / "renderer" / "shaders" / fileName,
      root / "renderer" / "shaders" / fileName,
      fs::path("shaders") / fileName,
  };

  for (const auto& candidate : candidates) {
    const fs::path found = FilePathOrEmpty(candidate);
    if (!found.empty())
      return found;
  }

  return candidates.back();
}

std::string BuildError(const SceneRuntimeOperationResult& result) {
  return result.error.empty() ? "Scene operation failed." : result.error;
}

bool IsPrimitiveTag(const std::string& meshTag) {
  return meshTag.empty() || meshTag == "box" || meshTag == "sphere" || meshTag == "cylinder" ||
         meshTag == "pyramid";
}

std::shared_ptr<Mesh> CreatePrimitiveMesh(const RuntimeSceneProp& prop) {
  if (prop.meshTag.empty() || prop.meshTag == "box")
    return std::make_shared<Mesh>(Mesh::CreateBox(prop.meshHalf.x, prop.meshHalf.y, prop.meshHalf.z));
  if (prop.meshTag == "sphere")
    return std::make_shared<Mesh>(Mesh::CreateSphere(0.5f, 18, 24));
  if (prop.meshTag == "cylinder")
    return std::make_shared<Mesh>(Mesh::CreateCylinder(0.35f, 0.5f, 24));
  if (prop.meshTag == "pyramid")
    return std::make_shared<Mesh>(Mesh::CreatePyramid(0.5f, 0.7f));
  return std::make_shared<Mesh>(Mesh::CreateBox(prop.meshHalf.x, prop.meshHalf.y, prop.meshHalf.z));
}

}  // namespace

class HoroStarterApp : public Application {
 public:
  HoroStarterApp()
      : Application(AppSpec{"Horo Engine Starter", 1280, 720, true, "assets/scenes/starter_world.json"}),
        m_runtime(&m_scene) {}

 protected:
  void OnInit() override {
    const RenderBackendInitResult backendInit = Renderer::InitializeBackend({RenderBackendId::OpenGL});
    if (!backendInit.ok)
      throw std::runtime_error("Failed to initialize renderer backend: " + backendInit.error);

    RenderContext::Init();
    DebugDraw::Init();
    m_camera.aspect = GetWindow().GetAspect();
    m_scene.AddSystem(std::make_unique<PhysicsSystem>(m_scene.physics));
    m_scene.AddRenderSystem(std::make_unique<RenderSystem>(m_camera, m_renderAlpha));

    InitMaterials();
    InitRuntimeCallbacks();

    m_sceneFilePath = fs::path(GetDefaultSceneFilePath());
    SceneDocument startupDoc;
    if (!TryLoadSceneFile(m_sceneFilePath, &startupDoc)) {
      startupDoc = MakeFallbackSceneDocument();
      startupDoc.filePath = m_sceneFilePath.string();
      std::string error;
      if (!ApplySceneDocument(startupDoc, &error))
        throw std::runtime_error(error);
    }
    m_activeDocument = std::move(startupDoc);

    if (m_runtime.GetSceneCamera().has_value())
      ApplySceneCamera(*m_runtime.GetSceneCamera());

    GetWindow().SetResizeCallback([this](int width, int height) {
      glViewport(0, 0, width, height);
      m_camera.aspect = static_cast<float>(width) / static_cast<float>(height);
    });
  }

  void OnFixedUpdate(float dt) override {
    m_scene.UpdateSystems(dt);
  }

  void OnUpdate(float dt) override {
    m_elapsedTime += dt;
    if (Input::IsKeyPressed(Key::Escape))
      glfwSetWindowShouldClose(GetWindow().GetNativeHandle(), 1);

    if (!m_runtime.GetSceneCamera().has_value()) {
      m_camera.position = {4.0f * Cos(m_elapsedTime * 0.35f), 3.25f, 7.5f};
      m_camera.target = {0.0f, 1.2f, 0.0f};
    }
  }

  void OnRender(float alpha) override {
    m_renderAlpha = alpha;

    RenderContext::BeginFrame({0.07f, 0.08f, 0.12f, 1.0f});
    glViewport(0, 0, GetWindow().GetWidth(), GetWindow().GetHeight());

    Renderer::BeginFrame(RenderFrameConfig{m_runtime.GetLights(), "starter-frame"});
    Renderer::BeginPass(RenderPassConfig{RenderPassId::OpaqueScene, BuildRenderView(m_camera), "starter-scene"});

    for (const PanelVisual& panel : m_panelVisuals)
      Renderer::Submit(panel.mesh, Mat4::Translate(panel.center), *m_panelMaterial);

    m_scene.RenderSystems(alpha);
    Renderer::EndPass();
    Renderer::EndFrame();

    DebugDraw::Box({0.0f, 0.5f, 0.0f}, {3.4f, 0.5f, 3.4f}, {0.35f, 0.8f, 1.0f, 1.0f});
    DebugDraw::Flush(m_camera);
    RenderContext::EndFrame();
  }

  void OnShutdown() override {
    m_runtime.Unload();
    DebugDraw::Shutdown();
  }

 private:
  bool TryLoadSceneFile(const fs::path& path, SceneDocument* outDocument) {
    try {
      SceneDocument doc = SceneSerializer::LoadFromFile(path.string());
      doc.filePath = path.string();
      std::string error;
      if (!ApplySceneDocument(doc, &error)) {
        LOG_ERROR("Failed to apply scene '%s': %s", path.string().c_str(), error.c_str());
        return false;
      }
      *outDocument = std::move(doc);
      return true;
    } catch (const std::exception& e) {
      LOG_WARN("Failed to load scene '%s': %s", path.string().c_str(), e.what());
      return false;
    }
  }

  SceneDocument MakeFallbackSceneDocument() const {
    SceneDocument doc;
    doc.sceneId = "starter_world";
    doc.sceneName = "Starter World";
    doc.filePath = m_sceneFilePath.string();
    doc.assets["placeholder_beacon"] = AssetDef{"assets/models/placeholder_beacon/placeholder_beacon.obj",
                                                 "1.0000,1.0000,1.0000",
                                                 "",
                                                 "starter-placeholder-beacon",
                                                 "Placeholder Beacon"};

    SceneObject floor;
    floor.id = "floor";
    floor.type = SceneObjectType::Panel;
    floor.position = {0.0f, -0.25f, 0.0f};
    floor.scale = {3.5f, 0.25f, 3.5f};

    SceneObject backWall;
    backWall.id = "back_wall";
    backWall.type = SceneObjectType::Panel;
    backWall.position = {0.0f, 1.75f, -3.75f};
    backWall.scale = {3.5f, 1.75f, 0.25f};

    SceneObject leftWall;
    leftWall.id = "left_wall";
    leftWall.type = SceneObjectType::Panel;
    leftWall.position = {-3.75f, 1.75f, 0.0f};
    leftWall.scale = {0.25f, 1.75f, 3.5f};

    SceneObject rightWall = leftWall;
    rightWall.id = "right_wall";
    rightWall.position.x = 3.75f;

    SceneObject crate;
    crate.id = "crate_prop";
    crate.type = SceneObjectType::Prop;
    crate.position = {-1.35f, 0.5f, 0.45f};
    crate.scale = {0.75f, 0.75f, 0.75f};
    crate.props["mesh"] = "box";

    SceneObject beacon;
    beacon.id = "beacon_prop";
    beacon.type = SceneObjectType::Prop;
    beacon.assetId = "placeholder_beacon";
    beacon.position = {1.25f, 0.0f, 0.25f};
    beacon.scale = {1.0f, 1.0f, 1.0f};

    SceneObject light;
    light.id = "main_light";
    light.type = SceneObjectType::Light;
    light.position = {0.0f, 2.75f, 1.75f};
    light.props["color"] = "1.0000,0.9500,0.8500";
    light.props["intensity"] = "3.0000";
    light.props["lightType"] = "point";
    light.props["radius"] = "10.0000";

    SceneObject camera;
    camera.id = "cam_000";
    camera.type = SceneObjectType::Camera;
    camera.position = {0.0f, 2.4f, 6.0f};
    camera.yaw = 180.0f;
    camera.props["fov"] = "60.0000";
    camera.props["nearClip"] = "0.1000";
    camera.props["farClip"] = "150.0000";

    doc.objects = {floor, backWall, leftWall, rightWall, crate, beacon, light, camera};
    return doc;
  }

  bool ApplySceneDocument(const SceneDocument& doc, std::string* outError) {
    const SceneRuntimeOperationResult result =
        m_runtime.GetCoordinator().IsActive() ? m_runtime.ReloadDocument(doc) : m_runtime.LoadDocument(doc);
    if (!result.ok) {
      if (outError)
        *outError = BuildError(result);
      return false;
    }

    RebuildPanelVisuals();
    if (m_runtime.GetSceneCamera().has_value())
      ApplySceneCamera(*m_runtime.GetSceneCamera());
    return true;
  }

  void RebuildPanelVisuals() {
    m_panelVisuals.clear();
    m_panelVisuals.reserve(m_runtime.GetPanels().size());
    for (const SceneReferencePanel& panel : m_runtime.GetPanels()) {
      PanelVisual visual;
      visual.center = panel.center;
      visual.mesh = Mesh::CreateBox(panel.half.x, panel.half.y, panel.half.z);
      m_panelVisuals.push_back(std::move(visual));
    }
  }

  void ApplySceneCamera(const RuntimeSceneCamera& sceneCamera) {
    m_camera.fovY = sceneCamera.fovY;
    m_camera.zNear = sceneCamera.nearClip;
    m_camera.zFar = sceneCamera.farClip;

    const float yawRad = ToRadians(sceneCamera.yaw);
    const float pitchRad = ToRadians(sceneCamera.pitch);
    const Vec3 lookDir = {-Sin(yawRad) * Cos(pitchRad), Sin(pitchRad), -Cos(yawRad) * Cos(pitchRad)};
    m_camera.position = sceneCamera.position;
    m_camera.target = sceneCamera.position + lookDir;
  }

  void InitMaterials() {
    const fs::path vertexShader = ResolveShaderPath("basic.vert");
    const fs::path fragmentShader = ResolveShaderPath("basic.frag");
    m_solidShader = std::make_shared<Shader>(Shader::FromFiles(vertexShader.string(), fragmentShader.string()));

    m_panelMaterial = std::make_shared<Material>();
    m_panelMaterial->shader = m_solidShader;
    m_panelMaterial->color = {0.22f, 0.24f, 0.30f, 1.0f};
    m_panelMaterial->roughness = 0.92f;

    m_propMaterialTemplate = std::make_shared<Material>();
    m_propMaterialTemplate->shader = m_solidShader;
    m_propMaterialTemplate->color = {0.80f, 0.87f, 1.00f, 1.0f};
    m_propMaterialTemplate->roughness = 0.55f;
  }

  void InitRuntimeCallbacks() {
    m_runtime.SetPropEntityCreatedCallback([this](const RuntimeSceneProp& prop, Entity entity, Scene& sceneRef) {
      std::shared_ptr<Mesh> mesh;
      if (IsPrimitiveTag(prop.meshTag)) {
        mesh = CreatePrimitiveMesh(prop);
      } else {
        try {
          mesh = m_meshCache.Get(prop.meshTag);
        } catch (const std::exception& e) {
          LOG_WARN("Failed to load mesh '%s': %s. Falling back to primitive box.", prop.meshTag.c_str(), e.what());
          mesh = CreatePrimitiveMesh(RuntimeSceneProp{});
        }
      }

      auto material = std::make_shared<Material>(*m_propMaterialTemplate);
      if (prop.id == "crate_prop")
        material->color = {0.98f, 0.72f, 0.34f, 1.0f};
      else if (prop.id == "beacon_prop")
        material->color = {0.35f, 0.85f, 1.0f, 1.0f};

      MeshComponent meshComponent;
      meshComponent.mesh = std::move(mesh);
      meshComponent.material = std::move(material);
      meshComponent.visible = true;
      meshComponent.meshTag = prop.meshTag;
      sceneRef.registry.Add<MeshComponent>(entity, std::move(meshComponent));
    });
  }

  Scene m_scene;
  SceneReferenceRuntime m_runtime;
  SceneDocument m_activeDocument;
  fs::path m_sceneFilePath;
  Camera m_camera;
  MeshCache m_meshCache;
  std::shared_ptr<Shader> m_solidShader;
  std::shared_ptr<Material> m_panelMaterial;
  std::shared_ptr<Material> m_propMaterialTemplate;
  std::vector<PanelVisual> m_panelVisuals;
  float m_renderAlpha = 0.0f;
  float m_elapsedTime = 0.0f;
};

int main(int argc, char** argv) {
  try {
    HoroStarterApp app;
    app.ParseArgs(argc, argv);
    app.Run();
  } catch (const std::exception& e) {
    LOG_ERROR("Fatal: %s", e.what());
    return 1;
  }

  return 0;
}
