#ifndef DATA_FORMAT_CONVERTER_H
#define DATA_FORMAT_CONVERTER_H

#include <json/json.h>
#include <string>
#include <unordered_map>
#include <algorithm>

/**
 * 数据格式转换器
 * 用于将甲方的数据格式转换为系统标准格式
 */
class DataFormatConverter {
public:
    DataFormatConverter();
    
    /**
     * 转换船只状态数据
     * @param clientData 甲方原始数据格式
     * @return 系统标准格式数据
     */
    Json::Value convertBoatState(const Json::Value& clientData);
    
    /**
     * 转换船坞信息数据
     * @param clientData 甲方原始数据格式
     * @return 系统标准格式数据
     */
    Json::Value convertDockInfo(const Json::Value& clientData);
    
    /**
     * 转换航线信息数据
     * @param clientData 甲方原始数据格式
     * @return 系统标准格式数据
     */
    Json::Value convertRouteInfo(const Json::Value& clientData);

private:
    // 状态映射表
    std::unordered_map<std::string, int> statusMapping;
    
    // 航线映射表
    std::unordered_map<std::string, int> routeMapping;
    
    /**
     * 从船只ID字符串中提取数字ID
     * @param vessel_id 船只ID字符串（如"BOAT001"）
     * @return 数字ID
     */
    int extractBoatId(const std::string& vessel_id);
    
    /**
     * 转换时间戳
     * @param timestamp JSON时间戳值
     * @return Unix时间戳
     */
    double convertTimestamp(const Json::Value& timestamp);
    
    /**
     * 将ISO 8601时间格式转换为Unix时间戳
     * @param iso_time ISO 8601格式时间字符串
     * @return Unix时间戳
     */
    double convertISOToUnixTimestamp(const std::string& iso_time);
    
    /**
     * 将节转换为米/秒
     * @param knots 速度（节）
     * @return 速度（米/秒）
     */
    double knotsToMeterPerSecond(double knots);
    
    /**
     * 映射状态字符串到数字
     * @param status 状态字符串
     * @return 状态数字
     */
    int mapStatus(const std::string& status);
    
    /**
     * 映射航线字符串到数字
     * @param route 航线字符串
     * @return 航线数字
     */
    int mapRoute(const std::string& route);
    
    /**
     * 转换航线点数据
     * @param waypoints 原始航线点数据
     * @return 标准格式航线点数据
     */
    Json::Value convertWaypoints(const Json::Value& waypoints);
};

#endif // DATA_FORMAT_CONVERTER_H
