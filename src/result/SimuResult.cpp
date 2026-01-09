// ------------------------------------------------
// SimuResult.hppの実装
// ------------------------------------------------

#include "SimuResult.hpp"

SimuResultLogger::SimuResultLogger(const RocketSpecification& rocketSpec,
                                   const MapData& map,
                                   double windSpeed,
                                   double windDirection) :
    m_rocketSpec(rocketSpec), m_map(map) {
    m_result.windSpeed     = windSpeed;
    m_result.windDirection = windDirection;
}

SimuResultSummary SimuResultLogger::getResult() const {
    return m_result;
}

SimuResultSummary SimuResultLogger::getResultScatterFormat() const {
    SimuResultSummary result = m_result;

    // remove steps that are not landing points
    for (auto& body : result.bodyResults) {
        const SimuResultStep lastBody = body.steps[body.steps.size() - 1];
        body.steps                    = std::vector<SimuResultStep>(1, lastBody);
    }

    // remove body result that not contain velid landing point
    for (int i = static_cast<int>(result.bodyResults.size() - 1); i >= 0; i--) {
        if (result.bodyResults[i].steps[0].rocket_pos.z > 0.0) {
            result.bodyResults.erase(result.bodyResults.begin() + i);
        }
    }

    return result;
}

void SimuResultLogger::pushBody() {
    m_result.bodyResults.emplace_back();
    m_result.bodyFinalPositions.emplace_back();
}

void SimuResultLogger::setLaunchClear(const Body& body) {
    m_result.launchClearTime     = body.elapsedTime;
    m_result.launchClearVelocity = body.velocity;
}

void SimuResultLogger::setBodyFinalPosition(size_t bodyIndex, const Vector3D& pos) {
    m_result.bodyFinalPositions[bodyIndex] = {.latitude  = m_map.coordinate.latitudeAt(pos.y),
                                              .longitude = m_map.coordinate.longitudeAt(pos.x)};
}

void SimuResultLogger::update(
    size_t bodyIndex, const Rocket& rocket, const Body& body, const WindModel& windModel, bool combusting) {
    const auto& spec = m_rocketSpec.bodySpec(bodyIndex);

    {
        SimuResultStep step = {0};

        // General
        step.gen_timeFromLaunch = rocket.timeFromLaunch;
        step.gen_elapsedTime    = body.elapsedTime;

        // Boolean
        step.launchClear     = rocket.launchClear;
        step.combusting      = combusting;
        step.parachuteOpened = body.parachuteOpenedList;

        // Air
        step.air_density     = windModel.density();
        step.air_gravity     = windModel.gravity();
        step.air_pressure    = windModel.pressure();
        step.air_temperature = windModel.temperature();
        step.air_wind        = windModel.wind();

        // Body
        step.rocket_mass        = body.mass;
        step.rocket_cgLength    = body.reflLength;
        step.rocket_iyz         = body.iyz;
        step.rocket_ix          = body.ix;
        step.rocket_attackAngle = body.attackAngle;
        step.rocket_pos         = body.pos;
        step.rocket_velocity    = body.velocity;
        step.rocket_airspeed_b  = body.airSpeed_b;
        step.rocket_force_b     = body.force_b;
        step.Cnp                = body.Cnp;
        step.Cny                = body.Cny;
        step.Cmqp               = body.Cmqp;
        step.Cmqy               = body.Cmqy;
        step.Cp                 = body.aeroCoef.Cp;
        step.Cd                 = body.aeroCoef.Cd;
        step.Cna                = body.aeroCoef.Cna;

        // Position
        step.latitude  = m_map.coordinate.latitudeAt(body.pos.y);
        step.longitude = m_map.coordinate.longitudeAt(body.pos.x);
        step.downrange = sqrt(body.pos.x * body.pos.x + body.pos.y * body.pos.y);

        // Calculated
        step.Fst = 100 * (step.Cp - step.rocket_cgLength) / spec.length;
        {
            const auto airspeed  = step.rocket_airspeed_b.length();
            step.dynamicPressure = 0.5 * step.air_density * airspeed * airspeed;
        }

        m_result.bodyResults[bodyIndex].steps.emplace_back(std::move(step));
    }

    // Update max
    {
        const bool rising = body.velocity.z > 0;
        if (m_result.maxAltitude < body.pos.z) {
            m_result.maxAltitude    = body.pos.z;
            m_result.detectPeakTime = body.elapsedTime;
        }
        if (const auto velocity = body.velocity.length(); m_result.maxVelocity < velocity) {
            m_result.maxVelocity = velocity;
        }
        if (const auto airspeed = body.airSpeed_b.length(); m_result.maxAirspeed < airspeed) {
            m_result.maxAirspeed = airspeed;
        }
        if (const double force = rising ? sqrt(body.force_b.z * body.force_b.z + body.force_b.y * body.force_b.y) : 0.0;
            m_result.maxNormalForceDuringRising < force) {
            m_result.maxNormalForceDuringRising = force;
        }
    }
}

void SimuResultLogger::organize() {
    for (auto& bodyResult : m_result.bodyResults) {
        for (auto& step : bodyResult.steps) {
            if (step.rocket_pos.z < 0) {
                step.rocket_pos.z = 0.0;
            }
        }
    }
}
