#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace OpenRCT2
{
    struct InvalidationGrid
    {
        // Threshold in percent for when to redraw the entire screen.
        static constexpr uint32_t FullRedrawThreshold = 65;

        uint32_t _blockWidth{};
        uint32_t _blockHeight{};
        uint32_t _blockColumns{};
        uint32_t _blockRows{};
        uint32_t _screenWidth{};
        uint32_t _screenHeight{};
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
                    if (_blocks[yOffset + x] == 1)
                        continue;

                    _blocksInvalidated++;
                    _blocks[yOffset + x] = 1;
                }
            }
        }

        bool ShouldRedrawAll() const
        {
            // Return true when the amount of invalidated cells exceeds the threshold.
            return _blocksInvalidated > (_blocks.size() * FullRedrawThreshold) / 100;
        }

        void ClearGrid()
        {
            _blocksInvalidated = 0;
            std::fill(_blocks.begin(), _blocks.end(), 0);
        }

        template<typename F> void TraverseDirtyCells(F&& func)
        {
            const auto columnCount = _blockColumns;
            const auto rowCount = _blockRows;
            const auto blockWidth = _blockWidth;
            const auto blockHeight = _blockHeight;

            for (uint32_t column = 0; column < columnCount; column++)
            {
                uint32_t index = 0;
                for (uint32_t row = 0; row < rowCount; row++)
                {
                    if (_blocks[index + column] != 0)
                    {
                        uint32_t columnStart = column;
                        uint32_t rowStart = row;
                        uint32_t index2 = index;

                        while (true)
                        {
                            rowStart++;
                            index2 += columnCount;
                            if (rowStart >= rowCount || _blocks[index2 + columnStart] == 0)
                                break;
                        }

                        rowStart--;
                        index2 -= columnCount;

                        clearBlocks(index, column, columnStart, index2);

                        const auto left = column * blockWidth;
                        const auto top = row * blockHeight;
                        const auto right = (columnStart + 1) * blockWidth;
                        const auto bottom = (rowStart + 1) * blockHeight;

                        if (right < _screenWidth && bottom < _screenWidth)
                        {
                            func(left, top, std::min<int32_t>(right, _screenWidth), std::min<int32_t>(bottom, _screenHeight));
                        }
                    }
                    index += columnCount;
                }
            }
        }

    private:
        void clearBlocks(uint32_t index, uint32_t column, uint32_t columnStart, uint32_t index2) noexcept
        {
            const auto columnCount = _blockColumns;
            do
            {
                uint32_t tempColumn = column;
                do
                {
                    _blocks[index + tempColumn] = 0;
                    tempColumn++;
                } while (tempColumn <= columnStart);

                index += columnCount;
            } while (index <= index2);
        }
    };

} // namespace OpenRCT2
