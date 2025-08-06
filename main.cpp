#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

// 常量定义
const double PI = 3.14159265358979323846;
const double EARTH_RADIUS = 6378137.0; // WGS84椭球半长轴(米)

// 枚举定义
enum class BoatStatus {
    LEAVING_DOCK = 1,    // 出坞
    NORMAL_SAILING = 2,  // 正常航行
    ENTERING_DOCK = 3    // 入坞
};

enum class RouteDirection {
    CLOCKWISE = 1,       // 顺时针
    COUNTERCLOCKWISE = 2 // 逆时针
};

enum class AlertLevel {
    NORMAL = 0,
    WARNING = 1,
    EMERGENCY = 2
};

// 简单JSON解析器
class SimpleJson {
private:
    std::map<std::string, std::string> data;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    std::string removeQuotes(const std::string& str) {
        std::string result = trim(str);
        if (result.length() >= 2 && result[0] == '"' && result.back() == '"') {
            return result.substr(1, result.length() - 2);
        }
        return result;
    }

public:
    bool parse(const std::string& json) {
        data.clear();
        std::string content = json;

        // 去除外层大括号
        size_t start = content.find('{');
        size_t end = content.rfind('}');
        if (start == std::string::npos || end == std::string::npos) {
            return false;
        }
        content = content.substr(start + 1, end - start - 1);

        // 简单解析键值对
        std::stringstream ss(content);
        std::string line;
        while (std::getline(ss, line, ',')) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = removeQuotes(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));
                data[key] = value;
            }
        }
        return true;
    }

    int getInt(const std::string& key) {
        auto it = data.find(key);
        if (it != data.end()) {
            return std::stoi(it->second);
        }
        return 0;
    }

    double getDouble(const std::string& key) {
        auto it = data.find(key);
        if (it != data.end()) {
            return std::stod(it->second);
        }
        return 0.0;
    }

    std::string getString(const std::string& key) {
        auto it = data.find(key);
        if (it != data.end()) {
            return removeQuotes(it->second);
        }
        return "";
    }
};

// 基础数据结构
struct Point {
    double lat;
    double lng;

    Point(double latitude = 0.0, double longitude = 0.0)
        : lat(latitude), lng(longitude) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6)
            << "{\"lat\":" << lat << ",\"lng\":" << lng << "}";
        return oss.str();
    }
};

struct BoatState {
    int sysid;
    double timestamp;
    double lat;
    double lng;
    double heading;    // 航向角，0°为正北
    double speed;      // 速度 m/s
    BoatStatus status;
    RouteDirection route_direction;

    Point getPosition() const {
        return Point(lat, lng);
    }

    bool parseFromJson(const std::string& json) {
        SimpleJson parser;
        if (!parser.parse(json)) return false;

        sysid = parser.getInt("sysid");
        timestamp = parser.getDouble("timestamp");
        lat = parser.getDouble("lat");
        lng = parser.getDouble("lng");
        heading = parser.getDouble("heading");
        speed = parser.getDouble("speed");
        status = static_cast<BoatStatus>(parser.getInt("status"));
        route_direction = static_cast<RouteDirection>(parser.getInt("route_direction"));

        return true;
    }
};

struct DockInfo {
    int dock_id;
    double lat;
    double lng;
    bool is_locked;

    Point getPosition() const {
        return Point(lat, lng);
    }

    bool parseFromJson(const std::string& json) {
        SimpleJson parser;
        if (!parser.parse(json)) return false;

        dock_id = parser.getInt("dock_id");
        lat = parser.getDouble("lat");
        lng = parser.getDouble("lng");
        is_locked = false; // 默认未锁定

        return true;
    }
};

struct RouteInfo {
    int route_id;
    RouteDirection direction;
    std::vector<Point> points;
};

struct SystemConfig {
    struct {
        double length;  // 船长 米
        double width;   // 船宽 米
    } boat;

    double emergency_threshold_s;  // 紧急判断时间阈值
    double warning_threshold_s;    // 警告判断时间阈值
    int max_boats;
    double min_route_gap_m;       // 最小航线横向间距

    bool parseFromJson(const std::string& json) {
        // 简化的配置解析
        size_t pos = 0;
        std::string content = json;

        // 查找boat配置
        pos = content.find("\"length\":");
        if (pos != std::string::npos) {
            pos += 9;
            size_t end = content.find(",", pos);
            if (end == std::string::npos) end = content.find("}", pos);
            boat.length = std::stod(content.substr(pos, end - pos));
        }

        pos = content.find("\"width\":");
        if (pos != std::string::npos) {
            pos += 8;
            size_t end = content.find("}", pos);
            boat.width = std::stod(content.substr(pos, end - pos));
        }

        pos = content.find("\"emergency_threshold_s\":");
        if (pos != std::string::npos) {
            pos += 24;
            size_t end = content.find(",", pos);
            emergency_threshold_s = std::stod(content.substr(pos, end - pos));
        }

        pos = content.find("\"warning_threshold_s\":");
        if (pos != std::string::npos) {
            pos += 22;
            size_t end = content.find(",", pos);
            warning_threshold_s = std::stod(content.substr(pos, end - pos));
        }

        pos = content.find("\"max_boats\":");
        if (pos != std::string::npos) {
            pos += 12;
            size_t end = content.find(",", pos);
            max_boats = std::stoi(content.substr(pos, end - pos));
        }

        pos = content.find("\"min_route_gap_m\":");
        if (pos != std::string::npos) {
            pos += 18;
            size_t end = content.find("}", pos);
            if (end == std::string::npos) end = content.find(",", pos);
            min_route_gap_m = std::stod(content.substr(pos, end - pos));
        }

        return true;
    }
};

struct CollisionAlert {
    AlertLevel level;
    int current_boat_id;
    std::vector<int> front_collision_boat_ids;  // 前向碰撞船只ID（最近一个）
    std::vector<int> head_on_collision_boat_ids; // 对向碰撞船只ID（所有）
    Point collision_position;
    double collision_time_s;
    double current_boat_heading;
    double other_boat_heading;
    std::string avoidance_suggestion;

    std::string toJson() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"alert_level\":" << static_cast<int>(level) << ",";
        oss << "\"current_boat_id\":" << current_boat_id << ",";

        oss << "\"front_collision_boat_ids\":[";
        for (size_t i = 0; i < front_collision_boat_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << front_collision_boat_ids[i];
        }
        oss << "],";

        oss << "\"head_on_collision_boat_ids\":[";
        for (size_t i = 0; i < head_on_collision_boat_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << head_on_collision_boat_ids[i];
        }
        oss << "],";

        oss << "\"collision_position\":" << collision_position.toString() << ",";
        oss << std::fixed << std::setprecision(2);
        oss << "\"collision_time_s\":" << collision_time_s << ",";
        oss << "\"current_boat_heading\":" << current_boat_heading << ",";
        oss << "\"other_boat_heading\":" << other_boat_heading << ",";
        oss << "\"avoidance_suggestion\":\"" << avoidance_suggestion << "\"";
        oss << "}";

        return oss.str();
    }
};

// 地理计算工具类
class GeoUtils {
public:
    // 计算两点间距离 (米)
    static double calculateDistance(const Point& p1, const Point& p2) {
        double lat1_rad = p1.lat * PI / 180.0;
        double lat2_rad = p2.lat * PI / 180.0;
        double delta_lat = (p2.lat - p1.lat) * PI / 180.0;
        double delta_lng = (p2.lng - p1.lng) * PI / 180.0;

        double a = sin(delta_lat / 2) * sin(delta_lat / 2) +
            cos(lat1_rad) * cos(lat2_rad) *
            sin(delta_lng / 2) * sin(delta_lng / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));

        return EARTH_RADIUS * c;
    }

    // 计算方位角 (度)
    static double calculateBearing(const Point& from, const Point& to) {
        double lat1_rad = from.lat * PI / 180.0;
        double lat2_rad = to.lat * PI / 180.0;
        double delta_lng = (to.lng - from.lng) * PI / 180.0;

        double y = sin(delta_lng) * cos(lat2_rad);
        double x = cos(lat1_rad) * sin(lat2_rad) -
            sin(lat1_rad) * cos(lat2_rad) * cos(delta_lng);

        double bearing = atan2(y, x) * 180.0 / PI;
        return fmod(bearing + 360.0, 360.0);
    }

    // 根据起点、方位角和距离计算终点
    static Point calculateDestination(const Point& start, double bearing_deg, double distance_m) {
        double lat1_rad = start.lat * PI / 180.0;
        double lng1_rad = start.lng * PI / 180.0;
        double bearing_rad = bearing_deg * PI / 180.0;
        double angular_distance = distance_m / EARTH_RADIUS;

        double lat2_rad = asin(sin(lat1_rad) * cos(angular_distance) +
            cos(lat1_rad) * sin(angular_distance) * cos(bearing_rad));

        double lng2_rad = lng1_rad + atan2(sin(bearing_rad) * sin(angular_distance) * cos(lat1_rad),
            cos(angular_distance) - sin(lat1_rad) * sin(lat2_rad));

        return Point(lat2_rad * 180.0 / PI, lng2_rad * 180.0 / PI);
    }
};

// 碰撞预测引擎
class CollisionPredictor {
private:
    SystemConfig config_;
    std::map<int, BoatState> boat_states_;
    std::vector<DockInfo> docks_;
    std::vector<RouteInfo> routes_;

public:
    CollisionPredictor(const SystemConfig& config) : config_(config) {}

    void updateBoatState(const BoatState& state) {
        boat_states_[state.sysid] = state;
    }

    void updateDockInfo(const std::vector<DockInfo>& docks) {
        docks_ = docks;
    }

    void updateRouteInfo(const std::vector<RouteInfo>& routes) {
        routes_ = routes;
    }

    // 预测两船碰撞时间和位置
    std::pair<double, Point> predictCollisionTimeAndPosition(const BoatState& boat1, const BoatState& boat2) {
        // 简化模型：假设船只直线航行
        Point p1 = boat1.getPosition();
        Point p2 = boat2.getPosition();

        // 计算船只的速度向量 (在笛卡尔坐标系中近似)
        double v1x = boat1.speed * sin(boat1.heading * PI / 180.0);
        double v1y = boat1.speed * cos(boat1.heading * PI / 180.0);
        double v2x = boat2.speed * sin(boat2.heading * PI / 180.0);
        double v2y = boat2.speed * cos(boat2.heading * PI / 180.0);

        // 相对速度
        double rel_vx = v1x - v2x;
        double rel_vy = v1y - v2y;

        // 相对位置 (转换为米)
        double rel_x = (p1.lng - p2.lng) * cos(p1.lat * PI / 180.0) * EARTH_RADIUS * PI / 180.0;
        double rel_y = (p1.lat - p2.lat) * EARTH_RADIUS * PI / 180.0;

        // 计算最近距离时间
        if (abs(rel_vx) < 1e-6 && abs(rel_vy) < 1e-6) {
            // 相对速度为0，不会碰撞
            return { -1, Point() };
        }

        double t_min = -(rel_x * rel_vx + rel_y * rel_vy) / (rel_vx * rel_vx + rel_vy * rel_vy);

        if (t_min < 0) {
            // 船只正在远离
            return { -1, Point() };
        }

        // 预测碰撞位置
        double collision_x = rel_x + rel_vx * t_min;
        double collision_y = rel_y + rel_vy * t_min;
        double min_distance = sqrt(collision_x * collision_x + collision_y * collision_y);

        // 考虑船只尺寸
        double safe_distance = sqrt(config_.boat.length * config_.boat.length +
            config_.boat.width * config_.boat.width);

        if (min_distance > safe_distance) {
            return { -1, Point() };
        }

        // 计算碰撞位置的地理坐标
        Point collision_pos = GeoUtils::calculateDestination(p1, boat1.heading, boat1.speed * t_min);

        return { t_min, collision_pos };
    }

    // 检查是否为对向航行
    bool isHeadOnCollision(const BoatState& boat1, const BoatState& boat2) {
        double heading_diff = abs(boat1.heading - boat2.heading);
        if (heading_diff > 180) heading_diff = 360 - heading_diff;
        return (heading_diff > 150 && heading_diff < 210); // 允许30度误差
    }

    // 检查优先级
    int getBoatPriority(const BoatState& boat) {
        switch (boat.status) {
        case BoatStatus::ENTERING_DOCK: return 3;  // 最高优先级
        case BoatStatus::NORMAL_SAILING: return 2;
        case BoatStatus::LEAVING_DOCK: return 1;   // 最低优先级
        }
        return 0;
    }

    // 生成避碰建议
    std::string generateAvoidanceSuggestion(const BoatState& current_boat,
        const std::vector<BoatState>& collision_boats,
        AlertLevel level) {
        std::string suggestion;

        switch (level) {
        case AlertLevel::EMERGENCY:
            if (!collision_boats.empty() &&
                getBoatPriority(current_boat) < getBoatPriority(collision_boats[0])) {
                suggestion = "立即停船等待高优先级船只通过";
            }
            else {
                suggestion = "立即减速并保持航向";
            }
            break;

        case AlertLevel::WARNING:
            suggestion = "减速并准备避让动作";
            break;

        case AlertLevel::NORMAL:
            suggestion = "恢复正常航行";
            break;
        }

        return suggestion;
    }

    // 主要碰撞检测函数
    std::vector<CollisionAlert> detectCollisions() {
        std::vector<CollisionAlert> alerts;

        for (auto& [boat_id, boat_state] : boat_states_) {
            CollisionAlert alert;
            alert.current_boat_id = boat_id;
            alert.level = AlertLevel::NORMAL;
            alert.current_boat_heading = boat_state.heading;
            alert.other_boat_heading = 0.0;
            alert.collision_time_s = 0.0;
            alert.collision_position = Point();

            std::vector<std::pair<double, int>> front_collisions;
            std::vector<int> head_on_collisions;
            double min_collision_time = std::numeric_limits<double>::max();
            Point collision_pos;

            // 检查与其他船只的碰撞
            for (auto& [other_id, other_state] : boat_states_) {
                if (boat_id == other_id) continue;

                auto [collision_time, collision_position] =
                    predictCollisionTimeAndPosition(boat_state, other_state);

                if (collision_time > 0) {
                    bool is_head_on = isHeadOnCollision(boat_state, other_state);

                    // 根据优先级和碰撞类型处理
                    if (is_head_on) {
                        head_on_collisions.push_back(other_id);
                    }
                    else {
                        // 检查是否为前向碰撞（同向或交叉）
                        front_collisions.push_back({ collision_time, other_id });
                    }

                    if (collision_time < min_collision_time) {
                        min_collision_time = collision_time;
                        collision_pos = collision_position;
                        alert.other_boat_heading = other_state.heading;
                    }

                    // 确定告警级别
                    if (collision_time <= config_.emergency_threshold_s) {
                        alert.level = AlertLevel::EMERGENCY;
                    }
                    else if (collision_time <= config_.warning_threshold_s &&
                        alert.level != AlertLevel::EMERGENCY) {
                        alert.level = AlertLevel::WARNING;
                    }
                }
            }

            // 整理碰撞船只ID
            if (!front_collisions.empty()) {
                // 对前向碰撞按时间排序，只报告最近的
                std::sort(front_collisions.begin(), front_collisions.end());
                alert.front_collision_boat_ids.push_back(front_collisions[0].second);
            }

            alert.head_on_collision_boat_ids = head_on_collisions;
            alert.collision_position = collision_pos;
            alert.collision_time_s = (min_collision_time == std::numeric_limits<double>::max()) ? 0.0 : min_collision_time;

            // 生成避碰建议
            std::vector<BoatState> collision_boats;
            for (int id : alert.front_collision_boat_ids) {
                collision_boats.push_back(boat_states_[id]);
            }
            for (int id : alert.head_on_collision_boat_ids) {
                collision_boats.push_back(boat_states_[id]);
            }

            alert.avoidance_suggestion = generateAvoidanceSuggestion(boat_state, collision_boats, alert.level);

            // 只添加有碰撞风险的告警
            if (alert.level != AlertLevel::NORMAL ||
                !alert.front_collision_boat_ids.empty() ||
                !alert.head_on_collision_boat_ids.empty()) {
                alerts.push_back(alert);
            }
        }

        return alerts;
    }

    void printBoatStates() {
        std::cout << "当前船只状态:" << std::endl;
        for (const auto& [id, state] : boat_states_) {
            std::cout << "船只ID: " << id
                << ", 位置: (" << state.lat << ", " << state.lng << ")"
                << ", 航向: " << state.heading << "°"
                << ", 速度: " << state.speed << "m/s"
                << ", 状态: " << static_cast<int>(state.status) << std::endl;
        }
    }
};

// 主系统类
class BoatSafetySystem {
private:
    SystemConfig config_;
    std::unique_ptr<CollisionPredictor> predictor_;

public:
    BoatSafetySystem(const SystemConfig& config)
        : config_(config), predictor_(std::make_unique<CollisionPredictor>(config)) {}

    bool updateBoatState(const std::string& json_str) {
        BoatState state;
        if (state.parseFromJson(json_str)) {
            predictor_->updateBoatState(state);
            return true;
        }
        return false;
    }

    bool updateSystemConfig(const std::string& json_str) {
        if (config_.parseFromJson(json_str)) {
            predictor_ = std::make_unique<CollisionPredictor>(config_);
            return true;
        }
        return false;
    }

    std::string detectCollisionsJson() {
        auto alerts = predictor_->detectCollisions();

        std::ostringstream result;
        result << "[";
        for (size_t i = 0; i < alerts.size(); ++i) {
            if (i > 0) result << ",";
            result << alerts[i].toJson();
        }
        result << "]";

        return result.str();
    }

    void printSystemStatus() {
        std::cout << "=== 无人船作业安全预测系统 ===" << std::endl;
        std::cout << "船只尺寸: " << config_.boat.length << "m x " << config_.boat.width << "m" << std::endl;
        std::cout << "紧急阈值: " << config_.emergency_threshold_s << "s" << std::endl;
        std::cout << "警告阈值: " << config_.warning_threshold_s << "s" << std::endl;
        std::cout << "最大船只数: " << config_.max_boats << std::endl;
        std::cout << "最小航线间距: " << config_.min_route_gap_m << "m" << std::endl;
        std::cout << "================================" << std::endl;
    }

    void printBoatStates() {
        predictor_->printBoatStates();
    }
};

// 示例使用和测试函数
void runExample() {
    // 系统配置
    SystemConfig config;
    config.boat.length = 0.75;
    config.boat.width = 0.47;
    config.emergency_threshold_s = 5.0;
    config.warning_threshold_s = 30.0;
    config.max_boats = 30;
    config.min_route_gap_m = 10.0;

    // 创建系统实例
    BoatSafetySystem system(config);
    system.printSystemStatus();

    std::cout << "\n正在模拟船只状态..." << std::endl;

    // 模拟船只状态更新 - 对向碰撞场景
    std::string boat1_json = R"({
        "sysid": 1,
        "timestamp": 1722325256.530,
        "lat": 30.549832,
        "lng": 114.342922,
        "heading": 90.0,
        "speed": 2.5,
        "status": 2,
        "route_direction": 1
    })";

    std::string boat2_json = R"({
        "sysid": 2,
        "timestamp": 1722325256.530,
        "lat": 30.549832,
        "lng": 114.343200,
        "heading": 270.0,
        "speed": 3.0,
        "status": 2,
        "route_direction": 2
    })";

    // 模拟入坞船只（高优先级）
    std::string boat3_json = R"({
        "sysid": 3,
        "timestamp": 1722325256.530,
        "lat": 30.549900,
        "lng": 114.342800,
        "heading": 180.0,
        "speed": 1.5,
        "status": 3,
        "route_direction": 1
    })";

    if (system.updateBoatState(boat1_json)) {
        std::cout << "船只1状态更新成功" << std::endl;
    }

    if (system.updateBoatState(boat2_json)) {
        std::cout << "船只2状态更新成功" << std::endl;
    }

    if (system.updateBoatState(boat3_json)) {
        std::cout << "船只3状态更新成功" << std::endl;
    }

    std::cout << "\n当前船只状态:" << std::endl;
    system.printBoatStates();

    // 执行碰撞检测
    std::cout << "\n执行碰撞检测..." << std::endl;
    std::string collision_results = system.detectCollisionsJson();
    std::cout << "\n碰撞检测结果JSON:" << std::endl;
    std::cout << collision_results << std::endl;

    // 格式化输出结果
    std::cout << "\n=== 碰撞告警详情 ===" << std::endl;
    if (collision_results == "[]") {
        std::cout << "当前无碰撞风险" << std::endl;
    }
    else {
        std::cout << "检测到碰撞风险，请查看上述JSON详情" << std::endl;
    }
}

int main() {
    try {
        runExample();

        std::cout << "\n=== 系统测试完成 ===" << std::endl;
        std::cout << "系统功能说明:" << std::endl;
        std::cout << "1. 支持三种船只状态：出坞(1)、正常航行(2)、入坞(3)" << std::endl;
        std::cout << "2. 三级告警：紧急(2)、警告(1)、正常(0)" << std::endl;
        std::cout << "3. 优先级：入坞 > 正常航行 > 出坞" << std::endl;
        std::cout << "4. 支持对向碰撞和前向碰撞检测" << std::endl;
        std::cout << "5. 提供避碰建议和预计碰撞时间位置" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "系统错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}