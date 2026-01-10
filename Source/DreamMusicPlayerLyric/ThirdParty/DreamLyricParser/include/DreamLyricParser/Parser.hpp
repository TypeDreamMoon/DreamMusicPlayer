#pragma once

#include <memory>

#include <DreamLyricParser/Export.hpp>
#include <DreamLyricParser/Types.hpp>

namespace dream_lyric_parser::parser
{
    /**
     * Parsers Lyric Interface
     */
    class DREAMLYRICPARSER_API IParserLyric
    {
    public:
        virtual ~IParserLyric() = default;

        [[nodiscard]] virtual bool CanParse(const FLyricString& content) const = 0;
        [[nodiscard]] virtual FParsedLyric Parse(const FLyricString& content, const FParserOptions& options) const = 0;
        [[nodiscard]] FParsedLyric Parse(const FLyricString& content) const;
    };

    class DREAMLYRICPARSER_API FParserFactory
    {
    public:
        [[nodiscard]] static std::unique_ptr<IParserLyric> CreateParser(EParserFileFormat file_format, EParserLrcFormat lrc_format = EParserLrcFormat::None);
    };
}
