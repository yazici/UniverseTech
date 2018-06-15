#include "UniBody.h"



UniBody::UniBody() {
	m_pTriangulator = new Triangulator(this);
	m_pPatch = new Patch(4);
	m_pPatch->SetPlanet(this);
}


UniBody::~UniBody() {
	delete m_pPatch;
	delete m_pTriangulator;
}

uint32_t UniBody::GetVertexCount() {
	return m_pTriangulator->GetVertexCount() * m_pPatch->GetVertexCount();
}

void UniBody::Initialize() {
	Load();
	m_pTriangulator->Init();
	m_pPatch->Init();
}

void UniBody::Update() {
	if(m_pTriangulator->Update()) {
		m_pTriangulator->GenerateGeometry();
		m_pPatch->BindInstances(m_pTriangulator->m_Positions);
		m_pPatch->UploadDistanceLUT(m_pTriangulator->m_DistanceLUT);
	}
}

void UniBody::Draw() {
	m_pPatch->Draw();
}
