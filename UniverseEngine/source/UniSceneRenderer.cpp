#include <algorithm>

#include "UniEngine.h"
#include "UniSceneManager.h"
#include "UniSceneRenderer.h"

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
  std::cout << "Prepare vertex descriptions..." << std::endl;
  SetupVertexDescriptions();

  std::cout << "Prepare uniform buffers..." << std::endl;
  PrepareUniformBuffers();

  std::cout << "Initialize descriptor set layouts..." << std::endl;
  SetupDescriptorSetLayout();

  std::cout << "Initialize pipelines..." << std::endl;
  PreparePipelines();

  std::cout << "Initialize descriptor pool..." << std::endl;
  SetupDescriptorPool();

  std::cout << "Initialize descriptor sets..." << std::endl;
  SetupDescriptorSets();

  std::cout << "Initialize command buffers..." << std::endl;
  BuildCommandBuffers();
}

void UniSceneRenderer::ShutDown() {
  // Uniform buffers
  m_uniformBuffers.vsForward.destroy();
  m_uniformBuffers.modelViews.destroy();
  m_uniformBuffers.fsLights.destroy();
}

void UniSceneRenderer::SetupVertexDescriptions() {
  // Binding description
  m_vertices.bindingDescriptions = {
      vks::initializers::vertexInputBindingDescription(
          VERTEX_BUFFER_BIND_ID, m_vertexLayout.stride(),
          VK_VERTEX_INPUT_RATE_VERTEX)};

  // Attribute descriptions
  m_vertices.attributeDescriptions = {
      vks::initializers::vertexInputAttributeDescription(
          VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
      // Location 1: Texture coordinates
      vks::initializers::vertexInputAttributeDescription(
          VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3),
      // Location 2: Color
      vks::initializers::vertexInputAttributeDescription(
          VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32B32_SFLOAT,
          sizeof(float) * 5),
      // Location 3: Normal
      vks::initializers::vertexInputAttributeDescription(
          VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT,
          sizeof(float) * 8),
      // Location 4: Tangent
      vks::initializers::vertexInputAttributeDescription(
          VERTEX_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32B32_SFLOAT,
          sizeof(float) * 11),
      // Location 5: Material ID
      vks::initializers::vertexInputAttributeDescription(
          VERTEX_BUFFER_BIND_ID, 5, VK_FORMAT_R32_SFLOAT,
          sizeof(float) * 14),
  };

  m_vertices.inputState =
      vks::initializers::pipelineVertexInputStateCreateInfo();
  m_vertices.inputState.vertexBindingDescriptionCount =
      static_cast<uint32_t>(m_vertices.bindingDescriptions.size());
  m_vertices.inputState.pVertexBindingDescriptions =
      m_vertices.bindingDescriptions.data();
  m_vertices.inputState.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(m_vertices.attributeDescriptions.size());
  m_vertices.inputState.pVertexAttributeDescriptions =
      m_vertices.attributeDescriptions.data();
}

// Prepare and initialize uniform buffer containing shader uniforms
void UniSceneRenderer::PrepareUniformBuffers() {
  auto& engine = UniEngine::GetInstance();

  // Fullscreen vertex shader
  VK_CHECK_RESULT(engine.vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_uniformBuffers.vsForward, sizeof(m_uboForward)));

  auto models = engine.SceneManager()->CurrentScene()->GetModels();
  auto dynamicAlignment = engine.getDynamicAlignment();
  size_t bufferSize =
      std::max(static_cast<int>(models.size()), 1) * dynamicAlignment;
  m_uboModelMatDynamic.model =
      (glm::mat4*)alignedAlloc(bufferSize, dynamicAlignment);
  assert(m_uboModelMatDynamic.model);

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
  UpdateUniformBufferDeferredLights();
  UpdateDynamicUniformBuffers();
}

void UniSceneRenderer::SetupDescriptorSetLayout() {
  // Deferred shading layout
  m_setLayoutBindings = {
      // Binding 0 : uniform buffer
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
      // Binding 1 : lights uniform buffer
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1),
      // Binding 2 : dynamic buffer for models
      vks::initializers::descriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 2)};

  auto device = UniEngine::GetInstance().GetDevice();

  m_descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(
      m_setLayoutBindings.data(),
      static_cast<uint32_t>(m_setLayoutBindings.size()));

  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &m_descriptorLayout,
                                              nullptr, &m_descriptorSetLayout));

  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
      vks::initializers::pipelineLayoutCreateInfo(&m_descriptorSetLayout, 1);

  VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      sizeof(m_TimeConstants), 0);

  pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

  VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo,
                                         nullptr, &m_pipelineLayouts.forward));
}

void UniSceneRenderer::PreparePipelines() {
  auto& engine = UniEngine::GetInstance();

  auto wfmode = VK_POLYGON_MODE_FILL;
  // if (m_useWireframe)
  //  wfmode = VK_POLYGON_MODE_LINE;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
      vks::initializers::pipelineInputAssemblyStateCreateInfo(
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

  VkPipelineRasterizationStateCreateInfo rasterizationState =
      vks::initializers::pipelineRasterizationStateCreateInfo(
          wfmode, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);

  VkPipelineColorBlendAttachmentState blendAttachmentState =
      vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

  VkPipelineColorBlendStateCreateInfo colorBlendState =
      vks::initializers::pipelineColorBlendStateCreateInfo(
          1, &blendAttachmentState);

  VkPipelineDepthStencilStateCreateInfo depthStencilState =
      vks::initializers::pipelineDepthStencilStateCreateInfo(
          VK_TRUE, VK_TRUE, VK_COMPARE_OP_GREATER_OR_EQUAL);

  VkPipelineViewportStateCreateInfo viewportState =
      vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

  VkPipelineMultisampleStateCreateInfo multisampleState =
      vks::initializers::pipelineMultisampleStateCreateInfo(
          VK_SAMPLE_COUNT_1_BIT, 0);

  std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                                     VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState =
      vks::initializers::pipelineDynamicStateCreateInfo(
          dynamicStateEnables.data(),
          static_cast<uint32_t>(dynamicStateEnables.size()), 0);

  VkGraphicsPipelineCreateInfo pipelineCreateInfo =
      vks::initializers::pipelineCreateInfo(
          m_pipelineLayouts.forward, UniEngine::GetInstance().GetRenderPass(),
          0);

  VkPipelineVertexInputStateCreateInfo emptyInputState{};
  emptyInputState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  emptyInputState.vertexAttributeDescriptionCount = 0;
  emptyInputState.pVertexAttributeDescriptions = nullptr;
  emptyInputState.vertexBindingDescriptionCount = 0;
  emptyInputState.pVertexBindingDescriptions = nullptr;
  pipelineCreateInfo.pVertexInputState = &m_vertices.inputState;

  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState = &colorBlendState;
  pipelineCreateInfo.pMultisampleState = &multisampleState;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pDepthStencilState = &depthStencilState;
  pipelineCreateInfo.pDynamicState = &dynamicState;
  pipelineCreateInfo.renderPass = UniEngine::GetInstance().GetRenderPass();

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

  shaderStages[0] = engine.loadShader(GetShader("omnishader.vert"),
                                      VK_SHADER_STAGE_VERTEX_BIT);
  shaderStages[1] = engine.loadShader(GetShader("omnishader.frag"),
                                      VK_SHADER_STAGE_FRAGMENT_BIT);

  pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineCreateInfo.pStages = shaderStages.data();

  // VK_CHECK_RESULT(vkCreateGraphicsPipelines(
  //    engine.GetDevice(), engine.GetPipelineCache(), 1, &pipelineCreateInfo,
  //    nullptr, &m_pipelines.forward));

  std::cout << "Doing material pipelines..." << std::endl;

  for (auto& material : m_materialInstances) {
    material->SetupMaterial(pipelineCreateInfo);
  }
}

void UniSceneRenderer::SetupDescriptorPool() {
  auto& engine = UniEngine::GetInstance();
  auto device = engine.GetDevice();
  auto& drawCmdBuffers = engine.GetCommandBuffers();

  auto modelCount = static_cast<uint32_t>(
      std::max((int)SceneManager()->CurrentScene()->GetModels().size(), 1));

  auto drawCmdBufferCount = static_cast<uint32_t>(drawCmdBuffers.size());

  std::vector<VkDescriptorPoolSize> poolSizes = {
      vks::initializers::descriptorPoolSize(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          2 * modelCount * drawCmdBufferCount),
      vks::initializers::descriptorPoolSize(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
          modelCount * drawCmdBufferCount)};

  VkDescriptorPoolCreateInfo descriptorPoolInfo =
      vks::initializers::descriptorPoolCreateInfo(
          static_cast<uint32_t>(poolSizes.size()), poolSizes.data(),
          static_cast<uint32_t>(drawCmdBufferCount));

  VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr,
                                         &m_descriptorPool));
}

void UniSceneRenderer::SetupDescriptorSets() {
  auto device = UniEngine::GetInstance().GetDevice();

  VkDescriptorSetAllocateInfo allocInfo =
      vks::initializers::descriptorSetAllocateInfo(m_descriptorPool,
                                                   &m_descriptorSetLayout, 1);

  VK_CHECK_RESULT(
      vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet));

  m_writeDescriptorSets = {
      // Binding 0 : view/proj buffer
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
          &m_uniformBuffers.vsForward.descriptor),
      // Binding 1 : lights buffer
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
          &m_uniformBuffers.fsLights.descriptor),
      // Binding 1 : Fragment shader uniform buffer
      vks::initializers::writeDescriptorSet(
          m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2,
          &m_uniformBuffers.modelViews.descriptor),

  };

  vkUpdateDescriptorSets(device,
                         static_cast<uint32_t>(m_writeDescriptorSets.size()),
                         m_writeDescriptorSets.data(), 0, nullptr);
}

void UniSceneRenderer::BuildCommandBuffers() {
  auto& engine = UniEngine::GetInstance();

  VkCommandBufferBeginInfo cmdBufInfo =
      vks::initializers::commandBufferBeginInfo();

  VkClearValue clearValues[2];
  clearValues[0].color = m_defaultClearColor;
  clearValues[1].depthStencil = {0.0f, 0};

  VkRenderPassBeginInfo renderPassBeginInfo =
      vks::initializers::renderPassBeginInfo();
  renderPassBeginInfo.renderPass = engine.GetRenderPass();
  renderPassBeginInfo.renderArea.offset.x = 0;
  renderPassBeginInfo.renderArea.offset.y = 0;
  renderPassBeginInfo.renderArea.extent.width = engine.width;
  renderPassBeginInfo.renderArea.extent.height = engine.height;
  renderPassBeginInfo.clearValueCount = 2;
  renderPassBeginInfo.pClearValues = clearValues;

  auto& drawCmdBuffers = engine.GetCommandBuffers();

  for (int32_t i = 0; i < drawCmdBuffers.size(); ++i) {
    // Set target frame buffer
    renderPassBeginInfo.framebuffer = engine.GetFrameBuffers()[i];

    VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

    vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = vks::initializers::viewport(
        (float)engine.width, (float)engine.height, 0.0f, 1.0f);
    vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

    VkRect2D scissor =
        vks::initializers::rect2D(engine.width, engine.height, 0, 0);
    vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

    // uint32_t dummy = 0;
    // vkCmdBindDescriptorSets(drawCmdBuffers[i],
    // VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                        pipelineLayouts.forward, 0, 1, &m_descriptorSet,
    //                        0, nullptr);

    // if(m_debugDisplay) {
    //	vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
    // pipelines.debug); 	vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);
    //	// Move viewport to display final composition in lower right corner
    //	viewport.x = viewport.width * 0.5f;
    //	viewport.y = viewport.height * 0.5f;
    //	viewport.width = (float)width * 0.5f;
    //	viewport.height = (float)height * 0.5f;
    //	vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
    //}

    UpdateCamera((float)viewport.width, (float)viewport.height);

    // Final composition as full screen quad
    // vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                  m_pipelines.forward);
    // vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);

    auto dynamicAlignment = engine.getDynamicAlignment();
    uint32_t index = 0;

    vkCmdPushConstants(
        drawCmdBuffers[i], m_pipelineLayouts.forward,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(m_TimeConstants), &m_TimeConstants);

    for (const auto& material : m_materialInstances) {
      // std::cout << "Building command buffer for material: "
      //          << material->GetName() << std::endl;
      index = material->AddToCommandBuffer(drawCmdBuffers[i], index);
    }

    vkCmdEndRenderPass(drawCmdBuffers[i]);

    VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
  }
}

void UniSceneRenderer::Render() {
  UpdateUniformBufferDeferredLights();
  UpdateDynamicUniformBuffers();
}

void UniSceneRenderer::Tick(uint32_t millis) {
  AddTimeDelta(millis);
}

void UniSceneRenderer::ViewChanged() {
  updateUniformBuffersScreen();
}

void UniSceneRenderer::updateUniformBuffersScreen() {
  m_uboForward.projection =
      SceneManager()->CurrentScene()->GetCameraComponent()->matrices.projection;
  m_uboForward.view =
      SceneManager()->CurrentScene()->GetCameraComponent()->matrices.view;
  memcpy(m_uniformBuffers.vsForward.mapped, &m_uboForward,
         sizeof(m_uboForward));
}

// Update light position uniform block
void UniSceneRenderer::UpdateUniformBufferDeferredLights() {
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
              glm::vec4 lPos = glm::vec4(transform->m_dPos, 0);
              glm::vec3 lCol = glm::vec3(light->color);
              uboLights.lights[lightCount].color = lCol;
              uboLights.lights[lightCount].radius = light->radius;
              uboLights.lights[lightCount].position =
                  lPos * glm::vec4(1.f, -1.f, 1.f, 1.f);
              // std::cout << ", radius: " << light->radius;
              // std::cout << ", pos: " << lPos.x << ", " << lPos.y << ", " <<
              // lPos.z << ". "; std::cout << ", col: " << lCol.r << ", " <<
              // lCol.g
              // << ", " << lCol.b << ", " << lCol.a << ". " << std::endl;
            } else {
              //std::cout << "Light is disabled!" << std::endl;
            }
            lightCount++;
          });

  uboLights.viewPos = glm::vec4(
      SceneManager()->CurrentScene()->GetCameraComponent()->GetPosition(),
      0.0f);
  uboLights.numLights = lightCount;

  // std::cout << "Enabled lights: " << lightCount << std::endl;

  memcpy(m_uniformBuffers.fsLights.mapped, &uboLights, sizeof(uboLights));
}

void UniSceneRenderer::UpdateDynamicUniformBuffers() {
  auto& engine = UniEngine::GetInstance();
  int index = 0;
  auto dynamicAlignment = engine.getDynamicAlignment();
  auto models = SceneManager()->CurrentScene()->GetModels();
  for_each(models.begin(), models.end(),
           [this, &index, dynamicAlignment](std::shared_ptr<ModelComponent> model) {
             glm::mat4* modelMat =
                 (glm::mat4*)(((uint64_t)m_uboModelMatDynamic.model +
                               (index * dynamicAlignment)));
             *modelMat = model->GetTransform()->GetModelMat();
             // std::cout << "Updating model matrix index: " << index <<
             // std::endl;
             index++;
           });

  memcpy(m_uniformBuffers.modelViews.mapped, m_uboModelMatDynamic.model,
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

void UniSceneRenderer::RegisterMaterial(std::shared_ptr<UniMaterial> mat) {
  m_materialInstances.push_back(mat);
}

void UniSceneRenderer::UnRegisterMaterial(std::shared_ptr<UniMaterial> mat) {
  int i = 0;
  while (i < m_materialInstances.size()) {
    if (m_materialInstances[i] == mat) {
      m_materialInstances.erase(m_materialInstances.begin() + i);
      i = 0;
    }
    i++;
  }
}

std::string UniSceneRenderer::GetShader(std::string shader) {
  auto& engine = UniEngine::GetInstance();
  auto aPath = engine.getAssetPath();

  return aPath + "shaders/" + shader + ".spv";
}

std::shared_ptr<UniSceneManager> UniSceneRenderer::SceneManager() {
  return UniEngine::GetInstance().SceneManager();
}
