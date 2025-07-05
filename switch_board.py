#!/usr/bin/env python3
"""
蹦跳小火箭 - 开发板环境切换脚本
支持在不同ESP32开发板之间快速切换配置
"""

import os
import sys
import subprocess
import argparse

# 支持的开发板配置
BOARD_CONFIGS = {
    'esp32c3': {
        'name': 'ESP32-C3 DevKit',
        'env': 'esp32c3dev',
        'description': 'ESP32-C3开发板 (USB串口, 21个GPIO)',
        'pins': {
            'I2C_SCL': 8,
            'I2C_SDA': 9,
            'BUTTON': 3,
            'BUZZER': 4,
            'UART_RX': 20,
            'UART_TX': 21
        }
    },
    'esp32': {
        'name': 'ESP32 DevKit',
        'env': 'esp32dev',
        'description': 'ESP32标准开发板 (UART串口, 39个GPIO)',
        'pins': {
            'I2C_SCL': 22,
            'I2C_SDA': 21,
            'BUTTON': 2,
            'BUZZER': 25,
            'UART_RX': 3,
            'UART_TX': 1
        }
    }
}

def print_banner():
    """打印横幅"""
    print("=" * 60)
    print("🚀 蹦跳小火箭 - 开发板环境切换工具")
    print("=" * 60)

def list_boards():
    """列出所有支持的开发板"""
    print("\n📋 支持的开发板:")
    print("-" * 50)
    for key, config in BOARD_CONFIGS.items():
        print(f"🔧 {key}: {config['name']}")
        print(f"   描述: {config['description']}")
        print(f"   环境: {config['env']}")
        print(f"   引脚配置:")
        for pin_name, pin_num in config['pins'].items():
            print(f"     {pin_name}: GPIO{pin_num}")
        print()

def get_current_environment():
    """获取当前默认环境"""
    try:
        result = subprocess.run(['pio', 'project', 'config'], 
                              capture_output=True, text=True, check=True)
        # 解析输出查找默认环境
        for line in result.stdout.split('\n'):
            if 'default_envs' in line:
                return line.split('=')[1].strip()
    except:
        pass
    return None

def build_project(env_name):
    """编译项目"""
    print(f"\n🔨 编译项目 (环境: {env_name})...")
    try:
        result = subprocess.run(['pio', 'run', '-e', env_name], 
                              check=True)
        print("✅ 编译成功!")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ 编译失败: {e}")
        return False

def upload_project(env_name):
    """上传项目"""
    print(f"\n📤 上传项目 (环境: {env_name})...")
    try:
        result = subprocess.run(['pio', 'run', '-e', env_name, '--target', 'upload'], 
                              check=True)
        print("✅ 上传成功!")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ 上传失败: {e}")
        return False

def monitor_serial(env_name):
    """监控串口"""
    print(f"\n📺 开始串口监控 (环境: {env_name})...")
    print("按 Ctrl+C 退出监控")
    try:
        subprocess.run(['pio', 'device', 'monitor', '-e', env_name])
    except KeyboardInterrupt:
        print("\n串口监控已停止")

def switch_board(board_key, build=False, upload=False, monitor=False):
    """切换开发板"""
    if board_key not in BOARD_CONFIGS:
        print(f"❌ 不支持的开发板: {board_key}")
        print("使用 --list 查看支持的开发板")
        return False
    
    config = BOARD_CONFIGS[board_key]
    env_name = config['env']
    
    print(f"\n🔄 切换到开发板: {config['name']}")
    print(f"环境: {env_name}")
    print("引脚配置:")
    for pin_name, pin_num in config['pins'].items():
        print(f"  {pin_name}: GPIO{pin_num}")
    
    # 编译
    if build:
        if not build_project(env_name):
            return False
    
    # 上传
    if upload:
        if not upload_project(env_name):
            return False
    
    # 监控
    if monitor:
        monitor_serial(env_name)
    
    print(f"\n✅ 已切换到 {config['name']}")
    print(f"💡 提示: 使用以下命令进行操作:")
    print(f"   编译: pio run -e {env_name}")
    print(f"   上传: pio run -e {env_name} --target upload")
    print(f"   监控: pio device monitor -e {env_name}")
    
    return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description='蹦跳小火箭开发板环境切换工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例用法:
  python switch_board.py --list                    # 列出支持的开发板
  python switch_board.py esp32c3                   # 切换到ESP32-C3
  python switch_board.py esp32 --build             # 切换到ESP32并编译
  python switch_board.py esp32c3 --build --upload  # 切换、编译并上传
  python switch_board.py esp32 --monitor           # 切换并开始串口监控
        """
    )
    
    parser.add_argument('board', nargs='?', 
                       help='开发板类型 (esp32c3, esp32)')
    parser.add_argument('--list', action='store_true',
                       help='列出所有支持的开发板')
    parser.add_argument('--build', action='store_true',
                       help='切换后自动编译')
    parser.add_argument('--upload', action='store_true',
                       help='切换后自动上传')
    parser.add_argument('--monitor', action='store_true',
                       help='切换后开始串口监控')
    parser.add_argument('--current', action='store_true',
                       help='显示当前环境')
    
    args = parser.parse_args()
    
    print_banner()
    
    # 检查PlatformIO是否安装
    try:
        subprocess.run(['pio', '--version'], 
                      capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ 错误: 未找到PlatformIO")
        print("请先安装PlatformIO: https://platformio.org/install")
        return 1
    
    # 显示当前环境
    if args.current:
        current_env = get_current_environment()
        if current_env:
            print(f"📍 当前默认环境: {current_env}")
        else:
            print("📍 未检测到默认环境")
        return 0
    
    # 列出开发板
    if args.list:
        list_boards()
        return 0
    
    # 切换开发板
    if args.board:
        success = switch_board(args.board, args.build, args.upload, args.monitor)
        return 0 if success else 1
    
    # 没有指定参数，显示帮助
    parser.print_help()
    return 0

if __name__ == '__main__':
    sys.exit(main())
