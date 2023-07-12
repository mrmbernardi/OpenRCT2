#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace OpenRCT2
{
    struct InvalidationGrid
    {
        // Threshold in percent for when to redraw the entire screen.
        static constexpr uint32_t FullRedrawThreshold = 80;

        uint32_t _blockWidth{};
        uint32_t _blockHeight{};
        uint32_t _blockColumns{};
        uint32_t _blockRows{};
        int32_t _screenWidth{};
        int32_t _screenHeight{};
        uint32_t _blocksInvalidated{};
        std::vector<uint8_t> _blocks;

    public:
        uint32_t GetRows() const
        {
            return _blockRows;
        }

        uint32_t GetColumns() const
        {
            return _blockColumns;
        }

        uint32_t GetBlockWidth() const
        {
            return _blockWidth;
        }

        uint32_t GetBlockHeight() const
        {
            return _blockHeight;
        }

        void Reset(int32_t width, int32_t height, uint32_t blockWidth, uint32_t blockHeight)
        {
            _blocksInvalidated = 0;

            _blockWidth = blockWidth;
            _blockHeight = blockHeight;

            _blockColumns = (width / _blockWidth) + 1;
            _blockRows = (height / _blockHeight) + 1;

            _screenWidth = width;
            _screenHeight = height;

            _blocks.resize(_blockColumns * _blockRows);
            std::fill(_blocks.begin(), _blocks.end(), 0);
        }

        void Invalidate(int32_t left, int32_t top, int32_t right, int32_t bottom)
        {
            left = std::max(left, 0);
            top = std::max(top, 0);
            right = std::min(right, static_cast<int32_t>(_screenWidth));
            bottom = std::min(bottom, static_cast<int32_t>(_screenHeight));

            if (left >= right)
                return;
            if (top >= bottom)
                return;

            left /= _blockWidth;
            right /= _blockWidth;

            top /= _blockHeight;
            bottom /= _blockHeight;

            for (int16_t y = top; y <= bottom; y++)
            {
                uint32_t yOffset = y * _blockColumns;
                for (int16_t x = left; x <= right; x++)
                {
                    _blocks[yOffset + x] = 1;
                    _blocksInvalidated++;
                }
            }
        }

        bool ShouldRedrawAll() const
        {
            // Return true when the amount of invalidated cells exceeds the threshold.
            return _blocksInvalidated > ((_blockColumns * _blockRows) * FullRedrawThreshold) / 100;
        }

        void ClearGrid()
        {
            _blocksInvalidated = 0;
            std::fill(_blocks.begin(), _blocks.end(), 0);
        }

        template<typename F> void TraverseDirtyCells(F&& func)
        {
            for (uint32_t x = 0; x < _blockColumns; x++)
            {
                for (uint32_t y = 0; y < _blockRows; y++)
                {
                    if (_blocks[y * _blockColumns + x])
                    {
                        const auto cols = GetNumDirtyColumns(x, y);
                        const auto rows = GetNumDirtyRows(x, y);

                        // Draw the region.
                        func(x, y, cols, rows);

                        // Unset rows and cols
                        ClearRegion(x, y, cols, rows);
                    }
                }
            }
            _blocksInvalidated = 0;
        }

    private:
        uint32_t GetNumDirtyRows(const uint32_t x, const uint32_t y) noexcept
        {
            uint32_t y2 = y;
            while (y2 < _blockRows && _blocks[y2 * _blockColumns + x])
                y2++;
            return y2 - y;
        }

        uint32_t GetNumDirtyColumns(const uint32_t x, const uint32_t y) noexcept
        {
            uint32_t x2 = x;
            while (x2 < _blockColumns && _blocks[y * _blockColumns + x2])
                x2++;
            return x2 - x;
        }

        void ClearRegion(uint32_t x, uint32_t y, uint32_t cols, uint32_t rows) noexcept
        {
            for (uint32_t x2 = x; x2 < x + cols; x2++)
            {
                for (uint32_t y2 = y; y2 < y + rows; y2++)
                {
                    _blocks[y2 * _blockColumns + x2] = false;
                }
            }
        }
    };

} // namespace OpenRCT2
