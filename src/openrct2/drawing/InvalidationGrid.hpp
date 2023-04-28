#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace OpenRCT2
{
    struct InvalidationGrid
    {
        uint32_t _blockShiftX{};
        uint32_t _blockShiftY{};
        uint32_t _blockWidth{};
        uint32_t _blockHeight{};
        uint32_t _blockColumns{};
        uint32_t _blockRows{};
        int32_t _screenWidth{};
        int32_t _screenHeight{};
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

        void Reset(int32_t width, int32_t height)
        {
            _blockShiftX = 7;
            _blockShiftY = 5; // Keep column at 32 (1 << 5)
            _blockWidth = 1 << _blockShiftX;
            _blockHeight = 1 << _blockShiftY;
            _blockColumns = (width >> _blockShiftX) + 1;
            _blockRows = (height >> _blockShiftY) + 1;
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

            right--;
            bottom--;

            left >>= _blockShiftX;
            right >>= _blockShiftX;
            top >>= _blockShiftY;
            bottom >>= _blockShiftY;

            for (int16_t y = top; y <= bottom; y++)
            {
                uint32_t yOffset = y * _blockColumns;
                for (int16_t x = left; x <= right; x++)
                {
                    _blocks[yOffset + x] = 1;
                }
            }
        }

        template<typename F> void TraverseDirtyCells(F&& func)
        {
            // TODO: For optimal performance it is currently limited to a single column.
            // The optimal approach would be to extract all dirty regions as rectangles not including
            // parts that are not marked dirty and have the grid more fine grained.
            // A situation like following:
            //
            //   0 1 2 3 4 5 6 7 8 9
            //   1 - - - - - - - - -
            //   2 - x x x x - - - -
            //   3 - x x - - - - - -
            //   4 - - - - - - - - -
            //   5 - - - - - - - - -
            //   6 - - - - - - - - -
            //   7 - - - - - - - - -
            //   8 - - - - - - - - -
            //   9 - - - - - - - - -
            //
            // Would currently redraw {2,2} to {3,5} where {3,4} and {3,5} are not dirty. Choosing to do this
            // per column eliminates this issue but limits it to rendering just a single column at a time.

            for (uint32_t x = 0; x < _blockColumns; x++)
            {
                for (uint32_t y = 0; y < _blockRows; y++)
                {
                    uint32_t yOffset = y * _blockColumns;
                    if (_blocks[yOffset + x] == 0)
                    {
                        continue;
                    }

                    // See comment above as to why this is 1.
                    const uint32_t columns = 1;

                    // Check rows
                    const auto rows = GetNumDirtyRows(x, y, columns);

                    func(x, y, columns, rows);

                    // Unset dirty rows.
                    for (uint32_t y2 = y; y2 < y + rows; y2++)
                    {
                        uint32_t topOffset = y2 * _blockColumns;
                        _blocks[topOffset + x] = 0;
                    }

                    y += rows;
                }
            }
        }

    private:
        uint32_t GetNumDirtyRows(const uint32_t x, const uint32_t y, const uint32_t columns)
        {
            uint32_t yy = y;
            for (yy = y; yy < _blockRows; yy++)
            {
                uint32_t yyOffset = yy * _blockColumns;
                for (uint32_t xx = x; xx < x + columns; xx++)
                {
                    if (_blocks[yyOffset + xx] == 0)
                    {
                        return yy - y;
                    }
                }
            }
            return yy - y;
        }
    };

} // namespace OpenRCT2
