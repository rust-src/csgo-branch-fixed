#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
/*
class CInfoPortalScore : public CBaseEntity {
public:
	DECLARE_SERVERCLASS();
	DECLARE_CLASS(CInfoPortalScore, CBaseEntity);
	DECLARE_DATADESC();
	virtual void Precache();
	virtual void Spawn();
	void ScoreThink();
	void CountdownThink();
	void InputStartGame(inputdata_t &inputdata);
	void InputResetScore(inputdata_t &inputdata);
	void InputAddBlueScore(inputdata_t &inputdata);
	void InputAddOrangeScore(inputdata_t &inputdata);
	void ResetScore();
	bool IsGameOver();
private:
	CNetworkVar(int, m_nBlueScore);
	CNetworkVar(int, m_nBlueRounds);
	CNetworkVar(int, m_nOrangeScore);
	CNetworkVar(int, m_nOrangeRounds);
	CNetworkVar(float, m_flTotalTime);
	CNetworkVar(string_t,  m_strGameDescription);
	CNetworkVar(int, m_nRoundsToWin);
	float m_flStartTime;
	float m_flNextTimerSoundTime;
	bool m_bShowCountdown;
	bool m_bInSuddenDeath;
	COutputEvent m_OnBlueWin;
	COutputEvent m_OnOrangeWin;
	COutputEvent m_OnTie;
	COutputEvent m_OnGameStart;
	COutputEvent m_OnGameEnd;
	COutputEvent m_OnRoundEnd;
	COutputEvent m_OnSuddenDeath;
};

IMPLEMENT_SERVERCLASS_ST(CInfoPortalScore, DT_InfoPortalScore)
SendPropInt(SENDINFO(m_nBlueScore)),
SendPropInt(SENDINFO(m_nBlueRounds)),
SendPropInt(SENDINFO(m_nOrangeScore)),
SendPropInt(SENDINFO(m_nOrangeRounds)),
SendPropInt(SENDINFO(m_nRoundsToWin)),
SendPropFloat(SENDINFO(m_flTotalTime)),
SendPropStringT(SENDINFO(m_strGameDescription)),
END_SEND_TABLE() 

BEGIN_DATADESC(CInfoPortalScore)
DEFINE_THINKFUNC(ScoreThink),
DEFINE_THINKFUNC(CountdownThink),
DEFINE_FIELD(m_flStartTime, FIELD_TIME),
DEFINE_KEYFIELD(m_flTotalTime, FIELD_FLOAT, "GameTime"),
DEFINE_KEYFIELD(m_bShowCountdown, FIELD_BOOLEAN, "DisplayCountdown"),
DEFINE_KEYFIELD(m_strGameDescription, FIELD_STRING, "GameDescription"),
DEFINE_KEYFIELD(m_nRoundsToWin, FIELD_INTEGER, "RoundsToWin"),
DEFINE_INPUTFUNC(FIELD_VOID, "StartGame", InputStartGame),
DEFINE_INPUTFUNC(FIELD_VOID, "ResetScore", InputResetScore),
DEFINE_INPUTFUNC(FIELD_INTEGER, "AddBlueScore", InputAddBlueScore),
DEFINE_INPUTFUNC(FIELD_INTEGER, "AddOrangeScore", InputAddOrangeScore),
DEFINE_OUTPUT(m_OnBlueWin,"OnBlueWin"),
DEFINE_OUTPUT(m_OnOrangeWin, "OnOrangeWin"),
DEFINE_OUTPUT(m_OnTie, "OnTie"),
DEFINE_OUTPUT(m_OnGameStart, "OnGameStart"),
DEFINE_OUTPUT(m_OnGameEnd, "OnGameEnd"),
DEFINE_OUTPUT(m_OnRoundEnd, "OnRoundEnd"),
DEFINE_OUTPUT(m_OnSuddenDeath, "OnSuddenDeath"),
END_DATADESC()

LINK_ENTITY_TO_CLASS(info_portal_score, CInfoPortalScore);

void CInfoPortalScore::Precache(){
	PrecacheScriptSound("World.ApertureFacilityAlarm");
	PrecacheScriptSound("World.CoreElevatorArrival");
	PrecacheScriptSound("Portal.room1_TickTock");
	PrecacheScriptSound("glados.COOP_TEAMWORK_EXERCISE_ALT13");//Glados: Game over. Back to testing.
}

void CInfoPortalScore::Spawn(){
	Precache();
	ResetScore();
}

//Process scoring. I know this is a mess, but it was way worse before. I think there is an inlined function for blue and orange scoring, but I cannot find it yet.
void CInfoPortalScore::ScoreThink()
{
	float flRemainingGameTime = m_flTotalTime - (gpGlobals->curtime - m_flStartTime);
	if (flRemainingGameTime <= 11.0 && m_flNextTimerSoundTime == 0.0)
		m_flNextTimerSoundTime = gpGlobals->curtime + 1.0;
	if (gpGlobals->curtime >= m_flNextTimerSoundTime)
	{
		m_flNextTimerSoundTime = gpGlobals->curtime + 1.0;
		CReliableBroadcastRecipientFilter player;
		if (flRemainingGameTime > 3.0)
		{
			if (flRemainingGameTime <= 10.0)
				EmitSound(player, -1, "Portal.room1_TickTock");
		}
		else
		{
			EmitSound(player, -1, "World.ApertureFacilityAlarm");
		}
	}
	if (!((gpGlobals->curtime - m_flStartTime) <= m_flTotalTime))
	{
		byte v10;
		if (m_nOrangeScore < m_nBlueScore)
		{
			m_nBlueRounds++;
			v10 = 0;
			if (m_nBlueRounds == m_nRoundsToWin)
			{
				m_OnBlueWin.FireOutput(this, this);
				m_OnGameEnd.FireOutput(this, this);
				CReliableBroadcastRecipientFilter v16;
				EmitSound(v16, -1, "glados.COOP_TEAMWORK_EXERCISE_ALT13");
				m_bInSuddenDeath = 0;
				CReliableBroadcastRecipientFilter player;
				player.AddAllPlayers();
				UserMessageBegin(player, "MPVSGameOver");
				WRITE_BYTE(v10);
				MessageEnd();
				goto LABEL_34;
			}
			m_bInSuddenDeath = 0;
			CReliableBroadcastRecipientFilter player;
			player.AddAllPlayers();
			UserMessageBegin(player, "MPVSRoundEnd");
			MessageWriteByte(v10);
			MessageEnd();
			m_OnRoundEnd.FireOutput(this, this);
			goto LABEL_34;
		}
		if (m_nOrangeScore > m_nBlueScore)
		{
			m_nOrangeRounds++;
			v10 = 1;
			if (m_nOrangeRounds == m_nRoundsToWin)
			{
				m_OnOrangeWin.FireOutput(this, this);
				m_OnGameEnd.FireOutput(this, this);
				CReliableBroadcastRecipientFilter v16;
				EmitSound(v16, -1, "glados.COOP_TEAMWORK_EXERCISE_ALT13");
				m_bInSuddenDeath = 0;
				CReliableBroadcastRecipientFilter player;
				player.AddAllPlayers();
				UserMessageBegin(player, "MPVSGameOver");
				WRITE_BYTE(v10);
				MessageEnd();
				goto LABEL_34;
			}
			m_bInSuddenDeath = 0;
			CReliableBroadcastRecipientFilter player;
			player.AddAllPlayers();
			UserMessageBegin(player, "MPVSRoundEnd");
			MessageWriteByte(v10);
			MessageEnd();
			m_OnRoundEnd.FireOutput(this, this);
			goto LABEL_34;
		}
		if (!m_bInSuddenDeath)
		{
			m_bInSuddenDeath = 1;
			m_OnSuddenDeath.FireOutput(this, this);
			goto LABEL_34;
		}
	LABEL_34:
		if (!m_bInSuddenDeath)
		{
			SetThink(NULL);
			return;
		}
	}
	SetNextThink(gpGlobals->curtime + 0.1);
}

//4 second countdown until game starts
void CInfoPortalScore::CountdownThink()
{

	float flTimePassed = gpGlobals->curtime - m_flStartTime;
	CRecipientFilter filter;
	filter.AddAllPlayers();

	if ((flTimePassed <= 1.0) || (4.0 < flTimePassed)) {
		if (flTimePassed <= 4.0) goto END;
		EmitSound(filter, -1, "World.CoreElevatorArrival");
	}
	else {
		EmitSound(filter, -1, "World.ApertureFacilityAlarm");
	}
	
	if (4.0 < flTimePassed) {
		m_flStartTime = gpGlobals->curtime;
		SetThink(&CInfoPortalScore::ScoreThink);
		SetNextThink(gpGlobals->curtime);
		m_OnGameStart.FireOutput(this, this);
		return;
	}
	END:
	SetNextThink(gpGlobals->curtime + 1.0);
}

void CInfoPortalScore::InputStartGame(inputdata_t &inputdata)
{
	m_flStartTime = gpGlobals->curtime;
	m_bInSuddenDeath = false;
	if (!m_bShowCountdown) {
		m_OnGameStart.FireOutput(this, this);
		SetThink(&CInfoPortalScore::ScoreThink);
	}
	else {
		SetThink(&CInfoPortalScore::CountdownThink);
	}
	SetNextThink(gpGlobals->curtime);
	CRecipientFilter filter;
	filter.AddAllPlayers();
	UserMessageBegin(filter, "MPVSGameStart");
	WRITE_BYTE(m_bShowCountdown);
	MessageEnd();
}

void CInfoPortalScore::InputResetScore(inputdata_t &inputdata){
	ResetScore();
}
void CInfoPortalScore::InputAddBlueScore(inputdata_t &inputdata){
	m_nBlueScore = m_nBlueScore + inputdata.value.Int();
}
void CInfoPortalScore::InputAddOrangeScore(inputdata_t &inputdata){
	m_nOrangeScore = m_nOrangeScore + inputdata.value.Int();
}
void CInfoPortalScore::ResetScore(){
	m_nBlueScore = 0;
	m_nOrangeScore = 0;
}
bool CInfoPortalScore::IsGameOver(){
	return false;
}
*/