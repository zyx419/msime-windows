#pragma once

#include <optional>
#include <string>
#include <string_view>

// AI 联想与 AI 候选词翻译共用的 OpenAI 兼容 Chat Completions 传输层。
namespace ChatCompletions
{
constexpr long kConnectTimeoutMs = 2500;
// 非思考模式的整体超时。推理类模型（默认开思考的 Qwen3 等）几秒内答不完，5 秒会让请求整批落空。
constexpr long kDefaultTimeoutMs = 8000;

// POST 一次请求并返回 choices[0].message.content。网络错误、非 2xx、响应超过 1 MiB
// 或响应结构不对都返回 nullopt。token 为空时不带 Authorization 头（Ollama 这类不鉴权的本机服务）。
// error 非空时，失败原因（curl 错误，或 HTTP 状态加服务端 error.message）写进去，供设置页的
// 凭据测试直接展示；运行时调用方不需要它。
std::optional<std::string> FetchContent(const std::string &endpoint, const std::string &token,
                                        const std::string &payload, long timeout_ms, std::string *error = nullptr);

// 不支持 response_format 的服务（Anthropic）会把 JSON 包进 ```json 代码块。去掉首尾的围栏，
// 其余原样返回；没有围栏时只去掉首尾空白。
inline std::string StripCodeFence(std::string_view text)
{
    const auto trim = [](std::string_view value) {
        const size_t begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string_view::npos)
            return std::string_view();
        const size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(begin, end - begin + 1);
    };
    std::string_view body = trim(text);
    if (body.substr(0, 3) != "```")
        return std::string(body);
    const size_t newline = body.find('\n');
    if (newline == std::string_view::npos)
        return std::string(body);
    body.remove_prefix(newline + 1);
    const size_t closing = body.rfind("```");
    if (closing != std::string_view::npos)
        body = body.substr(0, closing);
    return std::string(trim(body));
}
} // namespace ChatCompletions
