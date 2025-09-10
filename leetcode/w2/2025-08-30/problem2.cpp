// LeetCode Day 2025-08-30 解数独
class Solution {
public:
    void solveSudoku(vector<vector<char>>& board) {
        int row[9] = {}, col[9] = {}, box[9] = {};
        vector<pair<int, int>> spaces;

        // 1. 一次扫描：初始化位掩码并收集空格
        for (int i = 0; i < 9; ++i)
            for (int j = 0; j < 9; ++j)
                if (board[i][j] == '.')
                    spaces.emplace_back(i, j);
                else {
                    int bit = 1 << (board[i][j] - '1');
                    row[i] |= bit;
                    col[j] |= bit;
                    box[i / 3 * 3 + j / 3] |= bit;
                }

        // 2. 递归 + 位枚举 + 唯一候选天然合并
        auto dfs = [&](auto&& self, int pos) -> bool {
            if (pos == spaces.size()) return true;
            auto [i, j] = spaces[pos];
            int mask = ~(row[i] | col[j] | box[i / 3 * 3 + j / 3]) & 0x1ff;
            for (; mask; mask &= mask - 1) {
                int bit = mask & -mask;
                int d = __builtin_ctz(bit);

                row[i] ^= bit; col[j] ^= bit; box[i / 3 * 3 + j / 3] ^= bit;
                board[i][j] = '1' + d;
                if (self(self, pos + 1)) return true;
                row[i] ^= bit; col[j] ^= bit; box[i / 3 * 3 + j / 3] ^= bit;
            }
            return false;
        };
        dfs(dfs, 0);
    }
};