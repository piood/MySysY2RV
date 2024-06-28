#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

// 函数用于移除字符串末尾的多余零
static std::string removeTrailingZeros(const std::string &str) {
    size_t end = str.find_last_not_of('0');
    if (str[end] == '.') {
        end--;
    }
    return str.substr(0, end + 1);
}

// 主函数实现需求
static std::string float2str(float number) {
    // 使用 stringstream 来格式化浮点数，保留一位小数
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << number;
    std::string str = ss.str();
    
    // 移除末尾多余零
    str = removeTrailingZeros(str);
    
    // 用下划线替换小数点
    std::replace(str.begin(), str.end(), '.', '_');
    
    return str;
}
