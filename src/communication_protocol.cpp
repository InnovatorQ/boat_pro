    // ==================== src/communication_protocol.cpp ====================
#include "communication_protocol.h"
#include "geometry_utils.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <ctime>

namespace boat_pro {
namespace communication {

// Drone ID Basic Message 实现
std::vector<uint8_t> DroneIDBasicMessage::serialize() const {
    std::vector<uint8_t> data(getSize());
    size_t offset = 0;
    
    data[offset++] = static_cast<uint8_t>(message_type_);
    data[offset++] = basic_id.ua_type;
    data[offset++] = basic_id.id_type;
    
    std::memcpy(&data[offset], basic_id.uas_id, 20);
    offset += 20;
    
    // 填充剩余字节
    std::fill(data.begin() + offset, data.end(), 0);
    
    return data;
}

bool DroneIDBasicMessage::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < getSize()) return false;
    
    size_t offset = 1; // 跳过消息类型
    basic_id.ua_type = data[offset++];
    basic_id.id_type = data[offset++];
    
    std::memcpy(basic_id.uas_id, &data[offset], 20);
    basic_id.uas_id[19] = '\0'; // 确保字符串结束
    
    return true;
}

// Drone ID Location Message 实现
std::vector<uint8_t> DroneIDLocationMessage::serialize() const {
    std::vector<uint8_t> data(getSize());
    size_t offset = 0;
    
    data[offset++] = static_cast<uint8_t>(message_type_);
    data[offset++] = location.status;
    data[offset++] = location.height_type;
    data[offset++] = location.ew_direction;
    data[offset++] = location.ns_direction;
    
    // 小端序写入
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.speed;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.vert_speed;
    offset += 2;
    *reinterpret_cast<int32_t*>(&data[offset]) = location.latitude;
    offset += 4;
    *reinterpret_cast<int32_t*>(&data[offset]) = location.longitude;
    offset += 4;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.pressure_alt;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.geodetic_alt;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.height;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.h_accuracy;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.v_accuracy;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.baro_accuracy;
    offset += 2;
    *reinterpret_cast<uint16_t*>(&data[offset]) = location.speed_accuracy;
    offset += 2;
    *reinterpret_cast<uint64_t*>(&data[offset]) = location.timestamp;
    
    return data;
}

bool DroneIDLocationMessage::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < getSize()) return false;
    
    size_t offset = 1; // 跳过消息类型
    location.status = data[offset++];
    location.height_type = data[offset++];
    location.ew_direction = data[offset++];
    location.ns_direction = data[offset++];
    
    // 小端序读取
    location.speed = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.vert_speed = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.latitude = *reinterpret_cast<const int32_t*>(&data[offset]);
    offset += 4;
    location.longitude = *reinterpret_cast<const int32_t*>(&data[offset]);
    offset += 4;
    location.pressure_alt = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.geodetic_alt = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.height = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.h_accuracy = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.v_accuracy = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.baro_accuracy = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.speed_accuracy = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    location.timestamp = *reinterpret_cast<const uint64_t*>(&data[offset]);
    
    return true;
}

DroneIDLocationMessage DroneIDLocationMessage::fromBoatState(const BoatState& boat) {
    DroneIDLocationMessage msg;
    
    msg.location.status = static_cast<uint8_t>(boat.status);
    msg.location.height_type = 0; // 海平面高度
    msg.location.ew_direction = (boat.lng >= 0) ? 0 : 1; // 东(0)/西(1)
    msg.location.ns_direction = (boat.lat >= 0) ? 0 : 1; // 北(0)/南(1)
    msg.location.speed = static_cast<uint16_t>(boat.speed / 0.25); // 转换为0.25m/s单位
    msg.location.vert_speed = 0; // 船只无垂直速度
    msg.location.latitude = static_cast<int32_t>(boat.lat * 1e7); // 转换为1e-7度单位
    msg.location.longitude = static_cast<int32_t>(boat.lng * 1e7);
    msg.location.pressure_alt = 0;
    msg.location.geodetic_alt = 0;
    msg.location.height = 0;
    msg.location.h_accuracy = 3; // 3米精度
    msg.location.v_accuracy = 0;
    msg.location.baro_accuracy = 0;
    msg.location.speed_accuracy = 1; // 0.3m/s精度
    msg.location.timestamp = static_cast<uint64_t>(boat.timestamp);
    
    return msg;
}

// NMEA 2000 Position Message 实现
std::vector<uint8_t> NMEA2000PositionMessage::serialize() const {
    std::vector<uint8_t> data(getSize());
    size_t offset = 0;
    
    // NMEA 2000消息格式：PGN + 数据
    uint32_t pgn = static_cast<uint32_t>(pgn_);
    *reinterpret_cast<uint32_t*>(&data[offset]) = pgn;
    offset += 4;
    
    *reinterpret_cast<int32_t*>(&data[offset]) = position.latitude;
    offset += 4;
    
    *reinterpret_cast<int32_t*>(&data[offset]) = position.longitude;
    
    return data;
}

bool NMEA2000PositionMessage::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < getSize()) return false;
    
    size_t offset = 4; // 跳过PGN
    position.latitude = *reinterpret_cast<const int32_t*>(&data[offset]);
    offset += 4;
    
    // 检查是否还有经度数据
    if (data.size() >= offset + 4) {
        position.longitude = *reinterpret_cast<const int32_t*>(&data[offset]);
    }
    
    return true;
}

NMEA2000PositionMessage NMEA2000PositionMessage::fromBoatState(const BoatState& boat) {
    NMEA2000PositionMessage msg;
    msg.position.latitude = static_cast<int32_t>(boat.lat * 1e7);
    msg.position.longitude = static_cast<int32_t>(boat.lng * 1e7);
    return msg;
}

// NMEA 2000 COG/SOG Message 实现
std::vector<uint8_t> NMEA2000COGSOGMessage::serialize() const {
    std::vector<uint8_t> data(getSize());
    size_t offset = 0;
    
    uint32_t pgn = static_cast<uint32_t>(pgn_);
    *reinterpret_cast<uint32_t*>(&data[offset]) = pgn;
    offset += 4;
    
    data[offset++] = cog_sog.sid;
    *reinterpret_cast<uint16_t*>(&data[offset]) = cog_sog.cog;
    offset += 2;
    data[offset++] = static_cast<uint8_t>(cog_sog.sog & 0xFF); // 低字节
    
    return data;
}

bool NMEA2000COGSOGMessage::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < getSize()) return false;
    
    size_t offset = 4; // 跳过PGN
    cog_sog.sid = data[offset++];
    cog_sog.cog = *reinterpret_cast<const uint16_t*>(&data[offset]);
    offset += 2;
    cog_sog.sog = static_cast<uint16_t>(data[offset]);
    
    return true;
}

NMEA2000COGSOGMessage NMEA2000COGSOGMessage::fromBoatState(const BoatState& boat) {
    NMEA2000COGSOGMessage msg;
    msg.cog_sog.sid = 0;
    // 将度数转换为0.0001弧度单位
    msg.cog_sog.cog = static_cast<uint16_t>(geometry::toRadians(boat.heading) * 10000);
    // 将m/s转换为0.01m/s单位
    msg.cog_sog.sog = static_cast<uint16_t>(boat.speed * 100);
    return msg;
}

// Protocol Converter 实现
std::unique_ptr<DroneIDLocationMessage> ProtocolConverter::todroneIDLocation(const BoatState& boat) {
    auto msg = std::make_unique<DroneIDLocationMessage>();
    *msg = DroneIDLocationMessage::fromBoatState(boat);
    return msg;
}

std::unique_ptr<NMEA2000PositionMessage> ProtocolConverter::toNMEA2000Position(const BoatState& boat) {
    auto msg = std::make_unique<NMEA2000PositionMessage>();
    *msg = NMEA2000PositionMessage::fromBoatState(boat);
    return msg;
}

std::unique_ptr<NMEA2000COGSOGMessage> ProtocolConverter::toNMEA2000COGSOG(const BoatState& boat) {
    auto msg = std::make_unique<NMEA2000COGSOGMessage>();
    *msg = NMEA2000COGSOGMessage::fromBoatState(boat);
    return msg;
}

BoatState ProtocolConverter::fromDroneIDLocation(const DroneIDLocationMessage& msg, int sysid) {
    BoatState boat;
    boat.sysid = sysid;
    boat.timestamp = static_cast<double>(msg.location.timestamp);
    boat.lat = static_cast<double>(msg.location.latitude) / 1e7;
    boat.lng = static_cast<double>(msg.location.longitude) / 1e7;
    boat.speed = static_cast<double>(msg.location.speed) * 0.25;
    boat.status = static_cast<BoatStatus>(msg.location.status);
    boat.route_direction = RouteDirection::CLOCKWISE; // 默认值，需要从其他信息获取
    boat.heading = 0.0; // 需要从其他消息获取
    return boat;
}

BoatState ProtocolConverter::fromNMEA2000Messages(const NMEA2000PositionMessage& pos_msg,
                                                 const NMEA2000COGSOGMessage& cog_msg,
                                                 int sysid) {
    BoatState boat;
    boat.sysid = sysid;
    boat.timestamp = std::time(nullptr); // 当前时间
    boat.lat = static_cast<double>(pos_msg.position.latitude) / 1e7;
    boat.lng = static_cast<double>(pos_msg.position.longitude) / 1e7;
    boat.heading = geometry::toDegrees(static_cast<double>(cog_msg.cog_sog.cog) / 10000.0);
    boat.speed = static_cast<double>(cog_msg.cog_sog.sog) / 100.0;
    boat.status = BoatStatus::NORMAL_SAIL; // 默认状态
    boat.route_direction = RouteDirection::CLOCKWISE; // 默认方向
    return boat;
}

} // namespace communication
} // namespace boat_pro