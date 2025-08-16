#ifndef BOAT_SAFETY_SYSTEM_H
#define BOAT_SAFETY_SYSTEM_H

#include <json/json.h>

/**
 * 船只安全系统基类
 */
class BoatSafetySystem {
public:
    virtual ~BoatSafetySystem() = default;
    
    virtual void updateBoatState(const Json::Value& boat_state) = 0;
    virtual void updateDockInfo(const Json::Value& dock_info) = 0;
    virtual void updateRouteInfo(const Json::Value& route_info) = 0;
    virtual void updateSystemConfig(const Json::Value& config) = 0;
    virtual void processControlCommand(const Json::Value& command) = 0;
    virtual Json::Value processDockLockRequest(const Json::Value& request) = 0;
    virtual Json::Value processDockUnlockRequest(const Json::Value& request) = 0;
    virtual Json::Value processRouteAssignment(const Json::Value& assignment) = 0;
    virtual Json::Value processRouteQuery(const Json::Value& query) = 0;
};

#endif // BOAT_SAFETY_SYSTEM_H
