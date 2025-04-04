export module hu.net.ConnectionManager;


////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Import (ConnectionManager)
////////////////////////////////////////////////////////////////////////////////////////////////////

import <queue>;
import <unordered_map>;
import <vector>;

import hu.log.Log;
import hu.net.ConnectionBase;
import hu.net.Message;
import hu.net.NetType;
import hu.net.ServerConfig;
import hu.pattern.INoCopy;
import hu.rand.Random;
import hu.Type;

import "hu/net/NetMacro.hpp";


namespace hu {

// 연결 관리자를 구현한 클래스
export class ConnectionManager final : private INoCopy
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Constructor & Destructor (ConnectionManager)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    explicit ConnectionManager(
        const ServerConfigInfo& config
    ) :
        config_ { config }
    {
        create_id();
    }

    ~ConnectionManager() = default;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Public (ConnectionManager)
    ////////////////////////////////////////////////////////////////////////////////////////////////

public:
    // 연결된 개수를 얻는다.
    Size GetCount() const
    {
        return conn_map_.size();
    }

    // 연결된 개수가 최대인지 검사한다.
    bool IsMax() const
    {
        return unused_id_queue_.empty();
    }

    // 새로운 연결을 연다.
    bool Open(
        const ConnectionPtr& connection
    )
    {
        if ( connection == nullptr )
        {
            HU_NET_ERROR( _T( "연결이 null (ServerId = {})" ),
                config_.id );
            return false;
        }

        if ( unused_id_queue_.empty() )
        {
            HU_NET_ERROR( _T( "사용 가능한 아이디가 없음 (ServerId = {})" ),
                config_.id );
            return false;
        }

        const ConnectId id { unused_id_queue_.front() };
        unused_id_queue_.pop();

        connection->SetId( id );
        connection->SetCloseDelegate( std::bind_front( &ConnectionManager::Close, this ) );

        conn_map_[ id ] = connection;

        if ( connection->Open() == false )
        {
            HU_NET_ERROR( _T( "연결 open 실패 (ServerId = {})" ),
                config_.id );
            Close( id, ConnectionCloseType::kFailToOpen );
            return false;
        }

        return true;
    }

    // 연결을 찾아서 닫는다.
    void Close(
        const ConnectId           conn_id,
        const ConnectionCloseType close_type
    )
    {
        const auto it { conn_map_.find( conn_id ) };
        if ( it == conn_map_.end() )
        {
            HU_NET_ERROR( _T( "연결 찾기 실패 (ServerId = {}, ConnId = {}, CloseType = {})" ),
                config_.id, conn_id, ConnectionCloseTypeInfo::ToStr( close_type ) );
            return;
        }

        const ConnectionPtr conn { it->second };

        conn_map_.erase( it );
        unused_id_queue_.push( conn_id );

        conn->Close( close_type );
    }

    // 연결을 찾는다.
    ConnectionPtr Find(
        const ConnectId conn_id
    ) const
    {
        const auto it { conn_map_.find( conn_id ) };
        if ( it == conn_map_.cend() )
        {
            HU_NET_ERROR( _T( "연결 찾기 실패 (ServerId = {}, ConnId = {})" ),
                config_.id, conn_id );
            return nullptr;
        }

        return it->second;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Private (ConnectionManager)
    ////////////////////////////////////////////////////////////////////////////////////////////////

private:
    using IdQueue       = std::queue<ConnectId>;
    using ConnectionMap = std::unordered_map<ConnectId, ConnectionPtr>;

private:
    void create_id()
    {
        const ConnectId base_id   { config_.id * 10000 };
        const ConnectId max_count { static_cast<ConnectId>( config_.max_connection_count + 10 ) };

        std::vector<ConnectId> ids;
        ids.reserve( max_count );

        for ( ConnectId i = 1; i <= max_count; ++i )
        {
            const ConnectId id { base_id + i };
            ids.push_back( id );
        }

        util::random_shuffle( ids );

        for ( const ConnectId id : ids )
            unused_id_queue_.push( id );
    }

private:
    const ServerConfigInfo& config_;
    IdQueue                 unused_id_queue_;
    ConnectionMap           conn_map_;
};

} // hu
