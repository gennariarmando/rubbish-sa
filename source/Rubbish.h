#pragma once
#include "CVehicle.h"

enum {
	NUM_RUBBISH_SHEETS = 64
};

class COneSheet {
public:
	CVector m_basePos;
	CVector m_animatedPos;
	float m_targetZ;
	int8_t m_state;
	int8_t m_animationType;
	uint32_t m_moveStart;
	uint32_t m_moveDuration;
	float m_animHeight;
	float m_xDist;
	float m_yDist;
	float m_angle;
	bool m_isVisible;
	bool m_targetIsVisible;
	COneSheet* m_next;
	COneSheet* m_prev;

public:
	void AddToList(COneSheet* list);
	void RemoveFromList();
};

class CRubbish {
public:
	static bool bRubbishInvisible;
	static int RubbishVisibility;
	static COneSheet aSheets[NUM_RUBBISH_SHEETS];
	static COneSheet StartEmptyList;
	static COneSheet EndEmptyList;
	static COneSheet StartStaticsList;
	static COneSheet EndStaticsList;
	static COneSheet StartMoversList;
	static COneSheet EndMoversList;

public:
	static void Render();
	static void StirUp(CVehicle* veh);
	static void Update();
	static void SetVisibility(bool visible);
	static void Init();
	static void Shutdown();
};
