#include <algorithm>

#include "UniSceneRenderer.h"
#include "UniSceneManager.h"
#include "UniEngine.h"

// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all
// platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment) {
  void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
  data = _aligned_malloc(size, alignment);
#else
  int res = posix_memalign(&data, alignment, size);
  if (res != 0)
    data = nullptr;
#endif
  return data;
}

void alignedFree(void* data) {
#if defined(_MSC_VER) || defined(__MINGW32__)
  _aligned_free(data);
#else
  free(data);
#endif
}

void UniSceneRenderer::Initialise() {
  prepareUniformBuffers();
}


void UniSceneRenderer::ShutDown() {
  // Uniform buffers
  m_uniformBuffers.vsForward.destroy();
  m_uniformBuffers.modelViews.destroy();
  m_uniformBuffers.fsLights.destroy();
}

// Prepare and initialize uniform buffer containing shader uniforms
void UniSceneRenderer::prepareUniformBuffers() {

  auto &engine = UniEngine::GetInstance();

  // Fullscreen vertex shader
  VK_CHECK_RESULT(engine.vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_uniformBuffers.vsForward, sizeof(uboForward)));

  auto models = engine.SceneManager()->CurrentScene()->GetModels();
  auto dynamicAlignment = engine.getDynamicAlignment();
  size_t bufferSize =
      std::max(static_cast<int>(models.size()), 1) * dynamicAlignment;
  uboModelMatDynamic.model =
      (glm::mat4*)alignedAlloc(bufferSize, dynamicAlignment);
  assert(uboModelMatDynamic.model);

  // vertex shader dynamic
  VK_CHECK_RESULT(engine.vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      &m_uniformBuffers.modelViews, bufferSize));

  // fragment shader
  VK_CHECK_RESULT(engine.vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_uniformBuffers.fsLights, sizeof(uboLights)));

  // Map persistent
  VK_CHECK_RESULT(m_uniformBuffers.vsForward.map());
  VK_CHECK_RESULT(m_uniformBuffers.modelViews.map());
  VK_CHECK_RESULT(m_uniformBuffers.fsLights.map());

  // Update
  updateUniformBuffersScreen();
  updateUniformBufferDeferredLights();
  updateDynamicUniformBuffers();
}

void UniSceneRenderer::Render() {

  updateUniformBufferDeferredLights();
  updateDynamicUniformBuffers();
}

void UniSceneRenderer::ViewChanged() {
  updateUniformBuffersScreen();
}


void UniSceneRenderer::updateUniformBuffersScreen() {
  uboForward.model = glm::mat4(1.0f);

  uboForward.projection =
      SceneManager()->CurrentScene()->GetCameraComponent()->matrices.projection;
  uboForward.view =
      SceneManager()->CurrentScene()->GetCameraComponent()->matrices.view;
  uboForward.model = glm::mat4(1.f);
  memcpy(m_uniformBuffers.vsForward.mapped, &uboForward, sizeof(uboForward));
}

std::shared_ptr<UniSceneManager> UniSceneRenderer::SceneManager() {
  return UniEngine::GetInstance().SceneManager();
}

// Update light position uniform block
void UniSceneRenderer::updateUniformBufferDeferredLights() {
  // each scene light into uboFragmentLights.lights
  uint32_t lightCount = 0;
  SceneManager()
      ->CurrentScene()
      ->m_World->each<TransformComponent, LightComponent>(
          [&](ECS::Entity* ent,
              ECS::ComponentHandle<TransformComponent> transform,
              ECS::ComponentHandle<LightComponent> light) {
            // std::cout << "Found a light! " << lightCount;
            if (light->enabled && lightCount < MAX_LIGHT_COUNT) {
              auto lPos = glm::vec4(
                  transform->TransformLocalToWS(transform->m_dPos), 0);
              auto lCol = light->color;
              uboLights.lights[lightCount].color = lCol;
              uboLights.lights[lightCount].radius = light->radius;
              uboLights.lights[lightCount].position = lPos;
              // std::cout << ", radius: " << light->radius;
              // std::cout << ", pos: " << lPos.x << ", " << lPos.y << ", " <<
              // lPos.z << ". "; std::cout << ", col: " << lCol.r << ", " <<
              // lCol.g
              // << ", " << lCol.b << ", " << lCol.a << ". " << std::endl;
            } else {
              std::cout << "Light is disabled!" << std::endl;
            }
            lightCount++;
          });

  uboLights.viewPos =
      glm::vec4(
          SceneManager()->CurrentScene()->GetCameraComponent()->GetPosition(),
          0.0f) *
      glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
  uboLights.numLights = lightCount;

  //std::cout << "Enabled lights: " << lightCount << std::endl;

  memcpy(m_uniformBuffers.fsLights.mapped, &uboLights,
         sizeof(uboLights));
}

void UniSceneRenderer::updateDynamicUniformBuffers() {
  auto& engine = UniEngine::GetInstance();
  int index = 0;
  auto dynamicAlignment = engine.getDynamicAlignment();
  auto models = SceneManager()->CurrentScene()->GetModels();
  for_each(models.begin(), models.end(),
           [this, &index, dynamicAlignment](std::shared_ptr<UniModel> model) {
             glm::mat4* modelMat =
                 (glm::mat4*)(((uint64_t)uboModelMatDynamic.model +
                               (index * dynamicAlignment)));
             *modelMat = model->GetTransform()->GetModelMat();
             // std::cout << "Updating model matrix index: " << index <<
             // std::endl;
             index++;
           });

  memcpy(m_uniformBuffers.modelViews.mapped, uboModelMatDynamic.model,
         m_uniformBuffers.modelViews.size);
  // Flush to make changes visible to the host
  VkMappedMemoryRange memoryRange = vks::initializers::mappedMemoryRange();
  memoryRange.memory = m_uniformBuffers.modelViews.memory;
  memoryRange.size = m_uniformBuffers.modelViews.size;
  vkFlushMappedMemoryRanges(engine.GetDevice(), 1, &memoryRange);
}

void UniSceneRenderer::UpdateCamera(float width, float height) {
  SceneManager()->CurrentScene()->GetCameraComponent()->aspect = width / height;
  SceneManager()->CurrentScene()->GetCameraComponent()->CalculateProjection();
}
