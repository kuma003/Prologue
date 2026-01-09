// ------------------------------------------------
// シミュレーション結果に関するデータクラス
// ------------------------------------------------

#pragma once

#include <vector>

#include "dynamics/WindModel.hpp"
#include "env/Map.hpp"
#include "math/Vector3D.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

// ステップ毎の結果
struct SimuResultStep {
    // General
    double gen_timeFromLaunch = 0;
    double gen_elapsedTime    = 0;

    // Boolean
    bool launchClear     = false;
    bool combusting      = false;
    std::vector<bool> parachuteOpened = std::vector<bool>();

    // Air
    double air_density     = 0;
    double air_gravity     = 0;
    double air_pressure    = 0;
    double air_temperature = 0;
    Vector3D air_wind      = Vector3D();

    // Body
    double rocket_mass         = 0;
    double rocket_cgLength     = 0;
    double rocket_iyz          = 0;
    double rocket_ix           = 0;
    double rocket_attackAngle  = 0;
    Vector3D rocket_pos        = Vector3D();
    Vector3D rocket_velocity   = Vector3D();
    Vector3D rocket_airspeed_b = Vector3D();
    Vector3D rocket_force_b    = Vector3D();
    double Cnp = 0, Cny = 0;
    double Cmqp = 0, Cmqy = 0;
    double Cp  = 0;
    double Cd  = 0;
    double Cna = 0;

    // Position
    double latitude  = 0;
    double longitude = 0;
    double downrange = 0;

    // Calculated
    double Fst             = 0;
    double dynamicPressure = 0;
};

// 各body(rocket1, rocket2, rocket3, ...)でのSimuResultStepを格納
struct SimuResultBody {
    std::vector<SimuResultStep> steps;
};

// bodyの最終落下地点
struct BodyFinalPosition {
    double latitude  = 0;
    double longitude = 0;
};

// シミュレーション結果の主要な値を格納
struct SimuResultSummary {
    std::vector<SimuResultBody> bodyResults;
    std::vector<BodyFinalPosition> bodyFinalPositions;

    // condition
    double windSpeed     = 0;
    double windDirection = 0;

    // launch clear
    double launchClearTime       = 0;
    Vector3D launchClearVelocity = Vector3D();

    // max
    double maxAltitude = 0, detectPeakTime = 0;
    double maxVelocity                = 0;
    double maxAirspeed                = 0;
    double maxNormalForceDuringRising = 0;
};

// ステップごとに結果を格納、出力を行うクラス
class SimuResultLogger {
private:
    const RocketSpecification m_rocketSpec;
    const MapData m_map;
    SimuResultSummary m_result;

public:
    explicit SimuResultLogger(const RocketSpecification& rocketSpec,
                              const MapData& map,
                              double windSpeed,
                              double windDirection);

    SimuResultSummary getResult() const;

    SimuResultSummary getResultScatterFormat() const;

    void pushBody();

    void setLaunchClear(const Body& body);

    void setBodyFinalPosition(size_t bodyIndex, const Vector3D& pos);

    void update(size_t bodyIndex, const Rocket& rocket, const Body& body, const WindModel& windModel, bool combusting);

    void organize();
};
