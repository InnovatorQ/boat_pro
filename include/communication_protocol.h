// ==================== include/communication_protocol.h ====================
#ifndef BOAT_PRO_COMMUNICATION_PROTOCOL_H
#define BOAT_PRO_COMMUNICATION_PROTOCOL_H

#include "types.h"
#include <string>
#include <vector>
#include <cstdint>

namespace boat_pro {
namespace communication {

// Drone ID 标准消息类型
enum class DroneIDMessageType : uint8_t {
    BASIC_ID = 0x00,
    LOCATION = 0x01,
    AUTH = 0x02,
    SELF_ID = 0x03,
    SYSTEM = 0x04,
    OPERATOR_ID = 0x05
};

// NMEA 2000 参数组编号 (PGN)
enum class NMEA2000_PGN : uint32_t {
    VESSEL_HEADING = 127250,          // 船只航向
    RATE_OF_TURN = 127251,            // 转向率
    ATTITUDE = 127257,                // 姿态
    POSITION_RAPID_UPDATE = 129025,   // 位置快速更新
    COG_SOG_RAPID_UPDATE = 129026,    // 航向和航速快速更新
    GNSS_POSITION_DATA = 129029,      // GNSS位置数据
    AIS_CLASS_A = 129038,             // AIS A类消息
    AIS_CLASS_B = 129039              // AIS B类消息
};

/**
 * Drone ID 协议消息基类
 */
class DroneIDMessage {
public:
    DroneIDMessage(DroneIDMessageType type) : message_type_(type) {}
    virtual ~DroneIDMessage() = default;
    
    DroneIDMessageType getMessageType() const { return message_type_; }
    virtual std::vector<uint8_t> serialize() const = 0;
    virtual bool deserialize(const std::vector<uint8_t>& data) = 0;
    virtual size_t getSize() const = 0;

protected:
    DroneIDMessageType message_type_;
};

/**
 * Drone ID 基本信息消息
 */
class DroneIDBasicMessage : public DroneIDMessage {
public:
    DroneIDBasicMessage() : DroneIDMessage(DroneIDMessageType::BASIC_ID) {}
    
    struct {
        uint8_t ua_type;        // 无人设备类型 (船只=4)
        uint8_t id_type;        // ID类型
        char uas_id[20];        // 设备ID
    } basic_id;
    
    std::vector<uint8_t> serialize() const override;
    bool deserialize(const std::vector<uint8_t>& data) override;
    size_t getSize() const override { return 25; }
};

/**
 * Drone ID 位置消息
 */
class DroneIDLocationMessage : public DroneIDMessage {
public:
    DroneIDLocationMessage() : DroneIDMessage(DroneIDMessageType::LOCATION) {}
    
    struct {
        uint8_t status;         // 运行状态
        uint8_t height_type;    // 高度类型
        uint8_t ew_direction;   // 东西方向
        uint8_t ns_direction;   // 南北方向
        uint16_t speed;         // 速度 (0.25m/s单位)
        uint16_t vert_speed;    // 垂直速度
        int32_t latitude;       // 纬度 (1e-7度单位)
        int32_t longitude;      // 经度 (1e-7度单位)
        uint16_t pressure_alt;  // 气压高度
        uint16_t geodetic_alt;  // 大地高度
        uint16_t height;        // 相对高度
        uint16_t h_accuracy;    // 水平精度
        uint16_t v_accuracy;    // 垂直精度
        uint16_t baro_accuracy; // 气压精度
        uint16_t speed_accuracy;// 速度精度
        uint64_t timestamp;     // 时间戳
    } location;
    
    std::vector<uint8_t> serialize() const override;
    bool deserialize(const std::vector<uint8_t>& data) override;
    size_t getSize() const override { return 35; }
    
    // 从船只状态转换
    static DroneIDLocationMessage fromBoatState(const BoatState& boat);
};

/**
 * NMEA 2000 协议消息基类
 */
class NMEA2000Message {
public:
    NMEA2000Message(NMEA2000_PGN pgn) : pgn_(pgn) {}
    virtual ~NMEA2000Message() = default;
    
    NMEA2000_PGN getPGN() const { return pgn_; }
    virtual std::vector<uint8_t> serialize() const = 0;
    virtual bool deserialize(const std::vector<uint8_t>& data) = 0;
    virtual size_t getSize() const = 0;

protected:
    NMEA2000_PGN pgn_;
    uint8_t source_address_ = 0x17; // 默认船只地址
};

/**
 * NMEA 2000 位置快速更新消息 (PGN 129025)
 */
class NMEA2000PositionMessage : public NMEA2000Message {
public:
    NMEA2000PositionMessage() : NMEA2000Message(NMEA2000_PGN::POSITION_RAPID_UPDATE) {}
    
    struct {
        int32_t latitude;    // 纬度 (1e-7度单位)
        int32_t longitude;   // 经度 (1e-7度单位)
    } position;
    
    std::vector<uint8_t> serialize() const override;
    bool deserialize(const std::vector<uint8_t>& data) override;
    size_t getSize() const override { return 12; }
    
    // 从船只状态转换
    static NMEA2000PositionMessage fromBoatState(const BoatState& boat);
};

/**
 * NMEA 2000 航向速度消息 (PGN 129026)
 */
class NMEA2000COGSOGMessage : public NMEA2000Message {
public:
    NMEA2000COGSOGMessage() : NMEA2000Message(NMEA2000_PGN::COG_SOG_RAPID_UPDATE) {}
    
    struct {
        uint8_t sid;         // 序列ID
        uint16_t cog;        // 对地航向 (0.0001弧度单位)
        uint16_t sog;        // 对地速度 (0.01m/s单位)
    } cog_sog;
    
    std::vector<uint8_t> serialize() const override;
    bool deserialize(const std::vector<uint8_t>& data) override;
    size_t getSize() const override { return 8; }
    
    // 从船只状态转换
    static NMEA2000COGSOGMessage fromBoatState(const BoatState& boat);
};

/**
 * 通信协议转换器
 */
class ProtocolConverter {
public:
    // 船只状态转换为Drone ID位置消息
    static std::unique_ptr<DroneIDLocationMessage> todroneIDLocation(const BoatState& boat);
    
    // 船只状态转换为NMEA 2000位置消息
    static std::unique_ptr<NMEA2000PositionMessage> toNMEA2000Position(const BoatState& boat);
    
    // 船只状态转换为NMEA 2000航向速度消息
    static std::unique_ptr<NMEA2000COGSOGMessage> toNMEA2000COGSOG(const BoatState& boat);
    
    // Drone ID消息转换为船只状态
    static BoatState fromDroneIDLocation(const DroneIDLocationMessage& msg, int sysid);
    
    // NMEA 2000消息转换为船只状态
    static BoatState fromNMEA2000Messages(const NMEA2000PositionMessage& pos_msg,
                                         const NMEA2000COGSOGMessage& cog_msg,
                                         int sysid);
};

} // namespace communication
} // namespace boat_pro

#endif
