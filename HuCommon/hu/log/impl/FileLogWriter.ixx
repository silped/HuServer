export module hu.log.impl.FileLogWriter;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (FileLogWriter)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <cstdio>;
import <filesystem>;

import hu.log.AsyncLogWriterBase;
import hu.Type;
import hu.Util;

import "hu/Macro.hpp";


namespace hu {

// 파일 로그 기록기를 구현한 클래스
export class FileLogWriter final : public AsyncLogWriterBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor (FileLogWriter)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit FileLogWriter(
        const LogConfigInfo& config
    ) :
        AsyncLogWriterBase { config },
        file_config_       { config_.file }
    {
        open_file();
    }

    ~FileLogWriter()
    {
        if ( fs_ )
            fclose( fs_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Private (FileLogWriter)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    void write_impl(
        const LogMsgInfo& msg
    ) override
    {
        static const String fmt { _T( "{}\t{}\t{}\t{}\t{}\t\"{}\"\t\"{}\"\n" ) };

        const String str { util::get_format_str(
            fmt,
            msg.id,
            msg.time,
            util::astr_to_wstr( msg.location.function_name() ),
            msg.location.line(),
            LogTypeInfo::ToStr( msg.type ),
            msg.category,
            msg.desc
        ) };
        fwprintf( fs_, str.c_str() );

        // 최대 줄수를 채우면 새 파일에 기록한다.
        ++line_;
        if ( ( line_ % file_config_.max_line ) == 0 )
        {
            line_ = 0;
            open_file();
        }
    }

    void open_file()
    {
        if ( fs_ )
            fclose( fs_ );

        // 로그 디렉터리를 만든다.
        String dir { util::get_format_str(
            _T( "{}/{}/" ),
            util::get_cur_path_str(),
            file_config_.dir ) };
        if ( std::filesystem::exists( dir ) == false )
            std::filesystem::create_directory( dir );

        // 일자 디렉터리를 만든다.
        dir += util::get_local_time_str( _T( "%Y-%m-%d" ) );
        if ( std::filesystem::exists( dir ) == false )
            std::filesystem::create_directory( dir );

        // 파일을 만든다.
        const String file { util::get_format_str(
            _T( "{}/{}_{}_{}.csv" ),
            dir,
            file_config_.name,
            util::get_local_time_str( _T( "%Y-%m-%d_%H-%M-%S" ) ),
            ++index_ ) };
        if ( _wfopen_s( &fs_, file.c_str(), _T( "w, ccs=UTF-16LE" ) ) != 0 )
        {
            fs_ = nullptr;
            return;
        }

        static const Char* const kTitle { _T( "ID\tTIME\tFUNCTION\tLINE\tTYPE\tCATEGORY\tDESCRIPTION\n" ) };
        fwprintf( fs_, kTitle );
    }

private:
    const LogConfigInfo::FileInfo& file_config_;
    Int32                          index_       { 0 };
    Size                           line_        { 0 };
    FILE*                          fs_          { nullptr };
};

} // hu
