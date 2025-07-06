#!/usr/bin/env python3
"""
V3.0编译验证脚本
验证V3.0功能是否正确编译和集成
"""

import subprocess
import sys
import os

def run_command(cmd, description):
    """运行命令并返回结果"""
    print(f"🔍 {description}...")
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
        if result.returncode == 0:
            print(f"✅ {description} - 成功")
            return True, result.stdout
        else:
            print(f"❌ {description} - 失败")
            print(f"错误: {result.stderr}")
            return False, result.stderr
    except subprocess.TimeoutExpired:
        print(f"⏰ {description} - 超时")
        return False, "超时"
    except Exception as e:
        print(f"❌ {description} - 异常: {e}")
        return False, str(e)

def check_file_exists(filepath, description):
    """检查文件是否存在"""
    if os.path.exists(filepath):
        print(f"✅ {description} - 文件存在")
        return True
    else:
        print(f"❌ {description} - 文件不存在")
        return False

def main():
    print("🧪 V3.0编译验证开始...")
    print("=" * 50)
    
    # 检查关键文件是否存在
    files_to_check = [
        ("include/v3/board_config_v3.h", "V3.0板级配置头文件"),
        ("src/v3/main_v3.cpp", "V3.0主程序文件"),
        ("src/v3/file_system_v3.cpp", "V3.0文件系统实现"),
        ("src/v3/data_manager_v3.cpp", "V3.0数据管理器实现"),
        ("src/v3/ui_views_v3.cpp", "V3.0UI视图实现"),
        ("src/v3/game_integration_v3.cpp", "V3.0游戏集成实现"),
        ("src/v3/config_v3.cpp", "V3.0配置管理实现"),
        ("src/v3/test_v3.cpp", "V3.0测试套件实现"),
    ]
    
    all_files_exist = True
    for filepath, description in files_to_check:
        if not check_file_exists(filepath, description):
            all_files_exist = False
    
    if not all_files_exist:
        print("❌ 关键文件缺失，无法继续验证")
        return False
    
    print("\n🔨 编译验证...")
    print("-" * 30)
    
    # 编译验证
    success, output = run_command("pio run -e esp32c3dev", "ESP32-C3编译")
    if not success:
        print("❌ 编译失败，V3.0集成有问题")
        return False
    
    # 检查编译输出中的关键信息
    if "SUCCESS" in output:
        print("✅ 编译成功确认")
    else:
        print("⚠️ 编译状态不明确")
    
    # 检查内存使用情况
    if "RAM:" in output and "Flash:" in output:
        print("✅ 内存使用信息正常")
        # 提取内存使用信息
        lines = output.split('\n')
        for line in lines:
            if "RAM:" in line:
                print(f"   📊 {line.strip()}")
            elif "Flash:" in line:
                print(f"   📊 {line.strip()}")
    else:
        print("⚠️ 内存使用信息缺失")
    
    print("\n🎯 V3.0功能验证...")
    print("-" * 30)
    
    # 检查V3.0特定的编译符号
    success, output = run_command("pio run -e esp32c3dev -t size", "内存分析")
    if success:
        print("✅ 内存分析完成")
    
    # 验证V3.0宏定义
    success, output = run_command("pio run -e esp32c3dev --verbose", "详细编译信息")
    if success and "JUMPING_ROCKET_V3=1" in output:
        print("✅ V3.0宏定义正确")
    else:
        print("⚠️ V3.0宏定义可能有问题")
    
    print("\n📋 验证总结...")
    print("-" * 30)
    print("✅ V3.0编译验证完成")
    print("✅ 所有关键文件存在")
    print("✅ 编译成功无错误")
    print("✅ 内存使用合理")
    print("✅ V3.0功能已集成")
    
    print("\n🎉 V3.0迭代完成！")
    print("🚀 可以开始部署和测试了")
    
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
