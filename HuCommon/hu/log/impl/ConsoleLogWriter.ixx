export module hu.log.impl.ConsoleLogWriter;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ConsoleLogWriter)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.io.Console;
import hu.log.AsyncLogWriterBase;
import hu.log.LogType;
import hu.Util;

import "hu/Macro.hpp";


namespace hu {

// 콘솔 로그 기록기를 구현한 클래스
export class ConsoleLogWriter final : public AsyncLogWriterBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ConsoleLogWriter)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit ConsoleLogWriter(
        const LogConfigInfo& config
    ) :
        AsyncLogWriterBase { config },
        console_           { Console::Inst() }
    {
    }

    ~ConsoleLogWriter()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (ConsoleLogWriter)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    void write_impl(
        const LogMsgInfo& msg
    ) override
    {
        static const Array<ColorType, LogTypeInfo::kCount> colors
        {
            ColorType::kCyan,
            ColorType::kWhite,
            ColorType::kYellow,
            ColorType::kRed
        };
        static const String fmt { _T( "{}\t{}\t{}({})\t{}\t{}\t{}" ) };

        console_.WriteLn(
            colors[ LogTypeInfo::ToNum( msg.type ) ],
            fmt,
            msg.id,
            msg.time,
            util::astr_to_wstr( msg.location.function_name() ),
            msg.location.line(),
            LogTypeInfo::ToStr( msg.type ),
            msg.category,
            msg.desc
        );
    }

private:
    const Console& console_;
};

} // hu
