#include "plugin.h"

#include "Rubbish.h"
#include "RenderBuffer.h"

#include "CGeneral.h"
#include "CTimer.h"
#include "CWeather.h"
#include "CCamera.h"
#include "CWorld.h"
#include "CVehicle.h"
#include "CTxdStore.h"
#include "CVector.h"
#include "CCullZones.h"
#include "CBulletTraces.h"
#include "Utility.h"
#include "CCullZones.cpp"
#include "CGame.h"




#define RUBBISH_MAX_DIST (25.0f)
#define RUBBISH_FADE_DIST (25.0f)
RwTexture* gpRubbishTexture[14];
RwImVertexIndex RubbishIndexList[6];
RwImVertexIndex RubbishIndexList2[6];
RwIm3DVertex RubbishVertices[4];
bool CRubbish::bRubbishInvisible;
int32_t CRubbish::RubbishVisibility;
COneSheet CRubbish::aSheets[NUM_RUBBISH_SHEETS];
COneSheet CRubbish::StartEmptyList;
COneSheet CRubbish::EndEmptyList;
COneSheet CRubbish::StartStaticsList;
COneSheet CRubbish::EndStaticsList;
COneSheet CRubbish::StartMoversList;
COneSheet CRubbish::EndMoversList;

const float aAnimations[3][34] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

	{ 0.0f, 0.05f, 0.12f, 0.25f, 0.42f, 0.57f, 0.68f, 0.8f, 0.86f, 0.9f, 0.93f, 0.95f, 0.96f, 0.97f, 0.98f, 0.99f, 1.0f,
	0.15f, 0.35f, 0.6f, 0.9f, 1.2f, 1.25f, 1.3f, 1.2f, 1.1f, 0.95f, 0.8f, 0.6f, 0.45f, 0.3f, 0.2f, 0.1f, 0 },

	{ 0.0f, 0.05f, 0.12f, 0.25f, 0.42f, 0.57f, 0.68f, 0.8f, 0.95f, 1.1f, 1.15f, 1.18f, 1.15f, 1.1f, 1.05f, 1.03f, 1.0f,
	0.15f, 0.35f, 0.6f, 0.9f, 1.2f, 1.25f, 1.3f, 1.2f, 1.1f, 0.95f, 0.8f, 0.6f, 0.45f, 0.3f, 0.2f, 0.1f, 0 }
};

void CRubbish::Init() {
	for (int32_t i = 0; i < NUM_RUBBISH_SHEETS; i++) {
		aSheets[i].m_state = 0;
		if (i < NUM_RUBBISH_SHEETS - 1)
			aSheets[i].m_next = &aSheets[i + 1];
		else
			aSheets[i].m_next = &EndEmptyList;
		if (i > 0)
			aSheets[i].m_prev = &aSheets[i - 1];
		else
			aSheets[i].m_prev = &StartEmptyList;
	}
	StartEmptyList.m_next = &aSheets[0];
	StartEmptyList.m_prev = NULL;
	EndEmptyList.m_next = NULL;
	EndEmptyList.m_prev = &aSheets[NUM_RUBBISH_SHEETS - 1];

	StartStaticsList.m_next = &EndStaticsList;
	StartStaticsList.m_prev = NULL;
	EndStaticsList.m_next = NULL;
	EndStaticsList.m_prev = &StartStaticsList;

	StartMoversList.m_next = &EndMoversList;
	StartMoversList.m_prev = NULL;
	EndMoversList.m_next = NULL;
	EndMoversList.m_prev = &StartMoversList;

	RwIm3DVertexSetU(&RubbishVertices[0], 0.0f);
	RwIm3DVertexSetV(&RubbishVertices[0], 0.0f);
	RwIm3DVertexSetU(&RubbishVertices[1], 1.0f);
	RwIm3DVertexSetV(&RubbishVertices[1], 0.0f);
	RwIm3DVertexSetU(&RubbishVertices[2], 0.0f);
	RwIm3DVertexSetV(&RubbishVertices[2], 1.0f);
	RwIm3DVertexSetU(&RubbishVertices[3], 1.0f);
	RwIm3DVertexSetV(&RubbishVertices[3], 1.0f);

	RubbishIndexList2[0] = 0;
	RubbishIndexList2[1] = 2;
	RubbishIndexList2[2] = 1;
	RubbishIndexList2[3] = 1;
	RubbishIndexList2[4] = 2;
	RubbishIndexList2[5] = 3;

	RubbishIndexList[0] = 0;
	RubbishIndexList[1] = 1;
	RubbishIndexList[2] = 2;
	RubbishIndexList[3] = 1;
	RubbishIndexList[4] = 3;
	RubbishIndexList[5] = 2;

	CTxdStore::PushCurrentTxd();
	int32_t slot2 = CTxdStore::AddTxdSlot("rubbishSA");
	CTxdStore::LoadTxd(slot2, "MODELS\\RUBBISHSA.TXD");
	int32_t slot = CTxdStore::FindTxdSlot("rubbishSA");
	CTxdStore::SetCurrentTxd(slot);
	gpRubbishTexture[0] = RwTextureRead("gameleaf01_64", NULL);
	gpRubbishTexture[1] = RwTextureRead("gameleaf02_64", NULL);
	gpRubbishTexture[2] = RwTextureRead("gameleaf03_64", NULL);
	gpRubbishTexture[3] = RwTextureRead("gameleaf04_64", NULL);
	gpRubbishTexture[4] = RwTextureRead("gameleaf05_64", NULL);
	gpRubbishTexture[5] = RwTextureRead("gameleaf06_64", NULL);
	gpRubbishTexture[6] = RwTextureRead("newspaper01_64", NULL);
	gpRubbishTexture[7] = RwTextureRead("newspaper02_64", NULL);
	gpRubbishTexture[8] = RwTextureRead("newspaper03_64", NULL);
	gpRubbishTexture[9] = RwTextureRead("newspaper04_64", NULL);
	gpRubbishTexture[10] = RwTextureRead("newspaper05_64", NULL);
	gpRubbishTexture[11] = RwTextureRead("newspaper06_64", NULL);
	gpRubbishTexture[12] = RwTextureRead("newspaper07_64", NULL);
	gpRubbishTexture[13] = RwTextureRead("newspaper1", NULL);
	CTxdStore::PopCurrentTxd();
	RubbishVisibility = 255;
	bRubbishInvisible = false;
}

void CRubbish::Shutdown() {
	RwTextureDestroy(gpRubbishTexture[0]);
	gpRubbishTexture[0] = NULL;

	RwTextureDestroy(gpRubbishTexture[1]);
	gpRubbishTexture[1] = NULL;

	RwTextureDestroy(gpRubbishTexture[2]);
	gpRubbishTexture[2] = NULL;

	RwTextureDestroy(gpRubbishTexture[3]);
	gpRubbishTexture[3] = NULL;

	RwTextureDestroy(gpRubbishTexture[4]);
	gpRubbishTexture[4] = NULL;

	RwTextureDestroy(gpRubbishTexture[5]);
	gpRubbishTexture[5] = NULL;

	RwTextureDestroy(gpRubbishTexture[6]);
	gpRubbishTexture[6] = NULL;

	RwTextureDestroy(gpRubbishTexture[7]);
	gpRubbishTexture[7] = NULL;

	RwTextureDestroy(gpRubbishTexture[8]);
	gpRubbishTexture[8] = NULL;

	RwTextureDestroy(gpRubbishTexture[9]);
	gpRubbishTexture[9] = NULL;

	RwTextureDestroy(gpRubbishTexture[10]);
	gpRubbishTexture[10] = NULL;

	RwTextureDestroy(gpRubbishTexture[11]);
	gpRubbishTexture[11] = NULL;

	RwTextureDestroy(gpRubbishTexture[12]);
	gpRubbishTexture[12] = NULL;

	RwTextureDestroy(gpRubbishTexture[13]);
	gpRubbishTexture[13] = NULL;
}

void CRubbish::Render() {
	if (RubbishVisibility == 0 || CGame::currArea > 0)
		return;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);

	for (int32_t i = 0; i < 14; i++) {
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRubbishTexture[i]));

		TempBufferIndicesStored = 0;
		TempBufferVerticesStored = 0;

		COneSheet* sheet;
		for (sheet = &aSheets[i * NUM_RUBBISH_SHEETS / 14];
			sheet < &aSheets[(i + 1) * NUM_RUBBISH_SHEETS / 14];
			sheet++) {
			if (sheet->m_state == 0)
				continue;

			uint32_t alpha = 128;
			CVector pos;
			if (sheet->m_state == 1) {
				pos = sheet->m_basePos;
				if (!sheet->m_isVisible)
					alpha = 0;
			}
			else {
				pos = sheet->m_animatedPos;
				if (!sheet->m_isVisible || !sheet->m_targetIsVisible) {
					float t = (float)(CTimer::m_snTimeInMilliseconds - sheet->m_moveStart) / sheet->m_moveDuration;
					float f1 = sheet->m_isVisible ? 1.0f - t : 0.0f;
					float f2 = sheet->m_targetIsVisible ? t : 0.0f;
					alpha = 128 * (f1 + f2);
				}
			}

			float camDist = (pos - TheCamera.GetPosition()).Magnitude2D();
			if (camDist < RUBBISH_MAX_DIST) {
				if (camDist >= RUBBISH_FADE_DIST)
					alpha -= alpha * (camDist - RUBBISH_FADE_DIST) / (RUBBISH_MAX_DIST - RUBBISH_FADE_DIST);
				alpha = (RubbishVisibility * alpha) / 256;

				float vx1, vx2;
				float vy1, vy2;


				if (i == 0 || i == 1 || i == 3) {
					vx1 = 0.9f * sin(sheet->m_angle);
					vy1 = 0.9f * cos(sheet->m_angle);
					vx2 = 0.3f * cos(sheet->m_angle);
					vy2 = -0.3f * sin(sheet->m_angle);
				}
				else {
					vx1 = 0.3f * sin(sheet->m_angle);
					vy1 = 0.3f * cos(sheet->m_angle);
					vx2 = 0.3f * cos(sheet->m_angle);
					vy2 = -0.3f * sin(sheet->m_angle);
				}
				if (i == 5) {
					float widthScale = 0.95f; 
					float heightScale = 0.95; 

					vx1 = sin(sheet->m_angle) * widthScale;
					vy1 = cos(sheet->m_angle) * heightScale;
					vx2 = cos(sheet->m_angle) * widthScale;
					vy2 = -sin(sheet->m_angle) * heightScale;
				}
				if (i == 4) {
					float widthScale = 0.45f;  
					float heightScale = 0.45; 

					vx1 = sin(sheet->m_angle) * widthScale;
					vy1 = cos(sheet->m_angle) * heightScale;
					vx2 = cos(sheet->m_angle) * widthScale;
					vy2 = -sin(sheet->m_angle) * heightScale;
				}
				if (i == 10) {
					float widthScale = 0.45f;     
					float heightScale = 0.45; 

					vx1 = sin(sheet->m_angle) * widthScale;
					vy1 = cos(sheet->m_angle) * heightScale;
					vx2 = cos(sheet->m_angle) * widthScale;
					vy2 = -sin(sheet->m_angle) * heightScale;
				}
				if (i == 12 || i == 13) {
					float widthScale = 0.42f; 
					float heightScale = 0.42; 

					vx1 = sin(sheet->m_angle) * widthScale;
					vy1 = cos(sheet->m_angle) * heightScale;
					vx2 = cos(sheet->m_angle) * widthScale;
					vy2 = -sin(sheet->m_angle) * heightScale;
				}

				int v = TempBufferVerticesStored;
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v + 0], pos.x + vx1 + vx2, pos.y + vy1 + vy2, pos.z);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v + 1], pos.x + vx1 - vx2, pos.y + vy1 - vy2, pos.z);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v + 2], pos.x - vx1 + vx2, pos.y - vy1 + vy2, pos.z);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[v + 3], pos.x - vx1 - vx2, pos.y - vy1 - vy2, pos.z);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v + 0], 255, 255, 255, alpha);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v + 1], 255, 255, 255, alpha);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v + 2], 255, 255, 255, alpha);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[v + 3], 255, 255, 255, alpha);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v + 0], 0.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v + 0], 0.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v + 1], 1.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v + 1], 0.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v + 2], 0.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v + 2], 1.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[v + 3], 1.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[v + 3], 1.0f);

				int32_t v2 = TempBufferIndicesStored;
				TempBufferRenderIndexList[v2 + 0] = RubbishIndexList[0] + TempBufferVerticesStored;
				TempBufferRenderIndexList[v2 + 1] = RubbishIndexList[1] + TempBufferVerticesStored;
				TempBufferRenderIndexList[v2 + 2] = RubbishIndexList[2] + TempBufferVerticesStored;
				TempBufferRenderIndexList[v2 + 3] = RubbishIndexList[3] + TempBufferVerticesStored;
				TempBufferRenderIndexList[v2 + 4] = RubbishIndexList[4] + TempBufferVerticesStored;
				TempBufferRenderIndexList[v2 + 5] = RubbishIndexList[5] + TempBufferVerticesStored;
				TempBufferVerticesStored += 4;
				TempBufferIndicesStored += 6;
			}
		}

		if (TempBufferIndicesStored != 0) {
			if (RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, NULL, rwIM3D_VERTEXUV)) {
				RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
				RwIm3DEnd();
			}
		}
	}

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
}

void CRubbish::StirUp(CVehicle* veh) {
	if ((CTimer::m_FrameCounter ^ (veh->m_nRandomSeed & 3)) == 0)
		return;

	if (abs(veh->GetPosition().x - TheCamera.GetPosition().x) < 20.0f &&
		abs(veh->GetPosition().y - TheCamera.GetPosition().y) < 20.0f)
		if (abs(veh->m_vecMoveSpeed.x) > 0.05f || abs(veh->m_vecMoveSpeed.y) > 0.05f) {
			float speed = veh->m_vecMoveSpeed.Magnitude2D();
			if (speed > 0.05f) {
				bool movingForward = DotProduct2D(veh->m_vecMoveSpeed, veh->GetMatrix()->up) > 0.0f;
				COneSheet* sheet = StartStaticsList.m_next;
				CVector2D size = veh->GetColModel()->m_boundBox.m_vecMax;

				while (sheet != &EndStaticsList) {
					COneSheet* next = sheet->m_next;
					CVector2D carToSheet = sheet->m_basePos - veh->GetPosition();
					float distFwd = DotProduct2D(carToSheet, veh->GetMatrix()->up);

					if (movingForward && distFwd < -0.5f * size.y && distFwd > -1.5f * size.y ||
						!movingForward && distFwd > 0.5f * size.y && distFwd < 1.5f * size.y) {
						float distSide = abs(DotProduct2D(carToSheet, veh->GetMatrix()->right));
						if (distSide < 1.5 * size.x) {
							float speedToCheck = distSide < size.x ? speed : speed * 0.5f;
							if (speedToCheck > 0.05f) {
								sheet->m_state = 2;
								if (speedToCheck > 0.15f)
									sheet->m_animationType = 2;
								else
									sheet->m_animationType = 1;
								sheet->m_moveDuration = 2000;
								sheet->m_xDist = veh->m_vecMoveSpeed.x;
								sheet->m_yDist = veh->m_vecMoveSpeed.y;
								float dist = sqrt(SQR(sheet->m_xDist) + SQR(sheet->m_yDist));
								sheet->m_xDist *= 25.0f * speed / dist;
								sheet->m_yDist *= 25.0f * speed / dist;
								sheet->m_animHeight = 3.0f * speed;
								sheet->m_moveStart = CTimer::m_snTimeInMilliseconds;
								float tx = sheet->m_basePos.x + sheet->m_xDist;
								float ty = sheet->m_basePos.y + sheet->m_yDist;
								float tz = sheet->m_basePos.z + 3.0f;
								sheet->m_targetZ = CWorld::FindGroundZFor3DCoord(tx, ty, tz, NULL, NULL) + 0.1f;
								sheet->RemoveFromList();
								sheet->AddToList(&StartMoversList);
							}
						}
					}

					sheet = next;
				}
			}
		}
}

void CRubbish::Update() {
	bool foundGround = false;

	if (bRubbishInvisible)
		RubbishVisibility = max(RubbishVisibility - 5, 0);
	else
		RubbishVisibility = min(RubbishVisibility + 5, 255);

	COneSheet* sheet = StartEmptyList.m_next;
	if (sheet != &EndEmptyList) {
		float spawnDist;
		float spawnAngle;

		spawnDist = (GetRandomNumber() & 0xFF) / 256.0f + RUBBISH_MAX_DIST;
		uint8_t r = GetRandomNumber();
		if (r & 1)
			spawnAngle = (GetRandomNumber() & 0xFF) / 256.0f * 6.28f;
		else
			spawnAngle = (r - 128) / 160.0f + TheCamera.m_fOrientation;
		sheet->m_basePos.x = TheCamera.GetPosition().x + spawnDist * sin(spawnAngle);
		sheet->m_basePos.y = TheCamera.GetPosition().y + spawnDist * cos(spawnAngle);
		sheet->m_basePos.z = CWorld::FindGroundZFor3DCoord(sheet->m_basePos.x, sheet->m_basePos.y, TheCamera.GetPosition().z, &foundGround, NULL) + 0.1f;
		if (foundGround) {
			sheet->m_angle = (GetRandomNumber() & 0xFF) / 256.0f * 6.28f;
			sheet->m_state = 1;
			if (CCullZones::FindAttributesForCoors(sheet->m_basePos) & ATTRZONE_NORAIN)
				sheet->m_isVisible = false;
			else
				sheet->m_isVisible = true;
			sheet->RemoveFromList();
			sheet->AddToList(&StartStaticsList);
		}
	}

	bool hit = false;
	sheet = StartMoversList.m_next;
	while (sheet != &EndMoversList) {
		uint32_t currentTime = CTimer::m_snTimeInMilliseconds - sheet->m_moveStart;
		if (currentTime < sheet->m_moveDuration) {
			int32_t step = 16 * currentTime / sheet->m_moveDuration;
			int32_t stepTime = sheet->m_moveDuration / 16;
			float s = (float)(currentTime - stepTime * step) / stepTime;
			float t = (float)currentTime / sheet->m_moveDuration;
			float fxy = aAnimations[sheet->m_animationType][step] * (1.0f - s) + aAnimations[sheet->m_animationType][step + 1] * s;
			float fz = aAnimations[sheet->m_animationType][step + 17] * (1.0f - s) + aAnimations[sheet->m_animationType][step + 1 + 17] * s;
			sheet->m_animatedPos.x = sheet->m_basePos.x + fxy * sheet->m_xDist;
			sheet->m_animatedPos.y = sheet->m_basePos.y + fxy * sheet->m_yDist;
			sheet->m_animatedPos.z = (1.0f - t) * sheet->m_basePos.z + t * sheet->m_targetZ + fz * sheet->m_animHeight;
			sheet->m_angle += CTimer::ms_fTimeStep * 0.04f;
			if (sheet->m_angle > 6.28f)
				sheet->m_angle -= 6.28f;
			sheet = sheet->m_next;
		}
		else {
			sheet->m_basePos.x += sheet->m_xDist;
			sheet->m_basePos.y += sheet->m_yDist;
			sheet->m_basePos.z = sheet->m_targetZ;
			sheet->m_state = 1;
			sheet->m_isVisible = sheet->m_targetIsVisible;

			COneSheet* next = sheet->m_next;
			sheet->RemoveFromList();
			sheet->AddToList(&StartStaticsList);
			sheet = next;
		}
	}

	int32_t freq = 0;
	if (CWeather::Wind < 0.1f)
		freq = 31;
	else if (CWeather::Wind < 0.4f)
		freq = 7;
	else if (CWeather::Wind < 0.7f)
		freq = 1;

	if ((CTimer::m_FrameCounter & freq) == 0) {
		int32_t i = GetRandomNumber() % NUM_RUBBISH_SHEETS;
		if (aSheets[i].m_state == 1) {
			aSheets[i].m_moveStart = CTimer::m_snTimeInMilliseconds;
			aSheets[i].m_moveDuration = CWeather::Wind * 1500.0f + 1000.0f;
			aSheets[i].m_animHeight = 0.2f;
			aSheets[i].m_xDist = 3.0f * CWeather::Wind;
			aSheets[i].m_yDist = 3.0f * CWeather::Wind;

			float tx = aSheets[i].m_basePos.x + aSheets[i].m_xDist;
			float ty = aSheets[i].m_basePos.y + aSheets[i].m_yDist;
			float tz = aSheets[i].m_basePos.z + 3.0f;

			aSheets[i].m_targetZ = CWorld::FindGroundZFor3DCoord(tx, ty, tz, &foundGround, NULL) + 0.1f;
			if (CCullZones::FindAttributesForCoors(CVector(tx, ty, aSheets[i].m_targetZ)) & ATTRZONE_NORAIN)
				aSheets[i].m_targetIsVisible = false;
			else
				aSheets[i].m_targetIsVisible = true;
			if (foundGround) {
				aSheets[i].m_state = 2;
				aSheets[i].m_animationType = 1;
				aSheets[i].RemoveFromList();
				aSheets[i].AddToList(&StartMoversList);
			}
		}
	}

	for (int32_t i = 0; i < NUM_RUBBISH_SHEETS; i++) {
		for (int32_t j = 0; j < MAX_NUM_BULLETTRACES; j++) {
			COneSheet* sheet = &aSheets[i];
			CBulletTrace b = CBulletTraces::aTraces[j];

			if (b.m_bExists && aSheets[i].m_state == 1 && ((b.m_vecEnd) - sheet->m_basePos).Magnitude() < 0.8f) {
				sheet->m_state = 0;
				sheet->RemoveFromList();
				sheet->AddToList(&StartEmptyList);
				break;
			}
		}
	}

	for (int32_t i = (CTimer::m_FrameCounter % (NUM_RUBBISH_SHEETS / 4)) * 4; i < ((CTimer::m_FrameCounter % (NUM_RUBBISH_SHEETS / 4)) + 1) * 4; i++) {
		if (aSheets[i].m_state == 1 &&
			(aSheets[i].m_basePos - TheCamera.GetPosition()).MagnitudeSqr2D() > SQR(RUBBISH_MAX_DIST + 1.0f)) {
			aSheets[i].m_state = 0;
			aSheets[i].RemoveFromList();
			aSheets[i].AddToList(&StartEmptyList);
		}
	}
}

void CRubbish::SetVisibility(bool visible) {
	bRubbishInvisible = !visible;
}

void COneSheet::AddToList(COneSheet* list) {
	this->m_next = list->m_next;
	this->m_prev = list;
	list->m_next = this;
	this->m_next->m_prev = this;
}

void COneSheet::RemoveFromList() {
	m_next->m_prev = m_prev;
	m_prev->m_next = m_next;
}