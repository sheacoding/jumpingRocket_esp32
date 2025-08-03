# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Common Development Commands

### Multi-Board Support
This project supports multiple ESP32 development boards. Use the board switching utility:

```bash
# List supported boards
python switch_board.py --list

# Switch to ESP32-C3 (recommended)
python switch_board.py esp32c3

# Switch to ESP32 DevKit
python switch_board.py esp32

# Build and upload in one command
python switch_board.py esp32c3 --build --upload

# Monitor serial output
python switch_board.py esp32c3 --monitor
```

### PlatformIO Commands
```bash
# Build for ESP32-C3
pio run -e esp32c3dev

# Build for ESP32 DevKit
pio run -e esp32dev

# Upload to device
pio run -e esp32c3dev --target upload

# Monitor serial output
pio device monitor -e esp32c3dev

# Clean build
pio run --target clean
```

### Testing and Validation
```bash
# Test V3 compilation compatibility
python test_v3_compilation.py

# Windows board switching
switch_board.bat esp32c3 build upload
```

## Project Architecture

### Core System Design
This is a FreeRTOS-based ESP32 firmware for a jump-detection fitness game using an MPU6050 sensor and OLED display. The system uses a state machine approach with multiple concurrent tasks.

### Task Architecture (FreeRTOS)
The system runs 5 concurrent tasks with different priorities:
- `sensor_task` (Priority 5): Highest priority for real-time jump detection
- `display_task` (Priority 4): UI rendering and animations  
- `sound_task` (Priority 3): Audio feedback and sound effects
- `button_task` (Priority 6): User input handling
- `game_task` (Priority 5): Game logic and state management

### Version System
The project has two main versions:
- **V2.0**: Stable base system with core functionality
- **V3.0**: Advanced features with enhanced data management, file system, and extended UI

V3.0 features are conditionally compiled using `#ifdef JUMPING_ROCKET_V3` and located in `src/v3/` and `include/v3/` directories.

### Module Organization

#### Core Modules (`src/`)
- `main.cpp`: System initialization and task creation
- `hardware.cpp`: I2C, sensor, and device initialization
- `sensor.cpp`: MPU6050 interface and jump detection algorithm
- `display.cpp`: OLED UI rendering with SVG-based layout system
- `game.cpp`: Game state machine and logic
- `sound.cpp`: PWM buzzer control and audio effects
- `button.cpp`: Input handling with debouncing
- `data_processor.cpp`: Statistics and performance analysis
- `board_config.cpp`: Multi-board hardware abstraction

#### V3.0 Enhanced Modules (`src/v3/`)
- `main_v3.cpp`: V3.0 system integration layer
- `data_manager_v3.cpp`: Advanced data persistence and analytics
- `file_system_v3.cpp`: SPIFFS-based storage management
- `ui_views_v3.cpp`: Enhanced UI components and animations
- `game_integration_v3.cpp`: V3.0 game feature integration

### Hardware Abstraction
Multi-board support through compile-time configuration:

#### ESP32-C3 DevKit (`BOARD_ESP32_C3`)
- I2C: SDA=GPIO9, SCL=GPIO8 (100kHz)
- Button: GPIO3 (active HIGH, pull-down)
- Buzzer: GPIO4
- UART: RX=GPIO20, TX=GPIO21

#### ESP32 DevKit (`BOARD_ESP32_DEV`)  
- I2C: SDA=GPIO21, SCL=GPIO22 (400kHz)
- Button: GPIO2 (active LOW, pull-up)
- Buzzer: GPIO25
- UART: RX=GPIO3, TX=GPIO1

### Game Logic System

#### Jump Detection Algorithm
4-state finite state machine with multi-criteria validation:
- `JUMP_STATE_IDLE`: Waiting for jump initiation
- `JUMP_STATE_RISING`: Detecting jump peak
- `JUMP_STATE_FALLING`: Detecting descent phase
- `JUMP_STATE_COOLDOWN`: Preventing duplicate detection

#### Game States
7-state game flow managed by state machine:
- `GAME_STATE_IDLE`: Standby with breathing animation
- `GAME_STATE_DIFFICULTY_SELECT`: 3-level difficulty selection
- `GAME_STATE_PLAYING`: Active gameplay with real-time feedback
- `GAME_STATE_PAUSED`: Pause state with statistics display
- `GAME_STATE_LAUNCHING`: Rocket launch animation sequence
- `GAME_STATE_RESULT`: Final scoring and statistics

#### Fuel System
Fuel accumulates at 5% per jump with difficulty-based launch thresholds:
- Easy: 60% fuel threshold
- Normal: 80% fuel threshold  
- Hard: 100% fuel threshold

### UI System Design

#### SVG-Based Layout
The display system uses a pixel-perfect SVG-to-code mapping approach with 5-tier font system (TINY/SMALL/MEDIUM/LARGE/TITLE) for consistent 128x64 OLED rendering.

#### Animation Engine
Time-based animation system supporting:
- Breathing effects (standby screen)
- Ripple feedback (jump detection)
- Launch sequences (rocket animation)
- Progress indicators (fuel/time/statistics)

### Data Management (V3.0)
Enhanced analytics and persistence:
- SPIFFS-based storage for historical data
- Multi-day exercise tracking and analysis
- JSON-based configuration management
- Statistical trend analysis and reporting

### Development Guidelines

#### Board Configuration
Always use the board switching utility rather than manually editing platformio.ini. The system automatically configures pins, frequencies, and timing parameters for optimal operation on each board type.

#### Memory Management
The system is optimized for embedded constraints:
- Static memory allocation throughout
- Careful stack sizing for FreeRTOS tasks
- Efficient display buffer management
- V3.0 adds managed file I/O for persistence

#### Adding New Features
- Core functionality goes in `src/` with V2.0 compatibility
- Advanced features use V3.0 conditional compilation
- UI changes require SVG mapping table updates
- New sensors/hardware need board abstraction layer updates

#### Testing and Debugging
Use the serial monitor for system diagnostics. The system provides detailed logging including task status, memory usage, hardware initialization status, and real-time game state information.