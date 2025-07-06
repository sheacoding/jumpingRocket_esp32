#include "v3/board_config_v3.h"

// V3Config 静态成员实现

// 注意：难度配置数组已在头文件中定义为 DIFFICULTY_CONFIGS

// 注意：getDifficultyConfig 和 getDifficultyName 已在头文件中内联定义，这里不再重复定义

// 获取难度名称（英文）
const char* V3Config::getDifficultyNameEn(game_difficulty_t difficulty) {
    if (difficulty >= 0 && difficulty < DIFFICULTY_COUNT) {
        return DIFFICULTY_CONFIGS[difficulty].name_en;
    }
    return DIFFICULTY_CONFIGS[DIFFICULTY_NORMAL].name_en;
}

// 注意：getDifficultyMultiplier 已在头文件中内联定义，这里不再重复定义

// 获取目标跳跃次数
uint32_t V3Config::getTargetJumps(game_difficulty_t difficulty) {
    if (difficulty >= 0 && difficulty < DIFFICULTY_COUNT) {
        return DIFFICULTY_CONFIGS[difficulty].target_jumps;
    }
    return DIFFICULTY_CONFIGS[DIFFICULTY_NORMAL].target_jumps;
}

// 获取目标时间
uint32_t V3Config::getTargetTime(game_difficulty_t difficulty) {
    if (difficulty >= 0 && difficulty < DIFFICULTY_COUNT) {
        return DIFFICULTY_CONFIGS[difficulty].target_time;
    }
    return DIFFICULTY_CONFIGS[DIFFICULTY_NORMAL].target_time;
}

// 获取燃料阈值
uint32_t V3Config::getFuelThreshold(game_difficulty_t difficulty) {
    if (difficulty >= 0 && difficulty < DIFFICULTY_COUNT) {
        return DIFFICULTY_CONFIGS[difficulty].fuel_threshold;
    }
    return DIFFICULTY_CONFIGS[DIFFICULTY_NORMAL].fuel_threshold;
}

// 获取得分倍数
float V3Config::getScoreMultiplier(game_difficulty_t difficulty) {
    if (difficulty >= 0 && difficulty < DIFFICULTY_COUNT) {
        return DIFFICULTY_CONFIGS[difficulty].score_multiplier;
    }
    return DIFFICULTY_CONFIGS[DIFFICULTY_NORMAL].score_multiplier;
}

// 验证难度有效性
bool V3Config::isValidDifficulty(game_difficulty_t difficulty) {
    return (difficulty >= 0 && difficulty < DIFFICULTY_COUNT);
}

// 获取下一个难度
game_difficulty_t V3Config::getNextDifficulty(game_difficulty_t current) {
    int next = (int)current + 1;
    if (next >= DIFFICULTY_COUNT) {
        next = 0;
    }
    return (game_difficulty_t)next;
}

// 获取上一个难度
game_difficulty_t V3Config::getPreviousDifficulty(game_difficulty_t current) {
    int prev = (int)current - 1;
    if (prev < 0) {
        prev = DIFFICULTY_COUNT - 1;
    }
    return (game_difficulty_t)prev;
}

// 计算目标完成进度
float V3Config::calculateTargetProgress(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds) {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        difficulty = DIFFICULTY_NORMAL;
    }
    const difficulty_config_t& config = DIFFICULTY_CONFIGS[difficulty];

    // 计算跳跃进度和时间进度
    float jump_progress = (float)jumps / config.target_jumps;
    float time_progress = (float)time_seconds / config.target_time;

    // 取较小值作为整体进度
    float progress = (jump_progress < time_progress) ? jump_progress : time_progress;

    // 限制在0-1范围内
    if (progress > 1.0f) progress = 1.0f;
    if (progress < 0.0f) progress = 0.0f;

    return progress;
}

// 检查目标是否达成
bool V3Config::isTargetAchieved(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds) {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        difficulty = DIFFICULTY_NORMAL;
    }
    const difficulty_config_t& config = DIFFICULTY_CONFIGS[difficulty];

    // 需要同时满足跳跃次数和时间要求
    return (jumps >= config.target_jumps && time_seconds >= config.target_time);
}

// 计算得分
uint16_t V3Config::calculateScore(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds, float avg_frequency) {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        difficulty = DIFFICULTY_NORMAL;
    }
    const difficulty_config_t& config = DIFFICULTY_CONFIGS[difficulty];

    // 基础得分：跳跃次数 * 10
    float base_score = jumps * 10.0f;

    // 时间奖励：每分钟 +50分
    float time_bonus = (time_seconds / 60.0f) * 50.0f;

    // 频率奖励：平均频率 * 100
    float frequency_bonus = avg_frequency * 100.0f;

    // 难度倍数
    float total_score = (base_score + time_bonus + frequency_bonus) * config.score_multiplier;

    // 目标达成奖励
    if (isTargetAchieved(difficulty, jumps, time_seconds)) {
        total_score *= 1.5f; // 50%奖励
    }

    // 转换为整数并限制范围
    uint16_t final_score = (uint16_t)total_score;
    if (final_score > 9999) final_score = 9999; // 最大4位数

    return final_score;
}

// 计算卡路里消耗
float V3Config::calculateCalories(game_difficulty_t difficulty, uint32_t jumps, uint32_t time_seconds) {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        difficulty = DIFFICULTY_NORMAL;
    }
    const difficulty_config_t& config = DIFFICULTY_CONFIGS[difficulty];

    // 基础卡路里：每次跳跃0.5卡路里
    float base_calories = jumps * 0.5f;

    // 时间奖励：每分钟2卡路里
    float time_calories = (time_seconds / 60.0f) * 2.0f;

    // 难度倍数
    float total_calories = (base_calories + time_calories) * config.multiplier;

    return total_calories;
}

// 格式化难度信息
String V3Config::formatDifficultyInfo(game_difficulty_t difficulty) {
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT) {
        difficulty = DIFFICULTY_NORMAL;
    }
    const difficulty_config_t& config = DIFFICULTY_CONFIGS[difficulty];

    String info = String(config.name_en);
    info += " (";
    info += String((int)(config.multiplier * 100));
    info += "%) - ";
    info += String(config.target_jumps);
    info += " jumps/";
    info += String(config.target_time);
    info += " sec";

    return info;
}

// 打印所有难度配置
void V3Config::printAllDifficultyConfigs() {
    Serial.println("V3.0 Difficulty Configuration:");

    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        const difficulty_config_t& config = DIFFICULTY_CONFIGS[i];

        Serial.printf("  %d. %s\n", i + 1, config.name_en);
        Serial.printf("     Multiplier: %.1fx\n", config.multiplier);
        Serial.printf("     Target: %d jumps, %d seconds\n", config.target_jumps, config.target_time);
        Serial.printf("     Fuel Threshold: %d%%\n", config.fuel_threshold);
        Serial.printf("     Score Multiplier: %.1fx\n", config.score_multiplier);
        Serial.println();
    }
}

// 获取推荐难度
game_difficulty_t V3Config::getRecommendedDifficulty(uint32_t total_games, uint32_t best_score) {
    // 新手推荐简单模式
    if (total_games < 5) {
        return DIFFICULTY_EASY;
    }
    
    // 根据最佳得分推荐难度
    if (best_score < 500) {
        return DIFFICULTY_EASY;
    } else if (best_score < 1000) {
        return DIFFICULTY_NORMAL;
    } else {
        return DIFFICULTY_HARD;
    }
}

// 验证配置完整性
bool V3Config::validateConfigs() {
    bool valid = true;

    Serial.println("Validating V3.0 configuration...");

    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        const difficulty_config_t& config = DIFFICULTY_CONFIGS[i];

        // 检查名称
        if (!config.name_en) {
            Serial.printf("ERROR: Difficulty %d name is empty\n", i);
            valid = false;
        }

        // 检查倍数范围
        if (config.multiplier < 0.1f || config.multiplier > 5.0f) {
            Serial.printf("ERROR: Difficulty %d multiplier out of range: %.2f\n", i, config.multiplier);
            valid = false;
        }

        // 检查目标值
        if (config.target_jumps == 0 || config.target_time == 0) {
            Serial.printf("ERROR: Difficulty %d target values are zero\n", i);
            valid = false;
        }

        // 检查燃料阈值
        if (config.fuel_threshold > 100) {
            Serial.printf("ERROR: Difficulty %d fuel threshold exceeds 100%%: %d\n", i, config.fuel_threshold);
            valid = false;
        }
    }

    if (valid) {
        Serial.println("V3.0 configuration validation passed");
    } else {
        Serial.println("V3.0 configuration validation failed");
    }

    return valid;
}

// 获取配置版本
const char* V3Config::getConfigVersion() {
    return "V3.0.1";
}

// 获取配置更新时间
const char* V3Config::getConfigUpdateTime() {
    return __DATE__ " " __TIME__;
}
