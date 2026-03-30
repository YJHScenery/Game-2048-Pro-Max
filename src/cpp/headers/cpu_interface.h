//
// Created by jehor on 2026/3/30.
//

#ifndef GAME_2048_QUICK_CPU_INTERFACE_H
#define GAME_2048_QUICK_CPU_INTERFACE_H
struct StandardLineDesc
{
    std::uint64_t start; // element offset (not bytes)
    std::int64_t step;   // element stride (signed, not bytes)
};

template <typename T>
void move_lines_cpu(T* h_data, const StandardLineDesc *h_lines, std::size_t line_count, std::size_t line_len);


template <typename T>
void move_lines_cpu(T* h_data, const StandardLineDesc *h_lines, std::size_t line_count, std::size_t line_len)
{
    if (line_count == 0 || line_len == 0)
        return;

    for (std::size_t line_id = 0; line_id < line_count; ++line_id)
    {
        const StandardLineDesc desc = h_lines[line_id];
        const std::uint64_t base = desc.start;
        const std::int64_t step = desc.step;

        bool has_prev = false;
        T prev{};
        std::size_t write = 0;

        for (std::size_t read = 0; read < line_len; ++read)
        {
            const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
            const T v = h_data[pos];
            if (v == T{}) // 检查是否为零值
                continue;

            if (has_prev && v == prev)
            {
                const T merged = prev + v;
                const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
                h_data[wpos] = merged;
                ++write;
                has_prev = false;
            }
            else
            {
                if (has_prev)
                {
                    const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
                    h_data[wpos] = prev;
                    ++write;
                }
                prev = v;
                has_prev = true;
            }
        }

        if (has_prev)
        {
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
            h_data[wpos] = prev;
            ++write;
        }

        for (std::size_t i = write; i < line_len; ++i)
        {
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(i) * step;
            h_data[wpos] = T{}; // 填充零值
        }
    }
}

#endif //GAME_2048_QUICK_CPU_INTERFACE_H