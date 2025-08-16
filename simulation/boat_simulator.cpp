#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <cmath>
#include <json/json.h>
#include <mosquitto.h>
#include <atomic>
#include <mutex>
#include <iomanip>

// 船只仿真类
class BoatSimulator {
private:
    struct Boat {
        int id;
        double lat, lng;
        double heading;
        double speed;
        int status; // 1-出坞, 2-正常航行, 3-入坞
        int route_direction; // 1-顺时针, 2-逆时针
        double target_lat, target_lng;
        bool in_dock;
        int dock_id;
        
        Boat(int _id) : id(_id), lat(30.549832), lng(114.342922), 
                       heading(0), speed(0), status(1), route_direction(1),
                       target_lat(30.549832), target_lng(114.342922),
                       in_dock(true), dock_id(_id % 3 + 1) {}
    };
    
    struct Dock {
        int id;
        double lat, lng;
        bool occupied;
        int boat_id;
        
        Dock(int _id, double _lat, double _lng) : 
            id(_id), lat(_lat), lng(_lng), occupied(false), boat_id(-1) {}
    };
    
    struct RoutePoint {
        double lat, lng;
        RoutePoint(double _lat, double _lng) : lat(_lat), lng(_lng) {}
    };

    std::vector<Boat> boats;
    std::vector<Dock> docks;
    std::vector<std::vector<RoutePoint>> routes;
    struct mosquitto* mqtt_client;
    std::atomic<bool> running;
    std::mutex boats_mutex;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> noise_dist;
    
    // 仿真参数
    const double SIMULATION_SPEED = 10.0; // 仿真加速倍数
    const double MAX_SPEED = 3.0; // 最大速度 m/s
    const double MIN_SPEED = 0.5; // 最小速度 m/s
    const double COLLISION_DISTANCE = 15.0; // 碰撞检测距离 m
    const double DOCK_DISTANCE = 5.0; // 进坞距离 m
    
public:
    BoatSimulator() : running(false), gen(rd()), noise_dist(-0.1, 0.1) {
        initializeEnvironment();
        initializeMQTT();
    }
    
    ~BoatSimulator() {
        stop();
        if (mqtt_client) {
            mosquitto_destroy(mqtt_client);
        }
        mosquitto_lib_cleanup();
    }
    
    void initializeEnvironment() {
        // 初始化船坞
        docks.emplace_back(1, 30.549100, 114.343000);
        docks.emplace_back(2, 30.549200, 114.343100);
        docks.emplace_back(3, 30.549300, 114.343200);
        
        // 初始化航线 - 顺时针
        routes.emplace_back();
        routes[0].emplace_back(30.549500, 114.342800);
        routes[0].emplace_back(30.549800, 114.343300);
        routes[0].emplace_back(30.550100, 114.343800);
        routes[0].emplace_back(30.550400, 114.343300);
        routes[0].emplace_back(30.550100, 114.342800);
        routes[0].emplace_back(30.549800, 114.342300);
        
        // 初始化航线 - 逆时针
        routes.emplace_back();
        routes[1].emplace_back(30.549500, 114.342800);
        routes[1].emplace_back(30.549800, 114.342300);
        routes[1].emplace_back(30.550100, 114.342800);
        routes[1].emplace_back(30.550400, 114.343300);
        routes[1].emplace_back(30.550100, 114.343800);
        routes[1].emplace_back(30.549800, 114.343300);
        
        // 初始化船只（5艘船进行测试）
        for (int i = 1; i <= 5; ++i) {
            boats.emplace_back(i);
            // 随机分配到不同船坞
            boats.back().dock_id = (i - 1) % 3 + 1;
            boats.back().lat = docks[boats.back().dock_id - 1].lat;
            boats.back().lng = docks[boats.back().dock_id - 1].lng;
        }
        
        std::cout << "✓ 环境初始化完成: " << boats.size() << "艘船, " 
                  << docks.size() << "个船坞, " << routes.size() << "条航线" << std::endl;
    }
    
    bool initializeMQTT() {
        mosquitto_lib_init();
        mqtt_client = mosquitto_new("boat_simulator", true, nullptr);
        if (!mqtt_client) {
            std::cerr << "✗ MQTT客户端创建失败" << std::endl;
            return false;
        }
        
        int result = mosquitto_connect(mqtt_client, "localhost", 1883, 60);
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "✗ MQTT连接失败: " << mosquitto_strerror(result) << std::endl;
            return false;
        }
        
        // 等待连接建立
        for (int i = 0; i < 50; ++i) {
            mosquitto_loop(mqtt_client, 10, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "✓ MQTT连接建立成功" << std::endl;
        return true;
    }
    
    void publishBoatState(const Boat& boat) {
        Json::Value state;
        state["sysid"] = boat.id;
        state["timestamp"] = std::time(nullptr);
        state["lat"] = boat.lat + noise_dist(gen) * 0.0001; // 添加GPS噪声
        state["lng"] = boat.lng + noise_dist(gen) * 0.0001;
        state["heading"] = boat.heading;
        state["speed"] = boat.speed + noise_dist(gen) * 0.1; // 添加速度噪声
        state["status"] = boat.status;
        state["route_direction"] = boat.route_direction;
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::string json_str = Json::writeString(builder, state);
        
        std::string topic = "boat_safety/input/boat_state/" + std::to_string(boat.id);
        mosquitto_publish(mqtt_client, nullptr, topic.c_str(), 
                         json_str.length(), json_str.c_str(), 1, false);
    }
    
    void publishDockInfo() {
        for (const auto& dock : docks) {
            Json::Value info;
            info["dock_id"] = dock.id;
            info["lat"] = dock.lat;
            info["lng"] = dock.lng;
            info["occupied"] = dock.occupied;
            if (dock.occupied) {
                info["boat_id"] = dock.boat_id;
            }
            
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            std::string json_str = Json::writeString(builder, info);
            
            mosquitto_publish(mqtt_client, nullptr, "boat_safety/input/dock_info", 
                             json_str.length(), json_str.c_str(), 1, false);
        }
    }
    
    void publishRouteInfo() {
        for (size_t i = 0; i < routes.size(); ++i) {
            Json::Value info;
            info["route_id"] = static_cast<int>(i + 1);
            info["direction"] = static_cast<int>(i + 1); // 1-顺时针, 2-逆时针
            info["points"] = Json::Value(Json::arrayValue);
            
            for (const auto& point : routes[i]) {
                Json::Value p;
                p["lat"] = point.lat;
                p["lng"] = point.lng;
                info["points"].append(p);
            }
            
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            std::string json_str = Json::writeString(builder, info);
            
            mosquitto_publish(mqtt_client, nullptr, "boat_safety/input/route_info", 
                             json_str.length(), json_str.c_str(), 1, false);
        }
    }
    
    double calculateDistance(double lat1, double lng1, double lat2, double lng2) {
        // 简化的距离计算（适用于小范围）
        const double EARTH_RADIUS = 6371000; // 地球半径（米）
        double dlat = (lat2 - lat1) * M_PI / 180.0;
        double dlng = (lng2 - lng1) * M_PI / 180.0;
        double a = sin(dlat/2) * sin(dlat/2) + cos(lat1 * M_PI / 180.0) * 
                   cos(lat2 * M_PI / 180.0) * sin(dlng/2) * sin(dlng/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        return EARTH_RADIUS * c;
    }
    
    double calculateBearing(double lat1, double lng1, double lat2, double lng2) {
        double dlng = (lng2 - lng1) * M_PI / 180.0;
        double lat1_rad = lat1 * M_PI / 180.0;
        double lat2_rad = lat2 * M_PI / 180.0;
        
        double y = sin(dlng) * cos(lat2_rad);
        double x = cos(lat1_rad) * sin(lat2_rad) - sin(lat1_rad) * cos(lat2_rad) * cos(dlng);
        
        double bearing = atan2(y, x) * 180.0 / M_PI;
        return fmod(bearing + 360.0, 360.0);
    }
    
    void updateBoatMovement(Boat& boat, double dt) {
        if (boat.in_dock && boat.status == 1) {
            // 船只准备出坞
            std::uniform_real_distribution<> prob(0, 1);
            if (prob(gen) < 0.1 * dt) { // 10%概率每秒出坞
                boat.status = 2; // 正常航行
                boat.in_dock = false;
                boat.speed = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * prob(gen);
                boat.route_direction = (prob(gen) < 0.5) ? 1 : 2;
                
                // 设置第一个航线点作为目标
                int route_idx = boat.route_direction - 1;
                boat.target_lat = routes[route_idx][0].lat;
                boat.target_lng = routes[route_idx][0].lng;
                
                std::cout << "船只 " << boat.id << " 出坞，目标航线 " << boat.route_direction << std::endl;
            }
        }
        else if (!boat.in_dock && boat.status == 2) {
            // 正常航行状态
            double dist_to_target = calculateDistance(boat.lat, boat.lng, 
                                                    boat.target_lat, boat.target_lng);
            
            if (dist_to_target < 10.0) { // 到达航线点
                // 选择下一个航线点
                int route_idx = boat.route_direction - 1;
                static std::vector<int> boat_route_progress(6, 0); // 每艘船的航线进度
                
                boat_route_progress[boat.id] = (boat_route_progress[boat.id] + 1) % routes[route_idx].size();
                boat.target_lat = routes[route_idx][boat_route_progress[boat.id]].lat;
                boat.target_lng = routes[route_idx][boat_route_progress[boat.id]].lng;
                
                // 随机决定是否进坞
                std::uniform_real_distribution<> prob(0, 1);
                if (prob(gen) < 0.05 * dt) { // 5%概率每秒进坞
                    boat.status = 3; // 准备入坞
                    // 选择最近的空闲船坞
                    int nearest_dock = -1;
                    double min_dist = 1e9;
                    for (size_t i = 0; i < docks.size(); ++i) {
                        if (!docks[i].occupied) {
                            double dist = calculateDistance(boat.lat, boat.lng, 
                                                          docks[i].lat, docks[i].lng);
                            if (dist < min_dist) {
                                min_dist = dist;
                                nearest_dock = i;
                            }
                        }
                    }
                    
                    if (nearest_dock >= 0) {
                        boat.target_lat = docks[nearest_dock].lat;
                        boat.target_lng = docks[nearest_dock].lng;
                        boat.dock_id = docks[nearest_dock].id;
                        std::cout << "船只 " << boat.id << " 准备进入船坞 " << boat.dock_id << std::endl;
                    }
                }
            }
            
            // 更新船只位置
            boat.heading = calculateBearing(boat.lat, boat.lng, boat.target_lat, boat.target_lng);
            double distance = boat.speed * dt;
            double bearing_rad = boat.heading * M_PI / 180.0;
            
            // 简化的位置更新
            boat.lat += distance * cos(bearing_rad) / 111320.0; // 纬度
            boat.lng += distance * sin(bearing_rad) / (111320.0 * cos(boat.lat * M_PI / 180.0)); // 经度
        }
        else if (boat.status == 3) {
            // 入坞状态
            double dist_to_dock = calculateDistance(boat.lat, boat.lng, 
                                                  boat.target_lat, boat.target_lng);
            
            if (dist_to_dock < DOCK_DISTANCE) {
                // 到达船坞
                boat.in_dock = true;
                boat.status = 1;
                boat.speed = 0;
                boat.lat = boat.target_lat;
                boat.lng = boat.target_lng;
                
                // 占用船坞
                for (auto& dock : docks) {
                    if (dock.id == boat.dock_id) {
                        dock.occupied = true;
                        dock.boat_id = boat.id;
                        break;
                    }
                }
                
                std::cout << "船只 " << boat.id << " 已进入船坞 " << boat.dock_id << std::endl;
            } else {
                // 向船坞移动
                boat.heading = calculateBearing(boat.lat, boat.lng, boat.target_lat, boat.target_lng);
                boat.speed = std::max(0.5, boat.speed * 0.8); // 减速进坞
                
                double distance = boat.speed * dt;
                double bearing_rad = boat.heading * M_PI / 180.0;
                
                boat.lat += distance * cos(bearing_rad) / 111320.0;
                boat.lng += distance * sin(bearing_rad) / (111320.0 * cos(boat.lat * M_PI / 180.0));
            }
        }
    }
    
    void simulateStep(double dt) {
        std::lock_guard<std::mutex> lock(boats_mutex);
        
        // 更新所有船只
        for (auto& boat : boats) {
            updateBoatMovement(boat, dt);
        }
        
        // 发布船只状态
        for (const auto& boat : boats) {
            publishBoatState(boat);
        }
        
        // 处理MQTT消息
        mosquitto_loop(mqtt_client, 1, 1);
    }
    
    void start() {
        running = true;
        
        // 发布静态信息
        publishDockInfo();
        publishRouteInfo();
        
        std::cout << "🚀 开始真实场景仿真..." << std::endl;
        std::cout << "仿真参数:" << std::endl;
        std::cout << "  - 船只数量: " << boats.size() << std::endl;
        std::cout << "  - 仿真加速: " << SIMULATION_SPEED << "x" << std::endl;
        std::cout << "  - 最大速度: " << MAX_SPEED << " m/s" << std::endl;
        std::cout << "  - 碰撞距离: " << COLLISION_DISTANCE << " m" << std::endl;
        
        auto last_time = std::chrono::steady_clock::now();
        int step_count = 0;
        
        while (running) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - last_time).count();
            
            if (elapsed >= 100) { // 10Hz更新频率
                double dt = elapsed / 1000.0 * SIMULATION_SPEED;
                simulateStep(dt);
                last_time = current_time;
                step_count++;
                
                // 每50步输出一次状态
                if (step_count % 50 == 0) {
                    printStatus();
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void stop() {
        running = false;
        std::cout << "🛑 仿真停止" << std::endl;
    }
    
    void printStatus() {
        std::lock_guard<std::mutex> lock(boats_mutex);
        
        std::cout << "\n=== 仿真状态 ===" << std::endl;
        for (const auto& boat : boats) {
            std::string status_str;
            switch (boat.status) {
                case 1: status_str = "停靠"; break;
                case 2: status_str = "航行"; break;
                case 3: status_str = "入坞"; break;
            }
            
            std::cout << "船只" << boat.id << ": " << status_str 
                      << " | 位置(" << std::fixed << std::setprecision(6) 
                      << boat.lat << "," << boat.lng << ")"
                      << " | 速度" << std::setprecision(1) << boat.speed << "m/s"
                      << " | 航向" << std::setprecision(0) << boat.heading << "°" << std::endl;
        }
        
        int docked = 0, sailing = 0, docking = 0;
        for (const auto& boat : boats) {
            switch (boat.status) {
                case 1: docked++; break;
                case 2: sailing++; break;
                case 3: docking++; break;
            }
        }
        
        std::cout << "状态统计: 停靠" << docked << "艘, 航行" << sailing 
                  << "艘, 入坞" << docking << "艘" << std::endl;
    }
};

int main() {
    std::cout << "=== 无人船作业安全预测系统 - 真实场景仿真 ===" << std::endl;
    
    BoatSimulator simulator;
    
    // 启动仿真
    std::thread sim_thread([&simulator]() {
        simulator.start();
    });
    
    // 运行60秒
    std::cout << "仿真将运行60秒，按Ctrl+C提前停止..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
    
    simulator.stop();
    sim_thread.join();
    
    std::cout << "✅ 真实场景仿真完成" << std::endl;
    return 0;
}
