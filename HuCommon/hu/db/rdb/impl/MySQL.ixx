export module hu.db.rdb.impl.MySQL;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

import hu.db.DBType;

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


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Function (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

String to_str(
    const SQLError& e
)
{
    return util::format_str( _T( "SQLError = {}" ),
        util::utf8_to_str( e.what() ) );
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

    // 유효한지 검사한다.
    bool IsValid() const
    {
        return ( ( session != nullptr ) && ( db != nullptr ) );
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Class (MySQL)
////////////////////////////////////////////////////////////////////////////////////////////////////

class MySQLTrans final : public DBTransBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MySQLTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    MySQLTrans(
        const DBConfigInfo& config,
        const DBRollback&   rollback,
        const SrcLocation&  loc,
        MySQLStateInfo&     state
    ) :
        DBTransBase ( config, rollback, loc ),
        state_      ( state ),
        session_    ( state.session.get() ),
        db_         ( state.db.get() )
    {
    }

    ~MySQLTrans()
    {
        if ( session_ && start_ )
        {
            try
            {
                if ( rollback_ && rollback_() )
                    session_->rollback();
                else
                    session_->commit();
            }
            catch ( const SQLError& e )
            {
                util::log_error( loc_, kDB, _T( "트랜잭션 완료 실패 ({})" ),
                    to_str( e ) );
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (MySQLTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    virtual EDBResult Write(
        const DBTableName& table_name,
        const DBId&        id,
        const Buffer&      buffer,
        const SrcLocation& loc
    ) override
    {
        if ( check_table( table_name, loc ) && check_trans( loc ) )
        {
            static const AString kQuery { "INSERT INTO {} (id, value) VALUES (?, ?) AS NEW_VALUES ON DUPLICATE KEY UPDATE value = NEW_VALUES.value" };

            try
            {
                session_->sql( util::format_str( kQuery, table_name ) ).bind( id, to_param( buffer ) ).execute();
                return EDBResult::kSuccess;
            }
            catch ( const SQLError& e )
            {
                util::log_error( loc, kDB, _T( "DB 오류 발생 (Table = {}, Id = {}, {})" ),
                    to_str( table_name ), to_str( id ), to_str( e ) );
            }

            session_->rollback();
            session_ = nullptr;
        }

        util::log_error( loc, kDB, _T( "DB 쓰기 실패 (Table = {}, Id = {})" ),
            to_str( table_name ), to_str( id ) );
        return EDBResult::kFailToNative;
    }

    virtual EDBResult Read(
        const DBTableName& table_name,
        const DBId&        id,
        Buffer&            buffer,
        const SrcLocation& loc
    ) override
    {
        if ( check_table( table_name, loc ) )
        {
            try
            {
                auto table  = db_->getTable( table_name );
                auto result = table.select( "value" ).
                    where( "id = :param" ).
                    bind( "param", id ).
                    execute();

                auto row = result.fetchOne();
                if ( row.isNull() )
                    return EDBResult::kNotFound;

                std::stringstream ss;
                ss << row[ 0 ];

                auto param = ss.str();
                util::remove_space( param );

                to_buffer( param, buffer );
                return EDBResult::kSuccess;
            }
            catch ( const SQLError& e )
            {
                util::log_error( loc, kDB, _T( "DB 오류 발생 (Table = {}, Id = {}, {})" ),
                    to_str( table_name ), to_str( id ), to_str( e ) );
            }

            if ( start_ )
                session_->rollback();

            session_ = nullptr;
        }

        util::log_error( loc, kDB, _T( "DB 읽기 실패 (Table = {}, Id = {})" ),
            to_str( table_name ), to_str( id ) );
        return EDBResult::kFailToNative;
    }

    virtual EDBResult Delete(
        const DBTableName& table_name,
        const DBId&        id,
        const SrcLocation& loc
    ) override
    {
        if ( check_table( table_name, loc ) && check_trans( loc ) )
        {
            try
            {
                auto table = db_->getTable( table_name );

                const auto del_count = table.remove().
                    where( "id = :param" ).
                    bind( "param", id ).
                    execute().
                    getAffectedItemsCount();
                if ( del_count == 0 )
                    return EDBResult::kNotFound;

                return EDBResult::kSuccess;
            }
            catch ( const SQLError& e )
            {
                util::log_error( loc, kDB, _T( "DB 오류 발생 (Table = {}, Id = {}, {})" ),
                    to_str( table_name ), to_str( id ), to_str( e ) );
            }

            session_->rollback();
            session_ = nullptr;
        }

        util::log_error( loc, kDB, _T( "DB 삭제 실패 (Table = {}, Id = {})" ),
            to_str( table_name ), to_str( id ) );
        return EDBResult::kFailToNative;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: private (MySQLTrans)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    bool check_table(
        const DBTableName& table_name,
        const SrcLocation& loc
    )
    {
        if ( session_ )
        {
            if ( config_.tables.contains( table_name ) )
                return true;

            if ( start_ )
                session_->rollback();

            session_ = nullptr;
        }

        util::log_error( loc, kDB, _T( "테이블 검사 실패 (Table = {})" ),
            to_str( table_name ) );
        return false;
    }

    bool check_trans(
        const SrcLocation& loc
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
            util::log_error( loc, kDB, _T( "DB 오류 발생 ({})" ),
                to_str( e ) );
        }

        session_ = nullptr;
        return false;
    }

private:
    MySQLStateInfo& state_;
    SQLSession*     session_ { nullptr };
    SQLDB*          db_      { nullptr };
    bool            start_   { false };
};

export class MySQL final : public DBBase
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (MySQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit MySQL(
        const DBConfigInfo& config
    ) :
        DBBase( config )
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

            // 테이블이 존재하지 않으면 생성한다.
            for ( const auto& table_name : config_.tables )
            {
                auto table = state_.db->getTable( table_name );
                if ( table.existsInDatabase() )
                    continue;

                static const AString kQuery { "CREATE TABLE {} (id CHAR({}) NOT NULL, value JSON NOT NULL, PRIMARY KEY( id )) DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci" };

                state_.session->sql( util::format_str( kQuery, table_name, kUUIdSize ) ).execute();
            }

            return true;
        }
        catch ( const SQLError& e )
        {
            HU_LOG_ERROR( kDB, _T( "디비 연결 실패 ({}, {})" ),
                config_.ToStr(), to_str( e ) );
        }

        return false;
    }

    virtual DBTransImpl CreateTrans(
        const DBRollback&  rollback,
        const SrcLocation& loc
    ) override
    {
        return std::make_unique<MySQLTrans>( config_, rollback, loc, state_ );
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (MySQL)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    MySQLStateInfo state_;
};

} // hu
