#!/bin/bash

# 配置管理工具
# 用于管理和发布系统配置

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."
CONFIG_FILE="$PROJECT_ROOT/config/system_config.json"

MQTT_HOST="localhost"
MQTT_PORT="1883"

echo "=========================================="
echo "  无人船系统配置管理工具"
echo "=========================================="

# 显示帮助信息
show_help() {
    echo "用法: $0 [命令]"
    echo ""
    echo "命令:"
    echo "  show      显示当前配置"
    echo "  publish   发布配置到MQTT"
    echo "  clear     清除MQTT中的保留配置"
    echo "  validate  验证配置文件格式"
    echo "  backup    备份当前配置"
    echo "  restore   恢复配置备份"
    echo "  help      显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 show"
    echo "  $0 publish"
    echo "  $0 clear"
}

# 显示当前配置
show_config() {
    echo "📋 当前系统配置:"
    echo "配置文件: $CONFIG_FILE"
    echo ""
    
    if [ -f "$CONFIG_FILE" ]; then
        echo "=== 配置内容 ==="
        cat "$CONFIG_FILE" | jq . 2>/dev/null || cat "$CONFIG_FILE"
        echo ""
        
        # 解析并显示关键参数
        if command -v jq &> /dev/null; then
            echo "=== 关键参数 ==="
            echo "船只长度: $(jq -r '.boat.length' "$CONFIG_FILE")m"
            echo "船只宽度: $(jq -r '.boat.width' "$CONFIG_FILE")m"
            echo "紧急阈值: $(jq -r '.emergency_threshold_s' "$CONFIG_FILE")秒"
            echo "警告阈值: $(jq -r '.warning_threshold_s' "$CONFIG_FILE")秒"
            echo "最大船只数: $(jq -r '.max_boats' "$CONFIG_FILE")"
            echo "最小航线间距: $(jq -r '.min_route_gap_m' "$CONFIG_FILE")米"
        fi
    else
        echo "❌ 配置文件不存在: $CONFIG_FILE"
    fi
}

# 验证配置文件
validate_config() {
    echo "🔍 验证配置文件..."
    
    if [ ! -f "$CONFIG_FILE" ]; then
        echo "❌ 配置文件不存在: $CONFIG_FILE"
        return 1
    fi
    
    # 检查JSON格式
    if command -v jq &> /dev/null; then
        if jq . "$CONFIG_FILE" > /dev/null 2>&1; then
            echo "✅ JSON格式正确"
        else
            echo "❌ JSON格式错误"
            return 1
        fi
        
        # 检查必需字段
        required_fields=("boat.length" "boat.width" "emergency_threshold_s" "warning_threshold_s" "max_boats" "min_route_gap_m")
        
        for field in "${required_fields[@]}"; do
            if jq -e ".$field" "$CONFIG_FILE" > /dev/null 2>&1; then
                echo "✅ 字段存在: $field"
            else
                echo "❌ 缺少字段: $field"
                return 1
            fi
        done
        
        echo "✅ 配置文件验证通过"
    else
        echo "⚠️ 未安装jq，跳过详细验证"
        # 简单的JSON语法检查
        if python3 -m json.tool "$CONFIG_FILE" > /dev/null 2>&1; then
            echo "✅ JSON格式基本正确"
        else
            echo "❌ JSON格式错误"
            return 1
        fi
    fi
}

# 发布配置到MQTT
publish_config() {
    echo "📡 发布配置到MQTT..."
    
    if [ ! -f "$CONFIG_FILE" ]; then
        echo "❌ 配置文件不存在: $CONFIG_FILE"
        return 1
    fi
    
    # 验证配置
    if ! validate_config > /dev/null 2>&1; then
        echo "❌ 配置验证失败，请先修复配置文件"
        return 1
    fi
    
    # 检查MQTT工具
    if ! command -v mosquitto_pub &> /dev/null; then
        echo "❌ mosquitto_pub命令未找到，请安装mosquitto-clients"
        return 1
    fi
    
    echo "  MQTT代理: $MQTT_HOST:$MQTT_PORT"
    echo "  主题: boat_pro/system_config"
    echo "  配置文件: $CONFIG_FILE"
    
    # 发布配置（保留消息）
    if cat "$CONFIG_FILE" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/system_config" -s -r; then
        echo "✅ 配置发布成功（已设置为保留消息）"
        
        # 验证发布结果
        echo ""
        echo "验证发布的配置:"
        timeout 2s mosquitto_sub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/system_config" -v
    else
        echo "❌ 配置发布失败"
        return 1
    fi
}

# 清除MQTT中的保留配置
clear_mqtt_config() {
    echo "🗑️ 清除MQTT中的保留配置..."
    
    if ! command -v mosquitto_pub &> /dev/null; then
        echo "❌ mosquitto_pub命令未找到"
        return 1
    fi
    
    if mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/system_config" -n -r; then
        echo "✅ MQTT保留配置已清除"
        
        # 验证清除结果
        echo ""
        echo "验证清除结果:"
        timeout 2s mosquitto_sub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/system_config" -v || echo "✅ 确认无保留消息"
    else
        echo "❌ 清除失败"
        return 1
    fi
}

# 备份配置
backup_config() {
    echo "💾 备份当前配置..."
    
    if [ ! -f "$CONFIG_FILE" ]; then
        echo "❌ 配置文件不存在: $CONFIG_FILE"
        return 1
    fi
    
    BACKUP_DIR="$PROJECT_ROOT/config/backups"
    mkdir -p "$BACKUP_DIR"
    
    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    BACKUP_FILE="$BACKUP_DIR/system_config_$TIMESTAMP.json"
    
    if cp "$CONFIG_FILE" "$BACKUP_FILE"; then
        echo "✅ 配置已备份到: $BACKUP_FILE"
    else
        echo "❌ 备份失败"
        return 1
    fi
}

# 恢复配置
restore_config() {
    echo "🔄 恢复配置备份..."
    
    BACKUP_DIR="$PROJECT_ROOT/config/backups"
    
    if [ ! -d "$BACKUP_DIR" ]; then
        echo "❌ 备份目录不存在: $BACKUP_DIR"
        return 1
    fi
    
    echo "可用的备份文件:"
    ls -la "$BACKUP_DIR"/*.json 2>/dev/null || {
        echo "❌ 没有找到备份文件"
        return 1
    }
    
    echo ""
    echo "请输入要恢复的备份文件名（不含路径）:"
    read -r backup_name
    
    BACKUP_FILE="$BACKUP_DIR/$backup_name"
    
    if [ -f "$BACKUP_FILE" ]; then
        if cp "$BACKUP_FILE" "$CONFIG_FILE"; then
            echo "✅ 配置已从备份恢复: $backup_name"
            show_config
        else
            echo "❌ 恢复失败"
            return 1
        fi
    else
        echo "❌ 备份文件不存在: $BACKUP_FILE"
        return 1
    fi
}

# 主程序
case "${1:-help}" in
    "show")
        show_config
        ;;
    "publish")
        publish_config
        ;;
    "clear")
        clear_mqtt_config
        ;;
    "validate")
        validate_config
        ;;
    "backup")
        backup_config
        ;;
    "restore")
        restore_config
        ;;
    "help"|*)
        show_help
        ;;
esac
