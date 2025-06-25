export module hu.db.rdb.impl.MySQL;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.rdb.RDBType;

import <memory>;
import <mysqlx/xdevapi.h>;

import "hu/Core.hpp";


namespace hu {

////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Using (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

using SQLError   = mysqlx::Error;
using SQLSession = mysqlx::Session;
using SQLDB      = mysqlx::Schema;
using ParamType  = AString;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

String to_str(
    const SQLError& e
)
{
    return util::format_str( _T( "SQLError = {}" ), util::utf8_to_str( e.what() ) );
}

ParamType to_param(
    const String& str
)
{
    return util::to_utf8( str );
}

ParamType to_param(
    const Buffer& buffer
)
{
    return ParamType( buffer.cbegin(), buffer.cend() );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Struct (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

// 디비 상태
struct MySQLStateInfo final : private INoCopy
{
    // 디비 세션
    std::unique_ptr<SQLSession> session;

    // 디비 스키마
    std::unique_ptr<SQLDB> db;

    // 트랜잭션 생성 여부
    AtomBool create_trans { false };

    // 유효한지 검사한다.
    bool IsValid() const
    {
        return ( ( session != nullptr ) && ( db != nullptr ) );
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Class (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

class MySQLTrans final : public RDBTransBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MySQLTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    MySQLTrans(
        const RDBConfigInfo&    config,
        const RDBCheckRollback& check_rollback,
        MySQLStateInfo&         state
    ) :
        RDBTransBase ( config, check_rollback ),
        state_       ( state ),
        session_     ( state.session.get() ),
        db_          ( state.db.get() )
    {
        state_.create_trans = true;
    }

    ~MySQLTrans()
    {
        if ( session_ && start_ )
        {
            try
            {
                if ( check_rollback_ && check_rollback_() )
                    session_->rollback();
                else
                    session_->commit();
            }
            catch ( const SQLError& e )
            {
                HU_LOG_ERROR( kRDB, _T( "트랜잭션 완료 실패 ({})" ), to_str( e ) );
            }
        }

        state_.create_trans = false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (MySQLTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Write(
        const RDBTableName& table_name,
        const RDBId&        id,
        const Buffer&       buffer
    ) override
    {
        if ( session_ == nullptr )
            return false;

        if ( start_trans() == false )
            return false;

        static const AString kQuery { "INSERT INTO {} (id, value) VALUES (?, ?) AS NEW_VALUES ON DUPLICATE KEY UPDATE value = NEW_VALUES.value" };

        try
        {
            session_->sql( util::format_str( kQuery, table_name ) ).bind( id, to_param( buffer ) ).execute();
            return true;
        }
        catch ( const SQLError& e )
        {
            session_->rollback();
            session_ = nullptr;

            HU_LOG_ERROR( kRDB, _T( "디비 쓰기 실패 (Table = {}, Id = {}, {})" ),
                to_str( table_name ), to_str( id ), to_str( e ) );
        }

        return false;
    }

    virtual bool Read(
        const RDBTableName& table_name,
        const RDBId&        id,
        Buffer&             buffer
    ) override
    {
        if ( session_ == nullptr )
            return false;

        try
        {
            auto table = db_->getTable( table_name );
            auto result = table.select( "value" ).
                where( "id = :param" ).
                bind( "param", id ).
                execute();

            auto row = result.fetchOne();
            if ( row )
            {
                std::stringstream ss;
                ss << row[ 0 ];

                auto value = ss.str();
                util::remove_space( value );

                buffer.assign( value.begin(), value.end() );
                return true;
            }
        }
        catch ( const SQLError& e )
        {
            session_ = nullptr;

            HU_LOG_ERROR( kRDB, _T( "디비 읽기 실패 (Table = {}, Id = {}, {})" ),
                to_str( table_name ), to_str( id ), to_str( e ) );
        }

        return false;
    }

    virtual bool Delete(
        const RDBTableName& table_name,
        const RDBId&        id
    ) override
    {
        if ( session_ == nullptr )
            return false;

        if ( start_trans() == false )
            return false;

        try
        {
            auto table = db_->getTable( table_name );
            if ( table.remove().
                where( "id = :param" ).
                bind( "param", id ).
                execute().
                getAffectedItemsCount() > 0 )
            {
                return true;
            }
        }
        catch ( const SQLError& e )
        {
            session_->rollback();
            session_ = nullptr;

            HU_LOG_ERROR( kRDB, _T( "디비 삭제 실패 (Table = {}, Id = {}, {})" ),
                to_str( table_name ), to_str( id ), to_str( e ) );
        }

        return false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: private (MySQLTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    bool start_trans(
        const SrcLocation loc = SrcLocation::current()
    )
    {
        if ( start_ )
            return true;

        try
        {
            session_->startTransaction();
            start_ = true;
            return true;
        }
        catch ( const SQLError& e )
        {
            hu::util::log_error( loc, kRDB, _T( "트랜잭션 시작 실패 ({})" ), to_str( e ) );
        }

        return false;
    }

private:
    MySQLStateInfo& state_;
    SQLSession*     session_ { nullptr };
    SQLDB*          db_      { nullptr };
    bool            start_   { false };
};

export class MySQL final : public RDBBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MySQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit MySQL(
        const RDBConfigInfo& config
    ) :
        RDBBase( config )
    {
    }

    ~MySQL() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (MySQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual bool Connect() override
    {
        try
        {
            const mysqlx::SessionSettings set(
                to_param( config_.host ),
                config_.port,
                to_param( config_.user ),
                to_param( config_.password ),
                to_param( config_.db )
            );

            state_.session = std::make_unique<SQLSession>( set );
            state_.db      = std::make_unique<SQLDB>( state_.session->getDefaultSchema() );

            return true;
        }
        catch ( const SQLError& e )
        {
            HU_LOG_ERROR( kRDB, _T( "디비 연결 실패 ({}, {})" ), config_.ToStr(), to_str( e ) );
        }

        return false;
    }

    virtual bool CreateTable(
        const RDBTableName& table_name
    ) override
    {
        if ( state_.IsValid() == false )
            return false;

        mysqlx::Table table = state_.db->getTable( table_name );
        if ( table.existsInDatabase() )
            return true;

        static const AString kQuery { "CREATE TABLE {} (id CHAR(36) NOT NULL, value JSON NOT NULL, PRIMARY KEY( id )) DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci" };

        try
        {
            state_.session->sql( util::format_str( kQuery, table_name ) ).execute();
        }
        catch ( const SQLError& e )
        {
            HU_LOG_ERROR( kRDB, _T( "테이블 생성 실패 ({}, {})" ), config_.ToStr(), to_str( e ) );
        }

        return true;
    }

    virtual RDBTransImpl CreateTrans(
        const RDBCheckRollback& check_rollback
    ) override
    {
        if ( state_.create_trans )
            return nullptr;

        return std::make_unique<MySQLTrans>( config_, check_rollback, state_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (MySQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    MySQLStateInfo state_;
};

} // hu
