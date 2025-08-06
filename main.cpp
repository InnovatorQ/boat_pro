// #include <iostream>
// #include <vector>
// #include <map>
// #include <set>
// #include <cmath>
// #include <algorithm>
// #include <chrono>
// #include <string>
// #include <memory>
// #include <json/json.h>

// // 常量定义
// const double PI = 3.14159265358979323846;
// const double EARTH_RADIUS = 6378137.0; // WGS84椭球半长轴(米)

// // 枚举定义
// enum class BoatStatus {
//     LEAVING_DOCK = 1,    // 出坞
//     NORMAL_SAILING = 2,  // 正常航行
//     ENTERING_DOCK = 3    // 入坞
// };

// enum class RouteDirection {
//     CLOCKWISE = 1,       // 顺时针
//     COUNTERCLOCKWISE = 2 // 逆时针
// };

// enum class AlertLevel {
//     NORMAL = 0,
//     WARNING = 1,
//     EMERGENCY = 2
// };

// // 基础数据结构
// struct Point {
//     double lat;
//     double lng;
    
//     Point(double latitude = 0.0, double longitude = 0.0) 
//         : lat(latitude), lng(longitude) {}
// };

// struct BoatState {
//     int sysid;
//     double timestamp;
//     double lat;
//     double lng;
//     double heading;    // 航向角，0°为正北
//     double speed;      // 速度 m/s
//     BoatStatus status;
//     RouteDirection route_direction;
    
//     Point getPosition() const {
//         return Point(lat, lng);
//     }
// };

// struct DockInfo {
//     int dock_id;
//     double lat;
//     double lng;
//     bool is_locked;
    
//     Point getPosition() const {
//         return Point(lat, lng);
//     }
// };

// struct RouteInfo {
//     int route_id;
//     RouteDirection direction;
//     std::vector<Point> points;
// };

// struct SystemConfig {
//     struct {
//         double length;  // 船长 米
//         double width;   // 船宽 米
//     } boat;
    
//     double emergency_threshold_s;  // 紧急判断时间阈值
//     double warning_threshold_s;    // 警告判断时间阈值
//     int max_boats;
//     double min_route_gap_m;       // 最小航线横向间距
// };

// struct CollisionAlert {
//     AlertLevel level;
//     int current_boat_id;
//     std::vector<int> front_collision_boat_ids;  // 前向碰撞船只ID（最近一个）
//     std::vector<int> head_on_collision_boat_ids; // 对向碰撞船只ID（所有）
//     Point collision_position;
//     double collision_time_s;
//     double current_boat_heading;
//     double other_boat_heading;
//     std::string avoidance_suggestion;
// };

// // 地理计算工具类
// class GeoUtils {
// public:
//     // 计算两点间距离 (米)
//     static double calculateDistance(const Point& p1, const Point& p2) {
//         double lat1_rad = p1.lat * PI / 180.0;
//         double lat2_rad = p2.lat * PI / 180.0;
//         double delta_lat = (p2.lat - p1.lat) * PI / 180.0;
//         double delta_lng = (p2.lng - p1.lng) * PI / 180.0;
        
//         double a = sin(delta_lat/2) * sin(delta_lat/2) +
//                    cos(lat1_rad) * cos(lat2_rad) *
//                    sin(delta_lng/2) * sin(delta_lng/2);
//         double c = 2 * atan2(sqrt(a), sqrt(1-a));
        
//         return EARTH_RADIUS * c;
//     }
    
//     // 计算方位角 (度)
//     static double calculateBearing(const Point& from, const Point& to) {
//         double lat1_rad = from.lat * PI / 180.0;
//         double lat2_rad = to.lat * PI / 180.0;
//         double delta_lng = (to.lng - from.lng) * PI / 180.0;
        
//         double y = sin(delta_lng) * cos(lat2_rad);
//         double x = cos(lat1_rad) * sin(lat2_rad) - 
//                    sin(lat1_rad) * cos(lat2_rad) * cos(delta_lng);
        
//         double bearing = atan2(y, x) * 180.0 / PI;
//         return fmod(bearing + 360.0, 360.0);
//     }
    
//     // 根据起点、方位角和距离计算终点
//     static Point calculateDestination(const Point& start, double bearing_deg, double distance_m) {
//         double lat1_rad = start.lat * PI / 180.0;
//         double lng1_rad = start.lng * PI / 180.0;
//         double bearing_rad = bearing_deg * PI / 180.0;
//         double angular_distance = distance_m / EARTH_RADIUS;
        
//         double lat2_rad = asin(sin(lat1_rad) * cos(angular_distance) +
//                               cos(lat1_rad) * sin(angular_distance) * cos(bearing_rad));
        
//         double lng2_rad = lng1_rad + atan2(sin(bearing_rad) * sin(angular_distance) * cos(lat1_rad),
//                                           cos(angular_distance) - sin(lat1_rad) * sin(lat2_rad));
        
//         return Point(lat2_rad * 180.0 / PI, lng2_rad * 180.0 / PI);
//     }
// };

// // 碰撞预测引擎
// class CollisionPredictor {
// private:
//     SystemConfig config_;
//     std::map<int, BoatState> boat_states_;
//     std::vector<DockInfo> docks_;
//     std::vector<RouteInfo> routes_;
    
// public:
//     CollisionPredictor(const SystemConfig& config) : config_(config) {}
    
//     void updateBoatState(const BoatState& state) {
//         boat_states_[state.sysid] = state;
//     }
    
//     void updateDockInfo(const std::vector<DockInfo>& docks) {
//         docks_ = docks;
//     }
    
//     void updateRouteInfo(const std::vector<RouteInfo>& routes) {
//         routes_ = routes;
//     }
    
//     // 预测两船碰撞时间和位置
//     std::pair<double, Point> predictCollisionTimeAndPosition(const BoatState& boat1, const BoatState& boat2) {
//         // 简化模型：假设船只直线航行
//         Point p1 = boat1.getPosition();
//         Point p2 = boat2.getPosition();
        
//         // 计算船只的速度向量 (在笛卡尔坐标系中近似)
//         double v1x = boat1.speed * sin(boat1.heading * PI / 180.0);
//         double v1y = boat1.speed * cos(boat1.heading * PI / 180.0);
//         double v2x = boat2.speed * sin(boat2.heading * PI / 180.0);
//         double v2y = boat2.speed * cos(boat2.heading * PI / 180.0);
        
//         // 相对速度
//         double rel_vx = v1x - v2x;
//         double rel_vy = v1y - v2y;
        
//         // 相对位置
//         double rel_x = (p1.lng - p2.lng) * cos(p1.lat * PI / 180.0) * EARTH_RADIUS * PI / 180.0;
//         double rel_y = (p1.lat - p2.lat) * EARTH_RADIUS * PI / 180.0;
        
//         // 计算最近距离时间
//         if (abs(rel_vx) < 1e-6 && abs(rel_vy) < 1e-6) {
//             // 相对速度为0，不会碰撞
//             return {-1, Point()};
//         }
        
//         double t_min = -(rel_x * rel_vx + rel_y * rel_vy) / (rel_vx * rel_vx + rel_vy * rel_vy);
        
//         if (t_min < 0) {
//             // 船只正在远离
//             return {-1, Point()};
//         }
        
//         // 预测碰撞位置
//         double collision_x = rel_x + rel_vx * t_min;
//         double collision_y = rel_y + rel_vy * t_min;
//         double min_distance = sqrt(collision_x * collision_x + collision_y * collision_y);
        
//         // 考虑船只尺寸
//         double safe_distance = sqrt(config_.boat.length * config_.boat.length + 
//                                   config_.boat.width * config_.boat.width);
        
//         if (min_distance > safe_distance) {
//             return {-1, Point()};
//         }
        
//         // 计算碰撞位置的地理坐标
//         Point collision_pos = GeoUtils::calculateDestination(p1, boat1.heading, boat1.speed * t_min);
        
//         return {t_min, collision_pos};
//     }
    
//     // 检查是否为对向航行
//     bool isHeadOnCollision(const BoatState& boat1, const BoatState& boat2) {
//         double heading_diff = abs(boat1.heading - boat2.heading);
//         return (heading_diff > 150 && heading_diff < 210); // 允许30度误差
//     }
    
//     // 检查优先级
//     int getBoatPriority(const BoatState& boat) {
//         switch (boat.status) {
//             case BoatStatus::ENTERING_DOCK: return 3;  // 最高优先级
//             case BoatStatus::NORMAL_SAILING: return 2;
//             case BoatStatus::LEAVING_DOCK: return 1;   // 最低优先级
//         }
//         return 0;
//     }
    
//     // 生成避碰建议
//     std::string generateAvoidanceSuggestion(const BoatState& current_boat, 
//                                           const std::vector<BoatState>& collision_boats,
//                                           AlertLevel level) {
//         std::string suggestion;
        
//         switch (level) {
//             case AlertLevel::EMERGENCY:
//                 if (getBoatPriority(current_boat) < getBoatPriority(collision_boats[0])) {
//                     suggestion = "立即停船等待高优先级船只通过";
//                 } else {
//                     suggestion = "立即减速并保持航向";
//                 }
//                 break;
                
//             case AlertLevel::WARNING:
//                 suggestion = "减速并准备避让动作";
//                 break;
                
//             case AlertLevel::NORMAL:
//                 suggestion = "恢复正常航行";
//                 break;
//         }
        
//         return suggestion;
//     }
    
//     // 主要碰撞检测函数
//     std::vector<CollisionAlert> detectCollisions() {
//         std::vector<CollisionAlert> alerts;
        
//         for (auto& [boat_id, boat_state] : boat_states_) {
//             CollisionAlert alert;
//             alert.current_boat_id = boat_id;
//             alert.level = AlertLevel::NORMAL;
//             alert.current_boat_heading = boat_state.heading;
            
//             std::vector<std::pair<double, int>> front_collisions;
//             std::vector<int> head_on_collisions;
//             double min_collision_time = std::numeric_limits<double>::max();
//             Point collision_pos;
            
//             // 检查与其他船只的碰撞
//             for (auto& [other_id, other_state] : boat_states_) {
//                 if (boat_id == other_id) continue;
                
//                 auto [collision_time, collision_position] = 
//                     predictCollisionTimeAndPosition(boat_state, other_state);
                
//                 if (collision_time > 0) {
//                     bool is_head_on = isHeadOnCollision(boat_state, other_state);
                    
//                     // 根据优先级和碰撞类型处理
//                     if (is_head_on) {
//                         head_on_collisions.push_back(other_id);
//                     } else {
//                         // 检查是否为前向碰撞（同向或交叉）
//                         front_collisions.push_back({collision_time, other_id});
//                     }
                    
//                     if (collision_time < min_collision_time) {
//                         min_collision_time = collision_time;
//                         collision_pos = collision_position;
//                         alert.other_boat_heading = other_state.heading;
//                     }
                    
//                     // 确定告警级别
//                     if (collision_time <= config_.emergency_threshold_s) {
//                         alert.level = AlertLevel::EMERGENCY;
//                     } else if (collision_time <= config_.warning_threshold_s && 
//                              alert.level != AlertLevel::EMERGENCY) {
//                         alert.level = AlertLevel::WARNING;
//                     }
//                 }
//             }
            
//             // 整理碰撞船只ID
//             if (!front_collisions.empty()) {
//                 // 对前向碰撞按时间排序，只报告最近的
//                 std::sort(front_collisions.begin(), front_collisions.end());
//                 alert.front_collision_boat_ids.push_back(front_collisions[0].second);
//             }
            
//             alert.head_on_collision_boat_ids = head_on_collisions;
//             alert.collision_position = collision_pos;
//             alert.collision_time_s = min_collision_time;
            
//             // 生成避碰建议
//             std::vector<BoatState> collision_boats;
//             for (int id : alert.front_collision_boat_ids) {
//                 collision_boats.push_back(boat_states_[id]);
//             }
//             for (int id : alert.head_on_collision_boat_ids) {
//                 collision_boats.push_back(boat_states_[id]);
//             }
            
//             alert.avoidance_suggestion = generateAvoidanceSuggestion(boat_state, collision_boats, alert.level);
            
//             // 只添加非正常状态的告警
//             if (alert.level != AlertLevel::NORMAL || 
//                 !alert.front_collision_boat_ids.empty() || 
//                 !alert.head_on_collision_boat_ids.empty()) {
//                 alerts.push_back(alert);
//             }
//         }
        
//         return alerts;
//     }
// };

// // JSON工具类
// class JsonUtils {
// public:
//     static BoatState parseBoatState(const Json::Value& json) {
//         BoatState state;
//         state.sysid = json["sysid"].asInt();
//         state.timestamp = json["timestamp"].asDouble();
//         state.lat = json["lat"].asDouble();
//         state.lng = json["lng"].asDouble();
//         state.heading = json["heading"].asDouble();
//         state.speed = json["speed"].asDouble();
//         state.status = static_cast<BoatStatus>(json["status"].asInt());
//         state.route_direction = static_cast<RouteDirection>(json["route_direction"].asInt());
//         return state;
//     }
    
//     static DockInfo parseDockInfo(const Json::Value& json) {
//         DockInfo dock;
//         dock.dock_id = json["dock_id"].asInt();
//         dock.lat = json["lat"].asDouble();
//         dock.lng = json["lng"].asDouble();
//         dock.is_locked = false; // 默认未锁定
//         return dock;
//     }
    
//     static SystemConfig parseSystemConfig(const Json::Value& json) {
//         SystemConfig config;
//         config.boat.length = json["boat"]["length"].asDouble();
//         config.boat.width = json["boat"]["width"].asDouble();
//         config.emergency_threshold_s = json["emergency_threshold_s"].asDouble();
//         config.warning_threshold_s = json["warning_threshold_s"].asDouble();
//         config.max_boats = json["max_boats"].asInt();
//         config.min_route_gap_m = json["min_route_gap_m"].asDouble();
//         return config;
//     }
    
//     static Json::Value collisionAlertToJson(const CollisionAlert& alert) {
//         Json::Value json;
//         json["alert_level"] = static_cast<int>(alert.level);
//         json["current_boat_id"] = alert.current_boat_id;
        
//         Json::Value front_ids(Json::arrayValue);
//         for (int id : alert.front_collision_boat_ids) {
//             front_ids.append(id);
//         }
//         json["front_collision_boat_ids"] = front_ids;
        
//         Json::Value head_on_ids(Json::arrayValue);
//         for (int id : alert.head_on_collision_boat_ids) {
//             head_on_ids.append(id);
//         }
//         json["head_on_collision_boat_ids"] = head_on_ids;
        
//         json["collision_position"]["lat"] = alert.collision_position.lat;
//         json["collision_position"]["lng"] = alert.collision_position.lng;
//         json["collision_time_s"] = alert.collision_time_s;
//         json["current_boat_heading"] = alert.current_boat_heading;
//         json["other_boat_heading"] = alert.other_boat_heading;
//         json["avoidance_suggestion"] = alert.avoidance_suggestion;
        
//         return json;
//     }
// };

// // 主系统类
// class BoatSafetySystem {
// private:
//     SystemConfig config_;
//     std::unique_ptr<CollisionPredictor> predictor_;
    
// public:
//     BoatSafetySystem(const SystemConfig& config) 
//         : config_(config), predictor_(std::make_unique<CollisionPredictor>(config)) {}
    
//     void updateBoatState(const std::string& json_str) {
//         Json::Value json;
//         Json::Reader reader;
//         if (reader.parse(json_str, json)) {
//             BoatState state = JsonUtils::parseBoatState(json);
//             predictor_->updateBoatState(state);
//         }
//     }
    
//     void updateSystemConfig(const std::string& json_str) {
//         Json::Value json;
//         Json::Reader reader;
//         if (reader.parse(json_str, json)) {
//             config_ = JsonUtils::parseSystemConfig(json);
//             predictor_ = std::make_unique<CollisionPredictor>(config_);
//         }
//     }
    
//     std::string detectCollisionsJson() {
//         auto alerts = predictor_->detectCollisions();
        
//         Json::Value result(Json::arrayValue);
//         for (const auto& alert : alerts) {
//             result.append(JsonUtils::collisionAlertToJson(alert));
//         }
        
//         Json::StreamWriterBuilder builder;
//         return Json::writeString(builder, result);
//     }
    
//     void printSystemStatus() {
//         std::cout << "=== 无人船作业安全预测系统 ===" << std::endl;
//         std::cout << "船只尺寸: " << config_.boat.length << "m x " << config_.boat.width << "m" << std::endl;
//         std::cout << "紧急阈值: " << config_.emergency_threshold_s << "s" << std::endl;
//         std::cout << "警告阈值: " << config_.warning_threshold_s << "s" << std::endl;
//         std::cout << "最大船只数: " << config_.max_boats << std::endl;
//         std::cout << "================================" << std::endl;
//     }
// };

// // 示例使用
// int main() {
//     // 系统配置
//     SystemConfig config;
//     config.boat.length = 0.75;
//     config.boat.width = 0.47;
//     config.emergency_threshold_s = 5.0;
//     config.warning_threshold_s = 30.0;
//     config.max_boats = 30;
//     config.min_route_gap_m = 10.0;
    
//     // 创建系统实例
//     BoatSafetySystem system(config);
//     system.printSystemStatus();
    
//     // 模拟船只状态更新
//     std::string boat1_json = R"({
//         "sysid": 1,
//         "timestamp": 1722325256.530,
//         "lat": 30.549832,
//         "lng": 114.342922,
//         "heading": 90.0,
//         "speed": 2.5,
//         "status": 2,
//         "route_direction": 1
//     })";
    
//     std::string boat2_json = R"({
//         "sysid": 2,
//         "timestamp": 1722325256.530,
//         "lat": 30.549832,
//         "lng": 114.343000,
//         "heading": 270.0,
//         "speed": 3.0,
//         "status": 2,
//         "route_direction": 2
//     })";
    
//     system.updateBoatState(boat1_json);
//     system.updateBoatState(boat2_json);
    
//     // 执行碰撞检测
//     std::string collision_results = system.detectCollisionsJson();
//     std::cout << "碰撞检测结果: " << std::endl;
//     std::cout << collision_results << std::endl;
    
//     return 0;
// }