export module hu.log.Log;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (Log)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <atomic>;
import <memory>;
import <vector>;

import hu.log.impl.ConsoleLogWriter;
import hu.log.impl.FileLogWriter;
import hu.log.LogType;
import hu.log.LogWriterBase;
import hu.pattern.INoCopy;
import hu.pattern.ISingleton;
import hu.Type;
import hu.Util;

import "hu/Macro.hpp";


namespace hu {

// 로그를 구현한 클래스
export class Log final : public ISingleton<Log>
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (Log)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    friend class ISingleton;

    Log() = default;
    ~Log() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (Log)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 로그 기록을 시작한다.
    void Start()
    {
        util::set_locale();

        if ( config_.console.use )
            writers_.emplace_back( std::make_unique<ConsoleLogWriter>( config_ ) );

        if ( config_.file.use )
            writers_.emplace_back( std::make_unique<FileLogWriter>( config_ ) );

        for ( const WriterPtr& writer : writers_ )
            writer->Start();
    }

    // 로그 기록을 시작한다.
    void Start(
        const LogConfigInfo& config
    )
    {
        config_ = config;
        Start();
    }

    // 로그 기록을 중단한다.
    void Stop() const
    {
        for ( const WriterPtr& writer : writers_ )
            writer->Stop();
    }

    // 로그를 기록한다.
    void Write(
        const LogType     type,
        const StringView  category,
        const StringView  desc,
        const SrcLocation location = SrcLocation::current()
    )
    {
        const LogMsgInfo msg
        {
            ++id_,
            type,
            util::get_local_time_str(),
            category,
            desc,
            location
        };

        for ( const WriterPtr& writer : writers_ )
            writer->Write( msg );
    }

    // 로그를 기록한다.
    void Write(
        const SrcLocation location,
        const LogType     type,
        const StringView  category,
        const StringView  desc,
        const auto& ...   args
    )
    {
        Write( type, category, util::get_format_str( desc, args ... ), location );
    }

    // 분류 없이 로그를 기록한다.
    void WriteN(
        const LogType     type,
        const StringView  desc,
        const SrcLocation location = SrcLocation::current()
    )
    {
        static const String kCategory { _T( "None" ) };
        Write( type, kCategory, desc, location );
    }

    // 분류 없이 로그를 기록한다.
    void WriteN(
        const SrcLocation location,
        const LogType     type,
        const StringView  desc,
        const auto& ...   args
    )
    {
        WriteN( type, util::get_format_str( desc, args ... ), location );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (Log)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using WriterPtr = std::unique_ptr<LogWriterBase>;

private:
    LogConfigInfo          config_;
    std::vector<WriterPtr> writers_;
    std::atomic<UInt64>    id_ { 0 };
};


// 로그 인스턴스를 초기화하고 자동으로 정리하는 로그 실행기
export class LogRunner final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (LogRunner)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    LogRunner()
    {
        Log::Inst().Start();
    }

    explicit LogRunner(
        const LogConfigInfo& config
    )
    {
        Log::Inst().Start( config );
    }

    ~LogRunner()
    {
        Log::Inst().Stop();
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (Log)
////////////////////////////////////////////////////////////////////////////////////////////////////

export bool check_impl(
    const bool        cond,
    const StringView  fail_msg,
    const SrcLocation location = SrcLocation::current()
)
{
    if ( cond == false )
    {
        static const String kCategory { _T( "Check" ) };
        Log::Inst().Write( LogType::kError, kCategory, fail_msg, location );
        return false;
    }

    return true;
}

} // hu
