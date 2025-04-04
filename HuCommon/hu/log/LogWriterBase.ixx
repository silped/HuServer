export module hu.log.LogWriterBase;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (LogWriterBase)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.log.LogType;
import hu.pattern.INoCopy;


namespace hu {

// 로그 기록기 구현체를 위한 기반 클래스
export class LogWriterBase : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (LogWriterBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit LogWriterBase(
        const LogConfigInfo& config
    ) :
        config_ { config }
    {
    }

    virtual ~LogWriterBase()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (LogWriterBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 로그 메시지 쓰기를 시작한다.
    virtual void Start()
    {
    }

    // 로그 메시지를 쓴다.
    virtual void Write(
        const LogMsgInfo& msg
    )
    {
        write_impl( msg );
    }

    // 로그 메시지 쓰기를 중지한다.
    virtual void Stop()
    {
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Protected (LogWriterBase)
    ////////////////////////////////////////////////////////////////////////////////////////////////

protected:
    // 구현체 write를 구현한다.
    virtual void write_impl(
        const LogMsgInfo& msg
    ) = 0;

protected:
    const LogConfigInfo& config_;
};

} // hu
