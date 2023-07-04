#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace OpenRCT2
{
    struct InvalidationGrid
    {
        uint32_t _blockWidth{};
        uint32_t _blockHeight{};
        uint32_t _blockColumns{};
        uint32_t _blockRows{};
        int32_t _screenWidth{};
        int32_t _screenHeight{};
        size_t _blocksInvalidated{};
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

        size_t ShouldRedrawAll() const
        {
            // Return true when over 80% of the grid is invalidated.
            return _blocksInvalidated > static_cast<size_t>((_blockColumns * _blockRows) * 0.8);
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
                    const uint32_t yOffset = y * _blockColumns;
                    if (_blocks[yOffset + x] == 0)
                    {
                        continue;
                    }

                    const auto columns = GetNumDirtyColumns(x, yOffset);
                    const auto rows = GetNumDirtyRows(x, y, columns);

                    // Clear the invalidated blocks.
                    for (uint32_t top = y; top < y + rows; top++)
                    {
                        uint32_t topOffset = top * _blockColumns;
                        for (uint32_t left = x; left < x + columns; left++)
                        {
                            _blocks[topOffset + left] = 0;
                            _blocksInvalidated--;
                        }
                    }

                    func(x, y, columns, rows);
                }
            }
        }

    private:
        uint32_t GetNumDirtyRows(const uint32_t x, const uint32_t y, const uint32_t columns) noexcept
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

        uint32_t GetNumDirtyColumns(uint32_t x, const uint32_t yOffset) noexcept
        {
            uint32_t xx;
            for (xx = x; xx < _blockColumns; xx++)
            {
                if (_blocks[yOffset + x] == 0)
                {
                    break;
                }
            }
            return xx - x;
        }
    };

} // namespace OpenRCT2
