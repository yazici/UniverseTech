#include "UniBody.h"
#include "UniEngine.h"

UniBody::UniBody(double radius) {
	m_Radius = radius;
	m_pTriangulator = new Triangulator(this);
	m_pPatch = new Patch(4);
	m_pPatch->SetPlanet(this);
}


UniBody::~UniBody() {
	delete m_pPatch;
	delete m_pTriangulator;

	m_Texture.destroy();
	m_HeightMap.destroy();
	m_Detail1.destroy();
	m_Detail2.destroy();
	m_HeightDetail.destroy();

}

void UniBody::LoadTextures(vks::VulkanDevice *device, VkQueue copyQueue) {
	// Textures
	std::string texFormatSuffix;
	VkFormat texFormat;
	// Get supported compressed texture format
	if(device->features.textureCompressionBC) {
		texFormatSuffix = "_bc3_unorm";
		texFormat = VK_FORMAT_BC3_UNORM_BLOCK;
	} else if(device->features.textureCompressionASTC_LDR) {
		texFormatSuffix = "_astc_8x8_unorm";
		texFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
	} else if(device->features.textureCompressionETC2) {
		texFormatSuffix = "_etc2_unorm";
		texFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
	} else {
		vks::tools::exitFatal("Device does not support any compressed texture format!", VK_ERROR_FEATURE_NOT_PRESENT);
	}

	std::vector<std::pair<std::string, vks::Texture2D*>> textures = {
		{ m_HeightMapPath, &m_HeightMap },
	    { m_HeightDetailPath, &m_HeightDetail },
	    { m_TexturePath, &m_Texture },
	    { m_Detail1Path, &m_Detail1 },
	    { m_Detail2Path, &m_Detail2 },
	};

	for(auto tpair : textures) {
		auto tPath = tpair.first;
		auto tTex = tpair.second;


		if(!tPath.empty()) {
			tTex->loadFromFile(getAssetPath() + tPath + texFormatSuffix + ".ktx", texFormat, device, copyQueue);
		}
		else {
			std::vector<glm::vec4> buffer(4 * 4);
			for(int i = 0; i < buffer.size(); i++) {
				buffer[i] = glm::vec4(1.f, 1.f, 1.f, 1.f);
			}
			tTex->fromBuffer(buffer.data(), buffer.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT, 4, 4, device, copyQueue, VK_FILTER_LINEAR);
		}

	}

}

uint32_t UniBody::GetVertexCount() {
	return m_pTriangulator->GetVertexCount() *m_pPatch->GetVertexCount();
}

void UniBody::Initialize() {
	UniEngine& engine = UniEngine::GetInstance();
	auto device = engine.GetDevice();
	LoadTextures(engine.vulkanDevice, engine.GetQueue());
	GetTransform()->SetPosition(glm::vec3(0, 0, 0));
	m_pTriangulator->Init();
	m_pPatch->Init();
	Update();
}

void UniBody::Update() {
	if(m_pTriangulator->Update()) {
		m_pTriangulator->GenerateGeometry();
		m_pPatch->BindInstances(m_pTriangulator->m_Positions);
		m_pPatch->UploadDistanceLUT(m_pTriangulator->m_DistanceLUT);
		//std::cout << "Updated Triangulator." << std::endl;
	}
	//std::cout << "Body now has " << GetVertexCount() << " vertices." << std::endl;

	m_pPatch->Draw();
}

void UniBody::Draw() {
	// hahahah
}
