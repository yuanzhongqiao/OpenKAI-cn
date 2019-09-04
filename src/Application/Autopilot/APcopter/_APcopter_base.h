#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_base_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../_ActionBase.h"

#define AP_N_CUSTOM_MODE 24

namespace kai
{
// Auto Pilot Modes enumeration
enum custom_mode_t
{
	STABILIZE = 0,  // manual airframe angle with manual throttle
	ACRO = 1,  // manual body-frame angular rate with manual throttle
	ALT_HOLD = 2,  // manual airframe angle with automatic throttle
	AUTO = 3,  // fully automatic waypoint control using mission commands
	GUIDED = 4, // fully automatic fly to coordinate or fly at velocity/direction using GCS immediate commands
	LOITER = 5,  // automatic horizontal acceleration with automatic throttle
	RTL = 6,  // automatic return to launching point
	CIRCLE = 7,  // automatic circular flight with automatic throttle
	LAND = 9,  // automatic landing with horizontal position control
	DRIFT = 11,  // semi-automous position, yaw and throttle control
	SPORT = 13,  // manual earth-frame angular rate control with manual throttle
	FLIP = 14,  // automatically flip the vehicle on the roll axis
	AUTOTUNE = 15,  // automatically tune the vehicle's roll and pitch gains
	POSHOLD = 16, // automatic position hold with manual override, with automatic throttle
	BRAKE = 17,  // full-brake using inertial/GPS system, no pilot input
	THROW = 18, // throw to launch mode using inertial/GPS system, no pilot input
	AVOID_ADSB = 19, // automatic avoidance of obstacles in the macro scale - e.g. full-sized aircraft
	GUIDED_NOGPS = 20,  // guided mode but only accepts attitude and altitude
	SMART_RTL = 21,  // SMART_RTL returns to home by retracing its steps
	FLOWHOLD = 22, // FLOWHOLD holds position with optical flow without rangefinder
	FOLLOW = 23, // follow attempts to follow another vehicle or ground station
};

const string custom_mode_name[] =
{ "STABILIZE", "ACRO", "ALT_HOLD", "AUTO", "GUIDED", "LOITER", "RTL", "CIRCLE",
		"UNDEFINED", "LAND", "UNDEFINED", "DRIFT", "UNDEFINED", "SPORT", "FLIP",
		"AUTOTUNE", "POSHOLD", "BRAKE", "THROW", "AVOID_ADSB", "GUIDED_NOGPS",
		"SMART_RTL", "FLOWHOLD", "FOLLOW", };

struct AP_MOUNT
{
	bool	m_bEnable;
	mavlink_mount_control_t m_control;
	mavlink_mount_configure_t m_config;

	void init(void)
	{
		m_bEnable = false;
		m_control.input_a = 0;	//pitch
		m_control.input_b = 0;	//roll
		m_control.input_c = 0;	//yaw
		m_control.save_position = 0;
		m_config.stab_pitch = 0;
		m_config.stab_roll = 0;
		m_config.stab_yaw = 0;
		m_config.mount_mode = 2;
	}
};

class _APcopter_base: public _ActionBase
{
public:
	_APcopter_base();
	~_APcopter_base();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

	void setApMode(uint32_t iMode);
	uint32_t getApMode(void);
	string getApModeName(void);
	int getApModeByName(const string& name);
	bool bApModeChanged(void);

	void setApArm(bool bArm);
	bool getApArm(void);
	void setMount(AP_MOUNT& mount);
	bool getHomePos(vDouble3* pHome);
	vDouble3 getPos(void);
	double getHdg(void);
	double getApHdg(void);

private:
	void updateBase(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_base*) This)->update();
		return NULL;
	}

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	uint32_t m_apMode;
	uint32_t m_lastApMode;
	bool m_bApModeChanged;

	int m_freqRawSensors;
	int m_freqExtStat;
	int m_freqRC;
	int m_freqPos;
	int m_freqExtra1;
	int m_freqSendHeartbeat;

	bool m_bHomeSet;
	vDouble3 m_vHomePos;
	vDouble3 m_vPos;	//lat, lon, alt in meters
	double m_apHdg;		//heading in degree
};

}
#endif
